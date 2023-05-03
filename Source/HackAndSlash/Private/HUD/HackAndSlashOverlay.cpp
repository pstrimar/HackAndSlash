// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/HackAndSlashOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UHackAndSlashOverlay::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	if (HealthProgressBarBackground && HealthProgressBar && HealthProgressBarBackground->GetPercent() != HealthProgressBar->GetPercent())
	{
		const float Percent = FMath::FInterpTo(HealthProgressBarBackground->GetPercent(), HealthProgressBar->GetPercent(), InDeltaTime, 4.f);
		HealthProgressBarBackground->SetPercent(Percent);
	}
	if (MagicProgressBarBackground && MagicProgressBar && MagicProgressBarBackground->GetPercent() != MagicProgressBar->GetPercent())
	{
		const float Percent = FMath::FInterpTo(MagicProgressBarBackground->GetPercent(), MagicProgressBar->GetPercent(), InDeltaTime, 4.f);
		MagicProgressBarBackground->SetPercent(Percent);
	}
	if (StaminaProgressBarBackground && StaminaProgressBar && StaminaProgressBarBackground->GetPercent() != StaminaProgressBar->GetPercent())
	{
		const float Percent = FMath::FInterpTo(StaminaProgressBarBackground->GetPercent(), StaminaProgressBar->GetPercent(), InDeltaTime, 4.f);
		StaminaProgressBarBackground->SetPercent(Percent);
	}
}

void UHackAndSlashOverlay::SetHealthBarPercent(float Percent)
{
	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(Percent);
	}
}

void UHackAndSlashOverlay::SetHealthBarBackgroundPercent(float Percent)
{
	if (HealthProgressBarBackground)
	{
		HealthProgressBarBackground->SetPercent(Percent);
	}
}

void UHackAndSlashOverlay::SetMagicBarPercent(float Percent)
{
	if (MagicProgressBar)
	{
		MagicProgressBar->SetPercent(Percent);
	}
}

void UHackAndSlashOverlay::SetMagicBarBackgroundPercent(float Percent)
{
	if (MagicProgressBarBackground)
	{
		MagicProgressBarBackground->SetPercent(Percent);
	}
}

void UHackAndSlashOverlay::SetStaminaBarPercent(float Percent)
{
	if (StaminaProgressBar)
	{
		StaminaProgressBar->SetPercent(Percent);
	}
}

void UHackAndSlashOverlay::SetStaminaBarBackground(float Percent)
{
	if (StaminaProgressBarBackground)
	{
		StaminaProgressBarBackground->SetPercent(Percent);
	}
}

void UHackAndSlashOverlay::SetGold(int32 Gold)
{
	if (GoldText)
	{
		GoldText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Gold)));
	}
}

void UHackAndSlashOverlay::SetSouls(int32 Souls)
{
	if (SoulsText)
	{
		SoulsText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Souls)));
	}
}
