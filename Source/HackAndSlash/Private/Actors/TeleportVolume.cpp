// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/TeleportVolume.h"
#include "Components/BoxComponent.h"
#include "Characters/HackAndSlashCharacter.h"

ATeleportVolume::ATeleportVolume()
{
	TeleportTriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Teleport Trigger Box"));
	TeleportTriggerBox->SetupAttachment(GetRootComponent());
	TeleportTriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TeleportTriggerBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	TeleportTriggerBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

}

void ATeleportVolume::BeginPlay()
{
	Super::BeginPlay();

	TeleportTriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ATeleportVolume::OnBoxOverlap);
}

void ATeleportVolume::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AHackAndSlashCharacter* Character = Cast<AHackAndSlashCharacter>(OtherActor))
	{
		if (TeleportLocation)
		{
			ETeleportType Teleport = ETeleportType::ResetPhysics;
			const FVector Location = TeleportLocation->GetActorLocation();
			const FRotator Rotation = TeleportLocation->GetActorRotation();
			Character->SetActorLocationAndRotation(Location, Rotation, false, nullptr, Teleport);
			Character->PlaySpawnMontage();
		}
	}
}

