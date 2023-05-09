// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTTask_RunAway.h"
#include "Enemy/Enemy.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UBTTask_RunAway::UBTTask_RunAway()
{
	NodeName = TEXT("Run Away");
}

EBTNodeResult::Type UBTTask_RunAway::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (OwnerComp.GetAIOwner() == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerPawn == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	APawn* EnemyPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (EnemyPawn == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	FVector RunDirection = UKismetMathLibrary::FindLookAtRotation(EnemyPawn->GetActorLocation(), PlayerPawn->GetActorLocation()).Vector() * -500.f;
	return EBTNodeResult::Succeeded;
}
