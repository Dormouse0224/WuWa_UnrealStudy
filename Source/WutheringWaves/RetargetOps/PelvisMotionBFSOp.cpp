// Fill out your copyright notice in the Description page of Project Settings.


#include "PelvisMotionBFSOp.h"

#include "Retargeter/IKRetargetProcessor.h"

#define LOCTEXT_NAMESPACE "PelvisMotionBFSOp"

const UClass* FIKRetargetPelvisMotionBFSOpSettings::GetControllerType() const
{
	return UIKRetargetPelvisMotionBFSController::StaticClass();
}

void FIKRetargetPelvisMotionBFSOpSettings::CopySettingsAtRuntime(const FIKRetargetOpSettingsBase* InSettingsToCopyFrom)
{
	// copies everything except the bones we are operating on (those require reinit)
	const TArray<FName> PropertiesToIgnore = { "SourcePelvisBone", "TargetPelvisBone" };
	FIKRetargetOpBase::CopyStructProperties(
		FIKRetargetPelvisMotionBFSOpSettings::StaticStruct(),
		InSettingsToCopyFrom,
		this,
		PropertiesToIgnore);
}

bool FIKRetargetPelvisMotionBFSOp::Initialize(
	const FIKRetargetProcessor& InProcessor, 
	const FRetargetSkeleton& InSourceSkeleton, 
	const FTargetSkeleton& InTargetSkeleton, 
	const FIKRetargetOpBase* InParentOp, 
	FIKRigLogger& Log)
{
	bIsInitialized = false;

	// reset root data
	Reset();

	// initialize root encoder
	const FName SourcePelvisBoneName = Settings.SourcePelvisBone.BoneName;
	const bool bPelvisEncoderInit = InitializeSource(SourcePelvisBoneName, InSourceSkeleton, Log);
	if (!bPelvisEncoderInit)
	{
		Log.LogWarning(FText::Format(
			LOCTEXT("NoSourceRoot", "IK Retargeter unable to initialize source root, '{0}' on skeletal mesh: '{1}'"),
			FText::FromName(SourcePelvisBoneName), FText::FromString(InSourceSkeleton.SkeletalMesh->GetName())));
	}

	// initialize root decoder
	const FName TargetPelvisBoneName = Settings.TargetPelvisBone.BoneName;
	const bool bPelvisDecoderInit = InitializeTarget(TargetPelvisBoneName, InTargetSkeleton, Log);
	if (!bPelvisDecoderInit)
	{
		Log.LogWarning(FText::Format(
			LOCTEXT("NoTargetRoot", "IK Retargeter unable to initialize target root, '{0}' on skeletal mesh: '{1}'"),
			FText::FromName(TargetPelvisBoneName), FText::FromString(InTargetSkeleton.SkeletalMesh->GetName())));
	}

#if WITH_EDITOR
	// record skeletons for UI bone selector widgets
	Settings.SourceSkeletonAsset = InSourceSkeleton.SkeletalMesh->GetSkeleton();
	Settings.TargetSkeletonAsset = InTargetSkeleton.SkeletalMesh->GetSkeleton();
#endif

	bIsInitialized = bPelvisEncoderInit && bPelvisDecoderInit;
	return bIsInitialized;
}

void FIKRetargetPelvisMotionBFSOp::Run(
	FIKRetargetProcessor& InProcessor, 
	const double InDeltaTime, 
	const TArray<FTransform>& InSourceGlobalPose, 
	TArray<FTransform>& OutTargetGlobalPose)
{
	FTransform NewPelvisGlobalTransform;

	EncodePose(InSourceGlobalPose);
	DecodePose(NewPelvisGlobalTransform);

	// update global transforms below root
	FTargetSkeleton& TargetSkeleton = InProcessor.GetTargetSkeleton();
	SetGlobalTransformAndUpdateChildrenBFS(Target.BoneIndex, NewPelvisGlobalTransform, TargetSkeleton);
}

void FIKRetargetPelvisMotionBFSOp::OnAddedToStack(const UIKRetargeter* InRetargetAsset, const FIKRetargetOpBase* InParentOp)
{
	// copy the source/target pelvis from the default IK Rig
	if (const UIKRigDefinition* SourceIKRig = InRetargetAsset->GetIKRig(ERetargetSourceOrTarget::Source))
	{
		Settings.SourcePelvisBone.BoneName = SourceIKRig->GetPelvis();
	}

	if (const UIKRigDefinition* TargetIKRig = InRetargetAsset->GetIKRig(ERetargetSourceOrTarget::Target))
	{
		Settings.TargetPelvisBone.BoneName = TargetIKRig->GetPelvis();
	}
}

FIKRetargetOpSettingsBase* FIKRetargetPelvisMotionBFSOp::GetSettings()
{
	return &Settings;
}

const UScriptStruct* FIKRetargetPelvisMotionBFSOp::GetSettingsType() const
{
	return FIKRetargetPelvisMotionBFSOpSettings::StaticStruct();
}

const UScriptStruct* FIKRetargetPelvisMotionBFSOp::GetType() const
{
	return FIKRetargetPelvisMotionBFSOp::StaticStruct();
}

void FIKRetargetPelvisMotionBFSOp::CollectRetargetedBones(TSet<int32>& OutRetargetedBones) const
{
	// the pelvis bone is retargeted
	if (Target.BoneIndex != INDEX_NONE)
	{
		OutRetargetedBones.Add(Target.BoneIndex);
	}
}

void FIKRetargetPelvisMotionBFSOp::Reset()
{
	Source = FPelvisSource();
	Target = FPelvisTarget();
}

bool FIKRetargetPelvisMotionBFSOp::InitializeSource(
	const FName SourcePelvisBoneName,
	const FRetargetSkeleton& SourceSkeleton,
	FIKRigLogger& Log)
{
	// validate target root bone exists
	Source.BoneName = SourcePelvisBoneName;
	Source.BoneIndex = SourceSkeleton.FindBoneIndexByName(SourcePelvisBoneName);
	if (Source.BoneIndex == INDEX_NONE)
	{
		Log.LogWarning(FText::Format(
			LOCTEXT("MissingSourceRoot", "IK Retargeter could not find source root bone, {0} in mesh {1}"),
			FText::FromName(SourcePelvisBoneName), FText::FromString(SourceSkeleton.SkeletalMesh->GetName())));
		return false;
	}

	// record initial root data
	const FTransform InitialTransform = SourceSkeleton.RetargetPoses.GetGlobalRetargetPose()[Source.BoneIndex];
	float InitialHeight = static_cast<float>(InitialTransform.GetTranslation().Z);
	Source.InitialRotation = InitialTransform.GetRotation();

	// ensure root height is not at origin, this happens if user sets root to ACTUAL skeleton root and not pelvis
	if (InitialHeight < UE_KINDA_SMALL_NUMBER)
	{
		// warn user and push it up slightly to avoid divide by zero
		Log.LogError(LOCTEXT("BadPelvisHeight", "The source pelvis bone is very near the ground plane. This will cause the target to be moved very far. To resolve this, please create a retarget pose with the pelvis at the correct height off the ground."));
		InitialHeight = 1.0f;
	}

	// invert height
	Source.InitialHeightInverse = 1.0f / InitialHeight;

	return true;
}

bool FIKRetargetPelvisMotionBFSOp::InitializeTarget(
	const FName TargetPelvisBoneName,
	const FTargetSkeleton& TargetSkeleton,
	FIKRigLogger& Log)
{
	// validate target root bone exists
	Target.BoneName = TargetPelvisBoneName;
	Target.BoneIndex = TargetSkeleton.FindBoneIndexByName(TargetPelvisBoneName);
	if (Target.BoneIndex == INDEX_NONE)
	{
		Log.LogWarning(FText::Format(
			LOCTEXT("CountNotFindRootBone", "IK Retargeter could not find target root bone, {0} in mesh {1}"),
			FText::FromName(TargetPelvisBoneName), FText::FromString(TargetSkeleton.SkeletalMesh->GetName())));
		return false;
	}

	const FTransform TargetInitialTransform = TargetSkeleton.RetargetPoses.GetGlobalRetargetPose()[Target.BoneIndex];
	Target.InitialHeight = static_cast<float>(TargetInitialTransform.GetTranslation().Z);
	Target.InitialRotation = TargetInitialTransform.GetRotation();
	Target.InitialPosition = TargetInitialTransform.GetTranslation();

	// initialize the global scale factor
	const float ScaleFactor = Source.InitialHeightInverse * Target.InitialHeight;
	GlobalScaleFactor.Set(ScaleFactor, ScaleFactor, ScaleFactor);

	return true;
}

void FIKRetargetPelvisMotionBFSOp::EncodePose(const TArray<FTransform>& SourceGlobalPose)
{
	const FTransform& SourceTransform = SourceGlobalPose[Source.BoneIndex];
	Source.CurrentPosition = SourceTransform.GetTranslation();
	Source.CurrentPositionNormalized = Source.CurrentPosition * Source.InitialHeightInverse;
	Source.CurrentRotation = SourceTransform.GetRotation();
}

void FIKRetargetPelvisMotionBFSOp::DecodePose(FTransform& OutPelvisGlobalPose)
{
	// retarget position
	FVector Position;
	{
		// generate basic pelvis position by scaling the normalized position by root height
		const FVector RetargetedPosition = Source.CurrentPositionNormalized * Target.InitialHeight;

		// blend the pelvis position towards the source pelvis position
		const FVector PerAxisAlpha = Settings.BlendToSourceTranslation * Settings.BlendToSourceTranslationWeights;
		Position = FMath::Lerp(RetargetedPosition, Source.CurrentPosition, PerAxisAlpha);

		// apply vertical / horizontal scaling of motion
		FVector ScaledRetargetedPosition = Position;
		ScaledRetargetedPosition.Z *= Settings.ScaleVertical;
		const FVector HorizontalOffset = (ScaledRetargetedPosition - Target.InitialPosition) * FVector(Settings.ScaleHorizontal, Settings.ScaleHorizontal, 1.0f);
		Position = Target.InitialPosition + HorizontalOffset;

		// apply a static offset
		Position += Settings.TranslationOffset;

		// blend with alpha
		Position = FMath::Lerp(Target.InitialPosition, Position, Settings.TranslationAlpha);

		// record the delta created by all the modifications made to the root translation
		Target.PelvisTranslationDelta = Position - RetargetedPosition;
	}

	// retarget rotation
	FQuat Rotation;
	{
		// calc offset between initial source/target root rotations
		const FQuat RotationDelta = Source.CurrentRotation * Source.InitialRotation.Inverse();
		// add retarget pose delta to the current source rotation
		const FQuat RetargetedRotation = RotationDelta * Target.InitialRotation;

		// add static rotation offset
		Rotation = RetargetedRotation * Settings.RotationOffset.Quaternion();

		// blend with alpha
		Rotation = FQuat::FastLerp(Target.InitialRotation, Rotation, Settings.RotationAlpha);
		Rotation.Normalize();

		// record the delta created by all the modifications made to the root rotation
		Target.PelvisRotationDelta = RetargetedRotation * Target.InitialRotation.Inverse();
	}

	// apply to target
	OutPelvisGlobalPose.SetTranslation(Position);
	OutPelvisGlobalPose.SetRotation(Rotation);
}

void FIKRetargetPelvisMotionBFSOp::SetGlobalTransformAndUpdateChildrenBFS(
	const int32 InBoneToSetIndex,
	const FTransform& InNewTransform,
	FTargetSkeleton& InOutSkeleton) const
{
	check(InOutSkeleton.BoneNames.Num() == InOutSkeleton.OutputGlobalPose.Num());
	check(InOutSkeleton.OutputGlobalPose.IsValidIndex(InBoneToSetIndex));

	const FTransform PrevTransform = InOutSkeleton.OutputGlobalPose[InBoneToSetIndex];
	InOutSkeleton.OutputGlobalPose[InBoneToSetIndex] = InNewTransform;

	TMap<int32, TArray<int32>> ChildBones;
	for (int BoneIndex = 0; BoneIndex < InOutSkeleton.BoneNames.Num(); ++BoneIndex)
	{
		int ParentBoneIndex = InOutSkeleton.GetParentIndex(BoneIndex);
		if (ParentBoneIndex != INDEX_NONE)
		{
			ChildBones.FindOrAdd(ParentBoneIndex).Add(BoneIndex);
		}
	}

	TArray<int32> ChildBoneIndices;
	TQueue<int32> BFSChild;
	BFSChild.Enqueue(InBoneToSetIndex);
	int32 CurrentIndex = INDEX_NONE;
	while (BFSChild.Dequeue(CurrentIndex))
	{
		if (CurrentIndex != INDEX_NONE)
		{
			if (const TArray<int32>* ChildIndices = ChildBones.Find(CurrentIndex))
			{
				for (const int32 ChildIndex : *ChildIndices)
				{
					ChildBoneIndices.Add(ChildIndex);
					BFSChild.Enqueue(ChildIndex);
				}
			}
		}
	}

	for (const int32 ChildBoneIndex : ChildBoneIndices)
	{
		const FTransform RelativeToPrev = InOutSkeleton.OutputGlobalPose[ChildBoneIndex].GetRelativeTransform(PrevTransform);
		InOutSkeleton.OutputGlobalPose[ChildBoneIndex] = RelativeToPrev * InNewTransform;
	}
}

FIKRetargetPelvisMotionOpSettings UIKRetargetPelvisMotionBFSController::GetSettings()
{
	return GetPelvisOpSettings();
}

void UIKRetargetPelvisMotionBFSController::SetSettings(FIKRetargetPelvisMotionOpSettings InSettings)
{
	OpSettingsToControl->CopySettingsAtRuntime(&InSettings);
}

void UIKRetargetPelvisMotionBFSController::SetSourcePelvisBone(const FName InSourcePelvisBone)
{
	GetPelvisOpSettings().SourcePelvisBone.BoneName = InSourcePelvisBone;
}

FName UIKRetargetPelvisMotionBFSController::GetSourcePelvisBone()
{
	return GetPelvisOpSettings().SourcePelvisBone.BoneName;
}

void UIKRetargetPelvisMotionBFSController::SetTargetPelvisBone(const FName InTargetPelvisBone)
{
	GetPelvisOpSettings().TargetPelvisBone.BoneName = InTargetPelvisBone;
}

FName UIKRetargetPelvisMotionBFSController::GetTargetPelvisBone()
{
	return GetPelvisOpSettings().TargetPelvisBone.BoneName;
}

FIKRetargetPelvisMotionOpSettings& UIKRetargetPelvisMotionBFSController::GetPelvisOpSettings() const
{
	return *reinterpret_cast<FIKRetargetPelvisMotionOpSettings*>(OpSettingsToControl);
}

#undef LOCTEXT_NAMESPACE