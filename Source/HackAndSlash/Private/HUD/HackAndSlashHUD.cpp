// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/HackAndSlashHUD.h"
#include "HUD/HackAndSlashOverlay.h"

void AHackAndSlashHUD::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* Controller = World->GetFirstPlayerController();
		if (Controller && HackAndSlashOverlayClass)
		{
			Overlay = CreateWidget<UHackAndSlashOverlay>(Controller, HackAndSlashOverlayClass);
			Overlay->AddToViewport();
		}
	}
}
