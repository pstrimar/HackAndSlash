// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnEnemyVolume.generated.h"

class UBoxComponent;
class AEnemy;

UCLASS()
class HACKANDSLASH_API ASpawnEnemyVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	ASpawnEnemyVolume();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* SpawnTriggerBox;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AEnemy> SpawnedEnemyClass;

	UPROPERTY(EditInstanceOnly)
	AActor* SpawnLocation;
};
