// Fill out your copyright notice in the Description page of Project Settings.
#include <Components/BoxComponent.h>
#include <Components/WidgetComponent.h>
#include <Components/SphereComponent.h>
#include <AdvancedShooter/ShooterCharacter.h>
#include "Ammo.h"

AAmmo::AAmmo()
{
	// Contruct the ammo mesh comp and set as root
	ammoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ammo Mesh"));
	SetRootComponent(ammoMesh);

	GetCollisionBox()->SetupAttachment(GetRootComponent());
	GetPickupWidget()->SetupAttachment(GetRootComponent());
	GetAreaSphere()->SetupAttachment(GetRootComponent());

	ammoCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Ammo Collision Sphere"));
	ammoCollisionSphere->SetupAttachment(GetRootComponent());
	ammoCollisionSphere->SetSphereRadius(50.f);
}

void AAmmo::BeginPlay()
{
	Super::BeginPlay();

	ammoCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAmmo::AmmoSphereOverlap);
}

void AAmmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAmmo::SetItemProperties(EItemState state)
{
	Super::SetItemProperties(state);

	switch (state)
	{
		case EItemState::EIS_Pickup:
			// Set Ammo Mesh properties
			ammoMesh->SetSimulatePhysics(false);
			ammoMesh->SetEnableGravity(false);
			ammoMesh->SetVisibility(true);
			ammoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ammoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;

		case EItemState::EIS_Equipped:

			// Set Ammo Mesh properties
			ammoMesh->SetSimulatePhysics(false);
			ammoMesh->SetEnableGravity(false);
			ammoMesh->SetVisibility(true);
			ammoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ammoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;

		case EItemState::EIS_Falling:

			// Set Ammo mesh properties
			ammoMesh->SetSimulatePhysics(true);
			ammoMesh->SetEnableGravity(true);
			ammoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			ammoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ammoMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
			break;

		case EItemState::EIS_EquipInterping:

			// Set Ammo mesh properties
			ammoMesh->SetSimulatePhysics(false);
			ammoMesh->SetEnableGravity(false);
			ammoMesh->SetVisibility(true);
			ammoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ammoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;
	}
}

void AAmmo::AmmoSphereOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp,
	int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{
	if (!otherActor) return;
	AShooterCharacter* shooterCharacter = Cast<AShooterCharacter>(otherActor);
	
	if (shooterCharacter)
	{
		StartItemCurve(shooterCharacter);
		ammoCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AAmmo::SphereCollisionOverlap()
{
}

void AAmmo::EnableCustomDepth()
{
	ammoMesh->SetRenderCustomDepth(true);
}

void AAmmo::DisableCustomDepth()
{
	ammoMesh->SetRenderCustomDepth(false);
}
