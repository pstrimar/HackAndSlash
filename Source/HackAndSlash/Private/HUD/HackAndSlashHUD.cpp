// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/HackAndSlashHUD.h"
#include "HUD/HackAndSlashOverlay.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

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

void AHackAndSlashHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
    const float TextureWidth = Texture->GetSizeX();
    const float TextureHeight = Texture->GetSizeY();
    const FVector2D TextureDrawPoint(
        ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
        ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
    );

    DrawTexture(
        Texture,
        TextureDrawPoint.X,
        TextureDrawPoint.Y,
        TextureWidth,
        TextureHeight,
        0.f,
        0.f,
        1.f,
        1.f,
        CrosshairColor
    );
}

void AHackAndSlashHUD::DrawHUD()
{
    Super::DrawHUD();

    FVector2D ViewportSize;
    if (GEngine)
    {
        GEngine->GameViewport->GetViewportSize(ViewportSize);
        const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

        float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

        if (HUDPackage.CrosshairsCenter)
        {
            FVector2D Spread(0.f, 0.f);
            DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
        }
        if (HUDPackage.CrosshairsLeft)
        {
            FVector2D Spread(-SpreadScaled, 0.f);
            DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
        }
        if (HUDPackage.CrosshairsRight)
        {
            FVector2D Spread(SpreadScaled, 0.f);
            DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
        }
        if (HUDPackage.CrosshairsTop)
        {
            FVector2D Spread(0.f, -SpreadScaled);
            DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
        }
        if (HUDPackage.CrosshairsBottom)
        {
            FVector2D Spread(0.f, SpreadScaled);
            DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
        }
    }
}

void AHackAndSlashHUD::ShowDeathScreen()
{
    UWorld* World = GetWorld();
    if (World)
    {
        APlayerController* Controller = World->GetFirstPlayerController();
        if (Controller && DeathScreenClass)
        {
            if (Overlay)
            {
                Overlay->RemoveFromParent();
            }
            UUserWidget* DeathScreenOverlay = CreateWidget<UUserWidget>(Controller, DeathScreenClass);
            DeathScreenOverlay->AddToViewport();
            UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(Controller, DeathScreenOverlay);
            Controller->SetShowMouseCursor(true);
        }
    }
}

void AHackAndSlashHUD::ShowWinScreen()
{
    UWorld* World = GetWorld();
    if (World)
    {
        APlayerController* Controller = World->GetFirstPlayerController();
        if (Controller && WinScreenClass)
        {
            if (Overlay)
            {
                Overlay->RemoveFromParent();
            }
            UUserWidget* WinScreenOverlay = CreateWidget<UUserWidget>(Controller, WinScreenClass);
            WinScreenOverlay->AddToViewport();
            UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(Controller, WinScreenOverlay);
            Controller->SetShowMouseCursor(true);
        }
    }
}
