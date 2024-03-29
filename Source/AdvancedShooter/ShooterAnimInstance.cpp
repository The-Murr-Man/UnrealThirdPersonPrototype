// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include <GameFramework/CharacterMovementComponent.h>
#include <Kismet/KismetMathLibrary.h>

UShooterAnimInstance::UShooterAnimInstance()
{

}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	// Assign shooter character to shooter character pawn
	shooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::UpdateAnimProperties(float deltaTime)
{
	// If shooter character is null, reassign shooter character
	if (!shooterCharacter) shooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());

	// If shooter character is still null, return
	if (!shooterCharacter) return;
	
	bIsCrouching = shooterCharacter->GetIsCrouching();
	bIsReloading = shooterCharacter->GetCombatState() == ECombatState::ECS_Reloading;
	bIsEquipping = shooterCharacter->GetCombatState() == ECombatState::ECS_Equipping;
	bShouldUseFABRIKPoses = shooterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied || 
							shooterCharacter->GetCombatState() == ECombatState::ECS_ShootTimerInProgress;

	// Get lateral speed of character from velocity
	FVector velocity = shooterCharacter->GetVelocity();
	velocity.Z = 0;
	moveSpeed = velocity.Size();

	// Check if character in air using character movement comp
	bIsInAir = shooterCharacter->GetCharacterMovement()->IsFalling();

	// Check if character is moving
	if (shooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0)
		bIsAccelerating = true;
	else 
		bIsAccelerating = false;

	FRotator aimRotation = shooterCharacter->GetBaseAimRotation();
	FRotator movementRotation = UKismetMathLibrary::MakeRotFromX(shooterCharacter->GetVelocity());

	movementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(movementRotation, aimRotation).Yaw;

	if (shooterCharacter->GetVelocity().Size() > 0.f)
	{
		lastMovementOffsetYaw = movementOffsetYaw;
	}
	
	bIsAiming = shooterCharacter->GetIsAiming();

	if (bIsReloading)
	{
		offsetState = EOffsetState::EOS_Reloading;
	}

	else if (bIsInAir)
	{
		offsetState = EOffsetState::EOS_InAir;
	}

	else if (shooterCharacter->GetIsAiming())
	{
		offsetState = EOffsetState::EOS_Aiming;
	}

	else
	{
		offsetState = EOffsetState::EOS_Hip;
	}

	TurnInPlace();
	Lean(deltaTime);

	// CHeck if shooter char has a valid equipped weapon
	if (!shooterCharacter->GetEquippedWeapon()) return;
	equippedWeaponType = shooterCharacter->GetEquippedWeapon()->GetWeaponType();

}

void UShooterAnimInstance::TurnInPlace()
{
	pitch = shooterCharacter->GetBaseAimRotation().Pitch;

	if (moveSpeed > 0 || bIsInAir)
	{
		rootYawOffset = 0.f;

		TIPCharacterYaw = shooterCharacter->GetActorRotation().Yaw;
		TIPCharacterYawLastFrame = TIPCharacterYaw;

		rotationCurveLastFrame = 0.f;
		rotationCurve = 0.f;
	}

	else
	{
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		TIPCharacterYaw = shooterCharacter->GetActorRotation().Yaw;

		const float TIPYawDelta = TIPCharacterYaw - TIPCharacterYawLastFrame;

		// Rootyawoffset updated and clamped to [-180, 180]
		rootYawOffset = UKismetMathLibrary::NormalizeAxis(rootYawOffset - TIPYawDelta);

		// Gets and sets curve value of turning curve (1.0 if turning, 0.0 if not)
		const float turning = GetCurveValue(TEXT("Turning"));

		if (turning > 0)
		{
			bIsTurningInPlace = true;
			rotationCurveLastFrame = rotationCurve;
			rotationCurve = GetCurveValue(TEXT("Rotation"));

			const float deltaRotation = rotationCurve - rotationCurveLastFrame;

			// Rootyawoffset > 0, -> Turning left, Rootyawoffset < 0, -> Turning Right
			rootYawOffset > 0 ? rootYawOffset -= deltaRotation : rootYawOffset += deltaRotation; // Ternary operator

			// Get absulte value of root yaw offset
			const float ABSRootYawOffset = FMath::Abs(rootYawOffset);

			if (ABSRootYawOffset > 90)
			{
				// Get the yaw excess
				const float yawExcess = ABSRootYawOffset - 90.f;
				rootYawOffset > 0 ? rootYawOffset -= yawExcess : rootYawOffset += yawExcess;
			}
			else
				bIsTurningInPlace = false;

			
		}
	}

	SetRecoilWeight();
}

void UShooterAnimInstance::Lean(float deltaTime)
{
	if (!shooterCharacter) return;

	characterRotationLastFrame = characterRotation;
	characterRotation = shooterCharacter->GetActorRotation();

	const FRotator delta = UKismetMathLibrary::NormalizedDeltaRotator(characterRotation, characterRotationLastFrame);

	const float target = delta.Yaw / deltaTime;

	const float interp = FMath::FInterpTo(yawDelta, target, deltaTime, 6.f);

	yawDelta = FMath::Clamp(interp, -90.f, 90.f);
}

void UShooterAnimInstance::SetRecoilWeight()
{
	if (bIsTurningInPlace)
	{
		if (bIsReloading || bIsEquipping)
			recoilWeight = 1.f;
		else
			recoilWeight = 0.f;
	}

	else // Not turning in place
	{
		if (bIsCrouching)
		{
			if (bIsReloading || bIsEquipping)
				recoilWeight = 1.f;
			else
				recoilWeight = 0.1f;
		}

		else
		{
			if (bIsAiming || bIsReloading || bIsEquipping)
				recoilWeight = 1.f;
			else
				recoilWeight = 0.5f;
		}
	}
}
