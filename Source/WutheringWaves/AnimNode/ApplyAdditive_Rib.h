// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_Base.h"
#include "AnimNodes/AnimNode_ApplyAdditive.h"
#include "ApplyAdditive_Rib.generated.h"

USTRUCT(BlueprintInternalUseOnly)
struct FAnimNode_ApplyAdditive_Rib : public FAnimNode_ApplyAdditive
{
	GENERATED_BODY()

public:
	WUTHERINGWAVES_API virtual void Evaluate_AnyThread(FPoseContext& Output) override;
};

/**
 * 
 */
UCLASS()
class WUTHERINGWAVES_API UAnimGraphNode_ApplyAdditive_Rib : public UAnimGraphNode_Base
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Settings)
	FAnimNode_ApplyAdditive_Rib Node;
};
