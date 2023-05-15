// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Cast.generated.h"

/**
 * 
 */
UCLASS()
class HACKANDSLASH_API UBTTask_Cast : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_Cast();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	class AEnemy* Enemy;
};