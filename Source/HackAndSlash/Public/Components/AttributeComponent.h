// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HACKANDSLASH_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAttributeComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

private:
	// Current Health
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float Health;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxHealth;

	// Current Stamina
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float Stamina;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxStamina;

	// Current Magic
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float Magic;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxMagic;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 Gold;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 Souls;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 HealthDropped;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 MagicDropped;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 SoulsDropped;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 DodgeCost = 14.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 MagicCost = 20.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 StrongMagicCost = 40.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 SprintCost = 16.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float StaminaRegenRate = 8.f;

public:
	void ReceiveDamage(float Damage);
	void AddHealth(float HealAmount);
	void UseStamina(float StaminaAmount);
	void UseStaminaOverTime(float StaminaAmount, float DeltaTime);
	void RegenStamina(float DeltaTime);
	void UseMagic(float MagicAmount);
	void AddMagic(float MagicAmount);
	float GetHealthPercent();
	float GetStaminaPercent();
	float GetMagicPercent();
	bool IsAlive();
	void AddSouls(int32 NumberOfSouls);
	void AddGold(int32 AmountOfGold);
	FORCEINLINE int32 GetGold() const { return Gold; }
	FORCEINLINE int32 GetSouls() const { return Souls; }
	FORCEINLINE int32 GetSoulsDropped() const { return SoulsDropped; }
	FORCEINLINE float GetDodgeCost() const { return DodgeCost; }
	FORCEINLINE float GetMagicCost() const { return MagicCost; }
	FORCEINLINE float GetStrongMagicCost() const { return StrongMagicCost; }
	FORCEINLINE float GetSprintCost() const { return SprintCost; }
	FORCEINLINE float GetMagic() const { return Magic; }
	FORCEINLINE float GetMagicDropped() const { return MagicDropped; }
	FORCEINLINE float GetHealthDropped() const { return HealthDropped; }
	FORCEINLINE float GetStamina() const { return Stamina; }
};
