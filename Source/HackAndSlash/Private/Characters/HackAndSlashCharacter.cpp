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
#include "Enemy/Enemy.h"
#include "HUD/HackAndSlashHUD.h"
#include "HUD/HackAndSlashOverlay.h"
#include "Items/Soul.h"
#include "Items/Treasure.h"
#include "Items/Health.h"

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

	BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
	BoxTraceStart->SetupAttachment(GetRootComponent());

	BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
	BoxTraceEnd->SetupAttachment(GetRootComponent());
}

void AHackAndSlashCharacter::Tick(float DeltaTime)
{
	if (Attributes && Overlay)
	{
		Attributes->RegenStamina(DeltaTime);	
		Overlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
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

float AHackAndSlashCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	SetHUDHealth();
	return DamageAmount;
}

void AHackAndSlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);

	CombatTarget = nullptr;
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
	if (CanAttack())
	{
		CombatTarget = nullptr;
		TArray<FHitResult> BoxHits;
		BoxTrace(BoxHits);

		for (auto Hit : BoxHits)
		{
			if (Hit.GetActor())
			{
				if (Hit.GetActor()->ActorHasTag(FName("Enemy")))
				{
					CombatTarget = Hit.GetActor();
					break;
				}
			}
		}		

		PlayAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
}

void AHackAndSlashCharacter::Dodge()
{
	if (IsOccupied() || !HasEnoughStamina()) return;

	PlayDodgeMontage();
	ActionState = EActionState::EAS_Dodge;
	if (Attributes && Overlay)
	{
		Attributes->UseStamina(Attributes->GetDodgeCost());
		Overlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
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
	CombatTarget = nullptr;
	ActionState = EActionState::EAS_Unoccupied;
}

void AHackAndSlashCharacter::DodgeEnd()
{
	Super::DodgeEnd();
	ActionState = EActionState::EAS_Unoccupied;
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
	ActionState = EActionState::EAS_EquippingWeapon;
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
	ActionState = EActionState::EAS_EquippingWeapon;
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
	ActionState = EActionState::EAS_EquippingWeapon;
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
	const FVector Start = BoxTraceStart->GetComponentLocation();
	const FVector End = BoxTraceEnd->GetComponentLocation();

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	UKismetSystemLibrary::BoxTraceMulti(this, Start, End, BoxTraceExtend, BoxTraceStart->GetComponentRotation(), ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::None, BoxHits, true);
}

void AHackAndSlashCharacter::Die()
{
	Super::Die();

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

void AHackAndSlashCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void AHackAndSlashCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

bool AHackAndSlashCharacter::IsAttacking()
{
	return ActionState == EActionState::EAS_Attacking;
}

bool AHackAndSlashCharacter::IsUnoccupied()
{
	return ActionState == EActionState::EAS_Unoccupied;
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
