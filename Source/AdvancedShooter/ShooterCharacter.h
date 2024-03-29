// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <AdvancedShooter/Items/Weapon.h>
#include "ShooterCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USoundBase;
class UParticleSystem;
class UAnimMontage;
class AItem;
class AAmmo;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_ShootTimerInProgress UMETA(DisplayName = "Shoot Timer In Progress"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_Equipping UMETA(DisplayName = "Equipping"),
	ECS_Stunned UMETA(DisplayName = "Stunned"),

	ECS_MAX UMETA(DisplayName = "Default Max"),
};

USTRUCT(BlueprintType)
struct FInterpLocation
{
	GENERATED_BODY()

	// Scene comp to use for it interp location
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* sceneComp;

	// Amount of items interping at this scene comp
	int32 itemAmount;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipItemDelegate, int32, currentSlotIndex, int32, newSlotIndex );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHighlightIconDelegate, int32, slotIndex, bool, bSlotAnimation);

UCLASS()
class ADVANCEDSHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Returns camera boom object
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return cameraBoom; } 

	// Returns follow camera object
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return followCamera; }
	
	// Returns whether is aiming
	FORCEINLINE bool GetIsAiming() const { return bIsAiming; }

	FORCEINLINE bool GetIsCrouching() const { return bIsCrouching; }

	FORCEINLINE int8 GetNumberOfOverlappedItems() const { return overlappedItemCount; }

	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const { return crosshairSpreadMultiplier; };

	FVector2D GetViewportSize();

	AWeapon* GetEquippedWeapon() const { return equippedWeapon; }

	// Adds/subtracts to/from overlappeditemCount and updates should trace for items
	void IncrementOverlappedItemCount(int8 amount);

	void GetPickupItem(AItem* item);
	UAnimInstance* GetAnimInstance() const { return GetMesh()->GetAnimInstance(); }
	FInterpLocation GetInterpLocation(int32 index);

	FORCEINLINE float GetBaseMoveSpeed() const { return baseMoveSpeed; }
	FORCEINLINE float GetCrouchMoveSpeed() const { return crouchMoveSpeed; }

	FORCEINLINE ECombatState GetCombatState() const { return combatState; }

	int32 GetInterpLocationIndex();

	FORCEINLINE bool GetShouldPlayPickupSound() const { return bShouldPlayPickupSound; };
	FORCEINLINE bool GetShouldPlayEquipSound() const { return bShouldPlayEquipSound; };
	UParticleSystem* GetBloodParticles() const { return bloodParticles; }
	float GetStunChance() const {return stunChance; }

	void StartPickupSoundTimer();
	void StartEquipSoundTimer();
	void IncrementInterpLocItemCount(int32 index, int32 amount);

	void HighlightInventorySlot();
	void UnHighlightInventorySlot();

	// Take Combat damage
	virtual float TakeDamage(float damageAmount, struct FDamageEvent const& damageEvent, AController* eventInstigator, AActor* damageCauser) override;
	void Stun();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float value);
	void MoveRight(float value);

	// called when shoot button is pressed
	void ShootWeapon();

	/*
	Called via input to turn at given rate
	@param Rate This is a normalized rate, i.e 1.0 means 100% of desired turn rate
	*/
	void Turn(float rate);

	/*
	Called via input to look up/down at given rate
	@param Rate This is a normalized rate, i.e. 1.0 means 100% of desired rate
	*/
	void LookUp(float rate);

	bool GetTrailEndLocation(const FVector& muzzleSocketLocation, FHitResult& outHitResult);

	void ApplyRecoil();

	// Set bIsAiming to true or false
	void AimDownSight();
	void StopAimingDownSight();
	void CameraInterpZoom(float deltaTime);
	void CalculateCrosshairsSpread(float deltaTime);
	void StartCrosshairBulletFire();

	void ShootButtonPressed();
	void ShootButtonReleased();
	void StartShootTimer();

	void SelectButtonPressed();
	void SelectButtonReleased();

	void CrouchButtonPressed();
	// Spawns a default weapon
	AWeapon* SpawnDefaultWeapon();

	// Takes a weapon and attaches it to the mesh
	void EquipWeapon(AWeapon* weaponToEquip, bool bIsSwapping = false);

	// Drops current weapon and equips trace hit weapon
	void SwapWeapon(AWeapon* weaponToSwap);

	// Detach weapon and let it fall
	void DropWeapon();
	
	// Line trace for items under crosshair
	bool TraceUnderCrosshair(FHitResult& outHitResult, FVector& outHitLocation, float traceRange);

	// Check if overlapping items
	void TraceForItems();
	
	// Initaialize the ammo map
	void InitAmmoMap();

	void InitInterpLocations();

	// Check if weapon has ammo
	bool WeaponHasAmmo();

	// Shoot Weapon Functions
	void PlayShootSound();
	void SendBullet();
	void PlayGunFireMontage();

	// Reload weapon
	void ReloadButtonPressed();
	void ReloadWeapon();

	// Checks to see if we have ammo of the equipped type
	bool CarryingAmmo();

	void PickupAmmo(AAmmo* ammo);

	// Called from Anim BP with grab clip notify
	UFUNCTION(BlueprintCallable)
	void GrabClip();

	// Called from Anim BP with release clip notify
	UFUNCTION(BlueprintCallable)
	void ReplaceClip();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	UFUNCTION(BlueprintCallable)
	void EndStun();

	UFUNCTION()
	void AutoShootReset();

	UFUNCTION()
	void FinishCrosshairBulletFire();

	virtual void Jump() override;

	// Interps capsule half height
	void InterpCapsuleHalfHeight(float deltaTime);

	void Aim();
	void StopAiming();

	void ResetPickupSoundTimer();
	void ResetEquipSoundTimer();

	// Inventory Slot Keys
	void DefaultWeaponSlotKeyPressed();
	void WeaponSlot1KeyPressed();
	//void WeaponSlot2KeyPressed();
	//void WeaponSlot3KeyPressed();
	//void WeaponSlot4KeyPressed();
	//void WeaponSlot5KeyPressed();

	void ExchangeInventoryItems(int32 currentItemIndex, int32 newItemIndex);

	
	int32 GetEmptyInventorySlot();

	void Die();

	UFUNCTION(BlueprintCallable)
	void FinishDeath();
private:	

	// camera boom positioning the camera behind the player
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* cameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* followCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Sensitivity", meta = (AllowPrivateAccess = "true"))
	float baseTurnRate = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Sensitivity", meta = (AllowPrivateAccess = "true"))
	float baseLookUpRate = 0.f;

	// Turn rate in deg/sec
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Sensitivity", meta = (AllowPrivateAccess = "true"))
	float hipTurnRate = 50.f;

	// Lookup/down rate in deg/sec
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Sensitivity", meta = (AllowPrivateAccess = "true"))
	float hipLookUpRate = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Sensitivity", meta = (AllowPrivateAccess = "true"))
	float aimingTurnRate = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Sensitivity", meta = (AllowPrivateAccess = "true"))
	float aimingLookupRate = 20.f;

	// spawned at line trace hit point
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Particles", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* impactParticle;

	// trail that follows shot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Particles", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* trailParticles;

	// Particle system for damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Particles", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* bloodParticles;

	// Montage for firing weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* hipFireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* hitReactMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* deathMontage;

	// Range used for tracing bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|LineTrace", meta = (AllowPrivateAccess = "true"))
	float bulletTraceRange = 50000.f;

	// Range used for tracing items
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|LineTrace", meta = (AllowPrivateAccess = "true"))
	float itemTraceRange = 1000.f;

	// true when aiming
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|LineTrace", meta = (AllowPrivateAccess = "true"))
	bool bIsAiming = false;

	// default FOV value
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float cameraDefaultFOV = 0.f;

	// Zoomed in FOV value
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float cameraZoomedFOV = 35.f;

	// Current FOV this frame
	float cameraCurrentFOV = 0.f;

	// Interp speed when zoomg for aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float zoomInterpSpeed = 20.f;


	// CROSS HAIR VARIABLES
	///////////////////////////////////////

	// Determines the spread of the crosshairs
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CrossHairs", meta = (AllowPrivateAccess = "true"))
	float crosshairSpreadMultiplier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CrossHairs", meta = (AllowPrivateAccess = "true"))
	float crosshairVelocityFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CrossHairs", meta = (AllowPrivateAccess = "true"))
	float crosshairInAirFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CrossHairs", meta = (AllowPrivateAccess = "true"))
	float crosshairAimFactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CrossHairs", meta = (AllowPrivateAccess = "true"))
	float crosshairShootingFactor;

	float shootTimeDuration = 0.05f;
	bool bShootingBullet = false;
	FTimerHandle crosshairShootTimer;
	////////////////////////////////////////

	// Left mouse button or right trigger
	bool bShootPressed = false;

	// True when we can fire
	bool bShouldShoot = true;

	// True when crouching
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|LineTrace", meta = (AllowPrivateAccess = "true"))
	bool bIsCrouching = false;
	
	// Sets timer between shots
	FTimerHandle autoFireTimer;

	// True if we should trace
	bool bShouldTraceForItems = false;

	// Number of overlapped AItems
	int8 overlappedItemCount;

	// Store refrence to item last frame
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items", meta = (AllowPrivateAccess = "true"))
	AItem* traceHitItemLastFrame;

	// Currently equipped weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	AWeapon* equippedWeapon;

	// Set this a default weapon class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> defaultWeaponClass;

	// Item that is traced
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	AItem* traceHitItem;

	// CAMERA INTERPOLATION
	/////////////////////////////////
	// Distance outward from the camera for the interp destination
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Items", meta = (AllowPrivateAccess = "true"))
	float cameraInterpDistance = 250.f;

	// Distance upword from the camerafor the interp destination
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Items", meta = (AllowPrivateAccess = "true"))
	float cameraInterpElevation = 65.f;
	//////////////////////////////////
	
	// AMMO STUFF
	//////////////////////////////////
	
	// Map to keep track of different ammo types
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items|Ammo", meta = (AllowPrivateAccess = "true"))
	TMap<EAmmoType, int32> ammoMap;

	// Amount of 9mm you start with
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items|Ammo", meta = (AllowPrivateAccess = "true"))
	int32 starting9mmAmmo = 85;

	// Amount of AR you start with
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items|Ammo", meta = (AllowPrivateAccess = "true"))
	int32 startingARAmmo = 120;
	///////////////////////////////

	// Combat state can only fire or reload if unocupied
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	ECombatState combatState = ECombatState::ECS_Unoccupied;

	// Montage for reload anims
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* reloadMontage;

	// Montage for reload anims
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* equipMontage;

	// Transform of the clip
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FTransform clipTransform;

	// Scene comp to attach to char hands during reloading
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	USceneComponent* handSceneComponent;

	// Normal character movement speed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float baseMoveSpeed = 650.f;

	// Crouching movement speed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float crouchMoveSpeed = 300.f;

	// Current half height of the capsule
	float currentCapsuleHalfHeight = 0.f;

	// stading half height of the capsule
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float standingCapsuleHalfHeight = 88.f;

	// Crouching half height of the capsule
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float crouchingCapsuleHalfHeight = standingCapsuleHalfHeight / 2;

	//  Friction while not crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float baseGroundFriction = 2.f;

	// Friction while crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float crouchingGroundFriction = 100.f;

	// Lets us know when aim button is pressed
	bool bAimingButtonPressed = false;

	/// INTERP COMPONENTS
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* weaponInterpComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* interpComp1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* interpComp2;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* interpComp3;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* interpComp4;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* interpComp5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* interpComp6;
	/////////////////////////////////////////////////////

	// Array of interp location structs
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FInterpLocation> interpLocations;

	FTimerHandle pickupSoundTimer;
	FTimerHandle equipSoundTimer;

	bool bShouldPlayPickupSound = true;
	bool bShouldPlayEquipSound = true;

	// Time to wait till play next pickup sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items", meta = (AllowPrivateAccess = "true"))
	float pickupSoundsResetTime = 0.2f;

	// Time to wait till play next equip sounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items", meta = (AllowPrivateAccess = "true"))
	float equipSoundsResetTime = 0.2f;

	// Array of AItems for the inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TArray<AItem*> inventory;

	// No more than 6 items
	const int32 inventoryCap = 2;

	// Delegate for sending slot info to inventory bar when equipping
	UPROPERTY(BlueprintAssignable, Category = "Delegates", meta = (AllowPrivateAccess = "true"))
	FEquipItemDelegate equipItemDelegate;

	// Delegate for sending slot info for playing icon animation
	UPROPERTY(BlueprintAssignable, Category = "Delegates", meta = (AllowPrivateAccess = "true"))
	FHighlightIconDelegate highlightIconDelegate;

	// The index for the currently highlighted slot
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 highlightedSlot = -1;

	// Character health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = "true"))
	float health = 0.f;

	// Character Max Health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = "true"))
	float maxHealth =100;

	// Chance of being stunned
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stun", meta = (AllowPrivateAccess = "true"))
	float stunChance = 0.25f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	bool bIsDead = false;
};
