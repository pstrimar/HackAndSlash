// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/HackAndSlashAnimInstance.h"
#include "Characters/HackAndSlashCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetAnimationLibrary.h"

void UHackAndSlashAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	HackAndSlashCharacter = Cast<AHackAndSlashCharacter>(TryGetPawnOwner());
	if (HackAndSlashCharacter)
	{
		HackAndSlashMovement = HackAndSlashCharacter->GetCharacterMovement();
	}
}

void UHackAndSlashAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (HackAndSlashMovement)
	{		
		GroundSpeed = UKismetMathLibrary::VSizeXY(HackAndSlashMovement->Velocity);
		IsAccelerating = HackAndSlashMovement->GetCurrentAcceleration().Length() > 0.f;
		IsFalling = HackAndSlashMovement->IsFalling();
		CharacterState = HackAndSlashCharacter->GetCharacterState();
		ActionState = HackAndSlashCharacter->GetActionState();
		DeathPose = HackAndSlashCharacter->GetDeathPose();
		TargetLocked = HackAndSlashCharacter->GetTargetLocked();

		if (TargetLocked)
		{
			MovementRotation = UKismetAnimationLibrary::CalculateDirection(HackAndSlashMovement->Velocity, HackAndSlashCharacter->GetActorRotation());
			UE_LOG(LogTemp, Warning, TEXT("Movement Rotation: %f"), MovementRotation);
		}

		if (GroundSpeed > 0.f)
		{
			DeltaLean = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotationLastTick, HackAndSlashCharacter->GetActorRotation());
			const float Target = DeltaLean.Yaw / DeltaTime / LeanIntensity;
			YawDelta = FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f);
			CharacterRotationLastTick = HackAndSlashCharacter->GetActorRotation();
		}

		AimRotation = HackAndSlashCharacter->GetBaseAimRotation();
		CharacterRotationLastTick = HackAndSlashCharacter->GetActorRotation();
		DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, CharacterRotationLastTick);

		AO_Pitch = DeltaRotation.Pitch;
		AO_Yaw = DeltaRotation.Yaw;

		bUseAimOffsets = HackAndSlashCharacter->GetActionState() == EActionState::EAS_Unoccupied;
	}
}
