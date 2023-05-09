// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "AIController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/WidgetComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Items/Weapons/Weapon.h"
#include "Items/Soul.h"
#include "Items/Health.h"
#include "Items/Magic.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);

	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());

	TargetLockWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("TargetLockWidget"));
	TargetLockWidget->SetupAttachment(GetRootComponent());

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	CombatTarget = EventInstigator->GetPawn();

	return DamageAmount;
}

void AEnemy::Destroyed()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
	if (EquippedWeaponSecondary)
	{
		EquippedWeaponSecondary->Destroy();
	}
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);
	if (!IsDead()) ShowHealthBar();
	ClearDodgeTimer();
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	StopAttackMontage();
	if (IsInsideAttackRadius() && !IsDead())
	{
		const float ReactionChoice = FMath::RandRange(0.f, 1.f);
		if (ReactionChoice <= DodgeChance)
		{
			StartDodgeTimer();
		}
	}
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	InitializeEnemy();
	Tags.Add(FName("Enemy"));
}

void AEnemy::Die_Implementation()
{	
	Super::Die_Implementation();
	OnTargetDeath.Broadcast();
	EnemyState = EEnemyState::EES_Dead;
	HideHealthBar();
	HideTargetLock();
	DisableCapsule();
	SetLifeSpan(DeathLifeSpan);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	DisableMeshCollision();
	SpawnSoulPickup();
	SpawnHealthPickup();
	SpawnMagicPickup();
}

void AEnemy::Dodge()
{
	EnemyState = EEnemyState::EES_Engaged;
	PlayDodgeMontage(FName("DodgeBack"));
}

void AEnemy::JumpAttack()
{
	if (CombatTarget == nullptr) return;
	PlayAirAttackMontage();
	EnemyState = EEnemyState::EES_Engaged;
}

void AEnemy::SpawnHealthPickup()
{
	UWorld* World = GetWorld();
	if (World && HealthClass && Attributes)
	{
		const FVector SpawnLocation = GetActorLocation() + FVector(-125.f, 0.f, 0.f);
		AHealth* SpawnedHealth = World->SpawnActor<AHealth>(HealthClass, SpawnLocation, GetActorRotation());
		if (SpawnedHealth)
		{
			SpawnedHealth->SetHealth(Attributes->GetHealthDropped());
			SpawnedHealth->SetOwner(this);
		}
	}
}

void AEnemy::SpawnMagicPickup()
{
	UWorld* World = GetWorld();
	if (World && MagicClass && Attributes)
	{
		const FVector SpawnLocation = GetActorLocation() + FVector(0.f, -125.f, 0.f);
		AMagic* SpawnedMagic = World->SpawnActor<AMagic>(MagicClass, SpawnLocation, GetActorRotation());
		if (SpawnedMagic)
		{
			SpawnedMagic->SetMagic(Attributes->GetMagicDropped());
			SpawnedMagic->SetOwner(this);
		}
	}
}

void AEnemy::SpawnSoulPickup()
{
	UWorld* World = GetWorld();
	if (World && SoulClass && Attributes)
	{
		const FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 125.f);
		ASoul* SpawnedSoul = World->SpawnActor<ASoul>(SoulClass, SpawnLocation, GetActorRotation());
		if (SpawnedSoul)
		{
			SpawnedSoul->SetSouls(Attributes->GetSoulsDropped());
			SpawnedSoul->SetOwner(this);
		}
	}
}

void AEnemy::Attack()
{
	Super::Attack();
	if (CombatTarget == nullptr) return;
	EnemyState = EEnemyState::EES_Engaged;
	PlayRandomAttackMontage();
}

void AEnemy::AttackRootMotion()
{
	Super::Attack();
	if (CombatTarget == nullptr) return;
	EnemyState = EEnemyState::EES_Engaged;
	PlayRandomRootMotionAttackMontage();
}

bool AEnemy::CanAttackWithWeapon()
{
	return IsInsideAttackRadius() && !IsAttacking() && !IsEngaged() && !IsDead();
}

void AEnemy::AttackEnd()
{
	EnemyState = EEnemyState::EES_NoState;
}

void AEnemy::DodgeEnd()
{
	EnemyState = EEnemyState::EES_NoState;
	const float ReactionChoice = FMath::RandRange(0.f, 1.f);
	if (ReactionChoice <= RetaliationChance)
	{
		JumpAttack();
	}
}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);
	if (Attributes && HealthBarWidget)
	{
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());
	}
}

void AEnemy::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	Super::SetWeaponCollisionEnabled(CollisionEnabled);
	if (EquippedWeaponSecondary && EquippedWeaponSecondary->GetWeaponBox())
	{
		EquippedWeaponSecondary->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeaponSecondary->IgnoreActors.Empty();
	}
}

void AEnemy::InitializeEnemy()
{
	EnemyController = Cast<AAIController>(GetController());
	HideHealthBar();
	SpawnDefaultWeapons();
}

void AEnemy::HideHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);
	}
}

void AEnemy::ShowHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(true);
	}
}

void AEnemy::HideTargetLock()
{
	if (TargetLockWidget)
	{
		TargetLockWidget->SetVisibility(false);
	}
}

FOnTargetDeath* AEnemy::GetOnTargetDeath()
{
	return &OnTargetDeath;
}

void AEnemy::ShowTargetLock()
{
	if (TargetLockWidget)
	{
		TargetLockWidget->SetVisibility(true);
	}
}

void AEnemy::LoseInterest()
{
	CombatTarget = nullptr;
	HideHealthBar();
}

bool AEnemy::IsOutsideCombatRadius()
{
	return !InTargetRange(CombatTarget, CombatRadius);
}

bool AEnemy::IsOutsideAttackRadius()
{
	return !InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsInsideAttackRadius()
{
	return InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsAttacking()
{
	return EnemyState == EEnemyState::EES_Attacking;
}

bool AEnemy::IsDead()
{
	return EnemyState == EEnemyState::EES_Dead;
}

bool AEnemy::IsEngaged()
{
	return EnemyState == EEnemyState::EES_Engaged;
}

void AEnemy::StartDodgeTimer()
{
	EnemyState = EEnemyState::EES_Dodging;
	GetWorldTimerManager().SetTimer(DodgeTimer, this, &AEnemy::Dodge, DodgeTime);
}

void AEnemy::ClearDodgeTimer()
{
	GetWorldTimerManager().ClearTimer(DodgeTimer);
}

bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
	if (Target == nullptr) return false;
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	return DistanceToTarget <= Radius;
}

void AEnemy::SpawnDefaultWeapons()
{
	UWorld* World = GetWorld();
	if (World && WeaponClass)
	{
		if (TwoWeapons)
		{
			AWeapon* DefaultWeaponR = World->SpawnActor<AWeapon>(WeaponClass);
			AWeapon* DefaultWeaponL = World->SpawnActor<AWeapon>(WeaponClass);
			DefaultWeaponR->Equip(GetMesh(), FName("WeaponSocketR"), this, this);
			DefaultWeaponL->Equip(GetMesh(), FName("WeaponSocketL"), this, this);
			EquippedWeapon = DefaultWeaponR;
			EquippedWeaponSecondary = DefaultWeaponL;
		}
		else
		{
			AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
			DefaultWeapon->Equip(GetMesh(), FName("WeaponSocket"), this, this);
			EquippedWeapon = DefaultWeapon;
		}
	}
}

