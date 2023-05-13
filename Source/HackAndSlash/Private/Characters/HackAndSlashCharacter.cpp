// Fill out your copyright notice in the Description page of Project Settings.
// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/HackAndSlashCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Components/AttributeComponent.h"
#include "Items/Item.h"
#include "Items/Weapons/Weapon.h"
#include "Items/Weapons/Projectile.h"
#include "Animation/AnimMontage.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy/Enemy.h"
#include "HUD/HackAndSlashOverlay.h"
#include "Items/Soul.h"
#include "Items/Treasure.h"
#include "Items/Health.h"
#include "Items/Magic.h"
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
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
	GetMesh()->SetGenerateOverlapEvents(true);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.f;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 20.f;

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(GetRootComponent());

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);
	ViewCamera->FieldOfView = DefaultFOV;
	TargetFOV = DefaultFOV;
	
	JumpCount = 0;
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

	FHitResult HitResult;
	TraceUnderCrosshairs(HitResult);
	HitTarget = HitResult.ImpactPoint;

	SetHUDCrosshairs(DeltaTime);
	HideCharacterIfCameraClose();
}

void AHackAndSlashCharacter::FollowTarget(float DeltaTime)
{
	const FVector Target = CombatTarget->GetActorLocation();
	const FRotator CurrentRot = GetController()->GetControlRotation();
	const FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	const FRotator InterpRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 5.f);
	const FRotator NewRot = UKismetMathLibrary::MakeRotator(CurrentRot.Roll, CurrentRot.Pitch, InterpRot.Yaw);
	GetController()->SetControlRotation(NewRot);
}

void AHackAndSlashCharacter::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		float DistanceToCharacter = (GetActorLocation() - Start).Size();
		Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		FVector End = Start + CrosshairWorldDirection * TargetTraceLength;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);

		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}

		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}
	}
}

void AHackAndSlashCharacter::SetHUDCrosshairs(float DeltaTime)
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		AHackAndSlashHUD* HUD = Cast<AHackAndSlashHUD>(PlayerController->GetHUD());
		if (HUD)
		{
			if (!EquippedWeapon && IsAlive())
			{
				HUDPackage.CrosshairsCenter = HUD->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = HUD->CrosshairsLeft;
				HUDPackage.CrosshairsRight = HUD->CrosshairsRight;
				HUDPackage.CrosshairsTop = HUD->CrosshairsTop;
				HUDPackage.CrosshairsBottom = HUD->CrosshairsBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}
			// Calculate crosshair spread
			FVector2D WalkSpeedRange(0.f, GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplier(0.f, 1.f);
			FVector Velocity = GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplier, Velocity.Size());

			if (GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

			HUDPackage.CrosshairSpread =
				0.5f +
				CrosshairVelocityFactor +
				CrosshairInAirFactor -
				CrosshairAimFactor +
				CrosshairShootingFactor;

			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void AHackAndSlashCharacter::ShootProjectile(const FName& SocketName)
{
	ShootProjectile(HitTarget, SocketName);
}

void AHackAndSlashCharacter::ShootProjectile(const FVector& Target, const FName& SocketName)
{
	if (EnergyProjectileClass)
	{
		const FVector StartingLocation = GetMesh()->GetSocketLocation(SocketName);
		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(
				EnergyProjectileClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParams
				);
		}
	}
}

void AHackAndSlashCharacter::ShootStrongProjectile()
{
	if (StrongProjectileClass)
	{
		const FVector StartingLocation = GetMesh()->GetSocketLocation(FName("Muzzle_02"));
		FVector ToTarget = HitTarget - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(
				StrongProjectileClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParams
				);
		}
	}
}

void AHackAndSlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &AHackAndSlashCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHackAndSlashCharacter::Look);
		EnhancedInputComponent->BindAction(ControllerLookAction, ETriggerEvent::Triggered, this, &AHackAndSlashCharacter::ControllerLook);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AHackAndSlashCharacter::Jump);
		EnhancedInputComponent->BindAction(EKeyPressedAction, ETriggerEvent::Started, this, &AHackAndSlashCharacter::EKeyPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AHackAndSlashCharacter::Attack);
		EnhancedInputComponent->BindAction(PowerAttackAction, ETriggerEvent::Started, this, &AHackAndSlashCharacter::AttackRootMotion);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &AHackAndSlashCharacter::Dodge);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AHackAndSlashCharacter::SprintStart);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AHackAndSlashCharacter::SprintEnd);
		EnhancedInputComponent->BindAction(TargetLockAction, ETriggerEvent::Started, this, &AHackAndSlashCharacter::TargetLock);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AHackAndSlashCharacter::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AHackAndSlashCharacter::AimButtonReleased);
		EnhancedInputComponent->BindAction(DropWeaponAction, ETriggerEvent::Started, this, &AHackAndSlashCharacter::DropWeapon);
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
	if (!FinishedEquipping)
	{
		SwapAttachedWeapons();
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

void AHackAndSlashCharacter::AddMagic(AMagic* Magic)
{
	if (Attributes && Overlay)
	{
		Attributes->AddMagic(Magic->GetMagic());
		Overlay->SetMagicBarPercent(Attributes->GetMagicPercent());
	}
}

void AHackAndSlashCharacter::Jump()
{
	if (IsUnoccupied() || IsAttacking())
	{
		Super::Jump();
		JumpCount++;
		if (JumpCount == 2)
		{
			GetWorldTimerManager().SetTimer(DoubleJumpResetTimer, this, &AHackAndSlashCharacter::ResetDoubleJump, DoubleJumpResetTime);
			LaunchCharacter(FVector(0.f, 0.f, GetCharacterMovement()->JumpZVelocity), true, true);
		}
	}
}

void AHackAndSlashCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	JumpCount = 0;
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
	UE_LOG(LogTemp, Warning, TEXT("Begin Play"));
	Tags.Add(FName("EngageableTarget"));
	InitializeOverlay();
	SlowTime(1.f);
	PlaySpawnMontage();
}

bool AHackAndSlashCharacter::CanBeSeenFrom(const FVector& ObserverLocation, FVector& OutSeenLocation, int32& NumberOfLoSChecksPerformed, float& OutSightStrength, const AActor* IgnoreActor, const bool* bWasVisible, int32* UserData) const
{
	static const FName NAME_AILineOfSight = FName(TEXT("TestPawnLineOfSight"));

	FHitResult HitResult;
	FVector SocketLocation = GetMesh()->GetSocketLocation(TargetBone);
	const bool bHitSocket = GetWorld()->LineTraceSingleByObjectType(HitResult, ObserverLocation, SocketLocation, FCollisionObjectQueryParams(ECC_TO_BITFIELD(ECC_WorldStatic) | ECC_TO_BITFIELD(ECC_WorldDynamic)), FCollisionQueryParams(NAME_AILineOfSight, true, IgnoreActor));

	NumberOfLoSChecksPerformed++;

	if (bHitSocket == false || (HitResult.GetActor() && HitResult.GetActor()->IsOwnedBy(this)))
	{
		OutSeenLocation = SocketLocation;
		OutSightStrength = 1;

		return true;
	}

	const bool bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, ObserverLocation, GetActorLocation(), FCollisionObjectQueryParams(ECC_TO_BITFIELD(ECC_WorldStatic) | ECC_TO_BITFIELD(ECC_WorldDynamic)), FCollisionQueryParams(NAME_AILineOfSight, true, IgnoreActor));

	NumberOfLoSChecksPerformed++;

	if (bHit == false || (HitResult.GetActor() && HitResult.GetActor()->IsOwnedBy(this)))
	{
		OutSeenLocation = GetActorLocation();
		OutSightStrength = 1;

		return true;
	}

	OutSightStrength = 0;
	return false;
}

void AHackAndSlashCharacter::Move(const FInputActionValue& Value)
{
	MovementVector = Value.Get<FVector2D>();
	if (!CanMove()) return;

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
			AddControllerYawInput(LookAxisValue.X * (bAiming ? AimLookSensitivityYaw : 1.f));
		}
		AddControllerPitchInput(LookAxisValue.Y * (bAiming ? AimLookSensitivityPitch : 1.f));
	}
}

void AHackAndSlashCharacter::ControllerLook(const FInputActionValue& Value)
{
	const FVector2D LookAxisValue = Value.Get<FVector2D>();
	if (GetController() && LookAxisValue != FVector2D::ZeroVector)
	{
		if (!TargetLocked)
		{
			AddControllerYawInput(LookAxisValue.X * (bAiming ? AimLookSensitivityYaw : LookSensitivityYaw));
		}
		AddControllerPitchInput(LookAxisValue.Y * (bAiming ? AimLookSensitivityPitch : LookSensitivityPitch));
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
			FinishedEquipping = false;
			Disarm();
		}
		else if (CanArm())
		{
			FinishedEquipping = false;
			Arm();
		}
		else if (CanSwap())
		{
			FinishedEquipping = false;
			Swap();
		}
	}
}

void AHackAndSlashCharacter::Attack()
{
	Super::Attack();
	if (!FinishedEquipping)
	{
		SwapAttachedWeapons();
	}
	if (IsAttacking())
	{
		SaveAttack = true;
	}
	else if (CanAttackWithWeapon())
	{
		if (CombatTarget == nullptr) TraceForCombatTarget(TargetLocked);

		PlayAttackMontage(0);
		ActionState = EActionState::EAS_Attacking;
	}
	else if (CanUseMagic())
	{
		if (Attributes && Attributes->GetMagic() >= Attributes->GetMagicCost())
		{
			DoMagicAttack(0);
		}
		else
		{
			PlayNoMagicAudio();
		}
	}
}

void AHackAndSlashCharacter::AttackRootMotion()
{
	Super::Attack();
	if (IsAttacking())
	{
		SaveAttack = true;
	}
	else if (CanAttackWithWeapon())
	{
		if (CombatTarget == nullptr) TraceForCombatTarget(TargetLocked);

		if (GetMovementComponent()->IsFalling())
		{
			PlayAirAttackMontage();
		}
		else
		{
			PlayRootMotionAttackMontage(0);
		}
		ActionState = EActionState::EAS_Attacking;
	}
	else if (CanUseMagic())
	{
		if (Attributes && Attributes->GetMagic() >= Attributes->GetStrongMagicCost())
		{
			DoStrongMagicAttack();
		}
		else
		{
			PlayNoMagicAudio();
		}
	}
}

void AHackAndSlashCharacter::DoMagicAttack(int32 ComboCount)
{
	if (CombatTarget == nullptr) TraceForCombatTarget(TargetLocked);

	FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), FVector(HitTarget.X, HitTarget.Y, 0.f));
	TargetRot.Pitch = 0.f;
	SetActorRotation(TargetRot);

	PlayMagicAttackMontage(ComboCount);
	ActionState = EActionState::EAS_Attacking;
	if (Attributes && Overlay)
	{
		Attributes->UseMagic(Attributes->GetMagicCost());
		Overlay->SetMagicBarPercent(Attributes->GetMagicPercent());
	}
}

void AHackAndSlashCharacter::DoStrongMagicAttack()
{
	if (CombatTarget == nullptr) TraceForCombatTarget(TargetLocked);

	FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), FVector(HitTarget.X, HitTarget.Y, 0.f));
	TargetRot.Pitch = 0.f;
	SetActorRotation(TargetRot);

	PlayStrongMagicAttackMontage();
	ActionState = EActionState::EAS_Attacking;
	if (Attributes && Overlay)
	{
		Attributes->UseMagic(Attributes->GetStrongMagicCost());
		Overlay->SetMagicBarPercent(Attributes->GetMagicPercent());
	}
}

void AHackAndSlashCharacter::Dodge()
{
	if (IsOccupied() || !HasEnoughStamina()) return;

	FName SectionName = FName("DodgeForward");
	if (TargetLocked && GetLastMovementInputVector() != FVector::ZeroVector)
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
	else if (GetLastMovementInputVector() != FVector::ZeroVector)
	{
		const FRotator TargetRot = GetLastMovementInputVector().Rotation();
		SetActorRotation(TargetRot);
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
		ResetTargetLock();
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

void AHackAndSlashCharacter::AimButtonPressed()
{
	if (EquippedWeapon) return;

	bAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = AimWalkSpeed;
	TargetFOV = AimingFOV;
}

void AHackAndSlashCharacter::AimButtonReleased()
{
	if (EquippedWeapon) return;
	bAiming = false;
	GetCharacterMovement()->MaxWalkSpeed = DefaultSpeed;
	TargetFOV = DefaultFOV;
}

void AHackAndSlashCharacter::DropWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Drop();
		EquippedWeapon = nullptr;
		CharacterState = ECharacterState::ECS_Unequipped;
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

void AHackAndSlashCharacter::SpawnEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

bool AHackAndSlashCharacter::CanAttackWithWeapon()
{
	return EquippedWeapon && ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

bool AHackAndSlashCharacter::CanUseMagic()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState == ECharacterState::ECS_Unequipped;
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

bool AHackAndSlashCharacter::CanMove()
{
	return ActionState <= EActionState::EAS_Attacking;
}

void AHackAndSlashCharacter::PlayMagicAttackMontage(int32 ComboCount)
{
	if (MagicAttackMontage && MagicAttackMontageSections.Num() - 1 >= ComboCount)
	{
		PlayMontageSection(MagicAttackMontage, MagicAttackMontageSections[ComboCount]);
	}
}

void AHackAndSlashCharacter::PlayStrongMagicAttackMontage()
{
	if (StrongMagicAttackMontage)
	{
		PlayMontageSection(StrongMagicAttackMontage, FName("Attack1"));
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
					FOnTargetDeath* OnTargetDeathDelegate = TargetLockInterface->GetOnTargetDeath();
					if (OnTargetDeathDelegate)
					{
						(*OnTargetDeathDelegate).AddDynamic(this, &AHackAndSlashCharacter::OnTargetDeath);
					}
					TargetLockInterface->ShowTargetLock();
				}
			}
			return;
		}
	}
	if (CombatTarget)
	{
		ResetTargetLock();
	}	
}

void AHackAndSlashCharacter::Die_Implementation()
{
	Super::Die_Implementation();

	ActionState = EActionState::EAS_Dead;
	DisableMeshCollision();
	PlayDeathAudio();
	ResetTargetLock();
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		AHackAndSlashHUD* HUD = Cast<AHackAndSlashHUD>(PlayerController->GetHUD());
		if (HUD)
		{
			HUD->ShowDeathScreen();
		}		
	}

}

bool AHackAndSlashCharacter::HasEnoughStamina()
{
	return Attributes && Attributes->GetStamina() >= Attributes->GetDodgeCost();
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
	FinishedEquipping = true;
}

void AHackAndSlashCharacter::AttachWeaponToHand()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
	FinishedEquipping = true;
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
	FinishedEquipping = true;
}

void AHackAndSlashCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void AHackAndSlashCharacter::ComboAttackSave(bool PowerAttack)
{
	if (SaveAttack)
	{
		SaveAttack = false;

		switch (AttackCount)
		{
		case 0:
			AttackCount++;
			PowerAttack ? PlayRootMotionAttackMontage(AttackCount) : PlayAttackMontage(AttackCount);
			break;
		case 1:
			AttackCount++;
			PowerAttack ? PlayRootMotionAttackMontage(AttackCount) : PlayAttackMontage(AttackCount);
			break;
		case 2:
			AttackCount++;
			PowerAttack ? PlayRootMotionAttackMontage(AttackCount) : PlayAttackMontage(AttackCount);
			break;
		case 3:
			AttackCount++;
			PowerAttack ? PlayRootMotionAttackMontage(AttackCount) : PlayAttackMontage(AttackCount);
			AttackCount = 0;
			break;
		default:
			break;
		}
	}
}

void AHackAndSlashCharacter::MagicComboAttackSave(bool PowerAttack)
{
	if (SaveAttack)
	{
		SaveAttack = false;

		switch (AttackCount)
		{
		case 0:
			AttackCount++;
			DoMagicAttack(AttackCount);
			break;
		case 1:
			AttackCount++;
			DoMagicAttack(AttackCount);
			break;
		case 2:
			AttackCount++;
			DoMagicAttack(AttackCount);
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

void AHackAndSlashCharacter::SlowTime(float TimeDilation)
{
	UGameplayStatics::SetGlobalTimeDilation(this, TimeDilation);
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
	return GetCharacterMovement()->MaxWalkSpeed > DefaultSpeed && GetVelocity() != FVector::ZeroVector;
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

void AHackAndSlashCharacter::HideCharacterIfCameraClose()
{
	if ((ViewCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (EquippedWeapon && EquippedWeapon->GetItemMesh())
		{
			EquippedWeapon->GetItemMesh()->bOwnerNoSee = true;
		}
		if (StoredWeapon && StoredWeapon->GetItemMesh())
		{
			StoredWeapon->GetItemMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (EquippedWeapon && EquippedWeapon->GetItemMesh())
		{
			EquippedWeapon->GetItemMesh()->bOwnerNoSee = false;
		}
		if (StoredWeapon && StoredWeapon->GetItemMesh())
		{
			StoredWeapon->GetItemMesh()->bOwnerNoSee = false;
		}
	}
}

void AHackAndSlashCharacter::PlayDeathAudio()
{
	if (AudioComponent && DeathAudio && !AudioComponent->IsPlaying())
	{
		AudioComponent->Sound = DeathAudio;
		AudioComponent->Play();
	}
}

void AHackAndSlashCharacter::ResetDoubleJump()
{
	JumpCount = 0;
}

void AHackAndSlashCharacter::ResetTargetLock()
{
	if (ITargetLockInterface* TargetLockInterface = Cast<ITargetLockInterface>(CombatTarget))
	{
		FOnTargetDeath* OnTargetDeathDelegate = TargetLockInterface->GetOnTargetDeath();
		if (OnTargetDeathDelegate)
		{
			(*OnTargetDeathDelegate).RemoveDynamic(this, &AHackAndSlashCharacter::OnTargetDeath);
		}
		TargetLockInterface->HideTargetLock();
	}
	TargetLocked = false;
	CombatTarget = nullptr;
	SetMovementToDefault();
}

void AHackAndSlashCharacter::PlayNoMagicAudio()
{
	if (AudioComponent && NoMagicAudio && !AudioComponent->IsPlaying())
	{
		AudioComponent->Sound = NoMagicAudio;
		AudioComponent->Play();
	}
}

void AHackAndSlashCharacter::OnTargetDeath()
{
	SetMovementToDefault();
	TargetLocked = false;
	CombatTarget = nullptr;
}
