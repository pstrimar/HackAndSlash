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
	}
}
