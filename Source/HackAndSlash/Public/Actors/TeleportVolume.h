// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TeleportVolume.generated.h"

class UBoxComponent;

UCLASS()
class HACKANDSLASH_API ATeleportVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	ATeleportVolume();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* TeleportTriggerBox;

	UPROPERTY(EditInstanceOnly)
	AActor* TeleportLocation;

};
