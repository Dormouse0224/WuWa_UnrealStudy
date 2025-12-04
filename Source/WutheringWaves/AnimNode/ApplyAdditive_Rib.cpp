// Fill out your copyright notice in the Description page of Project Settings.


#include "ApplyAdditive_Rib.h"

void FAnimNode_ApplyAdditive_Rib::Evaluate_AnyThread(FPoseContext& Output)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Evaluate_AnyThread)
	ANIM_MT_SCOPE_CYCLE_COUNTER_VERBOSE(ApplyAdditive, !IsInGameThread());

	//@TODO: Could evaluate Base into Output and save a copy
	if (FAnimWeight::IsRelevant(ActualAlpha))
	{
		const bool bExpectsAdditivePose = true;
		FPoseContext AdditiveEvalContext(Output, bExpectsAdditivePose);

		Base.Evaluate(Output);
		Additive.Evaluate(AdditiveEvalContext);

		FCompactPoseBoneIndex RootIndex(0);
		if (AdditiveEvalContext.Pose.IsValidIndex(RootIndex))
		{
			FTransform& RootTransform = AdditiveEvalContext.Pose[RootIndex];
			RootTransform.SetRotation(FQuat::Identity);
		}

		FAnimationPoseData OutAnimationPoseData(Output);
		const FAnimationPoseData AdditiveAnimationPoseData(AdditiveEvalContext);

		FAnimationRuntime::AccumulateAdditivePose(OutAnimationPoseData, AdditiveAnimationPoseData, ActualAlpha, AAT_LocalSpaceBase);
		Output.Pose.NormalizeRotations();
	}
	else
	{
		Base.Evaluate(Output);
	}
}
