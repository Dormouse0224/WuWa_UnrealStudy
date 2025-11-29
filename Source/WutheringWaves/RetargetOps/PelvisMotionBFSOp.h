// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Retargeter/IKRetargetOps.h"
#include "Retargeter/IKRetargeter.h"
#include "Retargeter/RetargetOps/PelvisMotionOp.h"
#include "PelvisMotionBFSOp.generated.h"

#define LOCTEXT_NAMESPACE "PelvisMotionBFSOp"

USTRUCT(BlueprintType, meta = (DisplayName = "Pelvis Motion BFS Settings"))
struct FIKRetargetPelvisMotionBFSOpSettings : public FIKRetargetOpSettingsBase
{
	GENERATED_BODY()

	/** The Pelvis bone on the source skeleton to copy motion FROM. */
	UPROPERTY(EditAnywhere, Category = Setup, meta = (ReinitializeOnEdit))
	FBoneReference SourcePelvisBone;

	/** The Pelvis bone on the target skeleton to copy motion TO. */
	UPROPERTY(EditAnywhere, Category = Setup, meta = (ReinitializeOnEdit))
	FBoneReference TargetPelvisBone;

	/** Range 0 to 1. Default 1. Blends the amount of retargeted pelvis rotation to apply.
	*  At 0 the pelvis is left at the rotation from the retarget pose.
	*  At 1 the pelvis is rotated fully to match the source pelvis rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pelvis Rotation", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	double RotationAlpha = 1.0f;

	/** Applies a static local-space rotation offset to the retarget pelvis.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pelvis Rotation", meta = (ClampMin = "-180.0", ClampMax = "180.0", UIMin = "-180.0", UIMax = "180.0"))
	FRotator RotationOffset = FRotator::ZeroRotator;

	/** Range 0 to 1. Default 1. Blends the amount of retargeted pelvis translation to apply.
	*  At 0 the pelvis is left at the position from the retarget pose.
	*  At 1 the pelvis will follow the source motion according to the behavior defined in the subsequent settings. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pelvis Translation", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	double TranslationAlpha = 1.0f;

	/** Applies a static component-space translation offset to the pelvis.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pelvis Translation")
	FVector TranslationOffset = FVector::ZeroVector;

	/** Range 0 to 1. Default 0. Blends the retarget pelvis' translation to the exact source location.
	*  At 0 the pelvis is placed at the retargeted location.
	*  At 1 the pelvis is placed at the location of the source's pelvis bone.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pelvis Translation", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	double BlendToSourceTranslation = 0.0f;

	/** Per-axis weights for the Blend to Source. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pelvis Translation", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	FVector BlendToSourceTranslationWeights = FVector::OneVector;

	/** Default 1. Scales the translation of the pelvis in the horizontal plane (X,Y). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale Pelvis Translation", meta = (UIMin = "0.0", UIMax = "3.0"))
	double ScaleHorizontal = 1.0f;

	/** Default 1. Scales the translation of the pelvis in the vertical direction (Z). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale Pelvis Translation", meta = (UIMin = "0.0", UIMax = "3.0"))
	double ScaleVertical = 1.0f;

	/** Range 0 to 1. Default 1. Control whether modifications made to the pelvis will affect the horizontal component of IK positions.
	*  At 0 the IK positions are independent of the pelvis modifications.
	*  At 1 the IK positions are calculated relative to the modified pelvis location.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affect IK Settings", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0", DisplayName = "Affect IK Horizontal"))
	double AffectIKHorizontal = 1.0f;

	/** Range 0 to 1. Default 0. Control whether modifications made to the pelvis will affect the vertical component of IK positions.
	*  At 0 the IK positions are independent of the pelvis modifications.
	*  At 1 the IK positions are calculated relative to the modified pelvis location.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Affect IK Settings", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0", DisplayName = "Affect IK Vertical"))
	double AffectIKVertical = 0.0f;

	/** Toggle debug drawing on/off in the viewport */
	UPROPERTY(EditAnywhere, Category = Debug)
	bool bEnableDebugDraw = true;

	/** Adjust size of the debug drawing */
	UPROPERTY(EditAnywhere, Category = Debug)
	double DebugDrawSize = 5.0f;

	/** Adjust thickness of the debug drawing */
	UPROPERTY(EditAnywhere, Category = Debug)
	double DebugDrawThickness = 5.0f;

	virtual const UClass* GetControllerType() const override;

	/** (required) override to specify how settings should be applied in a way that will not require reinitialization (ie runtime compatible)*/
	virtual void CopySettingsAtRuntime(const FIKRetargetOpSettingsBase* InSettingsToCopyFrom) override;
};
/**
 * 
 */
USTRUCT(BlueprintType, meta = (DisplayName = "Pelvis Motion BFS"))
struct FIKRetargetPelvisMotionBFSOp : public FIKRetargetOpBase
{
	GENERATED_BODY()

	UPROPERTY()
	FIKRetargetPelvisMotionBFSOpSettings Settings;

	/** (optional) override to cache internal data when initializing the processor
 * NOTE: you must set bIsInitialized to true to inform the retargeter that this op is ok to execute. */
	virtual bool Initialize(
		const FIKRetargetProcessor& InProcessor,
		const FRetargetSkeleton& InSourceSkeleton,
		const FTargetSkeleton& InTargetSkeleton,
		const FIKRetargetOpBase* InParentOp,
		FIKRigLogger& Log) override;

	/** (optional) override to evaluate this operation and modify the output pose */
	virtual void Run(
		FIKRetargetProcessor& InProcessor,
		const double InDeltaTime,
		const TArray<FTransform>& InSourceGlobalPose,
		TArray<FTransform>& OutTargetGlobalPose) override;

	/** (optional) override to automate initial setup after being added to the stack */
	virtual void OnAddedToStack(const UIKRetargeter* InRetargetAsset, const FIKRetargetOpBase* InParentOp) override;

	/** (required) override and return a pointer to the settings struct used by this operation */
	virtual FIKRetargetOpSettingsBase* GetSettings() override;

	/** (required) override and return the type used to house the settings for this operation */
	virtual const UScriptStruct* GetSettingsType() const override;

	/** (required) override and return the type of this op (the derived subclass) */
	virtual const UScriptStruct* GetType() const override;

	/** (optional) override and add any bones that your op modifies to the output TSet of bone indices
	 * Add indices of any bone that this op modifies. Any bone not registered here will be FK parented by other operations*/
	virtual void CollectRetargetedBones(TSet<int32>& OutRetargetedBones) const override;

private:
	void Reset();

	bool InitializeSource(
		const FName SourcePelvisBoneName,
		const FRetargetSkeleton& SourceSkeleton,
		FIKRigLogger& Log);

	bool InitializeTarget(
		const FName TargetPelvisBoneName,
		const FTargetSkeleton& TargetSkeleton,
		FIKRigLogger& Log);

	void EncodePose(const TArray<FTransform>& SourceGlobalPose);

	void DecodePose(FTransform& OutPelvisGlobalPose);

	void SetGlobalTransformAndUpdateChildrenBFS(
		const int32 InBoneToSetIndex,
		const FTransform& InNewTransform,
		FTargetSkeleton& InOutSkeleton) const;

	// transient work data
	FPelvisSource Source;
	FPelvisTarget Target;
	FVector GlobalScaleFactor;
};

/* The blueprint/python API for editing a Pelvis Motion Op */
UCLASS(MinimalAPI, BlueprintType)
class UIKRetargetPelvisMotionBFSController : public UIKRetargetOpControllerBase
{
	GENERATED_BODY()

public:
	/* Get the current op settings as a struct.
	 * @return FIKRetargetPelvisMotionOpSettings struct with the current settings used by the op. */
	UFUNCTION(BlueprintCallable, Category = Settings)
	FIKRetargetPelvisMotionOpSettings GetSettings();

	/* Set the op settings. Input is a custom struct type for this op.
	 * @param InSettings a FIKRetargetPelvisMotionOpSettings struct containing all the settings to apply to this op */
	UFUNCTION(BlueprintCallable, Category = Settings)
	void SetSettings(FIKRetargetPelvisMotionOpSettings InSettings);

	/* Set the pelvis bone for the source.
	 * @param InSourcePelvisBone the name of the pelvis bone on the source skeleton. */
	UFUNCTION(BlueprintCallable, Category = Settings)
	void SetSourcePelvisBone(const FName InSourcePelvisBone);

	/* Get the pelvis bone for the source.
	 * @return the name of the pelvis bone on the source skeleton. */
	UFUNCTION(BlueprintCallable, Category = Settings)
	FName GetSourcePelvisBone();

	/* Set the pelvis bone for the target.
	 * @param InTargetPelvisBone the name of the pelvis bone on the target skeleton. */
	UFUNCTION(BlueprintCallable, Category = Settings)
	void SetTargetPelvisBone(const FName InTargetPelvisBone);

	/* Get the pelvis bone for the target.
	 * @return the name of the pelvis bone on the target skeleton. */
	UFUNCTION(BlueprintCallable, Category = Settings)
	FName GetTargetPelvisBone();

private:

	FIKRetargetPelvisMotionOpSettings& GetPelvisOpSettings() const;
};

#undef LOCTEXT_NAMESPACE