// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/* Engine */
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Components/InputComponent.h"

/* Plugin */
#include "CVR__const_types.h"
#include "CVR__InternalInputBindingsInterface.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UCVR__InternalInputBindingsInterface : public UInterface
{
	GENERATED_BODY()
};

class CVR_PLUGIN_API ICVR__InternalInputBindingsInterface
{
	GENERATED_BODY()

public:
	UFUNCTION()
	virtual void __BindDefaultActionsToInputComponent(UInputComponent* PlayerInputComponent) {};

	UFUNCTION()
	virtual bool __UseDefaultActions() { return true; };
};