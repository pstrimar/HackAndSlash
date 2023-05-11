// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/SpawnEnemyVolume.h"
#include "Components/BoxComponent.h"
#include "Enemy/Enemy.h"

ASpawnEnemyVolume::ASpawnEnemyVolume()
{
	SpawnTriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Spawn Trigger Box"));
	SpawnTriggerBox->SetupAttachment(GetRootComponent());
	SpawnTriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SpawnTriggerBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	SpawnTriggerBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
}

void ASpawnEnemyVolume::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnTriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ASpawnEnemyVolume::OnBoxOverlap);
}

void ASpawnEnemyVolume::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag(TEXT("EngageableTarget")))
	{
		FActorSpawnParameters SpawnParams;
		UWorld* World = GetWorld();
		if (World && SpawnLocations.Num() > 0 )
		{
			for (int i = 0; i < SpawnLocations.Num(); i++)
			{
				World->SpawnActor<AEnemy>(
					SpawnedEnemyClass,
					SpawnLocations[i]->GetActorLocation(),
					SpawnLocations[i]->GetActorRotation(),
					SpawnParams
					);
			}			
		}

		Destroy();
	}	
}

