// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include <Engine/DataTable.h>
#include "Weapon.generated.h"

class UParticleSystem;

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_SubmachineGun UMETA(DisplayName = "SubmachineGun"),
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	
	EWT_MAX UMETA(DisplayName = "Default Max"),
};

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	EAT_9mm UMETA(DisplayName = "9mm"),
	EAT_AR UMETA(DisplayName = "Assault Rifle"),

	EAT_MAX UMETA(DisplayName = "Default Max"),
};

USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString itemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName clipBoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName boneToHide;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName reloadMontageSection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	EAmmoType ammoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 weaponAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 magazineCap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	float fireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float recoil;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAutomatic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float headShotDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 materialIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundBase* shootSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundBase* pickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundBase* equipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* itemMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* inventoryIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* ammoIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
	UTexture2D* crosshairsMiddle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
	UTexture2D* crosshairsLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
	UTexture2D* crosshairsRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
	UTexture2D* crosshairsBottom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
	UTexture2D* crosshairsTop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* materialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> animBP;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	UParticleSystem* muzzleFlash;
};

UCLASS()
class ADVANCEDSHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()
	
public:
	AWeapon();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Adds impulse to weapon
	void ThrowWeapon();

	//Called from character class when firing weapon
	void DecrementAmmo();

	FORCEINLINE int32 GetAmmo() const { return ammo; }

	FORCEINLINE int32 GetMagazineCap() const { return magazineCap; }

	FORCEINLINE EWeaponType GetWeaponType() const { return weaponType; }

	FORCEINLINE EAmmoType GetAmmoType() const { return ammoType; }

	FORCEINLINE FName GetReloadMontageSection() const { return reloadMontageSection; }

	FORCEINLINE FName GetClipBoneName() const { return clipBoneName; }
	FORCEINLINE float GetFireRate() const { return fireRate; }
	FORCEINLINE float GetWeaponRecoil() const { return weaponRecoil; }
	FORCEINLINE USoundBase* GetShootSound() const { return shootSound; }
	FORCEINLINE UParticleSystem* GetMuzzleFlash() const { return muzzleFlash; }

	FORCEINLINE bool GetIsAutomatic() const { return bIsAutomatic; }

	FORCEINLINE float GetDamage() { return damage; }
	FORCEINLINE float GetHeadShotDamage() { return headShotDamage; }

	FORCEINLINE void SetIsMovingClip(bool move) { bIsMovingClip = move; }

	void SetClipBoneName(FName bone) { clipBoneName = bone; }
	void SetReloadMontageSection(FName section) { reloadMontageSection = section; }

	void ReloadAmmo(int32 amount);

	bool ClipIsFull() { return ammo >= magazineCap; };

	void StartSlideTimer();

protected:
	void StopFalling();

	virtual void OnConstruction(const FTransform& transform) override;
	virtual void BeginPlay() override;
	UDataTable* GetWeaponDataTable();
	void SetWeaponData();
	void FinishMovingSlide();

	void UpdateSlideDisplacement();
	void CalculateDamage();

private:
	FTimerHandle throwWeaponTimer;
	float throwWeaponTime = 0.7f;
	bool bIsFalling = false;

	// AMMO STUFF

	// Ammo count for this weapon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties|Ammo", meta = (AllowPrivateAccess = "true"))
	int32 ammo = 0;

	// Magazine capacity for this weapon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties|Ammo", meta = (AllowPrivateAccess = "true"))
	int32 magazineCap = 30;

	// Type of weapon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties", meta = (AllowPrivateAccess = "true"))
	EWeaponType weaponType = EWeaponType::EWT_SubmachineGun;

	// Type of ammo
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties|Ammo", meta = (AllowPrivateAccess = "true"))
	EAmmoType ammoType = EAmmoType::EAT_9mm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties|Ammo", meta = (AllowPrivateAccess = "true"))
	FName reloadMontageSection = FName(TEXT("Reload SMG"));

	// Name for the clip bone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties", meta = (AllowPrivateAccess = "true"))
	FName clipBoneName = TEXT("smg_clip");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Properties", meta = (AllowPrivateAccess = "true"))
	// True when reloading
	bool bIsMovingClip = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Properties", meta = (AllowPrivateAccess = "true"))
	float weaponRecoil = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UDataTable* weaponDataTable;

	int32 previousMatIndex;

	// Textures for the crosshairs
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Properties|Crosshair", meta = (AllowPrivateAccess = "true"))
	UTexture2D* crosshairsMiddle;												   

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Properties|Crosshair", meta = (AllowPrivateAccess = "true"))
	UTexture2D* crosshairsLeft;													   
																				  
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Properties|Crosshair", meta = (AllowPrivateAccess = "true"))
	UTexture2D* crosshairsRight;												   
																				   
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Properties|Crosshair", meta = (AllowPrivateAccess = "true"))
	UTexture2D* crosshairsBottom;												  
																				  
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Properties|Crosshair", meta = (AllowPrivateAccess = "true"))
	UTexture2D* crosshairsTop;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	float fireRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* muzzleFlash;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	USoundBase* shootSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	FName boneToHide;

	// amount that the pistols slide is pushed back
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pistol Properties", meta = (AllowPrivateAccess = "true"))
	float slideDisplacement = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pistol Properties", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* slideDisplacementCurve;

	FTimerHandle slideTimer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pistol Properties", meta = (AllowPrivateAccess = "true"))
	float slideDisplacementTime = 0.2f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pistol Properties", meta = (AllowPrivateAccess = "true"))
	bool bMovingSlide = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pistol Properties", meta = (AllowPrivateAccess = "true"))
	float maxSlideDisplacement = 4.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pistol Properties", meta = (AllowPrivateAccess = "true"))
	float maxRecoilRotation = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pistol Properties", meta = (AllowPrivateAccess = "true"))
	float recoilRotation = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties", meta = (AllowPrivateAccess = "true"))
	bool bIsAutomatic = true;

	// DAMAGE

	// Amount of damage casued
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties|Damage", meta = (AllowPrivateAccess = "true"))
	float damage;

	// Amount of damage when bullet hits head
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties|Damage", meta = (AllowPrivateAccess = "true"))
	float headShotDamage;


};
