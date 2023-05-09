// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTTaskNode_SetWalkSpeed.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTaskNode_SetWalkSpeed::UBTTaskNode_SetWalkSpeed()
{
	NodeName = TEXT("Set Walk Speed");
}

EBTNodeResult::Type UBTTaskNode_SetWalkSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (OwnerComp.GetAIOwner() && OwnerComp.GetAIOwner()->GetPawn() && OwnerComp.GetAIOwner()->GetPawn()->GetMovementComponent())
	{
		if (UCharacterMovementComponent* Movement = Cast<UCharacterMovementComponent>(OwnerComp.GetAIOwner()->GetPawn()->GetMovementComponent()))
		{
			Movement->MaxWalkSpeed = WalkSpeed;
			return EBTNodeResult::Succeeded;
		}
	}

	return EBTNodeResult::Failed;
}
