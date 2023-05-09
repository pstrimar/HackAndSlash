// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Enemy/Enemy.h"

AEnemyAIController::AEnemyAIController(const FObjectInitializer& ObjectInitializer)
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));

	Sight = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	Sight->SightRadius = 2000.f;
	Sight->LoseSightRadius = Sight->SightRadius + 500.f;
	Sight->PeripheralVisionAngleDegrees = 90.f;
	Sight->DetectionByAffiliation.bDetectNeutrals = true;

	AIPerceptionComponent->ConfigureSense(*Sight);
	AIPerceptionComponent->SetDominantSense(Sight->GetSenseImplementation());
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnPerception);
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	if (AEnemy* Enemy = Cast<AEnemy>(GetPawn()))
	{
		Agent = Enemy;
	}

	if (AIBehavior)
	{
		RunBehaviorTree(AIBehavior);
		GetBlackboardComponent()->SetValueAsVector(FName("StartLocation"), GetPawn()->GetActorLocation());
	}
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemyAIController::OnPerception(AActor* Actor, FAIStimulus Stimulus)
{
	if (Actor->ActorHasTag(TEXT("Dead")))
	{
		Target = nullptr;
		return;
	}

	if (Actor->ActorHasTag(TEXT("EngageableTarget")))
	{
		SetFocus(Stimulus.WasSuccessfullySensed() ? Actor : nullptr);
		if (Stimulus.WasSuccessfullySensed())
		{
			Target = Actor;
			if (Agent) Agent->SetCombatTarget(Target);
		}
		else
		{
			ClearFocus(EAIFocusPriority::Gameplay);
			GetBlackboardComponent()->ClearValue(TEXT("PlayerLocation"));
			Target = nullptr;
			if (Agent) Agent->SetCombatTarget(Target);
		}
	}
}
