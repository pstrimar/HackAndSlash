// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "Enemy/EnemyAIController.h"
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
#include "Items/Weapons/Projectile.h"

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

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	ProjectileStartLocation1 = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileStartLocation1"));
	ProjectileStartLocation1->SetupAttachment(GetRootComponent());
	ProjectileStartLocations.Emplace(ProjectileStartLocation1);
	ProjectileStartLocation2 = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileStartLocation2"));
	ProjectileStartLocation2->SetupAttachment(GetRootComponent());
	ProjectileStartLocations.Emplace(ProjectileStartLocation2);
	ProjectileStartLocation3 = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileStartLocation3"));
	ProjectileStartLocation3->SetupAttachment(GetRootComponent());
	ProjectileStartLocations.Emplace(ProjectileStartLocation3);
	ProjectileStartLocation4 = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileStartLocation4"));
	ProjectileStartLocation4->SetupAttachment(GetRootComponent());
	ProjectileStartLocations.Emplace(ProjectileStartLocation4);
	ProjectileStartLocation5 = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileStartLocation5"));
	ProjectileStartLocation5->SetupAttachment(GetRootComponent());
	ProjectileStartLocations.Emplace(ProjectileStartLocation5);
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	CombatTarget = EventInstigator->GetPawn();
	if (EnemyController)
	{
		EnemyController->SetTarget(CombatTarget);
	}
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
	if (!IsDead())
	{
		ShowHealthBar();
		EnemyState = EEnemyState::EES_NoState;
	}
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
	if (SpawnMontage)
	{
		PlaySpawnMontage();
		GetCharacterMovement()->DisableMovement();
	}
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
	SetGroundImpactCollisionEnabled(ECollisionEnabled::NoCollision);
	DisableMeshCollision();
	SpawnSoulPickup();
	SpawnHealthPickup();
	SpawnMagicPickup();

	// Start dissolve effect
	if (DissolveMaterialInstances.Num() > 0)
	{
		for (int i = 0; i < DissolveMaterialInstances.Num(); i++)
		{
			UMaterialInstanceDynamic* DynamicDissolveMatInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstances[i], this);
			DynamicDissolveMaterialInstances.Add(DynamicDissolveMatInstance);
			GetMesh()->SetMaterial(i, DynamicDissolveMaterialInstances[i]);
			DynamicDissolveMaterialInstances[i]->SetScalarParameterValue(TEXT("Dissolve"), -0.55f);
			DynamicDissolveMaterialInstances[i]->SetScalarParameterValue(TEXT("Glow"), 200.f);
		}

	}
	StartDissolve();
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

bool AEnemy::CanCast()
{
	return !IsAttacking() && !IsEngaged() && !IsDead();
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
	EnemyController = GetController<AEnemyAIController>();
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

void AEnemy::ShootProjectiles()
{
	if (ProjectileClass && ProjectileStartLocations.Num() > 0)
	{
		if (ProjectileTimers.Num() != ProjectileStartLocations.Num())
		{
			for (int i = 0; i < ProjectileStartLocations.Num(); i++)
			{
				FTimerHandle Timer;
				ProjectileTimers.Emplace(Timer);
			}
		}
		float FireTime = 0.f;
		for (int i = 0; i < ProjectileStartLocations.Num(); i++)
		{
			const FVector Location = ProjectileStartLocations[i]->GetComponentLocation();
			const FRotator Rotation = ProjectileStartLocations[i]->GetComponentRotation();
			ShootProjectileDelegate.BindUFunction(this, FName("ShootProjectile"), Location, Rotation);
			GetWorldTimerManager().SetTimer(ProjectileTimers[i], ShootProjectileDelegate, FireTime, false);
			FireTime += 0.1f;
		}
	}
}

void AEnemy::ShootProjectile(const FVector& StartLocation, const FRotator& Rotation)
{
	if (ProjectileClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(
				ProjectileClass,
				StartLocation,
				Rotation,
				SpawnParams
				);
		}		
	}	
}

bool AEnemy::IsEngaged()
{
	return EnemyState == EEnemyState::EES_Engaged;
}

void AEnemy::Cast()
{
	if (CombatTarget == nullptr) return;
	EnemyState = EEnemyState::EES_Engaged;
	PlayCastMontage();
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
		if (ImpactWeaponClass)
		{
			AWeapon* ImpactWeapon = World->SpawnActor<AWeapon>(ImpactWeaponClass);
			ImpactWeapon->Equip(GetMesh(), FName("ImpactWeaponSocket"), this, this);
			GroundImpactWeapon = ImpactWeapon;
		}
		if (FrontWeaponClass)
		{
			AWeapon* FrontCastWeapon = World->SpawnActor<AWeapon>(FrontWeaponClass);
			FrontCastWeapon->Equip(GetMesh(), FName("FrontWeaponSocket"), this, this);
			FrontWeapon = FrontCastWeapon;
		}
	}
}

void AEnemy::PlaySpawnMontage()
{
	PlayMontageSection(SpawnMontage, FName("Spawn"));
}

void AEnemy::PlayCastMontage()
{
	PlayMontageSection(CastMontage, FName("Attack1"));
}

void AEnemy::OnSpawnEnd()
{
	GetCharacterMovement()->SetDefaultMovementMode();
}

void AEnemy::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstances.Num() > 0)
	{
		for (auto Instance : DynamicDissolveMaterialInstances)
		{
			Instance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		}
	}
}

void AEnemy::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &AEnemy::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

