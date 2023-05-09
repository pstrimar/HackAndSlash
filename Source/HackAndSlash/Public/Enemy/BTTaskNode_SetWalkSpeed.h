// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTaskNode_SetWalkSpeed.generated.h"

/**
 * 
 */
UCLASS()
class HACKANDSLASH_API UBTTaskNode_SetWalkSpeed : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTaskNode_SetWalkSpeed();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	UPROPERTY(EditAnywhere)
	float WalkSpeed;
};
