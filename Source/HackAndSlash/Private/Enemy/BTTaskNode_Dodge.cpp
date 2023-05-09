// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTTaskNode_Dodge.h"
#include "Enemy/Enemy.h"
#include "AIController.h"

UBTTaskNode_Dodge::UBTTaskNode_Dodge()
{
	NodeName = TEXT("Dodge");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTaskNode_Dodge::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (OwnerComp.GetAIOwner() == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	Enemy = Cast<AEnemy>(OwnerComp.GetAIOwner()->GetPawn());
	if (Enemy == nullptr)
	{
		return EBTNodeResult::Failed;
	}
	if (!Enemy->IsEngaged())
	{
		Enemy->Dodge();
	}

	return EBTNodeResult::InProgress;
}

void UBTTaskNode_Dodge::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!Enemy->IsEngaged())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
