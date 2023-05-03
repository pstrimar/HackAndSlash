// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Weapons/Projectile.h"
#include "ProjectileEnergy.generated.h"

/**
 * 
 */
UCLASS()
class HACKANDSLASH_API AProjectileEnergy : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileEnergy();

protected:
	virtual void BeginPlay() override;
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	void ExecuteGetHit(const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	UParticleSystemComponent* ProjectileParticle;

	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;

	UPROPERTY(EditAnywhere)
	USoundBase* ProjectileLoop;

	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;

private:
};
