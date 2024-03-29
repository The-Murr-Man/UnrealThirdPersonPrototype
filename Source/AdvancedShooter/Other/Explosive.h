// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <AdvancedShooter/BulletHitInterface.h>
#include "Explosive.generated.h"

class UParticleSystem;
class USoundBase;
class USphereComponent;

UCLASS()
class ADVANCEDSHOOTER_API AExplosive : public AActor, public IBulletHitInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExplosive();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void BulletHit_Implementation(FHitResult hitResult, AActor* shooter, AController* instigator) override;

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* explosiveMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Particles", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* explodeParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Sounds", meta = (AllowPrivateAccess = "true"))
	USoundBase* impactSound;

	// Used to determine which actors are overlapped
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	USphereComponent* overlapSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float explosiveDamage = 10;
};
