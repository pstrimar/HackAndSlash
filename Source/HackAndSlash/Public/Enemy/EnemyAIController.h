// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class HACKANDSLASH_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AEnemyAIController(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite)
	class AEnemy* Agent;

	UFUNCTION()
	void OnPerception(AActor* Actor, FAIStimulus Stimulus);

	class UAISenseConfig_Sight* Sight;

	virtual void OnPossess(APawn* InPawn) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	class UAIPerceptionComponent* AIPerceptionComponent;

private:
	UPROPERTY(EditAnywhere)
	class UBehaviorTree* AIBehavior;

	AActor* Target;

public:
	FORCEINLINE AActor* GetTarget() const { return Target; }
};
