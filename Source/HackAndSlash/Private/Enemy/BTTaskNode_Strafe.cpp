// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BTTaskNode_Strafe.h"
#include "Enemy/Enemy.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/Navigation/PathFollowingAgentInterface.h"
#include "Navigation/PathFollowingComponent.h"

UBTTaskNode_Strafe::UBTTaskNode_Strafe()
{
	NodeName = TEXT("Strafe");
}

EBTNodeResult::Type UBTTaskNode_Strafe::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (OwnerComp.GetAIOwner() == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	Enemy = Cast<AEnemy>(OwnerComp.GetAIOwner()->GetPawn());
	if (Enemy == nullptr || Enemy->IsEngaged() || Enemy->IsAttacking())
	{
		return EBTNodeResult::Failed;
	}
	int Negation = StrafeRight ? 1 : -1;

	// Trace to left or right, if no hit, continue
	FHitResult TraceHitResult;
	const FVector Start = Enemy->GetActorLocation() + Enemy->GetActorRightVector() * 200.f * Negation;
	FVector End = Start + Enemy->GetActorRightVector() * 200.f * Negation;
	GetWorld()->LineTraceSingleByChannel(
		TraceHitResult,
		Start,
		End,
		ECollisionChannel::ECC_WorldStatic
	);

	if (TraceHitResult.bBlockingHit)
	{
		return EBTNodeResult::Aborted;
	}

	// Trace down, if no hit, do not continue
	End = Start + (FVector::DownVector * 300.f);
	GetWorld()->LineTraceSingleByChannel(
		TraceHitResult,
		Start,
		End,
		ECollisionChannel::ECC_WorldStatic
	);

	if (!TraceHitResult.bBlockingHit)
	{
		return EBTNodeResult::Aborted;
	}

	Enemy->AddMovementInput(Enemy->GetActorRightVector() * Negation);
	Enemy->SetIsStrafing(true);	

	return EBTNodeResult::Succeeded;
}

void UBTTaskNode_Strafe::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (Enemy)
	{
		int Negation = StrafeRight ? 1 : -1;
		Enemy->AddMovementInput(Enemy->GetActorRightVector() * Negation);
	}
}

EBTNodeResult::Type UBTTaskNode_Strafe::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::AbortTask(OwnerComp, NodeMemory);
	if (Enemy)
	{
		Enemy->SetIsStrafing(false);
	}
	return EBTNodeResult::Aborted;
}
