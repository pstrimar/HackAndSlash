// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TargetLockInterface.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTargetDeath);

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTargetLockInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class HACKANDSLASH_API ITargetLockInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void ShowTargetLock() = 0;
	virtual void HideTargetLock() = 0;	
	virtual FOnTargetDeath* GetOnTargetDeath() = 0;
};
