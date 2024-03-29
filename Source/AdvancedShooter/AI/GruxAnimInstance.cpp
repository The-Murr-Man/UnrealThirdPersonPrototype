
#include "GruxAnimInstance.h"
#include <AdvancedShooter/AI/Enemy.h>

UGruxAnimInstance::UGruxAnimInstance()
{

}

void UGruxAnimInstance::UpdateAnimProperties(float deltaTime)
{
	if (!enemy) 
		enemy = Cast<AEnemy>(TryGetPawnOwner());
	if (enemy)
	{
		FVector velocity = enemy->GetVelocity();
		velocity.Z = 0;

		speed = velocity.Size();
	}

}
