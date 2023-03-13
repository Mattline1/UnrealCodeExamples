// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/* Engine */
#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "Haptics/HapticFeedbackEffect_Base.h"
#include "MotionControllerComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

/* Plugin */
#include "CVR__const_types.h"
#include "CVR_MotionControllerComponent.generated.h"

#define DRAW_CONTROLLER_DEBUG 1

//
UCLASS(ClassGroup = (VirtualReality), meta = (BlueprintSpawnableComponent))
class CVR_PLUGIN_API UCVR_MotionControllerComponent : public UMotionControllerComponent
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	class USphereComponent* GrabLocationComponent;

protected:
	UPROPERTY()
	AActor* _HighlightedActor;

	UPROPERTY()
	AActor* HandActor;

	UPROPERTY()
	uint32 bHasAttemptedGrab : 1;

	UPROPERTY()
	float GrabAxisPriorValue;

	UPROPERTY(Transient, meta = (EditCondition = false, EditConditionHides))
	class UPhysicsHandleComponent* PhysicsHandleComponent;

	// MotionControllerComponents need to know if another controller is grabbing the same object,
	// this is so they can properly call grab/release functions on those objects
	static TMap<UCVR_MotionControllerComponent*, FGrabParams> GrabParams;

	TArray<TPair<float, FVector>> trackedPositions;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction")
	TSubclassOf<AActor> HandActorClass;

	// the collision channel this Motion Controller will use to interact with objects
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction")
		TEnumAsByte<ECollisionChannel> InteractionChannel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction")
		float GrabRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction")
		FVector GrabLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction")
		uint32 bGravityGrab : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction")
		float GravityGrabDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction")
		float GravityGrabRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction")
		float GravityGrabStrength;

	// the socket used for actors that try to snap
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, AdvancedDisplay, Category = "Interaction")
		FName SnapSocketName;

	// the socket used for actors that try to snap
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, AdvancedDisplay, Category = "Interaction")
	UHapticFeedbackEffect_Base* GrabHaptic;

protected:
	UCVR_MotionControllerComponent();

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
	virtual void PostLoad() override;

	// Called when the game starts
	virtual void BeginPlay() override;

	// find colliding object to grab
	// or trace to find an object to grab if bAllowGravityGrab == true
	FGrabResult GetGrabTarget();

	bool OverlapMultiByChannel(bool isGravityGrab, TArray<FOverlapResult>& OverlapResults);

	// find colliding object to grab
	// or trace to find an object to grab if bAllowGravityGrab == true
	USceneComponent* GetCompoundComponent(USceneComponent* current);

	bool GetCompoundComponent(FGrabParams& _GrabParams);

	FTransform GetGrabCapsule();

	USceneComponent* FindInterfaceComponentInActor(USceneComponent* current, FGrabResult GrabResult);

	void MoveComponent(FGrabParams& _GrabParams);

	void TrackGrabbedObjectMovementVelocity(FVector newLocation);

	void TryMoveCompoundComponentRecursive(FGrabParams& _GrabParams);

	void TryMoveGrabbedComponent(FGrabParams& _GrabParams);

	void UpdateSphereComponent();

public:
	/**
	* Get the averaged velocity of an object over the last 0.3 seconds.
	* This helps to even out instant changes in velocity that are possible in VR but impossible in reality.
	* Without this things like throwing objects can feel unnatural.
	*/
	FVector GetGrabbedActorVelocity();

	AActor* GetHandActor();

	// attempt to grab/release objects
	void GrabAxis(float Value);

	bool IsActorConstrained();

	bool IsActorAlreadyGrabbed(FGrabParams& _GrabParams);

	bool IsGrabbingActor();

	void ReleaseGrabbedActor(bool bHeldByOtherController, bool bPreservePhysics = true);

	void ReleasePhysicsConstraint(USceneComponent* component, bool bHeldByOtherController, bool bWithPhysics);

	void SetGrabbedActor(FGrabResult GrabResult);

	void SetPhysicsConstraint(USceneComponent* component, FGrabParams& _GrabParams);

	// release trigger
	void ReleaseInteraction();

	// trigger trigger
	void TriggerInteraction();

	static bool StaticForceGrabRelease(AActor* actorToRelease);

	static void StaticReleaseGrabbedActor(UCVR_MotionControllerComponent* motionController, AActor* actorToRelease);

	UFUNCTION(BlueprintCallable)
	void PlayHapticEffect(UHapticFeedbackEffect_Base* Haptic, float Scale = 1.f, bool bLoop = false);

	UFUNCTION(BlueprintCallable)
	EControllerHand GetControllerHand();
};
