// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "Characters/CharacterTypes.h"
#include "BaseCharacter.generated.h"

class AWeapon;
class UAttributeComponent;
class UAnimMontage;

UCLASS()
class HACKANDSLASH_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void Attack();
	virtual void Dodge();
	virtual void AttackRootMotion();
	virtual bool CanAttackWithWeapon();

protected:
	virtual void BeginPlay() override;
	/** Combat */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;

	UFUNCTION(BlueprintNativeEvent)
	void Die();

	void DirectionalHitReact(const FVector& ImpactPoint);
	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);
	virtual void HandleDamage(float DamageAmount);
	void DisableCapsule();
	bool IsAlive();
	virtual bool IsDead();
	void DisableMeshCollision();

	/** Montage */
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	virtual int32 PlayRandomAttackMontage();
	virtual int32 PlayRandomRootMotionAttackMontage();
	virtual void PlayAttackMontage(int32 ComboCount);
	virtual void PlayRootMotionAttackMontage(int32 ComboCount);
	virtual void PlayAirAttackMontage();
	virtual int32 PlayDeathMontage();
	virtual void PlayDodgeMontage(const FName& SectionName);
	void PlayHitReactMontage(const FName& SectionName);
	void StopAttackMontage();

	UFUNCTION(BlueprintCallable)
	FVector GetTranslationWarpTarget();

	UFUNCTION(BlueprintCallable)
	FVector GetRotationWarpTarget();

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();

	UFUNCTION(BlueprintCallable)
	virtual void DodgeEnd();

	UFUNCTION(BlueprintCallable)
	virtual void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

	UFUNCTION(BlueprintCallable)
	virtual void SetGroundImpactCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

	UFUNCTION(BlueprintCallable)
	virtual void SetFrontWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

	UPROPERTY(VisibleAnywhere, Category = Weapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, Category = Weapon)
	AWeapon* GroundImpactWeapon;

	UPROPERTY(VisibleAnywhere, Category = Weapon)
	AWeapon* FrontWeapon;

	UPROPERTY(VisibleAnywhere)
	UAttributeComponent* Attributes;

	UPROPERTY(BlueprintReadOnly, Category = Combat)
	AActor* CombatTarget;

	UPROPERTY(EditAnywhere, Category = Combat)
	double WarpTargetDistance = 75.f;

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EDeathPose> DeathPose;

private:
	int32 PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);

	UPROPERTY(EditAnywhere, Category = Combat)
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = Combat)
	USoundBase* PainSound;

	UPROPERTY(EditAnywhere, Category = Combat)
	UParticleSystem* HitParticles;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* OneHandedAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* OneHandedRootMotionAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* TwoHandedAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* TwoHandedRootMotionAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* DodgeMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FName> OneHandedAttackMontageSections;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FName> OneHandedRootMotionAttackMontageSections;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FName> TwoHandedAttackMontageSections;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FName> TwoHandedRootMotionAttackMontageSections;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FName> DeathMontageSections;

	UPROPERTY(EditAnywhere, Category = Combat)
	float MaxWarpTranslation = 500.f;

public:
	FORCEINLINE TEnumAsByte<EDeathPose> GetDeathPose() const { return DeathPose; }
};
