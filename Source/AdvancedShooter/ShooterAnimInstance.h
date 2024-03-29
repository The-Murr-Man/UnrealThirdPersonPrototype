// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Items/Weapon.h"
#include "ShooterAnimInstance.generated.h"

class AShooterCharacter;

UENUM(BlueprintType)
enum class EOffsetState : uint8
{
	EOS_Aiming UMETA(DisplayName = "Aiming"),
	EOS_Hip UMETA(DisplayName = "Hip"),
	EOS_Reloading UMETA(DisplayName = "Reloading"),
	EOS_InAir UMETA(DisplayName = "In Air"),

	EOS_MAX UMETA(DisplayName = "Default Max")
};

UCLASS()
class ADVANCEDSHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UShooterAnimInstance();
	UFUNCTION(BlueprintCallable)
	void UpdateAnimProperties(float deltaTime);
	virtual void NativeInitializeAnimation() override;

protected:

	// Handle turning in place variables
	void TurnInPlace();

	// Handle lean calculations while running
	void Lean(float deltaTime);

	void SetRecoilWeight();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	AShooterCharacter* shooterCharacter;

	// The speed of the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float moveSpeed = 0.f;

	// Change  the recoil weight
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float recoilWeight = 1.f;

	// Check if character is in air
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsInAir =false;

	// check if character is moving
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating = false;

	// True when turning in place
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsTurningInPlace = false;

	// Offset yaw used for strafing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float movementOffsetYaw = 0.f;

	
	// Offset yaw frame before we stop moving
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float lastMovementOffsetYaw = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsAiming = false;

	// Turn in place Yaw of the character this frame; Only updated if character is standing still or is in air
	float TIPCharacterYaw = 0.f;

	// Turn in place Yaw of the character in the previous frame; Only updated if character is standing still or is in air
	float TIPCharacterYawLastFrame = 0.f;

	// Yaw of the character this frame;
	FRotator characterRotation = FRotator::ZeroRotator;

	//  Yaw of the character in the previous frame;
	FRotator characterRotationLastFrame = FRotator::ZeroRotator;

	// Yaw delta used for leaning in running BSP
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Lean", meta = (AllowPrivateAccess = "true"))
	float yawDelta = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	float rootYawOffset = 0.f;

	// Rotation curve value this frame
	float rotationCurve = 0.f;

	// Rotation curve value last frame
	float rotationCurveLastFrame = 0.f;

	// The pitch of aim rotation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	float pitch = 0.f;

	// True when reloading, prevents aim offset while reloading
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	bool bIsReloading = false;

	// True when crouching
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crouch", meta = (AllowPrivateAccess = "true"))
	bool bIsCrouching = false;

	// True when equipping
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsEquipping = false;

	// Offset state used to determine which state offset to use
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	EOffsetState offsetState = EOffsetState::EOS_Hip;

	// Weapon type for the currently equipped weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	EWeaponType equippedWeaponType = EWeaponType::EWT_MAX;

	// True when we want to use FABRIK
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bShouldUseFABRIKPoses = false;
};
