#include "Explosive.h"
#include <Kismet/GameplayStatics.h>
#include <Sound/SoundBase.h>
#include <Particles/ParticleSystemComponent.h>
#include <Components/SphereComponent.h>
#include <GameFramework/Character.h>
// Sets default values
AExplosive::AExplosive()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	explosiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Explosive Mesh"));
	SetRootComponent(explosiveMesh);

	overlapSphere = CreateAbstractDefaultSubobject<USphereComponent>(TEXT("Overlap Sphere"));
	overlapSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AExplosive::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AExplosive::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AExplosive::BulletHit_Implementation(FHitResult hitResult, AActor* shooter, AController* instigator)
{
	if (!impactSound && !explodeParticles) return;
	
	UGameplayStatics::PlaySoundAtLocation(this, impactSound, GetActorLocation());
	
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), explodeParticles, hitResult.ImpactPoint, FRotator::ZeroRotator, true);

	TArray<AActor*> overlappingActors;
	GetOverlappingActors(overlappingActors, ACharacter::StaticClass());

	for (AActor* actor : overlappingActors)
	{
		UGameplayStatics::ApplyDamage(actor, explosiveDamage, instigator, shooter, UDamageType::StaticClass());
	}
	Destroy();
}



