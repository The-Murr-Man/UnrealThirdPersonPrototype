#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include <GameFramework/CharacterMovementComponent.h>
#include <Kismet/GameplayStatics.h>
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include <DrawDebugHelpers.h>
#include "Items/Item.h"
#include "Items/Ammo.h"
#include <Components/WidgetComponent.h>
#include "Components/CapsuleComponent.h"
#include <Components/BoxComponent.h>
#include <Components/SphereComponent.h>
#include <AdvancedShooter/BulletHitInterface.h>
#include <AdvancedShooter/AI/Enemy.h>
#include <AdvancedShooter/AI/EnemyController.h>
#include <BehaviorTree/BlackboardComponent.h>

// Sets default values
AShooterCharacter::AShooterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Creates camera boom object
	cameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	cameraBoom->SetupAttachment(RootComponent); // Attaches new boom to root comp
	cameraBoom->TargetArmLength = 180.f; // Sets distance from character // TODO: MAKE THIS VARIABLE
	cameraBoom->bUsePawnControlRotation = true; // Makes boom use pawns rotation
	cameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f); // TODO: MAKE THIS VARIABLE

	// Create follow camera object
	followCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow Camera"));
	followCamera->SetupAttachment(cameraBoom, USpringArmComponent::SocketName); // Attaches camera to boom
	followCamera->bUsePawnControlRotation = false; // Makes camera not rotate relative to arm

	// don't rotate when controller rotate
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configue character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; //  Character moves in the direction of input
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // At this set rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	health = maxHealth;

	// Create hand scene comp
	handSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Hand Scene Component"));

	// INTERP COMPONENTS
	weaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interp Component"));
	weaponInterpComp->SetupAttachment(GetFollowCamera());

	interpComp1 =CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component 1"));
	interpComp1->SetupAttachment(GetFollowCamera());

	interpComp2 =CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component 2"));
	interpComp2->SetupAttachment(GetFollowCamera());

	interpComp3 =CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component 3"));
	interpComp3->SetupAttachment(GetFollowCamera());

	interpComp4=CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component 4"));
	interpComp4->SetupAttachment(GetFollowCamera());

	interpComp5=CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component 5"));
	interpComp5->SetupAttachment(GetFollowCamera());

	interpComp6=CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component 6"));
	interpComp6->SetupAttachment(GetFollowCamera());
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (!followCamera) return;
	cameraDefaultFOV = GetFollowCamera()->FieldOfView;
	cameraCurrentFOV = cameraDefaultFOV;

	// Spawn the default weapon and equip to mesh
	EquipWeapon(SpawnDefaultWeapon());

	// Add equipped weapon to inventory
	inventory.Add(equippedWeapon);
	equippedWeapon->SetSlotIndex(0);
	equippedWeapon->DisableCustomDepth();
	equippedWeapon->DisableGlowMaterial();
	equippedWeapon->SetCharacter(this);

	InitAmmoMap();
	InitInterpLocations();
	GetCharacterMovement()->MaxWalkSpeed = baseMoveSpeed;
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraInterpZoom(DeltaTime);

	CalculateCrosshairsSpread(DeltaTime);

	if (combatState == ECombatState::ECS_ShootTimerInProgress)
		ApplyRecoil();

	TraceForItems();

	// Interpolate capsule half height based on crouching/standing
	InterpCapsuleHalfHeight(DeltaTime);
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (!PlayerInputComponent) return;

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);

	PlayerInputComponent->BindAxis("MouseX", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("MouseY", this, &AShooterCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("ShootButton", IE_Pressed, this, &AShooterCharacter::ShootButtonPressed);
	PlayerInputComponent->BindAction("ShootButton", IE_Released, this, &AShooterCharacter::ShootButtonReleased);

	PlayerInputComponent->BindAction("AimDownSight", IE_Pressed, this, &AShooterCharacter::AimDownSight);
	PlayerInputComponent->BindAction("AimDownSight", IE_Released, this, &AShooterCharacter::StopAimingDownSight);

	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::SelectButtonReleased);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);



	// Inventory Slot Keys
	PlayerInputComponent->BindAction("DefaultWeaponSlotKey", IE_Pressed, this, &AShooterCharacter::DefaultWeaponSlotKeyPressed);
	PlayerInputComponent->BindAction("WeaponSlot1Key", IE_Pressed, this, &AShooterCharacter::WeaponSlot1KeyPressed);
	//PlayerInputComponent->BindAction("WeaponSlot2Key", IE_Pressed, this, &AShooterCharacter::WeaponSlot2KeyPressed);
	//PlayerInputComponent->BindAction("WeaponSlot3Key", IE_Pressed, this, &AShooterCharacter::WeaponSlot3KeyPressed);
	//PlayerInputComponent->BindAction("WeaponSlot4Key", IE_Pressed, this, &AShooterCharacter::WeaponSlot4KeyPressed);
	//PlayerInputComponent->BindAction("WeaponSlot5Key", IE_Pressed, this, &AShooterCharacter::WeaponSlot5KeyPressed);
}

// PLAYER MOVEMENT
///////////////////////////////////////////////////////
void AShooterCharacter::MoveForward(float value)
{
	if (!Controller && value != 0.f) return;

	// find out where is forward
	const FRotator rotation = Controller->GetControlRotation();

	// Only use yaw 
	const FRotator yawRotation = FRotator(0, rotation.Yaw, 0);

	// Gets x axis from rotation matrix
	const FVector direction = FRotationMatrix{yawRotation}.GetUnitAxis(EAxis::X);

	AddMovementInput(direction, value);
}

void AShooterCharacter::MoveRight(float value)
{
	if (!Controller && value != 0.f) return;

	// find out where is right
	const FRotator rotation = Controller->GetControlRotation();

	// Only use yaw 
	const FRotator yawRotation = FRotator(0, rotation.Yaw, 0);

	// Gets y axis from rotation matrix
	const FVector direction = FRotationMatrix{ yawRotation }.GetUnitAxis(EAxis::Y);

	AddMovementInput(direction, value);
}

void AShooterCharacter::Turn(float rate)
{
	// calcuate yaw input using delta time and turn rate
	AddControllerYawInput(rate * baseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::LookUp(float rate)
{
	// calcuate pitch input using delta time and turn rate
	AddControllerPitchInput(rate * baseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Jump()
{
	if (bIsCrouching)
	{
		bIsCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = baseMoveSpeed;
	}
		
	else
		ACharacter::Jump();
}

void AShooterCharacter::InterpCapsuleHalfHeight(float deltaTime)
{
	float targetCapsuleHalfHeight = 0.f;

	if (bIsCrouching)
	{
		targetCapsuleHalfHeight = crouchingCapsuleHalfHeight;
	}
	else
	{
		targetCapsuleHalfHeight = standingCapsuleHalfHeight;
	}

	const float interpHalfHeight = FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), targetCapsuleHalfHeight, deltaTime, 20.f);
	
	// Negative if crouching, Positive if standing
	const float deltaCapsuleHalfHeight = interpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	
	const FVector meshOffset = FVector(0, 0, -deltaCapsuleHalfHeight);

	GetMesh()->AddLocalOffset(meshOffset);
	GetCapsuleComponent()->SetCapsuleHalfHeight(interpHalfHeight);
}

//////////////////////////////////////////////////////

// CROUCHING
/// //////////////////////////////////////////////////
void AShooterCharacter::CrouchButtonPressed()
{
	if (!GetCharacterMovement()->IsFalling())
		bIsCrouching = !bIsCrouching;        // Negate bIsCrouching
	
	if (bIsCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = crouchMoveSpeed;
		GetCharacterMovement()->GroundFriction = crouchingGroundFriction;
	}
		
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = baseMoveSpeed;
		GetCharacterMovement()->GroundFriction = baseGroundFriction;
	}
}
/// //////////////////////////////////////////////////

// PLAYER SHOOTING
////////////////////////////////////////////////////
void AShooterCharacter::ShootWeapon()
{
	if (!equippedWeapon) return;
	if (combatState != ECombatState::ECS_Unoccupied) return;
	
	if (WeaponHasAmmo())
	{
		PlayShootSound();

		SendBullet();

		//ApplyRecoil(GetWorld()->GetDeltaSeconds());
		
		PlayGunFireMontage();

		// Subtract 1 from weapon ammo
		equippedWeapon->DecrementAmmo();

		StartCrosshairBulletFire();
		// Starts bullet fire timer for crosshair
		StartShootTimer();

		if (equippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol)
		{
			equippedWeapon->StartSlideTimer();
		}
	}
}

void AShooterCharacter::ApplyRecoil()
{
	float recoil = equippedWeapon->GetWeaponRecoil();

	AddControllerPitchInput(-recoil * aimingLookupRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::PlayShootSound()
{
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), equippedWeapon->GetShootSound(), GetActorLocation());
}

void AShooterCharacter::SendBullet()
{
	// Send Bullet

	// Get and assign barrel socket from mesh
	const USkeletalMeshSocket* barrelSocket = equippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");

	if (!barrelSocket) return;
	const FTransform socketTransform = barrelSocket->GetSocketTransform(equippedWeapon->GetItemMesh());

	if (equippedWeapon->GetMuzzleFlash())
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), equippedWeapon->GetMuzzleFlash(), socketTransform);
	}
	
	FHitResult trailHitResult;

	bool bTrailEnd = GetTrailEndLocation(socketTransform.GetLocation(), trailHitResult);

	if (!bTrailEnd) return;

	

	if (trailHitResult.Actor.IsValid())
	{
		// cast will only succed if hits an actor which implements bullet hit interface
		IBulletHitInterface* bulletHitInterface = Cast<IBulletHitInterface>(trailHitResult.Actor.Get());

		if (bulletHitInterface)
		{
			bulletHitInterface->BulletHit_Implementation(trailHitResult, this, GetController());

			AEnemy* hitEnemy = Cast<AEnemy>(trailHitResult.Actor.Get());

			float weaponDamage;
			bool bIsHeadShot;
			

			if (hitEnemy)
			{
				if (trailHitResult.BoneName.ToString() == hitEnemy->GetHeadBoneName())
				{
					weaponDamage = equippedWeapon->GetHeadShotDamage();
					bIsHeadShot = true;
					UGameplayStatics::ApplyDamage(hitEnemy, weaponDamage, GetController(), this, UDamageType::StaticClass());
				}

				else
				{
					weaponDamage = equippedWeapon->GetDamage();
					bIsHeadShot = false;
					UGameplayStatics::ApplyDamage(hitEnemy, weaponDamage, GetController(), this, UDamageType::StaticClass());
				}	
				hitEnemy->ShowDamageNumber(weaponDamage, trailHitResult.ImpactPoint, bIsHeadShot);
			}
		}
		
		else
		{
			// Spawn impact particles after updating trail end point
			if (!impactParticle) return;
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), impactParticle, trailHitResult.ImpactPoint);
		}
	}


	if (!trailParticles) return;
	UParticleSystemComponent* trail = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), trailParticles, socketTransform);

	if (!trail) return;
	trail->SetVectorParameter(FName("Target"), trailHitResult.ImpactPoint);
}

void AShooterCharacter::PlayGunFireMontage()
{
	// Player hipfire montage 
	if (!hipFireMontage) return;

	// Plays hip fire montage
	GetAnimInstance()->Montage_Play(hipFireMontage);

	// Jumps to start fire section of the montage
	GetAnimInstance()->Montage_JumpToSection(FName("StartFire"));
}
void AShooterCharacter::ShootButtonPressed()
{
	bShootPressed = true;
	ShootWeapon();
}

void AShooterCharacter::ShootButtonReleased()
{
	bShootPressed = false;
}

void AShooterCharacter::StartShootTimer()
{
	if (!equippedWeapon) return;
	combatState = ECombatState::ECS_ShootTimerInProgress;

	GetWorldTimerManager().SetTimer(autoFireTimer, this, &AShooterCharacter::AutoShootReset, equippedWeapon->GetFireRate());
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bShootingBullet = true;

	GetWorldTimerManager().SetTimer(crosshairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFire, shootTimeDuration);
}
void AShooterCharacter::FinishCrosshairBulletFire()
{
	bShootingBullet = false;
}

void AShooterCharacter::AutoShootReset()
{
	if (combatState == ECombatState::ECS_Stunned) return;
	combatState = ECombatState::ECS_Unoccupied;
	if (WeaponHasAmmo())
	{
		if (bShootPressed && equippedWeapon->GetIsAutomatic())
		{
			ShootWeapon();
		}
	}

	else
	{
		ReloadWeapon();
	}
}
////////////////////////////////////////////////////

// TRACING
////////////////////////////////////////////////////
bool AShooterCharacter::TraceUnderCrosshair(FHitResult& outHitResult, FVector& outHitLocation ,float traceRange)
{
	// Get screen space location of crosshairs
	FVector2D crossHairLocation = FVector2D(GetViewportSize().X / 2.f, GetViewportSize().Y / 2.f);
	// Out params
	FVector crossHairWorldPosition;
	FVector crossHairWorldDirection;

	// Get world position and direction of crosshairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
		crossHairLocation, crossHairWorldPosition, crossHairWorldDirection);

	if (bScreenToWorld)
	{
		const FVector start = crossHairWorldPosition;
		const FVector end = start + crossHairWorldDirection * traceRange;
		outHitLocation = end;
		GetWorld()->LineTraceSingleByChannel(outHitResult, start, end, ECollisionChannel::ECC_Visibility);

		if (outHitResult.bBlockingHit)
		{
			outHitLocation = outHitResult.Location;
			return true;
		}
	}

	return false;
}

void AShooterCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		FHitResult itemTraceResult;
		FVector hitLocation;
		TraceUnderCrosshair(itemTraceResult, hitLocation, itemTraceRange);

		if (itemTraceResult.bBlockingHit)
		{
			traceHitItem= Cast<AItem>(itemTraceResult.GetActor());

			const AWeapon* traceHitWeapon = Cast<AWeapon>(traceHitItem);
			
			if (traceHitWeapon)
			{
				if (highlightedSlot == -1) // Not currently highlighting slot 
					HighlightInventorySlot(); // Highlight the slot
			}

			else
			{
				if (highlightedSlot != -1) // Slot is Highlighted
					UnHighlightInventorySlot(); // Unhighlight the slot
			}

			if (!traceHitItem) return;

			if (traceHitItem->GetItemState() == EItemState::EIS_EquipInterping)
			{
				traceHitItem = NULL;
			}

			if (traceHitItem->GetPickupWidget())
			{
				// Show Items pickup widget
				traceHitItem->GetPickupWidget()->SetVisibility(true);
				traceHitItem->EnableCustomDepth();
				
				if (inventory.Num() >= inventoryCap) // Inventory is full
					traceHitItem->SetCharacterInventoryFull(true);

				else // Inventory is not full
					traceHitItem->SetCharacterInventoryFull(false);
			}

			// We hit an item last frame
			if (traceHitItemLastFrame)
			{
				if (traceHitItem != traceHitItemLastFrame)
				{
					// We are hiting a different item or the item is null
					traceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
					traceHitItemLastFrame->DisableCustomDepth();
				}
			}

			// Store refrence to hit item for next frame
			traceHitItemLastFrame = traceHitItem;
		}
	}

	else if (traceHitItemLastFrame)
	{
		// No longer overlapping
		traceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
		traceHitItemLastFrame->DisableCustomDepth();
	}
}

void AShooterCharacter::SelectButtonPressed()
{
	if (combatState != ECombatState::ECS_Unoccupied) return;
	if (!traceHitItem) return;

	traceHitItem->StartItemCurve(this, true);
	traceHitItem = NULL;
}

void AShooterCharacter::SelectButtonReleased()
{
	
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 amount)
{
	if (overlappedItemCount + amount <= 0)
	{
		overlappedItemCount = 0;
		bShouldTraceForItems = false;
	}

	else
	{
		overlappedItemCount += amount;
		bShouldTraceForItems = true;
	}
}
////////////////////////////////////////////////////

// CAMERA
////////////////////////////////////////////////////
void AShooterCharacter::AimDownSight()
{
	bAimingButtonPressed = true;

	if (combatState != ECombatState::ECS_Reloading && combatState != ECombatState::ECS_Equipping && combatState != ECombatState::ECS_Stunned)
	{
		Aim();
	}
}

void AShooterCharacter::StopAimingDownSight()
{
	bAimingButtonPressed = false;
	StopAiming();
}

void AShooterCharacter::Aim()
{
	bIsAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = crouchMoveSpeed;
}
void AShooterCharacter::StopAiming()
{
	bIsAiming = false;

	if (!bIsCrouching)
		GetCharacterMovement()->MaxWalkSpeed = baseMoveSpeed;
}

void AShooterCharacter::CameraInterpZoom(float deltaTime)
{
	// Aiming
	if (bIsAiming)
	{
		// Interpolate from current FOV to zoomed FOV
		cameraCurrentFOV = FMath::FInterpTo(cameraCurrentFOV, cameraZoomedFOV, deltaTime, zoomInterpSpeed);
		baseTurnRate = aimingTurnRate;
		baseLookUpRate = aimingLookupRate;
	}

	else
	{
		// Interpolate from current FOV to default FOV
		cameraCurrentFOV = FMath::FInterpTo(cameraCurrentFOV, cameraDefaultFOV, deltaTime, zoomInterpSpeed);
		baseTurnRate = hipTurnRate;
		baseLookUpRate = hipLookUpRate;
	}
	GetFollowCamera()->SetFieldOfView(cameraCurrentFOV);
}

void AShooterCharacter::InitInterpLocations()
{
	FInterpLocation weaponLocation{ weaponInterpComp, 0 };
	interpLocations.Add(weaponLocation);

	FInterpLocation interpLoc1{ interpComp1, 0 };
	interpLocations.Add(interpLoc1);

	FInterpLocation interpLoc2{ interpComp2, 0 };
	interpLocations.Add(interpLoc2);

	FInterpLocation interpLoc3{ interpComp3, 0 };
	interpLocations.Add(interpLoc3);

	FInterpLocation interpLoc4{ interpComp4, 0 };
	interpLocations.Add(interpLoc4);

	FInterpLocation interpLoc5{ interpComp5, 0 };
	interpLocations.Add(interpLoc5);

	FInterpLocation interpLoc6{ interpComp6, 0 };
	interpLocations.Add(interpLoc6);
}

void AShooterCharacter::IncrementInterpLocItemCount(int32 index, int32 amount)
{
	if (amount < -1 && amount > 1) return;

	if (interpLocations.Num() >= index)
	{
		interpLocations[index].itemAmount += amount;
	}
}

////////////////////////////////////////////////////

void AShooterCharacter::CalculateCrosshairsSpread(float deltaTime)
{
	FVector2D walkSpeedRange = FVector2D(0.f, 600.f);
	FVector2D velocityMultiplierRange = FVector2D(0.f, 1.f);

	FVector velocity = GetVelocity();
	velocity.Z = 0;

	crosshairVelocityFactor = FMath::GetMappedRangeValueClamped(walkSpeedRange, velocityMultiplierRange, velocity.Size());

	// Calulate crosshair air factor
	if (GetCharacterMovement()->IsFalling())
	{
		//spread the crosshair slowly while in air
		crosshairInAirFactor = FMath::FInterpTo(crosshairInAirFactor, 2.25f, deltaTime, 2.25f);
	}

	else
	{
		//Shrink the crosshair quickly while on ground
		crosshairInAirFactor = FMath::FInterpTo(crosshairInAirFactor, 0, deltaTime, 30.f);
	}

	// Calulate crosshair aim factor
	if (bIsAiming)
	{
		// Quickly shrink crosshair a small amount 
		crosshairAimFactor = FMath::FInterpTo(crosshairAimFactor, -0.4f, deltaTime, 30.f);
	}

	else
	{
		// Quickly spread crosshair back to normal
		crosshairAimFactor = FMath::FInterpTo(crosshairAimFactor, 0, deltaTime, 30.f);
	}

	// true 0.05 seconds after firing
	if (bShootingBullet)
	{
		crosshairShootingFactor = FMath::FInterpTo(crosshairShootingFactor, 0.3, deltaTime, 60.f);
	}

	else
	{
		crosshairShootingFactor = FMath::FInterpTo(crosshairShootingFactor, 0, deltaTime, 60.f);
	}

	crosshairSpreadMultiplier = 0.5f + crosshairVelocityFactor + crosshairInAirFactor + crosshairAimFactor + crosshairShootingFactor;
}

// RELOADING
////////////////////////////////////////////////////
void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	if (combatState != ECombatState::ECS_Unoccupied) return;
	
	if (!equippedWeapon) return;
	// De we have ammo of the correct type
	// TODO: Create function bool CarryingAmmo

	if (CarryingAmmo() && !equippedWeapon->ClipIsFull()) // Replace with CarryingAmmo
	{
		// If your aiming, stop it
		if (bIsAiming) StopAiming();

		combatState = ECombatState::ECS_Reloading;
		if (!reloadMontage) return;

		// Plays hip fire montage
		GetAnimInstance()->Montage_Play(reloadMontage);

		// Jumps to start fire section of the montage
		GetAnimInstance()->Montage_JumpToSection(equippedWeapon->GetReloadMontageSection());
	}
}

void AShooterCharacter::GrabClip()
{
	if (!equippedWeapon && !handSceneComponent) return;

	// Index for the clip bone on the equipped weapon
	int32 clipBoneIndex = equippedWeapon->GetItemMesh()->GetBoneIndex(equippedWeapon->GetClipBoneName());

	// Stor the transform of the clip
	clipTransform =  equippedWeapon->GetItemMesh()->GetBoneTransform(clipBoneIndex);

	FAttachmentTransformRules attachmentRules(EAttachmentRule::KeepRelative, true);

	handSceneComponent->AttachToComponent(GetMesh(), attachmentRules, FName(TEXT("hand_l")));
	handSceneComponent->SetWorldTransform(clipTransform);

	equippedWeapon->SetIsMovingClip(true);

}

void AShooterCharacter::ReplaceClip()
{
	equippedWeapon->SetIsMovingClip(false);
}

void AShooterCharacter::FinishReloading()
{
	if (combatState == ECombatState::ECS_Stunned) return;
	// Set combat state to unoccupied
	combatState = ECombatState::ECS_Unoccupied;

	if (bAimingButtonPressed) Aim();

	if (!equippedWeapon) return;

	const EAmmoType ammoType = equippedWeapon->GetAmmoType();

	// Update ammo map
	if (ammoMap.Contains(ammoType))
	{
		// Amount of ammo character is caarring of the equipped weapn type
		int32 carriedAmmo = ammoMap[ammoType];

		// Space left in the mag of equiped weapon
		const int32 magEmptySpace = equippedWeapon->GetMagazineCap() - equippedWeapon->GetAmmo();

		if (magEmptySpace > carriedAmmo)
		{
			// Reload the mag with all the ammo we are carrying
			equippedWeapon->ReloadAmmo(carriedAmmo);
			carriedAmmo = 0;
			ammoMap.Add(ammoType, carriedAmmo);
		}

		else 
		{
			// Fill the mag
			equippedWeapon->ReloadAmmo(magEmptySpace);
			carriedAmmo -= magEmptySpace;
			ammoMap.Add(ammoType, carriedAmmo);
		}
	}


}
////////////////////////////////////////////////////

// WEAPON SPAWING/CHANGING
////////////////////////////////////////////////////
void AShooterCharacter::EquipWeapon(AWeapon* weaponToEquip, bool bIsSwapping)
{
	if (!weaponToEquip) return;

	// Get the hand socket
	const USkeletalMeshSocket* handSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));

	// Check if hand socket is null
	if (!handSocket) return;

	// Attach default weapon to hand socket
	handSocket->AttachActor(weaponToEquip, GetMesh());

	if (equippedWeapon == NULL)
	{
		// -1 == no Equipped weapon yet. No need to reverse the icon animation
		equipItemDelegate.Broadcast(-1, weaponToEquip->GetSlotIndex());
	}

	else if(!bIsSwapping)
	{
		equipItemDelegate.Broadcast(equippedWeapon->GetSlotIndex(), weaponToEquip->GetSlotIndex());
	}

	equippedWeapon = weaponToEquip;

	equippedWeapon->SetItemState(EItemState::EIS_Equipped);
}
void AShooterCharacter::FinishEquipping()
{
	if (combatState == ECombatState::ECS_Stunned) return;

	combatState = ECombatState::ECS_Unoccupied;

	if (bAimingButtonPressed)
	{
		Aim();
	}
}

void AShooterCharacter::SwapWeapon(AWeapon* weaponToSwap)
{
	if (inventory.Num() - 1 >= equippedWeapon->GetSlotIndex())
	{
		inventory[equippedWeapon->GetSlotIndex()] = weaponToSwap;
		weaponToSwap->SetSlotIndex(equippedWeapon->GetSlotIndex());
	}

	DropWeapon();
	EquipWeapon(weaponToSwap, true);
	traceHitItem = NULL;
	traceHitItemLastFrame = NULL;
}

void AShooterCharacter::DropWeapon()
{
	if (!equippedWeapon) return;

	FDetachmentTransformRules detachmentTransformRules(EDetachmentRule::KeepWorld, true);
	equippedWeapon->GetItemMesh()->DetachFromComponent(detachmentTransformRules);

	equippedWeapon->SetItemState(EItemState::EIS_Falling);
	equippedWeapon->ThrowWeapon();
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	// Check if default weapon class is null
	if (!defaultWeaponClass) return NULL;

	// Spawn the default weapon
	return  GetWorld()->SpawnActor<AWeapon>(defaultWeaponClass);
}
////////////////////////////////////////////////////

// AMMO
/////////////////////////////////////////
void AShooterCharacter::InitAmmoMap()
{
	// Adding ammo amounts to the ammo map
	ammoMap.Add(EAmmoType::EAT_9mm, starting9mmAmmo);
	ammoMap.Add(EAmmoType::EAT_AR, startingARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo()
{
	if (!equippedWeapon) return false;
	return equippedWeapon->GetAmmo() > 0;
}

bool AShooterCharacter::CarryingAmmo()
{
	if (!equippedWeapon) return false;

	EAmmoType ammoType = equippedWeapon->GetAmmoType();

	if (ammoMap.Contains(ammoType))
	{
		return ammoMap[ammoType] > 0;
	}

	return false;
}

void AShooterCharacter::PickupAmmo(AAmmo* ammo)
{
	if (ammoMap.Contains(ammo->GetAmmoType()))
	{
		// Get amount of ammo in ammo map for ammo's type
		int32 ammoCount = ammoMap[ammo->GetAmmoType()];
		ammoCount += ammo->GetItemAmount();

		// Set the amount of ammo in the map for this type
		ammoMap[ammo->GetAmmoType()] = ammoCount;
	}
	
	if (equippedWeapon->GetAmmoType() == ammo->GetAmmoType())
	{
		// Check if gun is empty
		if (equippedWeapon->GetAmmo() == 0)
		{
			ReloadWeapon();
		}
	}

	ammo->Destroy();
}


/////////////////////////////////////////

// Inventory
//////////////////////////////////////////
void AShooterCharacter::DefaultWeaponSlotKeyPressed()
{
	if (equippedWeapon->GetSlotIndex() == 0) return;
	ExchangeInventoryItems(equippedWeapon->GetSlotIndex(), 0);
}

void AShooterCharacter::WeaponSlot1KeyPressed()
{
	if (equippedWeapon->GetSlotIndex() == 1) return;
	ExchangeInventoryItems(equippedWeapon->GetSlotIndex(), 1);
}

//void AShooterCharacter::WeaponSlot2KeyPressed()
//{
//	if (equippedWeapon->GetSlotIndex() == 2) return;
//	ExchangeInventoryItems(equippedWeapon->GetSlotIndex(), 2);
//}

//void AShooterCharacter::WeaponSlot3KeyPressed()
//{
//	if (equippedWeapon->GetSlotIndex() == 3) return;
//	ExchangeInventoryItems(equippedWeapon->GetSlotIndex(), 3);
//}
//
//void AShooterCharacter::WeaponSlot4KeyPressed()
//{
//	if (equippedWeapon->GetSlotIndex() == 4) return;
//	ExchangeInventoryItems(equippedWeapon->GetSlotIndex(), 4);
//}
//
//void AShooterCharacter::WeaponSlot5KeyPressed()
//{
//	if (equippedWeapon->GetSlotIndex() == 5) return;
//	ExchangeInventoryItems(equippedWeapon->GetSlotIndex(), 5);
//}

void AShooterCharacter::ExchangeInventoryItems(int32 currentItemIndex, int32 newItemIndex)
{
	// Check to see if we can swap weapons
	const bool bCanExchangeItems = currentItemIndex != newItemIndex && newItemIndex < inventory.Num() 
		&& (combatState == ECombatState::ECS_Unoccupied || combatState != ECombatState::ECS_Equipping);
	
	if (combatState == ECombatState::ECS_ShootTimerInProgress) return;

	if (bCanExchangeItems)
	{
		AWeapon* oldEquippedWeapon = equippedWeapon;
		AWeapon* newWeapon = Cast<AWeapon>(inventory[newItemIndex]);

		EquipWeapon(newWeapon);
		oldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
		newWeapon->SetItemState(EItemState::EIS_Equipped);
		combatState = ECombatState::ECS_Equipping;

		if (!GetAnimInstance() && !equipMontage) return;

		GetAnimInstance()->Montage_Play(equipMontage, 1.0f);
		GetAnimInstance()->Montage_JumpToSection(FName("Equip"));

		newWeapon->PlayEquipSound(true);
	}
}

void AShooterCharacter::HighlightInventorySlot()
{
	const int32 emptySlot = GetEmptyInventorySlot();

	highlightIconDelegate.Broadcast(emptySlot, true);
	highlightedSlot = emptySlot;
}

void AShooterCharacter::UnHighlightInventorySlot()
{
	highlightIconDelegate.Broadcast(highlightedSlot, false);
	highlightedSlot = -1;
}

int32 AShooterCharacter::GetEmptyInventorySlot()
{
	for (int32 i = 0; i < inventory.Num(); ++i)
	{
		if (inventory[i] == NULL)
		{
			return i;
		}
	}

	if (inventory.Num() < inventoryCap)
	{
		return inventory.Num();
	}

	return -1; // Inventory is full
}

//////////////////////////////////////////

void AShooterCharacter::StartPickupSoundTimer()
{
	bShouldPlayPickupSound = false;

	GetWorldTimerManager().SetTimer(pickupSoundTimer, this, &AShooterCharacter::ResetPickupSoundTimer, pickupSoundsResetTime);
}

void AShooterCharacter::StartEquipSoundTimer()
{
	bShouldPlayEquipSound = false;

	GetWorldTimerManager().SetTimer(equipSoundTimer, this, &AShooterCharacter::ResetEquipSoundTimer, equipSoundsResetTime);
}
void AShooterCharacter::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
}
void AShooterCharacter::ResetPickupSoundTimer()
{
	bShouldPlayPickupSound = true;
}

// DAMAGE

///////////////////////////////////

float AShooterCharacter::TakeDamage(float damageAmount, FDamageEvent const& damageEvent, AController* eventInstigator, AActor* damageCauser)
{
	if (health - damageAmount <= 0.f)
	{
		health = 0.f;
		Die();

		AEnemyController* enemyController = Cast<AEnemyController>(eventInstigator);

		if (!enemyController) return 0;
		enemyController->GetBlackboardComponent()->SetValueAsBool(FName("CharacterDead"), true);
	}
	else
	{
		health -= damageAmount;
	}
	
	return damageAmount;
}

void AShooterCharacter::Die()
{
	bIsDead = true;
	if (!deathMontage) return;
	GetAnimInstance()->Montage_Play(deathMontage);

	APlayerController* controller = UGameplayStatics::GetPlayerController(this, 0);

	if (!controller) return;
	DisableInput(controller);
}

void AShooterCharacter::FinishDeath()
{
	GetMesh()->bPauseAnims = true;
}

void AShooterCharacter::Stun()
{
	if (health >= 0) return;
	combatState = ECombatState::ECS_Stunned;

	if (!hitReactMontage) return;
	GetAnimInstance()->Montage_Play(hitReactMontage);
}

void AShooterCharacter::EndStun()
{
	combatState = ECombatState::ECS_Unoccupied;

	if (bAimingButtonPressed)
	{
		Aim();
	}
}
////////////////////////////////////////////////////

// GETTERS
////////////////////////////////////////////////////
FVector2D AShooterCharacter::GetViewportSize()
{
	// Get current Size of viewport
	FVector2D viewportSize;
	if (!GEngine && !GEngine->GameViewport) return false;

	GEngine->GameViewport->GetViewportSize(viewportSize);
	return viewportSize;
}

void AShooterCharacter::GetPickupItem(AItem* item)
{
	item->PlayEquipSound();

	auto weapon = Cast<AWeapon>(item);

	if (weapon)
	{
		if (inventory.Num() < inventoryCap) // Room in inventory, add weapon to inventory
		{
			weapon->SetSlotIndex(inventory.Num());
			inventory.Add(weapon);
			weapon->SetItemState(EItemState::EIS_PickedUp);
		}

		else // No room in inventory, Swap Weapon
		{
			SwapWeapon(weapon);
		}
	}

	auto ammo = Cast<AAmmo>(item);
	if (ammo)
	{
		PickupAmmo(ammo);
	}
}

int32 AShooterCharacter::GetInterpLocationIndex()
{
	int32 lowestIndex = 1;
	int32 lowestAmount = INT_MAX;

	for (int32 i = 1; i < interpLocations.Num(); ++i)
	{
		if (interpLocations[i].itemAmount < lowestAmount)
		{
			lowestIndex = i;
			lowestAmount = interpLocations[i].itemAmount;
		}
	}
	return lowestIndex;
}

FInterpLocation AShooterCharacter::GetInterpLocation(int32 index)
{
	if (index <= interpLocations.Num())
	{
		return interpLocations[index];
	}

	return FInterpLocation();
}

bool AShooterCharacter::GetTrailEndLocation(const FVector& muzzleSocketLocation, FHitResult& outHitResult)
{
	FVector outTrailLocation;
	FHitResult crosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshair(crosshairHitResult, outTrailLocation, bulletTraceRange);

	if (bCrosshairHit)
	{
		outTrailLocation = crosshairHitResult.Location;
	}

	// Perform second trace from gun barrel
	const FVector weaponTraceStart = muzzleSocketLocation;
	const FVector StartToEnd = outTrailLocation - muzzleSocketLocation;
	const FVector weaponTraceEnd = muzzleSocketLocation + StartToEnd * 1.25f;

	GetWorld()->LineTraceSingleByChannel(outHitResult, weaponTraceStart, weaponTraceEnd, ECollisionChannel::ECC_Visibility);

	if (!outHitResult.bBlockingHit)
	{
		outHitResult.Location = outTrailLocation;
		
		return false;
	}

	return true;
}

////////////////////////////////////////////////////