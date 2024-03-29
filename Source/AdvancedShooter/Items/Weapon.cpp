// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (boneToHide == FName("")) return;
	GetItemMesh()->HideBoneByName(boneToHide, EPhysBodyOp::PBO_None);
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Keep the weapon uprights
	if (GetItemState() == EItemState::EIS_Falling && bIsFalling)
	{
		const FRotator meshRotation = FRotator(0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f);
		GetItemMesh()->SetWorldRotation(meshRotation, false, NULL, ETeleportType::TeleportPhysics);
	}

	UpdateSlideDisplacement();
}

void AWeapon::OnConstruction(const FTransform& transform)
{
	Super::OnConstruction(transform);
	SetWeaponData();
}

UDataTable* AWeapon::GetWeaponDataTable()
{
	// Load data in the item rarity data table

	// Path to the item rarity data table
	FString weaponTablePath = TEXT("DataTable'/Game/_Game/DataTable/DT_Weapon.DT_Weapon'");

	UDataTable* weaponTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), NULL, *weaponTablePath));

	if (!weaponTableObject) return NULL;

	return weaponTableObject;
}

void AWeapon::SetWeaponData()
{
	FWeaponDataTable* weaponDataRow = NULL;

	switch (weaponType)
	{
		case EWeaponType::EWT_SubmachineGun:
			weaponDataRow =  GetWeaponDataTable()->FindRow<FWeaponDataTable>(FName("SMG"), TEXT(""));
			break;

		case EWeaponType::EWT_AssaultRifle:
			weaponDataRow = GetWeaponDataTable()->FindRow<FWeaponDataTable>(FName("AR"), TEXT(""));
			break;

		case EWeaponType::EWT_Pistol:
			weaponDataRow = GetWeaponDataTable()->FindRow<FWeaponDataTable>(FName("Pistol"), TEXT(""));
			break;
		default:
			break;
	}

	if (!weaponDataRow) return;

	ammoType = weaponDataRow->ammoType;
	ammo = weaponDataRow->weaponAmmo;
	magazineCap = weaponDataRow->magazineCap;
	weaponRecoil = weaponDataRow->recoil;

	SetPickupSound(weaponDataRow->pickupSound);
	SetEquipSound(weaponDataRow->equipSound);
	SetItemName(weaponDataRow->itemName);
	GetItemMesh()->SetSkeletalMesh(weaponDataRow->itemMesh);
	SetItemIcon(weaponDataRow->inventoryIcon);
	SetAmmoIcon(weaponDataRow->ammoIcon);
	SetClipBoneName(weaponDataRow->clipBoneName);
	SetReloadMontageSection(weaponDataRow->reloadMontageSection);
	SetMaterialInstance(weaponDataRow->materialInstance);
	GetItemMesh()->SetAnimInstanceClass(weaponDataRow->animBP);

	previousMatIndex = GetMaterialIndex();
	GetItemMesh()->SetMaterial(previousMatIndex, NULL);
	SetMaterialIndex(weaponDataRow->materialIndex);
	
	crosshairsMiddle = weaponDataRow->crosshairsMiddle;
	crosshairsLeft = weaponDataRow->crosshairsLeft;
	crosshairsRight = weaponDataRow->crosshairsRight;
	crosshairsBottom = weaponDataRow->crosshairsBottom;
	crosshairsTop = weaponDataRow->crosshairsTop;


	fireRate = weaponDataRow->fireRate;
	shootSound = weaponDataRow->shootSound;
	muzzleFlash = weaponDataRow->muzzleFlash;

	boneToHide = weaponDataRow->boneToHide;
	bIsAutomatic = weaponDataRow->bIsAutomatic;
	damage = weaponDataRow->damage;
	headShotDamage = weaponDataRow->headShotDamage;

	// Scale damage based of rarity
	CalculateDamage();

	if (!GetMaterialInstance()) return;

	// Create a new material instance
	SetDynamicMaterialInstance(UMaterialInstanceDynamic::Create(GetMaterialInstance(), this));
	GetDynamicMaterialInstance()->SetVectorParameterValue(TEXT("FresnelColor"), GetGlowColor());
	GetItemMesh()->SetMaterial(GetMaterialIndex(), GetDynamicMaterialInstance());

	EnableGlowMaterial();
}

void AWeapon::FinishMovingSlide()
{
	bMovingSlide = false;
}

void AWeapon::StartSlideTimer()
{
	bMovingSlide = true;
	GetWorldTimerManager().SetTimer(slideTimer, this, &AWeapon::FinishMovingSlide, slideDisplacementTime);
}

void AWeapon::UpdateSlideDisplacement()
{
	if (!slideDisplacementCurve  && !bMovingSlide) return;

	const float elapsedTime = GetWorldTimerManager().GetTimerElapsed(slideTimer);
	const float curveValue = slideDisplacementCurve->GetFloatValue(elapsedTime);
	slideDisplacement = curveValue * maxSlideDisplacement;
	recoilRotation = curveValue * maxRecoilRotation;
}


void AWeapon::CalculateDamage()
{
	damage *= GetDamageScalar();
	headShotDamage *= GetHeadshotDamageScalar();
}
void AWeapon::ThrowWeapon()
{
	FRotator meshRotation = FRotator(0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f);
	GetItemMesh()->SetWorldRotation(meshRotation, false, NULL, ETeleportType::TeleportPhysics);

	const FVector meshForward = GetItemMesh()->GetForwardVector();
	const FVector meshRight = GetItemMesh()->GetRightVector();

	// Direction which we throw the weapon
	FVector impulseDirection = meshRight.RotateAngleAxis(-20, meshForward);

	float randomRotation = 30.f;//FMath::FRandRange(30, 50);

	impulseDirection = impulseDirection.RotateAngleAxis(randomRotation, FVector(0.f, 0.f, 1.f));

	impulseDirection *= 10'000.f;
	GetItemMesh()->AddImpulse(impulseDirection);

	bIsFalling = true;

	GetWorldTimerManager().SetTimer(throwWeaponTimer, this, &AWeapon::StopFalling, throwWeaponTime);

	EnableGlowMaterial();
}

void AWeapon::ReloadAmmo(int32 amount)
{
	checkf(ammo + amount <= magazineCap, TEXT("Attempted to reload with more than mag cap"));
	
	ammo += amount;
}

void AWeapon::DecrementAmmo()
{
	if (ammo - 1 <= 0)
		ammo = 0;
	
	else
		--ammo;
}

void AWeapon::StopFalling()
{
	bIsFalling = false;
	SetItemState(EItemState::EIS_Pickup);
	ResetPulseTimer();
}
