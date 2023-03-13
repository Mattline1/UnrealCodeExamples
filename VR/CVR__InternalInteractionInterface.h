// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CVR__InternalInteractionInterface.generated.h"

USTRUCT(BlueprintType)
struct F__InternalInteractionParameters
{
	GENERATED_BODY()

	UPROPERTY()
	uint32 bIsGrabbed : 1;

	UPROPERTY()
	AActor* Outer;
};

UDELEGATE(BlueprintCallable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractionDelegate, const F__InternalInteractionParameters&, InteractionParams);

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UCVR__InternalInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class CVR_PLUGIN_API ICVR__InternalInteractionInterface
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual void __OnTriggerPress(const F__InternalInteractionParameters& InteractionParams) {};

	UFUNCTION()
	virtual void __OnTriggerRelease(const F__InternalInteractionParameters& InteractionParams) {};

	UFUNCTION()
	virtual void __OnTriggerAxisUpdated(const float axis, const F__InternalInteractionParameters& InteractionParams) {};
};
