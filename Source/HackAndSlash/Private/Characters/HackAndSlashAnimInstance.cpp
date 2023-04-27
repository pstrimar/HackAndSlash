// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/HackAndSlashAnimInstance.h"
#include "Characters/HackAndSlashCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

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
		IsFalling = HackAndSlashMovement->IsFalling();
		CharacterState = HackAndSlashCharacter->GetCharacterState();
		ActionState = HackAndSlashCharacter->GetActionState();
		DeathPose = HackAndSlashCharacter->GetDeathPose();

		AimRotation = HackAndSlashCharacter->GetBaseAimRotation();
		CharacterRotation = HackAndSlashCharacter->GetActorRotation();
		DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, CharacterRotation);

		AO_Pitch = DeltaRotation.Pitch;
		AO_Yaw = DeltaRotation.Yaw;

		bUseAimOffsets = HackAndSlashCharacter->GetActionState() == EActionState::EAS_Unoccupied;
	}
}
