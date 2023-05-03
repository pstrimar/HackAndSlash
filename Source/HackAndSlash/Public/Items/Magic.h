// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Magic.generated.h"

/**
 * 
 */
UCLASS()
class HACKANDSLASH_API AMagic : public AItem
{
	GENERATED_BODY()

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere, Category = "Magic Properties")
	int32 Magic;

public:
	FORCEINLINE int32 GetMagic() const { return Magic; }
	FORCEINLINE void SetMagic(int32 AmountOfMagic) { Magic = AmountOfMagic; }
	
};
