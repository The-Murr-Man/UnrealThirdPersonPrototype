// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerController.h"
#include <Components/WidgetComponent.h>

AShooterPlayerController::AShooterPlayerController()
{

}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Check HUDOverlay class var

	if (!HUDOverlayClass) return;
	HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayClass);

	if (!HUDOverlay) return;
	HUDOverlay->AddToViewport();
	HUDOverlay->SetVisibility(ESlateVisibility::Visible);
}