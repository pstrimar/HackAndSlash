// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "InputActionValue.h"
#include "CharacterTypes.h"
#include "Interfaces/PickupInterface.h"
#include "Perception/AISightTargetInterface.h"
#include "HUD/HackAndSlashHUD.h"
#include "HackAndSlashCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class AItem;
class UAnimMontage;
class UHackAndSlashOverlay;
class ASoul;
class ATreasure;
class AHealth;
class AMagic;

UCLASS()
class HACKANDSLASH_API AHackAndSlashCharacter : public ABaseCharacter, public IPickupInterface, public IAISightTargetInterface
{
	GENERATED_BODY()

public:
	AHackAndSlashCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void Attack() override;
	virtual void AttackRootMotion() override;
	virtual bool CanAttackWithWeapon() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	virtual void SetOverlappingItem(AItem* Item) override;
	virtual void AddSouls(ASoul* Soul) override;
	virtual void AddGold(ATreasure* Treasure) override;
	virtual void AddHealth(AHealth* Health) override;
	virtual void AddMagic(AMagic* Magic) override;
	virtual void Jump() override;
	virtual void Landed(const FHitResult& Hit) override;

protected:
	virtual void BeginPlay() override;

	// Custom View Target for AI Perception Component
	virtual bool CanBeSeenFrom(const FVector& ObserverLocation, FVector& OutSeenLocation, int32& NumberOfLoSChecksPerformed, float& OutSightStrength, const AActor* IgnoreActor = nullptr, const bool* bWasVisible = nullptr, int32* UserData = nullptr) const override;

	/** Callbacks for input */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void ControllerLook(const FInputActionValue& Value);
	void EKeyPressed();
	void DoMagicAttack(int32 ComboCount);
	void DoStrongMagicAttack();
	virtual void Dodge() override;
	void SprintStart();
	void SprintEnd();
	void TargetLock();
	void AimButtonPressed();
	void AimButtonReleased();
	void DropWeapon();
	void SetMovementToStrafing();
	void SetMovementToDefault();

	/** Combat */
	void EquipWeapon(AWeapon* Weapon);
	virtual void AttackEnd() override;
	virtual void DodgeEnd() override;
	virtual void SpawnEnd() override;
	bool CanUseMagic();
	bool CanDisarm();
	void Disarm();
	bool CanArm();
	void Arm();
	bool CanSwap();
	void Swap();
	bool CanMove();
	void PlayMagicAttackMontage(int32 ComboCount);
	void PlayStrongMagicAttackMontage();
	void PlayEquipMontage(const FName& SectionName);
	void BoxTrace(TArray<FHitResult>& BoxHits);
	void TraceForCombatTarget(bool ShouldTargetLock);
	virtual void Die_Implementation() override;
	bool HasEnoughStamina();
	bool IsOccupied();
	virtual bool IsDead() override;
	void FollowTarget(float DeltaTime);
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);
	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void ShootProjectile(const FName& SocketName);

	void ShootProjectile(const FVector& Target, const FName& SocketName);

	UFUNCTION(BlueprintCallable)
	void ShootStrongProjectile();

	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<class AProjectile> EnergyProjectileClass;

	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<class AProjectile> StrongProjectileClass;

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToBack();

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToHand();

	UFUNCTION(BlueprintCallable)
	void SwapAttachedWeapons();

	UFUNCTION(BlueprintCallable)
	void HitReactEnd();	

	UFUNCTION(BlueprintCallable)
	void ComboAttackSave(bool PowerAttack);

	UFUNCTION(BlueprintCallable)
	void MagicComboAttackSave(bool PowerAttack);

	UFUNCTION(BlueprintCallable)
	void ResetCombo();

	UFUNCTION(BlueprintCallable)
	void SlowTime(float TimeDilation);	

	UPROPERTY(EditAnywhere, Category = "Combat")
	FVector BoxTraceExtend = FVector(100.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
	FName TargetBone = "Head";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* HackAndSlashMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MovementAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ControllerLookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* EKeyPressedAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* DodgeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* TargetLockAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* DropWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* PowerAttackAction;

private:
	bool IsAttacking();
	bool IsUnoccupied();
	bool IsSprinting();
	void InitializeOverlay();
	void SetHUDHealth();
	void HideCharacterIfCameraClose();
	void PlayNoMagicAudio();
	void PlayDeathAudio();
	void ResetDoubleJump();
	void ResetTargetLock();

	UFUNCTION()
	void OnTargetDeath();

	/** Character Components*/
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleAnywhere)
	UAudioComponent* AudioComponent;

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	UPROPERTY(VisibleAnywhere, Category = Weapon)
	AWeapon* StoredWeapon;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* EquipMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* MagicAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* StrongMagicAttackMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FName> MagicAttackMontageSections;

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Spawning;

	UPROPERTY()
	UHackAndSlashOverlay* Overlay;

	UPROPERTY(EditAnywhere, Category = Combat)
	USoundBase* NoMagicAudio;

	UPROPERTY(EditAnywhere, Category = Combat)
	USoundBase* DeathAudio;

	bool SaveAttack;
	int32 AttackCount;
	float DefaultSpeed = 600.f;
	bool bAiming;
	int32 JumpCount;
	FTimerHandle DoubleJumpResetTimer;
	float DoubleJumpResetTime = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float SprintSpeed = 1000.f;

	UPROPERTY(EditAnywhere, Category = Movement)
	float AimWalkSpeed = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float DefaultFOV = 90.f;

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float SprintFOV = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float AimingFOV = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float LookSensitivityYaw = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float LookSensitivityPitch = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float AimLookSensitivityYaw = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float AimLookSensitivityPitch = .5f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float TargetTraceLength = 80000.f;

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	float TargetFOV;
	bool TargetLocked;
	FVector2D MovementVector;
	FVector HitTarget;
	FHUDPackage HUDPackage;
	bool FinishedEquipping;

	/**
	 * HUD and crosshairs
	*/

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

public:
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }
	FORCEINLINE bool GetTargetLocked() const { return TargetLocked; }
	FORCEINLINE int32 GetJumpCount() const { return JumpCount; }
};
