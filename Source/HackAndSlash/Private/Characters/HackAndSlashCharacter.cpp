// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/HackAndSlashCharacter.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Items/Item.h"
#include "Items/Weapons/Weapon.h"
#include "Animation/AnimMontage.h"
#include "Components/BoxComponent.h"

AHackAndSlashCharacter::AHackAndSlashCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);
}

void AHackAndSlashCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(HackAndSlashMappingContext, 0);
		}
	}
}

void AHackAndSlashCharacter::Move(const FInputActionValue& Value)
{
	if (ActionState != EActionState::EAS_Unoccupied) return;

	const FVector2D MovementVector = Value.Get<FVector2D>();
	if (GetController() && MovementVector != FVector2D::ZeroVector)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(ForwardDirection, MovementVector.Y);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AHackAndSlashCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisValue = Value.Get<FVector2D>();
	if (GetController() && LookAxisValue != FVector2D::ZeroVector)
	{
		AddControllerYawInput(LookAxisValue.X);
		AddControllerPitchInput(LookAxisValue.Y);
	}
}

void AHackAndSlashCharacter::EKeyPressed()
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if (OverlappingWeapon)
	{
		if (EquippedWeapon)
		{
			EquippedWeapon->Drop();
		}
		OverlappingWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		CharacterState = OverlappingWeapon->IsTwoHanded ? ECharacterState::ECS_EquippedTwoHandedWeapon : ECharacterState::ECS_EquippedOneHandedWeapon;
		OverlappingItem = nullptr;
		EquippedWeapon = OverlappingWeapon;
	}
	else
	{
		if (CanDisarm())
		{
			StoredWeapon = EquippedWeapon;
			EquippedWeapon = nullptr;

			PlayEquipMontage(FName("Unequip"));
			CharacterState = ECharacterState::ECS_Unequipped;
			ActionState = EActionState::EAS_EquippingWeapon;
		}
		else if (CanArm())
		{
			EquippedWeapon = StoredWeapon;
			StoredWeapon = nullptr;

			PlayEquipMontage(FName("Equip"));
			CharacterState = EquippedWeapon->IsTwoHanded ? ECharacterState::ECS_EquippedTwoHandedWeapon : ECharacterState::ECS_EquippedOneHandedWeapon;
			ActionState = EActionState::EAS_EquippingWeapon;
		}
		else if (CanSwap())
		{
			AWeapon* TempWeapon = EquippedWeapon;
			EquippedWeapon = StoredWeapon;
			StoredWeapon = TempWeapon;

			PlayEquipMontage(FName("Swap"));
			CharacterState = EquippedWeapon->IsTwoHanded ? ECharacterState::ECS_EquippedTwoHandedWeapon : ECharacterState::ECS_EquippedOneHandedWeapon;
			ActionState = EActionState::EAS_EquippingWeapon;
		}
	}
}

void AHackAndSlashCharacter::Dodge()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 30.f, FColor::Blue, TEXT("Dodge"));
	}
}

void AHackAndSlashCharacter::Attack()
{
	if (CanAttack())
	{
		EquippedWeapon->IsTwoHanded ? PlayTwoHandedAttackMontage() : PlayOneHandedAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
}

bool AHackAndSlashCharacter::CanAttack()
{
	return EquippedWeapon && ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

bool AHackAndSlashCharacter::CanDisarm()
{
	return ActionState == EActionState::EAS_Unoccupied && 
		CharacterState != ECharacterState::ECS_Unequipped &&
		StoredWeapon == nullptr &&
		EquippedWeapon != nullptr;
}

bool AHackAndSlashCharacter::CanArm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState == ECharacterState::ECS_Unequipped &&
		StoredWeapon != nullptr &&
		EquippedWeapon == nullptr;
}

bool AHackAndSlashCharacter::CanSwap()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped &&
		StoredWeapon != nullptr &&
		EquippedWeapon != nullptr;
}

void AHackAndSlashCharacter::Disarm()
{
	if (StoredWeapon)
	{
		StoredWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void AHackAndSlashCharacter::Arm()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void AHackAndSlashCharacter::Swap()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
	if (StoredWeapon)
	{
		StoredWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void AHackAndSlashCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void AHackAndSlashCharacter::PlayOneHandedAttackMontage()
{
	UAnimInstance* AnimInstace = GetMesh()->GetAnimInstance();
	if (AnimInstace && OneHandedAttackMontage)
	{
		AnimInstace->Montage_Play(OneHandedAttackMontage);
		const int32 Selection = FMath::RandRange(0, 3);
		FName SectionName = FName();
		switch (Selection)
		{
		case 0:
			SectionName = FName("Attack1");
			break;
		case 1:
			SectionName = FName("Attack2");
			break;
		case 2:
			SectionName = FName("Attack3");
			break;
		case 3:
			SectionName = FName("Attack4");
		}
		AnimInstace->Montage_JumpToSection(SectionName, OneHandedAttackMontage);
	}
}

void AHackAndSlashCharacter::PlayTwoHandedAttackMontage()
{
	UAnimInstance* AnimInstace = GetMesh()->GetAnimInstance();
	if (AnimInstace && TwoHandedAttackMontage)
	{
		AnimInstace->Montage_Play(TwoHandedAttackMontage);
		const int32 Selection = FMath::RandRange(0, 1);
		FName SectionName = FName();
		switch (Selection)
		{
		case 0:
			SectionName = FName("Attack1");
			break;
		case 1:
			SectionName = FName("Attack2");
			break;
		}
		AnimInstace->Montage_JumpToSection(SectionName, TwoHandedAttackMontage);
	}
}

void AHackAndSlashCharacter::PlayEquipMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstace = GetMesh()->GetAnimInstance();
	if (AnimInstace && EquipMontage)
	{
		AnimInstace->Montage_Play(EquipMontage);
		AnimInstace->Montage_JumpToSection(SectionName, EquipMontage);
	}
}

void AHackAndSlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void AHackAndSlashCharacter::Jump()
{
	Super::Jump();
}

void AHackAndSlashCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeapon->IgnoreActors.Empty();
	}
}

void AHackAndSlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHackAndSlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &AHackAndSlashCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHackAndSlashCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AHackAndSlashCharacter::Jump);
		EnhancedInputComponent->BindAction(EKeyPressedAction, ETriggerEvent::Started, this, &AHackAndSlashCharacter::EKeyPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AHackAndSlashCharacter::Attack);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &AHackAndSlashCharacter::Dodge);
	}

}

