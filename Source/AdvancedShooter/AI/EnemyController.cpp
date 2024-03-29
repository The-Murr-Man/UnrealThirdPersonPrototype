#include "EnemyController.h"
#include <BehaviorTree/BlackboardComponent.h>
#include <BehaviorTree/BehaviorTreeComponent.h>
#include <BehaviorTree/BehaviorTree.h>
#include <AdvancedShooter/AI/Enemy.h>

AEnemyController::AEnemyController()
{
	blackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("Blackboard Component"));
	if (!blackboardComponent) return;

	behaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("Behavior Tree Component"));
	if (!behaviorTreeComponent) return;
}

void AEnemyController::OnPossess(APawn* inPawn)
{
	Super::OnPossess(inPawn);

	if (!inPawn) return;

	AEnemy* enemy = Cast<AEnemy>(inPawn);

	if (!enemy) return;

	if (enemy->GetBehaviorTree())
	{
		blackboardComponent->InitializeBlackboard(*enemy->GetBehaviorTree()->BlackboardAsset);
	}
}
