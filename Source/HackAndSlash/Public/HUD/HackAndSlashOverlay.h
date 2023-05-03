// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HackAndSlashOverlay.generated.h"

/**
 * 
 */
UCLASS()
class HACKANDSLASH_API UHackAndSlashOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	void SetHealthBarPercent(float Percent);
	void SetMagicBarPercent(float Percent);
	void SetStaminaBarPercent(float Percent);
	void SetGold(int32 Gold);
	void SetSouls(int32 Souls);

private:
	void SetHealthBarBackgroundPercent(float Percent);
	void SetMagicBarBackgroundPercent(float Percent);
	void SetStaminaBarBackground(float Percent);

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthProgressBar;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthProgressBarBackground;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* MagicProgressBar;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* MagicProgressBarBackground;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* StaminaProgressBar;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* StaminaProgressBarBackground;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* GoldText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SoulsText;
	
};
