// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HackAndSlashHUD.generated.h"

class UHackAndSlashOverlay;

UCLASS()
class HACKANDSLASH_API AHackAndSlashHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = HackAndSlash)
	TSubclassOf<UHackAndSlashOverlay> HackAndSlashOverlayClass;

	UPROPERTY()
	UHackAndSlashOverlay* Overlay;

public:
	FORCEINLINE UHackAndSlashOverlay* GetOverlay() const { return Overlay; }
};
