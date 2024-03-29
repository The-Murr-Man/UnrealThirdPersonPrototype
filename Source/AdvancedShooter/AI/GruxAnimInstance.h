
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GruxAnimInstance.generated.h"

class AEnemy;

UCLASS()
class ADVANCEDSHOOTER_API UGruxAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UGruxAnimInstance();

	UFUNCTION(BlueprintCallable)
	void UpdateAnimProperties(float deltaTime);

private:
	
	// Latteral move speed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float speed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AEnemy* enemy;
};
