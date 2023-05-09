// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTTask_FocusOnTarget.h"
#include "Enemy/Enemy.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

UBTTask_FocusOnTarget::UBTTask_FocusOnTarget()
{
	NodeName = TEXT("Focus On Target");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_FocusOnTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	if (OwnerComp.GetAIOwner() == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerPawn == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	OwnerComp.GetAIOwner()->SetFocus(PlayerPawn);

	OwnerComp.GetAIOwner()->GetPawn()->bUseControllerRotationYaw = true;

	return EBTNodeResult::InProgress;
}

void UBTTask_FocusOnTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (OwnerComp.GetAIOwner() && PlayerPawn)
	{
		OwnerComp.GetAIOwner()->SetFocus(PlayerPawn);
	}
}

EBTNodeResult::Type UBTTask_FocusOnTarget::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::AbortTask(OwnerComp, NodeMemory);
	if (OwnerComp.GetAIOwner())
	{
		OwnerComp.GetAIOwner()->ClearFocus(EAIFocusPriority::Gameplay);

		OwnerComp.GetAIOwner()->GetPawn()->bUseControllerRotationYaw = false;
	}
	return EBTNodeResult::Aborted;
}
