// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTTask_PowerAttack.h"
#include "Enemy/Enemy.h"
#include "AIController.h"

UBTTask_PowerAttack::UBTTask_PowerAttack()
{
	NodeName = TEXT("Power Attack");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_PowerAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
	if (Enemy->CanAttackWithWeapon())
	{
		Enemy->AttackRootMotion();
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_PowerAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!Enemy->IsEngaged())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}