// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class WUTHERINGWAVES_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Spring Arm Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* SpringArmComp;

	// Camera Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* CameraComp;

	// Input Mapping Context
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;

	// Input Action - Jump
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* JumpAction;

	// Input Action - Move
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* MoveAction;

	// Input Action - Look
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* LookAction;

	// Input Action - Switch Charactor
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* SwitchCharactorAction;

	// Switch Character Array
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Meshes")
	TArray<class USkeletalMesh*> CharacterMeshArray;


public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Action Functions
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void SwitchCharactor(const FInputActionValue& Value);
};
