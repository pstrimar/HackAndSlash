// Fill out your copyright notice in the Description page of Project Settings.


#include "Breakable/BreakableActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Items/Treasure.h"
#include "Items/Magic.h"
#include "Items/Health.h"
#include "Components/CapsuleComponent.h"
#include "Chaos/ChaosGameplayEventDispatcher.h"

ABreakableActor::ABreakableActor()
{
	PrimaryActorTick.bCanEverTick = true;

	GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
	SetRootComponent(GeometryCollection);
	GeometryCollection->SetGenerateOverlapEvents(true);
	GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetupAttachment(GetRootComponent());
	Capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
}

void ABreakableActor::BeginPlay()
{
	Super::BeginPlay();
	
	GeometryCollection->OnChaosBreakEvent.AddDynamic(this, &ABreakableActor::OnBreak);
}

void ABreakableActor::SpawnTreasure()
{
	if (bBroken) return;

	bBroken = true;
	if (UWorld* World = GetWorld())
	{		
		if (TreasureClasses.Num() > 0)
		{
			const FVector RandomVector = FMath::VRand();
			const FVector Offset = FVector(RandomVector.X, RandomVector.Y, 0.f).GetSafeNormal() * 125.f;
			FVector Location = GetActorLocation() + Offset;
			Location.Z += TreasureZOffset;
			const int32 Index = FMath::RandRange(0, TreasureClasses.Num() - 1);
			World->SpawnActor<ATreasure>(TreasureClasses[Index], Location, GetActorRotation());
		}
		if (MagicClass)
		{
			const FVector RandomVector = FMath::VRand();
			const FVector Offset = FVector(RandomVector.X, RandomVector.Y, 0.f).GetSafeNormal() * 125.f;
			FVector Location = GetActorLocation() + Offset;
			Location.Z += TreasureZOffset;
			World->SpawnActor<AMagic>(MagicClass, Location, GetActorRotation());
		}
		if (HealthClass)
		{
			const FVector RandomVector = FMath::VRand();
			const FVector Offset = FVector(RandomVector.X, RandomVector.Y, 0.f).GetSafeNormal() * 125.f;
			FVector Location = GetActorLocation() + Offset;
			Location.Z += TreasureZOffset;
			World->SpawnActor<AHealth>(HealthClass, Location, GetActorRotation());
		}
	}
}

void ABreakableActor::OnBreak(const FChaosBreakEvent& BreakEvent)
{
	SpawnTreasure();
}

void ABreakableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABreakableActor::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	SpawnTreasure();
}