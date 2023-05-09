// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTTask_EnemyMoveTo.h"
#include "Enemy/Enemy.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_EnemyMoveTo::UBTTask_EnemyMoveTo()
{
	NodeName = TEXT("Enemy Move To");
}

EBTNodeResult::Type UBTTask_EnemyMoveTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	if (OwnerComp.GetAIOwner() == nullptr)
	{
		return EBTNodeResult::Failed;
	}
	AEnemy* Enemy = Cast<AEnemy>(OwnerComp.GetAIOwner()->GetPawn());
	if (Enemy == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	AcceptableRadius = Enemy->GetAttackRadius();

	return EBTNodeResult::InProgress;
}
