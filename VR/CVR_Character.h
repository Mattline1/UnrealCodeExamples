// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/*Engine*/
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HeadMountedDisplayFunctionLibrary.h"

/*Plugin*/
#include "CVR__const_types.h"
#include "CVR_HMDComponent.h"
#include "CVR_MotionControllerComponent.h"
#include "CVR_VolumeComponent.h"
#include "CVR_Character.generated.h"

UCLASS(config = Game, BlueprintType, meta = (ShortTooltip = "A VR character is a type of Pawn that natively supports virtual reality."))
class CVR_PLUGIN_API ACVR_Character : public ACharacter
{
	GENERATED_BODY()

	/** Core Components */
public:
	/** VR volume component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	class UCVR_VolumeComponent* VolumeComponent;

	/** HMD camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	class UCVR_HMDComponent* HMDComponent;

	/** Motion controller component (right hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	class UCVR_MotionControllerComponent* MotionControllerRight;

	/** Motion controller component (left hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	class UCVR_MotionControllerComponent* MotionControllerLeft;

	/** Widget interaction component*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	class UWidgetInteractionComponent* WidgetInteractionComponent;

	/*
	* inline getters for private components
	*/
public:
	/** Returns FirstPersonCameraComponent Component **/
	FORCEINLINE class UCVR_VolumeComponent* GetVolumeComponent() const { return VolumeComponent; }

	/** Returns FirstPersonCameraComponent Component **/
	FORCEINLINE class UCVR_HMDComponent* GetHMDComponent() const { return HMDComponent; }

	/** Returns Right Motion Controller Component **/
	FORCEINLINE class UCVR_MotionControllerComponent* GetRightMotionControllerComponent() const { return MotionControllerRight; }

	/** Returns Left Motion Controller Component **/
	FORCEINLINE class UCVR_MotionControllerComponent* GetLeftMotionControllerComponent() const { return MotionControllerLeft; }

	/** Returns Widget Interaction Component **/
	FORCEINLINE class UWidgetInteractionComponent* GetWidgetInteractionComponent() const { return WidgetInteractionComponent; }


public:
	/** Which hand is dominant, or whether to adapt based on usage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Motion)
	EHandedness Handedness;

public:
	// Sets default values for this character's properties
	ACVR_Character();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Moves controls to VR mode. */
	void OnEnterVR();

	/** Moves controls to Desktop mode. */
	void OnExitVR();

	/** Press widget interaction pointer (right handed) */
	void PressPointerRight();

	/** Press widget interaction pointer (left handed) */
	void PressPointerLeft();

	/** Release widget interaction pointer (right handed) */
	void ReleasePointerRight();

	/** Release widget interaction pointer (left handed) */
	void ReleasePointerLeft();

	/** Moves controls to VR mode. */
	void ResetHandedness();

	/** Resets HMD orientation and position in VR. */
	void ResetVR();
};
