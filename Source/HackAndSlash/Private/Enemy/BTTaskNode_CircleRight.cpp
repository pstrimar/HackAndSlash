// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTTaskNode_CircleRight.h"
#include "Enemy/Enemy.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTaskNode_CircleRight::UBTTaskNode_CircleRight()
{
	NodeName = TEXT("Circle Right");
}

EBTNodeResult::Type UBTTaskNode_CircleRight::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
	Enemy->AddMovementInput(Enemy->GetActorRightVector());
	Enemy->SetIsStrafing(true);

	return EBTNodeResult::Succeeded;
}

void UBTTaskNode_CircleRight::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (Enemy)
	{
		Enemy->AddMovementInput(Enemy->GetActorRightVector());
	}
}

EBTNodeResult::Type UBTTaskNode_CircleRight::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::AbortTask(OwnerComp, NodeMemory);
	UE_LOG(LogTemp, Warning, TEXT("Aborted"));
	if (Enemy)
	{
		Enemy->SetIsStrafing(false);
	}
	return EBTNodeResult::Aborted;
}
