// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/ChangeLevelVolume.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"

AChangeLevelVolume::AChangeLevelVolume()
{
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Spawn Trigger Box"));
	TriggerBox->SetupAttachment(GetRootComponent());
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	TriggerBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
}

void AChangeLevelVolume::BeginPlay()
{
	Super::BeginPlay();
	
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AChangeLevelVolume::OnBoxOverlap);
}

void AChangeLevelVolume::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->ActorHasTag(TEXT("EngageableTarget")))
	{
		UGameplayStatics::OpenLevel(this, Level, false);
	}
}

