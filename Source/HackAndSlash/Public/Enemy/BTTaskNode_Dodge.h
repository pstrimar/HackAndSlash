// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskNode_Dodge.generated.h"

/**
 * 
 */
UCLASS()
class HACKANDSLASH_API UBTTaskNode_Dodge : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTaskNode_Dodge();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	class AEnemy* Enemy;
};
