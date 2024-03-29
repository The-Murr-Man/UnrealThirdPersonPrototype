// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include <AdvancedShooter/Items/Weapon.h>
#include "Ammo.generated.h"

class USphereComponent;
UCLASS()
class ADVANCEDSHOOTER_API AAmmo : public AItem
{
	GENERATED_BODY()
	
public:
	AAmmo();

	FORCEINLINE UStaticMeshComponent* GetAmmoMesh() const { return ammoMesh; }

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FORCEINLINE EAmmoType GetAmmoType() const { return ammoType; }

	virtual void EnableCustomDepth() override;
	virtual void DisableCustomDepth() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Overide of set item properties to set ammo mesh properties
	virtual void SetItemProperties(EItemState state) override;

	// Called when overlapping area sphere
	UFUNCTION()
		void AmmoSphereOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp,
			int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult);

	void SphereCollisionOverlap();

private:

	// Mesh for the ammo pickup
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ammoMesh;

	// Ammo type for the ammo pickup
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	EAmmoType ammoType;

	// Ammo icon the ammo pickup
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	UTexture2D* ammoIconTexture;

	// Overlap sphere for picking up ammo
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	USphereComponent* ammoCollisionSphere;
};
