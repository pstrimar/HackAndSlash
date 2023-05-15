// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTService_PlayerLocationIfSeen.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy/EnemyAIController.h"

UBTService_PlayerLocationIfSeen::UBTService_PlayerLocationIfSeen()
{
	NodeName = TEXT("Update Player Location If Seen");
	ForceInstancing(true);
}

void UBTService_PlayerLocationIfSeen::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	if (OwnerComp.GetAIOwner() == nullptr)
	{
		return;
	}

	EnemyAIController = EnemyAIController == nullptr ? Cast<AEnemyAIController>(OwnerComp.GetAIOwner()) : EnemyAIController;
	if (EnemyAIController == nullptr)
	{
		return;
	}

	if (!EnemyAIController->GetPawn()->ActorHasTag(FName("Spawning")) && EnemyAIController->GetTarget() != nullptr && !EnemyAIController->GetTarget()->ActorHasTag(FName("Dead")))
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), EnemyAIController->GetTarget()->GetActorLocation());
	}
	else
	{
		OwnerComp.GetBlackboardComponent()->ClearValue(GetSelectedBlackboardKey());
	}
}
