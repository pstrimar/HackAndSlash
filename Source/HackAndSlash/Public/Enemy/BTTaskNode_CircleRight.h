// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskNode_CircleRight.generated.h"

/**
 * 
 */
UCLASS()
class HACKANDSLASH_API UBTTaskNode_CircleRight : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTaskNode_CircleRight();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	class AEnemy* Enemy;
};
