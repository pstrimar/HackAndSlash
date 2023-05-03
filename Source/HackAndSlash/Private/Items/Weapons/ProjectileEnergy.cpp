// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapons/ProjectileEnergy.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Interfaces/HitInterface.h"
#include "NiagaraComponent.h"

AProjectileEnergy::AProjectileEnergy()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ProjectileParticle"));
	ProjectileParticle->SetupAttachment(GetRootComponent());
}

void AProjectileEnergy::BeginPlay()
{
	Super::BeginPlay();

	SpawnTrailSystem();
	StartDestroyTimer();

	if (ProjectileLoop && LoopingSoundAttenuation)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.f,
			1.f,
			0.f,
			LoopingSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false
		);
	}
}

void AProjectileEnergy::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner())
	{
		return;
	}

	if (Hit.GetActor())
	{
		UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
		ExecuteGetHit(Hit);
		CreateFields(Hit.ImpactPoint);
	}

	// Determine impact type
	if (OtherActor->ActorHasTag(FName("Enemy")))
	{
		ImpactParticles = ImpactParticlesHitCharacter;
	}
	else
	{
		ImpactParticles = ImpactParticlesHitWorld;
	}

	if (ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}
	if (ProjectileParticle)
	{
		ProjectileParticle->Deactivate();
	}
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstance())
	{
		TrailSystemComponent->GetSystemInstance()->Deactivate();
	}
	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileEnergy::ExecuteGetHit(const FHitResult& Hit)
{
	IHitInterface* HitInterface = Cast<IHitInterface>(Hit.GetActor());
	if (HitInterface)
	{
		HitInterface->Execute_GetHit(Hit.GetActor(), Hit.ImpactPoint, GetOwner());
	}
}
