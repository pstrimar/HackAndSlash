// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTTask_Cast.h"
#include "Enemy/Enemy.h"
#include "AIController.h"

UBTTask_Cast::UBTTask_Cast()
{
	NodeName = TEXT("Cast");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_Cast::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
	if (Enemy->CanCast())
	{
		Enemy->Cast();
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_Cast::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!Enemy->IsEngaged())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
