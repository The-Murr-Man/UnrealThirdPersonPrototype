// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Engine/DataTable.h>
#include "Item.generated.h"

class UBoxComponent;
class USphereComponent;
class UWidgetComponent;
class UCurveFloat;
class UCurveVector;
class AShooterCharacter;
class USoundBase;

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Damaged UMETA(DisplayName = "Damaged"),
	EIR_Common UMETA(DisplayName = "Common"),
	EIR_Uncommon UMETA(DisplayName = "Uncommon"),
	EIR_Rare UMETA(DisplayName = "Rare"),
	EIR_Legendary UMETA(DisplayName = "Legendary"),

	EIR_MAX UMETA(DisplayName = "DefaultMax")
};

UENUM(BlueprintType)
enum class EItemState : uint8
{
	EIS_Pickup UMETA(DisplayName = "Pickup"),
	EIS_EquipInterping UMETA(DisplayName = "EquipInterping"),
	EIS_PickedUp UMETA(DisplayName = "PickedUp"),
	EIS_Equipped UMETA(DisplayName = "Equipped"),
	EIS_Falling UMETA(DisplayName = "Falling"),

	EIS_MAX UMETA(DisplayName = "DefaultMax")
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
	EIT_Ammo UMETA(DisplayName = "Ammo"),
	EIT_Weapon UMETA(DisplayName = "Weapon"),
	  
	EIT_MAX UMETA(DisplayName = "DefaultMax")
};

USTRUCT(BlueprintType)
struct FItemRarityTable : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor glowColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor lightColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor darkColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 numOfstars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* iconBackground;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 customDepthStencil;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float damageScalar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float headshotDamageScalar;
};

UCLASS()
class ADVANCEDSHOOTER_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// GETTERS
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return pickupWidget; }
	FORCEINLINE USphereComponent* GetAreaSphere() const { return areaSphere; }
	FORCEINLINE UBoxComponent* GetCollisionBox() const { return boxCollision; }
	FORCEINLINE EItemState GetItemState() const { return itemState; }
	FORCEINLINE EItemType GetItemType() const { return itemType; }
	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return itemMesh; }

	FORCEINLINE USoundBase* GetPickupSound() const { return pickupSound; }
	FORCEINLINE USoundBase* GetEquipSound() const { return equipSound; }
	FORCEINLINE int32 GetItemAmount() const { return itemAmount; }
	FORCEINLINE int32 GetSlotIndex() const { return slotIndex; }
	FORCEINLINE UMaterialInstance* GetMaterialInstance() const { return materialInstance; }
	FORCEINLINE UMaterialInstanceDynamic* GetDynamicMaterialInstance() const { return dynamicMaterialInstance; }
	FORCEINLINE FLinearColor GetGlowColor() const { return glowColor; }
	FORCEINLINE int32 GetMaterialIndex() const { return materialIndex; }
	FORCEINLINE EItemRarity GetItemRarity() const { return itemRarity; }
	FORCEINLINE float GetDamageScalar() const { return damageScalar; }
	FORCEINLINE float GetHeadshotDamageScalar() const { return headshotDamageScalar; }

	// SETTERS
	FORCEINLINE void SetSlotIndex(int32 index) { slotIndex = index; }
	FORCEINLINE void SetCharacter(AShooterCharacter* _character) { character = _character; }
	FORCEINLINE void SetCharacterInventoryFull(bool bIsFull) { bIsCharacterInventoryFull = bIsFull; }
	FORCEINLINE void SetItemState(EItemState state);
	FORCEINLINE void SetPickupSound(USoundBase* sound) { pickupSound = sound; }
	FORCEINLINE void SetEquipSound(USoundBase* sound) { equipSound = sound; }
	FORCEINLINE void SetItemName(FString name) { itemName = name; }
	FORCEINLINE void SetItemIcon(UTexture2D* icon) { iconItem = icon; }
	FORCEINLINE void SetAmmoIcon(UTexture2D* icon) { iconAmmo = icon; }
	FORCEINLINE void SetMaterialInstance(UMaterialInstance* inst) { materialInstance = inst; }
	FORCEINLINE void SetDynamicMaterialInstance(UMaterialInstanceDynamic* dynamicInst) { dynamicMaterialInstance = dynamicInst; }
	FORCEINLINE void SetMaterialIndex(int32 index) { materialIndex = index; }

	// Called from aShooterCharacterClass
	void StartItemCurve(AShooterCharacter* _character, bool bForcePlaySound = false);
	void PlayEquipSound(bool bForcePlaySound = false);

	// Turn on Custom Depth postproccessing 
	virtual void EnableCustomDepth();

	// Turn off Custom Depth postproccessing 
	virtual void DisableCustomDepth();

	void EnableGlowMaterial();
	void DisableGlowMaterial();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when overlapping area sphere
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, 
						 int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult);
	
	// Called when end overlapping area sphere
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor, UPrimitiveComponent* otherComp, int32 otherBodyIndex);

	// sets active star array of bools based on rarity
	void SetActiveStars();

	// Sets properties of items components based on current state
	virtual void SetItemProperties(EItemState state);
	void FinishInterping();

	// Handles item interpolation
	void ItemInterp(float deltaTime);

	// Get interp location based on item type
	FVector GetInterpLocation();

	void PlayPickupSound(bool bForcePlaySound = false);

	virtual void InitCustomDepth();

	virtual void OnConstruction(const FTransform& transform) override;

	void ResetPulseTimer();

	void UpdatePulse();

	UDataTable* GetRarityDataTable();
	void SetRarityData();
private:	
	
	// Items skeletal mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* itemMesh;

	// Line trace collides with box to show hud widgets
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* boxCollision;

	// Popup widdget for when looks at item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* pickupWidget;

	// Enables item tracing when overlapped
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USphereComponent* areaSphere;

	// Name which appears on item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FString itemName = "Default";

	// Number that appears on item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 itemAmount = 0;

	// Rarity of the item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemRarity itemRarity = EItemRarity::EIR_Common;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TArray<bool> activeStars;

	// State of the item
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties|Enums", meta = (AllowPrivateAccess = "true"))
	EItemState itemState = EItemState::EIS_Pickup;

	// Type of item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties|Enums", meta = (AllowPrivateAccess = "true"))
	EItemType itemType = EItemType::EIT_Ammo;

	// Curve assets to use for the items z location when interping
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties|Curves", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* itemZCurve;

	// Start location of the item 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FVector itemInterpStartlocation = FVector::ZeroVector;

	// Target interp location in front of camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FVector cameraTargetLocation = FVector::ZeroVector;

	// True when interping
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	bool bIsInterping = false;

	// Plays when we start interping
	FTimerHandle itemInterpTimer;

	// Reference to character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	AShooterCharacter* character;

	// Duration of curve and timer
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties|Curves", meta = (AllowPrivateAccess = "true"))
	float zCurveTime = 0.7f;

	// X and Y for item interping
	float itemInterpX = 0.f;
	float itemInterpY = 0.f;

	// Initial yaw offset between camera and interping item
	float interpInitialYawOffset = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties|Curves", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* itemScaleCurve;

	// Sound played when Item is picked up
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties|Sounds", meta = (AllowPrivateAccess = "true"))
	USoundBase* pickupSound;

	// Sound played when Item is equipped
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties|Sounds", meta = (AllowPrivateAccess = "true"))
	USoundBase* equipSound;

	// Index of the interp location
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 interpLocIndex = 0.f;

	// Index of the material to change at runtime
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 materialIndex = 0;

	// Dynamic instance to change at runtime
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UMaterialInstanceDynamic* dynamicMaterialInstance;

	// Material Instance used with the dynamic instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* materialInstance;

	bool bCanChangeCustomDepth = true;

	// Curve to drive the dynamic mat params
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties|Curves", meta = (AllowPrivateAccess = "true"))
	UCurveVector* pulseCurve;

	// Curve to drive the dynamic mat params
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties|Curves", meta = (AllowPrivateAccess = "true"))
	UCurveVector* interpPulseCurve;

	FTimerHandle pulseTimer;

	// Time for the pulse timer
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties|Curves", meta = (AllowPrivateAccess = "true"))
	float pulseCurveTime = 5.f;

	// DYNAMIC MATERIAL PARAMS
	UPROPERTY(VisibleAnywhere, Category = "Item Properties|Curves", meta = (AllowPrivateAccess = "true"))
	float glowAmount = 150.f;

	UPROPERTY(VisibleAnywhere, Category = "Item Properties|Curves", meta = (AllowPrivateAccess = "true"))
	float fresnelExponent = 3.f;

	UPROPERTY(VisibleAnywhere, Category = "Item Properties|Curves", meta = (AllowPrivateAccess = "true"))
	float fresnelReflectFraction = 4.f;

	// INVENTORY VARIABLES

	// Icon for this item in the inventory
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Icons", meta = (AllowPrivateAccess = "true"))
	UTexture2D* iconItem;

	// Ammo Icon for this item in the inventory
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Icons", meta = (AllowPrivateAccess = "true"))
	UTexture2D* iconAmmo;

	// Slot in the inventory array
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 slotIndex = 0;

	// True when chars inventory is full
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	bool bIsCharacterInventoryFull = false;

	// Item rarity data table
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UDataTable* itemRarityDataTable;

	// Color of the glow mat
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	FLinearColor glowColor;

	// Color of the light color in the pickup widget
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	FLinearColor lightColor;

	// Color of the dark color in the pickup widget
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	FLinearColor darkColor;

	// Number of stars in the pickup widget
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	int32 numOfStars;

	// Background icon for the inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	UTexture2D* iconBackground;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	float damageScalar = 1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	float headshotDamageScalar = 1.f;
};
