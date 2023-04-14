// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Treasure.h"
#include "Characters/HackAndSlashCharacter.h"
#include "Kismet/GameplayStatics.h"

void ATreasure::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AHackAndSlashCharacter* HackAndSlashCharacter = Cast<AHackAndSlashCharacter>(OtherActor);
	if (HackAndSlashCharacter)
	{
		if (PickupSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
		}
		Destroy();
	}
}
