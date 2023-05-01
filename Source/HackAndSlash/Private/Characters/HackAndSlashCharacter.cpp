// Fill out your copyright notice in the Description page of Project Settings.
// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/HackAndSlashCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AttributeComponent.h"
#include "Items/Item.h"
#include "Items/Weapons/Weapon.h"
#include "Animation/AnimMontage.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Enemy/Enemy.h"
#include "HUD/HackAndSlashHUD.h"
#include "HUD/HackAndSlashOverlay.h"
#include "Items/Soul.h"
#include "Items/Treasure.h"
#include "Items/Health.h"
#include "Interfaces/TargetLockInterface.h"

AHackAndSlashCharacter::AHackAndSlashCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);
	ViewCamera->FieldOfView = DefaultFOV;
	TargetFOV = DefaultFOV;
}

void AHackAndSlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Attributes && Overlay)
	{
		if (IsSprinting())
		{
			Attributes->UseStaminaOverTime(Attributes->GetSprintCost(), DeltaTime);
			if (Attributes->GetStamina() <= 0.f)
			{
				SprintEnd();
			}
		}
		else
		{
			Attributes->RegenStamina(DeltaTime);	
		}
		Overlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
	if (ViewCamera->FieldOfView != TargetFOV)
	{
		ViewCamera->FieldOfView = FMath::FInterpTo(ViewCamera->FieldOfView, TargetFOV, DeltaTime, 6.f);
	}
	if (TargetLocked && CombatTarget)
	{
		FollowTarget(DeltaTime);
	}
}

void AHackAndSlashCharacter::FollowTarget(float DeltaTime)
{
	const FVector Target = CombatTarget->GetActorLocation();
	const FRotator CurrentRot = GetController()->GetControlRotation();
	const FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), FVector(Target.X, Target.Y, Target.Z));
	const FRotator InterpRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 5.f);
	const FRotator NewRot = UKismetMathLibrary::MakeRotator(CurrentRot.Roll, CurrentRot.Pitch, InterpRot.Yaw);
	GetController()->SetControlRotation(NewRot);
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
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AHackAndSlashCharacter::Attack);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &AHackAndSlashCharacter::Dodge);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AHackAndSlashCharacter::SprintStart);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AHackAndSlashCharacter::SprintEnd);
		EnhancedInputComponent->BindAction(TargetLockAction, ETriggerEvent::Started, this, &AHackAndSlashCharacter::TargetLock);
	}
}

float AHackAndSlashCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	SetHUDHealth();
	return DamageAmount;
}

void AHackAndSlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);
	if (!TargetLocked)
	{
		CombatTarget = nullptr;
	}
	SaveAttack = false;
	SprintEnd();
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	if (IsAlive())
	{
		ActionState = EActionState::EAS_HitReaction;
	}
}

void AHackAndSlashCharacter::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;
}

void AHackAndSlashCharacter::AddSouls(ASoul* Soul)
{
	if (Attributes && Overlay)
	{
		Attributes->AddSouls(Soul->GetSouls());
		Overlay->SetSouls(Attributes->GetSouls());
	}
}

void AHackAndSlashCharacter::AddGold(ATreasure* Treasure)
{
	if (Attributes && Overlay)
	{
		Attributes->AddGold(Treasure->GetGold());
		Overlay->SetGold(Attributes->GetGold());
	}
}

void AHackAndSlashCharacter::AddHealth(AHealth* Health)
{
	if (Attributes && Overlay)
	{
		Attributes->AddHealth(Health->GetHealth());
		Overlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}

void AHackAndSlashCharacter::Jump()
{
	if (IsUnoccupied() || IsAttacking()) Super::Jump();
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

	Tags.Add(FName("EngageableTarget"));

	InitializeOverlay();
}

void AHackAndSlashCharacter::Move(const FInputActionValue& Value)
{
	MovementVector = Value.Get<FVector2D>();
	if (ActionState != EActionState::EAS_Unoccupied) return;

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
		if (!TargetLocked)
		{
			AddControllerYawInput(LookAxisValue.X);
		}
		AddControllerPitchInput(LookAxisValue.Y);
	}
}

void AHackAndSlashCharacter::EKeyPressed()
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if (OverlappingWeapon)
	{
		EquipWeapon(OverlappingWeapon);
	}
	else
	{
		if (CanDisarm())
		{
			Disarm();
		}
		else if (CanArm())
		{
			Arm();
		}
		else if (CanSwap())
		{
			Swap();
		}
	}
}

void AHackAndSlashCharacter::Attack()
{
	Super::Attack();
	if (IsAttacking())
	{
		SaveAttack = true;
	}
	else if (CanAttack())
	{
		TraceForCombatTarget(TargetLocked);

		PlayAttackMontage();	
		ActionState = EActionState::EAS_Attacking;
	}
}

void AHackAndSlashCharacter::Dodge()
{
	if (IsOccupied() || !HasEnoughStamina()) return;

	FName SectionName = FName("DodgeForward");
	if (TargetLocked && MovementVector != FVector2D::ZeroVector)
	{
		if (MovementVector.X > 0.f)
		{
			SectionName = FName("DodgeRight");
		}
		else if (MovementVector.X < 0.f)
		{
			SectionName = FName("DodgeLeft");
		}
		else if (MovementVector.Y < 0.f)
		{
			SectionName = FName("DodgeBack");
		}
	}	
	PlayDodgeMontage(SectionName);
	ActionState = EActionState::EAS_Dodge;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Attributes && Overlay)
	{
		Attributes->UseStamina(Attributes->GetDodgeCost());
		Overlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
}

void AHackAndSlashCharacter::SprintStart()
{
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	if (TargetLocked && CombatTarget)
	{
		SetMovementToDefault();
		if (ITargetLockInterface* TargetLockInterface = Cast<ITargetLockInterface>(CombatTarget))
		{
			TargetLockInterface->HideTargetLock();
		}
		TargetLocked = false;
		CombatTarget = nullptr;
	}
	TargetFOV = SprintFOV;
}

void AHackAndSlashCharacter::SprintEnd()
{
	GetCharacterMovement()->MaxWalkSpeed = DefaultSpeed;
	TargetFOV = DefaultFOV;
}

void AHackAndSlashCharacter::TargetLock()
{
	TraceForCombatTarget(true);

	if (CombatTarget)
	{
		SetMovementToStrafing();
		TargetLocked = true;
	}
	else
	{
		SetMovementToDefault();
		TargetLocked = false;
	}
}

void AHackAndSlashCharacter::SetMovementToStrafing()
{
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseSeparateBrakingFriction = false;
	bUseControllerRotationYaw = true;
}

void AHackAndSlashCharacter::SetMovementToDefault()
{
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseSeparateBrakingFriction = true;
	bUseControllerRotationYaw = false;
}

void AHackAndSlashCharacter::EquipWeapon(AWeapon* Weapon)
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Drop();
	}
	Weapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
	CharacterState = Weapon->IsTwoHanded ? ECharacterState::ECS_EquippedTwoHandedWeapon : ECharacterState::ECS_EquippedOneHandedWeapon;
	OverlappingItem = nullptr;
	EquippedWeapon = Weapon;
}

void AHackAndSlashCharacter::AttackEnd()
{
	if (!TargetLocked)
	{
		CombatTarget = nullptr;
	}
	ActionState = EActionState::EAS_Unoccupied;
}

void AHackAndSlashCharacter::DodgeEnd()
{
	Super::DodgeEnd();
	ActionState = EActionState::EAS_Unoccupied;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
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

void AHackAndSlashCharacter::Disarm()
{
	StoredWeapon = EquippedWeapon;
	EquippedWeapon = nullptr;

	PlayEquipMontage(FName("Unequip"));
	CharacterState = ECharacterState::ECS_Unequipped;
}

bool AHackAndSlashCharacter::CanArm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState == ECharacterState::ECS_Unequipped &&
		StoredWeapon != nullptr &&
		EquippedWeapon == nullptr;
}

void AHackAndSlashCharacter::Arm()
{
	EquippedWeapon = StoredWeapon;
	StoredWeapon = nullptr;

	PlayEquipMontage(FName("Equip"));
	CharacterState = EquippedWeapon->IsTwoHanded ? ECharacterState::ECS_EquippedTwoHandedWeapon : ECharacterState::ECS_EquippedOneHandedWeapon;
}

bool AHackAndSlashCharacter::CanSwap()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped &&
		StoredWeapon != nullptr &&
		EquippedWeapon != nullptr;
}

void AHackAndSlashCharacter::Swap()
{
	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = StoredWeapon;
	StoredWeapon = TempWeapon;

	PlayEquipMontage(FName("Swap"));
	CharacterState = EquippedWeapon->IsTwoHanded ? ECharacterState::ECS_EquippedTwoHandedWeapon : ECharacterState::ECS_EquippedOneHandedWeapon;
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

void AHackAndSlashCharacter::BoxTrace(TArray<FHitResult>& BoxHits)
{
	const FVector Start = GetActorLocation() + FVector(0.f, 0.f, BoxTraceExtend.Z);
	const FVector End = Start + ViewCamera->GetForwardVector() * 1000.f;

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	UKismetSystemLibrary::BoxTraceMulti(this, Start, End, BoxTraceExtend, GetActorRotation(), ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::None, BoxHits, true);
}

void AHackAndSlashCharacter::TraceForCombatTarget(bool ShouldTargetLock)
{
	TArray<FHitResult> BoxHits;
	BoxTrace(BoxHits);

	for (auto Hit : BoxHits)
	{
		if (Hit.GetActor() && Hit.GetActor()->ActorHasTag(FName("Enemy")) && Hit.GetActor() != CombatTarget)
		{
			CombatTarget = Hit.GetActor();
			if (ShouldTargetLock)
			{
				if (ITargetLockInterface* TargetLockInterface = Cast<ITargetLockInterface>(CombatTarget))
				{
					TargetLockInterface->ShowTargetLock();
				}
			}
			return;
		}
	}
	if (CombatTarget)
	{
		if (ITargetLockInterface* TargetLockInterface = Cast<ITargetLockInterface>(CombatTarget))
		{
			TargetLockInterface->HideTargetLock();
		}
		CombatTarget = nullptr;
	}	
}

void AHackAndSlashCharacter::Die_Implementation()
{
	Super::Die_Implementation();

	ActionState = EActionState::EAS_Dead;
	DisableMeshCollision();
}

bool AHackAndSlashCharacter::HasEnoughStamina()
{
	return Attributes && Attributes->GetStamina() > Attributes->GetDodgeCost();
}

bool AHackAndSlashCharacter::IsOccupied()
{
	return ActionState != EActionState::EAS_Unoccupied;
}

bool AHackAndSlashCharacter::IsDead()
{
	return ActionState == EActionState::EAS_Dead;
}

void AHackAndSlashCharacter::AttachWeaponToBack()
{
	if (StoredWeapon)
	{
		StoredWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void AHackAndSlashCharacter::AttachWeaponToHand()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void AHackAndSlashCharacter::SwapAttachedWeapons()
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

void AHackAndSlashCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void AHackAndSlashCharacter::ComboAttackSave()
{
	if (SaveAttack)
	{
		SaveAttack = false;

		switch (AttackCount)
		{
		case 0:
			PlayAttackMontage(AttackCount);
			AttackCount++;
			break;
		case 1:
			PlayAttackMontage(AttackCount);
			AttackCount++;
			break;
		case 2:
			PlayAttackMontage(AttackCount);
			AttackCount = 0;
			break;
		default:
			break;
		}
	}
}

void AHackAndSlashCharacter::ResetCombo()
{
	AttackCount = 0;
	SaveAttack = false;
}

bool AHackAndSlashCharacter::IsAttacking()
{
	return ActionState == EActionState::EAS_Attacking;
}

bool AHackAndSlashCharacter::IsUnoccupied()
{
	return ActionState == EActionState::EAS_Unoccupied;
}

bool AHackAndSlashCharacter::IsSprinting()
{
	return GetCharacterMovement()->MaxWalkSpeed > DefaultSpeed && GetCharacterMovement()->IsMovingOnGround();
}

void AHackAndSlashCharacter::InitializeOverlay()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		AHackAndSlashHUD* HUD = Cast<AHackAndSlashHUD>(PlayerController->GetHUD());
		if (HUD)
		{
			Overlay = HUD->GetOverlay();
			if (Overlay && Attributes)
			{
				Overlay->SetHealthBarPercent(Attributes->GetHealthPercent());
				Overlay->SetStaminaBarPercent(1.f);
				Overlay->SetGold(0);
				Overlay->SetSouls(0);
			}
		}
	}
}

void AHackAndSlashCharacter::SetHUDHealth()
{
	if (Overlay && Attributes)
	{
		Overlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}
