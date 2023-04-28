// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterTypes.h"
#include "HackAndSlashAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class HACKANDSLASH_API UHackAndSlashAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly)
	class AHackAndSlashCharacter* HackAndSlashCharacter;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	class UCharacterMovementComponent* HackAndSlashMovement;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	float GroundSpeed;

	FRotator DeltaRotation;
	FRotator DeltaLean;
	FRotator AimRotation;
	FRotator CharacterRotationLastTick;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	float AO_Yaw;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	float YawDelta;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	float AO_Pitch;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool bUseAimOffsets;

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool IsFalling;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	ECharacterState CharacterState;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	EActionState ActionState;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	TEnumAsByte<EDeathPose> DeathPose;

private:
	float LeanIntensity = 10.f;
};
