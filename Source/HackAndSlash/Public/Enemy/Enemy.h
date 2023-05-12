// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CharacterTypes.h"
#include "Interfaces/TargetLockInterface.h"
#include "Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "Enemy.generated.h"

class UHealthBarComponent;
class UWidgetComponent;
class AEnemyAIController;

UCLASS()
class HACKANDSLASH_API AEnemy : public ABaseCharacter, public ITargetLockInterface, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	AEnemy();
	virtual void Tick(float DeltaTime) override;
	virtual void Attack() override;
	virtual void Dodge() override;
	virtual void AttackRootMotion() override;
	virtual bool CanAttackWithWeapon() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	virtual void ShowTargetLock() override;
	virtual void HideTargetLock() override;
	virtual FOnTargetDeath* GetOnTargetDeath() override;
	bool IsOutsideCombatRadius();
	bool IsOutsideAttackRadius();
	bool IsInsideAttackRadius();
	bool IsAttacking();
	bool IsEngaged();
	bool CanCast();
	void Cast();

	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnTargetDeath OnTargetDeath;

protected:	
	virtual void BeginPlay() override;

	virtual void Die_Implementation() override;
	void JumpAttack();
	void SpawnSoulPickup();
	void SpawnHealthPickup();
	void SpawnMagicPickup();
	virtual void AttackEnd() override;
	virtual void DodgeEnd() override;
	virtual void HandleDamage(float DamageAmount) override;
	virtual void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled) override;
	virtual bool IsDead() override;

	UFUNCTION(BlueprintCallable)
	void ShootProjectiles();

	UFUNCTION()
	void ShootProjectile(const FVector& StartLocation, const FRotator& Rotation);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

	UPROPERTY(BlueprintReadOnly)
	bool bIsStrafing;

private:
	/** AI Behavior */
	void InitializeEnemy();
	void HideHealthBar();
	void ShowHealthBar();
	void LoseInterest();
	void StartDodgeTimer();
	void ClearDodgeTimer();
	bool InTargetRange(AActor* Target, double Radius);
	void SpawnDefaultWeapons();
	void PlaySpawnMontage();
	void PlayCastMontage();

	UFUNCTION(BlueprintCallable)
	void OnSpawnEnd();

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* SpawnMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* CastMontage;

	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* TargetLockWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> WeaponClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> ImpactWeaponClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> FrontWeaponClass;
	
	UPROPERTY(VisibleAnywhere, Category = Weapon)
	AWeapon* EquippedWeaponSecondary;	

	UPROPERTY(EditAnywhere, Category = Combat)
	double CombatRadius = 1000;

	UPROPERTY(EditAnywhere, Category = Combat)
	double AttackRadius = 150.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	double AcceptanceRadius = 50.f;

	UPROPERTY(EditAnywhere)
	double PatrolRadius = 200.f;

	UPROPERTY()
	AEnemyAIController* EnemyController;

	FTimerHandle DodgeTimer;

	UPROPERTY(EditAnywhere, Category = Combat)
	float DodgeTime = .75f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float DodgeChance = .5f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float RetaliationChance = .25f;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	float ChasingSpeed = 300.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float PatrolSpeed = 200.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float DeathLifeSpan = 8.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<class ASoul> SoulClass;

	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<class AHealth> HealthClass;

	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<class AMagic> MagicClass;

	UPROPERTY(EditAnywhere, Category = Combat)
	bool TwoWeapons;

	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<class AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<USceneComponent*> ProjectileStartLocations;

	UPROPERTY(EditAnywhere, Category = Combat)
	USceneComponent* ProjectileStartLocation1;

	UPROPERTY(EditAnywhere, Category = Combat)
	USceneComponent* ProjectileStartLocation2;

	UPROPERTY(EditAnywhere, Category = Combat)
	USceneComponent* ProjectileStartLocation3;

	UPROPERTY(EditAnywhere, Category = Combat)
	USceneComponent* ProjectileStartLocation4;

	UPROPERTY(EditAnywhere, Category = Combat)
	USceneComponent* ProjectileStartLocation5;

	TArray<FTimerHandle> ProjectileTimers;
	FTimerDelegate ShootProjectileDelegate;

	/**
	* Dissolve effect
	*/

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	void StartDissolve();

	// Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = Death)
	TArray<UMaterialInstanceDynamic*> DynamicDissolveMaterialInstances;

	// Material instance set on the Blueprint, used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = Death)
	TArray<UMaterialInstance*> DissolveMaterialInstances;

public:
	FORCEINLINE double GetAttackRadius() const { return AttackRadius; }
	FORCEINLINE void SetCombatTarget(AActor* Target) { CombatTarget = Target; }
	FORCEINLINE void SetIsStrafing(bool IsStrafing) { bIsStrafing = IsStrafing; }
};
