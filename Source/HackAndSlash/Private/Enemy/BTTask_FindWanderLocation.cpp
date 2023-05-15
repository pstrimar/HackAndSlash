// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTTask_FindWanderLocation.h"
#include "Enemy/Enemy.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

UBTTask_FindWanderLocation::UBTTask_FindWanderLocation()
{
	NodeName = TEXT("Find Wander Location");
}

EBTNodeResult::Type UBTTask_FindWanderLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (OwnerComp.GetAIOwner() == nullptr || OwnerComp.GetAIOwner()->GetPawn() == nullptr)
	{
		return EBTNodeResult::Failed;
	}
	Enemy = Cast<AEnemy>(OwnerComp.GetAIOwner()->GetPawn());
	if (Enemy == nullptr)
	{
		return EBTNodeResult::Failed;
	}
	FNavLocation ResultLocation;

	if (!Enemy->ActorHasTag(FName("Spawning")) && UNavigationSystemV1::GetCurrent(GetWorld())->GetRandomReachablePointInRadius(bWanderFromStartLocation ? OwnerComp.GetBlackboardComponent()->GetValueAsVector(FName("StartLocation")) : Enemy->GetActorLocation(), Radius, ResultLocation))
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), ResultLocation.Location);
	}
	else
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), Enemy->GetActorLocation());
	}
	Enemy->SetIsStrafing(!bWanderFromStartLocation);

	return EBTNodeResult::Succeeded;
}

EBTNodeResult::Type UBTTask_FindWanderLocation::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::AbortTask(OwnerComp, NodeMemory);
	if (Enemy)
	{
		Enemy->SetIsStrafing(false);
	}
	return EBTNodeResult::Aborted;
}
