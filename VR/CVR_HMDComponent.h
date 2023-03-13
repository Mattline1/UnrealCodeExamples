// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "CVR_HMDComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (VirtualReality), Blueprintable, meta = (BlueprintSpawnableComponent))
class CVR_PLUGIN_API UCVR_HMDComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	UCVR_HMDComponent();

protected:
	virtual void BeginPlay() override;

public:
	/** Moves controls to VR mode. */
	void OnEnterVR();

	/** Moves controls to Desktop mode. */
	void OnExitVR();

	/** Resets HMD orientation and position in VR. */
	UFUNCTION(BlueprintCallable)
	void ResetVR();
};
