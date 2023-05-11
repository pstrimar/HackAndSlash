// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Items/Weapons/Weapon.h"
#include "Components/AttributeComponent.h"
#include "Kismet/GameplayStatics.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (IsAlive() && Hitter)
	{
		DirectionalHitReact(Hitter->GetActorLocation());
	}
	else Die();

	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);
}

void ABaseCharacter::Attack()
{
	
}

void ABaseCharacter::AttackRootMotion()
{
}

void ABaseCharacter::Die_Implementation()
{
	//CharacterDeath.Broadcast();
	Tags.Add(FName("Dead"));
	PlayDeathMontage();
}

int32 ABaseCharacter::PlayRandomAttackMontage()
{
	if (EquippedWeapon)
	{
		return EquippedWeapon->IsTwoHanded ? PlayRandomMontageSection(TwoHandedAttackMontage, TwoHandedAttackMontageSections) : PlayRandomMontageSection(OneHandedAttackMontage, OneHandedAttackMontageSections);
	}
	return -1;
}

int32 ABaseCharacter::PlayRandomRootMotionAttackMontage()
{
	if (EquippedWeapon)
	{
		return EquippedWeapon->IsTwoHanded ? PlayRandomMontageSection(TwoHandedRootMotionAttackMontage, TwoHandedRootMotionAttackMontageSections) : PlayRandomMontageSection(OneHandedRootMotionAttackMontage, OneHandedRootMotionAttackMontageSections);
	}
	return -1;
}

void ABaseCharacter::PlayAttackMontage(int32 ComboCount)
{
	if (EquippedWeapon->IsTwoHanded && TwoHandedAttackMontageSections.Num() - 1 >= ComboCount)
	{
		PlayMontageSection(TwoHandedAttackMontage, TwoHandedAttackMontageSections[ComboCount]);
	}
	else if (!EquippedWeapon->IsTwoHanded && OneHandedAttackMontageSections.Num() - 1 >= ComboCount)
	{
		PlayMontageSection(OneHandedAttackMontage, OneHandedAttackMontageSections[ComboCount]);
	}
}

void ABaseCharacter::PlayRootMotionAttackMontage(int32 ComboCount)
{
	if (EquippedWeapon->IsTwoHanded && TwoHandedRootMotionAttackMontageSections.Num() - 1 >= ComboCount)
	{
		PlayMontageSection(TwoHandedRootMotionAttackMontage, TwoHandedRootMotionAttackMontageSections[ComboCount]);
	}
	else if (!EquippedWeapon->IsTwoHanded && OneHandedRootMotionAttackMontageSections.Num() - 1 >= ComboCount)
	{
		PlayMontageSection(OneHandedRootMotionAttackMontage, OneHandedRootMotionAttackMontageSections[ComboCount]);
	}
}

void ABaseCharacter::PlayAirAttackMontage()
{
	if (EquippedWeapon->IsTwoHanded)
	{
		PlayMontageSection(TwoHandedRootMotionAttackMontage, FName("AirAttack"));
	}
	else if (!EquippedWeapon->IsTwoHanded)
	{
		PlayMontageSection(OneHandedRootMotionAttackMontage, FName("AirAttack"));
	}
}

int32 ABaseCharacter::PlayDeathMontage()
{
	const int32 Selection = PlayRandomMontageSection(DeathMontage, DeathMontageSections);
	TEnumAsByte<EDeathPose> Pose(Selection);
	if (Pose < EDeathPose::EDP_MAX)
	{
		DeathPose = Pose;
	}
	return Selection;
}

void ABaseCharacter::PlayDodgeMontage(const FName& SectionName)
{
	PlayMontageSection(DodgeMontage, SectionName);
}

void ABaseCharacter::DisableCapsule()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstace = GetMesh()->GetAnimInstance();
	if (AnimInstace && HitReactMontage)
	{
		AnimInstace->Montage_Play(HitReactMontage);
		AnimInstace->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

void ABaseCharacter::StopAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquippedWeapon)
	{
		AnimInstance->Montage_Stop(0.25f, EquippedWeapon->IsTwoHanded ? TwoHandedAttackMontage : OneHandedAttackMontage);
	}
}

FVector ABaseCharacter::GetTranslationWarpTarget()
{
	if (CombatTarget == nullptr) return FVector();
	
	const FVector CombatTargetLocation = CombatTarget->GetActorLocation();
	const FVector Location = GetActorLocation();

	if ((Location - CombatTargetLocation).Size() - WarpTargetDistance > MaxWarpTranslation)
	{
		return Location;
	}
	FVector TargetToMe = (Location - CombatTargetLocation).GetSafeNormal();
	TargetToMe *= WarpTargetDistance;
	return CombatTargetLocation + TargetToMe;
}

FVector ABaseCharacter::GetRotationWarpTarget()
{
	if (CombatTarget)
	{
		return CombatTarget->GetActorLocation();
	}
	return FVector();
}

void ABaseCharacter::Dodge()
{
}

void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
{
	const FVector Forward = GetActorForwardVector();
	const FVector ImpactLowered(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	const FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();

	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	double Theta = FMath::Acos(CosTheta);
	Theta = FMath::RadiansToDegrees(Theta);

	// if CrossProduct points down, Theta should be negative
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if (CrossProduct.Z < 0)
	{
		Theta *= -1.f;
	}
	FName Section("FromBack");

	if (Theta >= -45.f && Theta < 45.f)
	{
		Section = FName("FromFront");
	}
	else if (Theta >= -135.f && Theta < -45.f)
	{
		Section = FName("FromLeft");
	}
	else if (Theta >= 45.f && Theta < 135.f)
	{
		Section = FName("FromRight");
	}

	PlayHitReactMontage(Section);
}

void ABaseCharacter::PlayHitSound(const FVector& ImpactPoint)
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);
	}
	if (PainSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PainSound, GetActorLocation());
	}
}

void ABaseCharacter::SpawnHitParticles(const FVector& ImpactPoint)
{
	if (HitParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, ImpactPoint);
	}
}

void ABaseCharacter::HandleDamage(float DamageAmount)
{
	if (Attributes)
	{
		Attributes->ReceiveDamage(DamageAmount);
	}
}

void ABaseCharacter::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName)
{
	UAnimInstance* AnimInstace = GetMesh()->GetAnimInstance();
	if (AnimInstace && Montage)
	{
		AnimInstace->Montage_Play(Montage);
		AnimInstace->Montage_JumpToSection(SectionName, Montage);
	}
}

int32 ABaseCharacter::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames)
{
	if (SectionNames.Num() <= 0) return -1;
	const int32 MaxSectionIndex = SectionNames.Num() - 1;
	const int32 Selection = FMath::RandRange(0, MaxSectionIndex);
	PlayMontageSection(Montage, SectionNames[Selection]);
	return Selection;
}

bool ABaseCharacter::CanAttackWithWeapon()
{
	return false;
}

bool ABaseCharacter::IsAlive()
{
	return Attributes && Attributes->IsAlive();
}

bool ABaseCharacter::IsDead()
{
	return false;
}

void ABaseCharacter::DisableMeshCollision()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::AttackEnd()
{
}

void ABaseCharacter::DodgeEnd()
{
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeapon->IgnoreActors.Empty();
	}
}

void ABaseCharacter::SetGroundImpactCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (GroundImpactWeapon && GroundImpactWeapon->GetWeaponBox())
	{
		GroundImpactWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		GroundImpactWeapon->IgnoreActors.Empty();
	}
}

void ABaseCharacter::SetFrontWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (FrontWeapon && FrontWeapon->GetWeaponBox())
	{
		FrontWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		FrontWeapon->IgnoreActors.Empty();
	}
}

