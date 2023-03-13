// Fill out your copyright notice in the Description page of Project Settings.

#include "CVR_MotionControllerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "XRMotionControllerBase.h"
#include "Components/SphereComponent.h"

#if DRAW_CONTROLLER_DEBUG
#include "DrawDebugHelpers.h"
#endif

#include "CVR_DefaultHandActor.h"
#include "Interfaces/CVR__InternalInteractionInterface.h"
#include "Interfaces/CVR__InternalGrabInterface.h"

TMap<UCVR_MotionControllerComponent*, FGrabParams> UCVR_MotionControllerComponent::GrabParams;

UCVR_MotionControllerComponent::UCVR_MotionControllerComponent()
{
	InteractionChannel = ECollisionChannel::ECC_WorldDynamic;
	bGravityGrab = false;
	bHasAttemptedGrab = false;
	GrabAxisPriorValue = 0.0f;
	GrabHaptic = nullptr;
	GrabRadius = 7.f;
	GrabLocation = FVector::ZeroVector;
	GravityGrabDistance = 1.f;
	GravityGrabStrength = 1.f;
	GravityGrabRadius = 30.f;
	SnapSocketName = "";

	_HighlightedActor = nullptr;
	HandActor = nullptr;

	SetGenerateOverlapEvents(false);
	SetCollisionProfileName("NoCollision");
	CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	PhysicsHandleComponent = CreateDefaultSubobject<UPhysicsHandleComponent>("PhysicsHandleComponent");

	GrabLocationComponent = CreateDefaultSubobject<USphereComponent>(TEXT("GrabColliderComponent"), true);
	GrabLocationComponent->SetupAttachment(this);
	GrabLocationComponent->SetCollisionProfileName("OverlapAllDynamic");
	GrabLocationComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GrabLocationComponent->SetSphereRadius(GrabRadius);

	GrabParams.Empty(0);
}

// Called when the game starts
void UCVR_MotionControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	// update static grab parameters
	FGrabParams _GrabParams;
	_GrabParams.Outer = GetOwner();
	_GrabParams.MotionControllerComponent = this;
	_GrabParams.GrabbedActor = NULL;
	_GrabParams.GrabbedComponent = NULL;
	_GrabParams.InterfaceComponent = NULL;
	_GrabParams.RotationDelta = FQuat::Identity;
	_GrabParams.TranslationDelta = FVector::ZeroVector;
	_GrabParams.bWasSimulating = false;
	_GrabParams.bIsAttached = false;
	_GrabParams.bIsWelded = false;

	GrabParams.Add(this, _GrabParams);

	if (HandActorClass == nullptr)
	{
		HandActorClass = ACVR_DefaultHandActor::StaticClass();
		this->SetShowDeviceModel(true);
	}

	FVector Location = GetComponentLocation();
	FRotator Rotation = GetComponentRotation();

	FActorSpawnParameters params = FActorSpawnParameters();
	params.bNoFail = true;

	HandActor = GetWorld()->SpawnActor(
		HandActorClass.Get(),
		&Location,
		&Rotation,
		params
	);

	HandActor->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
}

USceneComponent* UCVR_MotionControllerComponent::FindInterfaceComponentInActor(USceneComponent* current, FGrabResult GrabResult)
{
	if (current)
	{
		ICVR__InternalGrabInterface* in = Cast<ICVR__InternalGrabInterface>(current);

		if (in)
		{
			if (in->__ShouldIgnoreGrab(GrabResult) == false)
			{
				return current;
			}
			if (in->__ShouldIgnoreGravityGrab(GrabResult) && GrabResult.bIsGravityGrab)
			{
				return current;
			}
		}

		return FindInterfaceComponentInActor(current->GetAttachParent(), GrabResult);
	}
	return NULL;
}

USceneComponent* UCVR_MotionControllerComponent::GetCompoundComponent(USceneComponent* current)
{
	if (current)
	{
		ICVR__InternalGrabInterface* in = Cast<ICVR__InternalGrabInterface>(current);

		if (in && in->__IsCompound())
		{
			return current;
		}
		else
		{
			return GetCompoundComponent(current->GetAttachParent());
		}
	}
	return NULL;
}

bool UCVR_MotionControllerComponent::GetCompoundComponent(FGrabParams& _GrabParams)
{
	if (_GrabParams.IsValid())
	{
		_GrabParams.InterfaceComponent = GetCompoundComponent(_GrabParams.InterfaceComponent);
		return true;
	}
	return false;
}

EControllerHand UCVR_MotionControllerComponent::GetControllerHand()
{
	if (MotionSource == FXRMotionControllerBase::RightHandSourceId)
	{
		return EControllerHand::Right;
	}
	if (MotionSource == FXRMotionControllerBase::LeftHandSourceId)
	{
		return EControllerHand::Left;
	}
	return EControllerHand::AnyHand;
}

FGrabResult UCVR_MotionControllerComponent::GetGrabTarget()
{
	FOverlapResult optimalResult;
	float currentMinimumDistance = MAX_flt;
	bool bHasFoundValidResult = false;
	bool bGravityGrabWasUsed = false;

	// test grabs
	TArray<FOverlapResult> GrabResults;
	OverlapMultiByChannel(false, GrabResults);

	for (auto const& R : GrabResults)
	{
		FGrabResult GrabResult = FGrabResult::Identity();
		GrabResult.bIsGravityGrab = false;

		if (!FindInterfaceComponentInActor(R.Component.Get(), GrabResult)){ continue; }

		FVector OutPointOnBody;
		float dist = R.GetComponent()->GetClosestPointOnCollision(GetComponentLocation(), OutPointOnBody);
		dist = dist < 0.f ? MAX_flt : dist;
		//float dist = FVector::Dist(R.GetComponent()->GetComponentLocation(), GetComponentLocation());

		if (dist < currentMinimumDistance)
		{
			currentMinimumDistance = dist;
			optimalResult = R;
			bHasFoundValidResult = true;
		}
	}

	if (bGravityGrab && bHasFoundValidResult == false)
	{
		TArray<FOverlapResult> GravityGrabResults;
		OverlapMultiByChannel(true, GravityGrabResults);

		// test gravity grabs
		for (auto const& R : GravityGrabResults)
		{
			FGrabResult GrabResult = FGrabResult::Identity();
			GrabResult.bIsGravityGrab = bGravityGrab;

			if (!FindInterfaceComponentInActor(R.Component.Get(), GrabResult)) { continue; }

			// reject off angle gravity grab
			FVector VectToObject = (R.GetComponent()->GetComponentLocation() - GetComponentLocation()).GetSafeNormal();
			if (FVector::DotProduct(VectToObject, GetForwardVector()) < 0.0) { continue; }

			// distance to central line
			FVector location = UKismetMathLibrary::FindClosestPointOnSegment(
				R.GetComponent()->GetComponentLocation(),
				GetComponentLocation(),
				GetComponentLocation() + GetForwardVector() * GravityGrabDistance
			);

			float LineDist = FVector::Dist(R.GetComponent()->GetComponentLocation(), location);

			FVector OutPointOnBody;
			float AbsoluteDist = R.GetComponent()->GetClosestPointOnCollision(GetComponentLocation(), OutPointOnBody);
			AbsoluteDist = AbsoluteDist <= 0.f ? MAX_flt : AbsoluteDist;

			// the distance to the central line is the dominant factor in a gravity grab
			float dist = FMath::Lerp(LineDist, AbsoluteDist, 0.05f);

			if (dist < currentMinimumDistance)
			{
				currentMinimumDistance = dist;
				optimalResult = R;
			}

			bHasFoundValidResult = true;
			bGravityGrabWasUsed = true;
		}
	}

	if (bHasFoundValidResult == false) { FGrabResult::Identity(); }

	FGrabResult GrabResult = FGrabResult::Identity();
	GrabResult.InterfaceComponent = FindInterfaceComponentInActor(optimalResult.Component.Get(), GrabResult);

	if (GrabResult.InterfaceComponent != NULL)
	{
		GrabResult.Actor = optimalResult.Actor.Get();
		GrabResult.Component = optimalResult.Component.Get();
		GrabResult.bIsGravityGrab = bGravityGrabWasUsed;

		return GrabResult;
	}

	return FGrabResult::Identity();
}

FVector UCVR_MotionControllerComponent::GetGrabbedActorVelocity()
{
	int numVectors = trackedPositions.Num() - 1;
	if (numVectors < 1)
	{
		GetComponentVelocity();
	}

	FVector SumVelocities = FVector::ZeroVector;

	for (int i = 0; i < numVectors; i++)
	{
		SumVelocities += trackedPositions[i + 1].Value - trackedPositions[i].Value;
	}
	//SumVelocities /= numVectors;

	return SumVelocities;
}

FTransform UCVR_MotionControllerComponent::GetGrabCapsule()
{
	return FTransform(GetUpVector(), GetRightVector(), GetForwardVector(),
		GetComponentLocation() + GetForwardVector() * (GravityGrabDistance * 0.5f));
}

AActor* UCVR_MotionControllerComponent::GetHandActor()
{
	return HandActor;
}

void UCVR_MotionControllerComponent::GrabAxis(float Value)
{
	// cannot grab or release anything with a welded object
	if (GrabParams[this].bIsWelded)
	{
		return;
	}

	float gradient = Value - GrabAxisPriorValue;
	if (Value <= (gradient > 0.0f ? 0.1f : 0.9f))
	{
		if (IsGrabbingActor())
		{
			ReleaseGrabbedActor(IsActorAlreadyGrabbed(GrabParams[this]));
		}
		bHasAttemptedGrab = false;
	}
	else
	{
		if (!bHasAttemptedGrab && !IsGrabbingActor())
		{
			SetGrabbedActor(GetGrabTarget());
			bHasAttemptedGrab = true;
		}

		TryMoveCompoundComponentRecursive(GrabParams[this]);

#if DRAW_CONTROLLER_DEBUG
		DrawDebugCapsule
		(
			GetWorld(),
			bGravityGrab ? GetGrabCapsule().GetTranslation() : GetComponentLocation(),
			bGravityGrab ? GravityGrabDistance * 0.5f : GrabLocationComponent->GetScaledSphereRadius(),
			bGravityGrab ? GravityGrabRadius : GrabLocationComponent->GetScaledSphereRadius(),
			bGravityGrab ? GetGrabCapsule().GetRotation() : GetComponentQuat(),
			FColor::White,
			false,
			0.f,
			0,
			1.f
		);
#endif
	}

	GrabAxisPriorValue = Value;
}

bool UCVR_MotionControllerComponent::IsActorConstrained()
{
	return false;
}

bool UCVR_MotionControllerComponent::IsActorAlreadyGrabbed(FGrabParams& _GrabParams)
{
	for (auto const& controller : GrabParams)
	{
		if (controller.Key != this &&
			controller.Value.GrabbedActor == _GrabParams.GrabbedActor &&
			controller.Value.GrabbedComponent == _GrabParams.GrabbedComponent &&
			controller.Value.InterfaceComponent == _GrabParams.InterfaceComponent
			)
		{
			return true;
		};
	}
	return false;
}

bool UCVR_MotionControllerComponent::IsGrabbingActor()
{
	FGrabParams _GrabParams = GrabParams[this];
	return (_GrabParams.GrabbedActor && _GrabParams.InterfaceComponent);
}

void UCVR_MotionControllerComponent::MoveComponent(FGrabParams& _GrabParams)
{
	// special case for gravity grab
	// update translation delta to interpolate towards the motion controller component
	if (bGravityGrab && _GrabParams.TranslationDelta.Size() > GrabLocationComponent->GetScaledSphereRadius())
	{
		_GrabParams.TranslationDelta = FMath::VInterpTo(
			_GrabParams.TranslationDelta,
			FVector::ZeroVector,
			GetWorld()->GetDeltaSeconds(),
			GravityGrabStrength
		);
	}

	// calculate new location
	FRotator NewRotator = GetComponentTransform().TransformRotation(_GrabParams.RotationDelta).Rotator();
	FVector	NewLocation = GetComponentTransform().TransformPosition(_GrabParams.TranslationDelta);

	// potentially modify the actors location and rotation
	FTransform NewTransform = FTransform(NewRotator, NewLocation);

	if (_GrabParams.InterfaceComponent->Implements<UCVR__InternalGrabInterface>())
	{
		NewTransform = Cast<ICVR__InternalGrabInterface>(_GrabParams.InterfaceComponent)->__OnUpdateLocationAndRotation(_GrabParams, NewLocation, NewRotator);
	}

	// update actor transformation
	if (_GrabParams.GrabbedActor == nullptr) { return; }
	if (_GrabParams.bWasSimulating)
	{
		USceneComponent* root = _GrabParams.GrabbedActor->GetRootComponent();
		UPrimitiveComponent* primitive = Cast<UPrimitiveComponent>(root);

		if (primitive)
		{
			PhysicsHandleComponent->SetTargetLocationAndRotation(
				NewTransform.GetLocation(),
				NewTransform.Rotator()
			);
		};
	}
	else
	{
		FHitResult OutSweepHitResult;
		_GrabParams.GrabbedActor->GetRootComponent()->SetWorldLocationAndRotation(
			NewTransform.GetLocation(),
			NewTransform.GetRotation(),
			true,
			&OutSweepHitResult,
			ETeleportType::ResetPhysics
		);
	}

	TrackGrabbedObjectMovementVelocity(NewTransform.GetLocation());

	if (_GrabParams.InterfaceComponent->Implements<UCVR__InternalGrabInterface>())
	{
		Cast<ICVR__InternalGrabInterface>(_GrabParams.InterfaceComponent)->__OnMove(_GrabParams);
	}
}

bool UCVR_MotionControllerComponent::OverlapMultiByChannel(bool isGravityGrab, TArray<FOverlapResult>& OverlapResults)
{
	FCollisionQueryParams Params = FCollisionQueryParams::DefaultQueryParam;

	return GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		isGravityGrab ? GetGrabCapsule().GetTranslation() : GetComponentLocation(),
		isGravityGrab ? GetGrabCapsule().GetRotation() : GetComponentQuat(),
		InteractionChannel,
		isGravityGrab ? FCollisionShape::MakeCapsule(GravityGrabRadius, GravityGrabDistance) : GrabLocationComponent->GetCollisionShape(),
		Params
	);
};

void UCVR_MotionControllerComponent::PlayHapticEffect(UHapticFeedbackEffect_Base* Haptic, float Scale, bool bLoop)
{
	if (!Haptic) { return; }
	if (GetOwner()->IsA<APawn>())
	{
		APawn* pawn = Cast<APawn>(GetOwner());
		AController* cont = pawn->GetController();
		if (cont && cont->IsA<APlayerController>())
		{
			Cast<APlayerController>(cont)->PlayHapticEffect(Haptic, GetControllerHand(), Scale, bLoop);
		}
	}
}

void UCVR_MotionControllerComponent::ReleaseGrabbedActor(bool bHeldByOtherController, bool bWithPhysics)
{
	FGrabParams _GrabParams = GrabParams[this];

	if (_GrabParams.bIsWelded)
	{
		return;
	}

	// release object
	if (_GrabParams.GrabbedActor)
	{
		if (_GrabParams.bWasSimulating && _GrabParams.GrabbedActor->GetRootComponent()->IsA<UPrimitiveComponent>())
		{
			ReleasePhysicsConstraint(_GrabParams.GrabbedActor->GetRootComponent(), bHeldByOtherController, bWithPhysics);
		}

		// call internal callback
		if (_GrabParams.InterfaceComponent && _GrabParams.InterfaceComponent->Implements<UCVR__InternalGrabInterface>())
		{
			Cast<ICVR__InternalGrabInterface>(_GrabParams.InterfaceComponent)->__OnRelease(_GrabParams);
		}

		// set external pointers to NULL
		FGrabParams GrabParamsOut = GrabParams[this];

		GrabParamsOut.GrabbedActor = NULL;
		GrabParamsOut.GrabbedComponent = NULL;
		GrabParamsOut.InterfaceComponent = NULL;
		GrabParamsOut.RotationDelta = FQuat::Identity;
		GrabParamsOut.TranslationDelta = FVector::ZeroVector;
		GrabParamsOut.bWasSimulating = false;

		GrabParams[this] = GrabParamsOut;
	}
}

void UCVR_MotionControllerComponent::ReleaseInteraction()
{
	// build FInteractionParams
	F__InternalInteractionParameters InteractionParams;
	InteractionParams.bIsGrabbed = true;
	InteractionParams.Outer = GetOwner();

	// get from static map
	FGrabParams _GrabParams = GrabParams[this];

	// callback
	if (_GrabParams.InterfaceComponent && _GrabParams.InterfaceComponent->Implements<UCVR__InternalInteractionInterface>())
	{
		Cast<ICVR__InternalInteractionInterface>(_GrabParams.InterfaceComponent)->__OnTriggerRelease(InteractionParams);
	}
}

void UCVR_MotionControllerComponent::ReleasePhysicsConstraint(USceneComponent* component, bool bHeldByOtherController, bool bPreservePhysics)
{
	PhysicsHandleComponent->ReleaseComponent();

	if (bPreservePhysics == false)
	{
		UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(component);
		PrimitiveComponent->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
		PrimitiveComponent->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}
	else if (bHeldByOtherController == false)
	{
		UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(component);
		FVector velocityToImpart = GetGrabbedActorVelocity();
		PrimitiveComponent->AddImpulse(velocityToImpart, NAME_None, true);
	}
}

void UCVR_MotionControllerComponent::SetGrabbedActor(FGrabResult GrabResult)
{
	// set null?
	if (GrabResult.Component == NULL || GrabResult.Actor == NULL)
	{
		// get from static map
		FGrabParams NewGrabParams = FGrabParams::Identity();

		// set grab parameters
		NewGrabParams.Outer = GrabParams[this].Outer;
		NewGrabParams.MotionControllerComponent = GrabParams[this].MotionControllerComponent;

		GrabParams[this] = NewGrabParams;
		return;
	}

	// calculate matrix
	FTransform WorldToComponent = GetComponentTransform().Inverse();

	// get from static map
	FGrabParams _GrabParams = GrabParams[this];

	// set grab parameters
	_GrabParams.TranslationDelta = WorldToComponent.TransformPosition(GrabResult.Actor->GetRootComponent()->GetComponentLocation());
	_GrabParams.RotationDelta = WorldToComponent.TransformRotation(GrabResult.Actor->GetRootComponent()->GetComponentQuat());
	_GrabParams.GrabbedComponent = GrabResult.Component;
	_GrabParams.InterfaceComponent = GrabResult.InterfaceComponent;
	_GrabParams.GrabbedActor = GrabResult.Actor;
	_GrabParams.bIsAttached = false;
	_GrabParams.bIsGravityGrab = GrabResult.bIsGravityGrab;
	_GrabParams.bIsWelded = false;
	_GrabParams.bWasSimulating = false;

	// check interfaces for should snap
	if (Cast<ICVR__InternalGrabInterface>(GrabResult.InterfaceComponent)->__ShouldSnap(_GrabParams))
	{
		_GrabParams.TranslationDelta = FVector::ZeroVector;
		_GrabParams.RotationDelta = FQuat::Identity;
		_GrabParams.bIsAttached = true;
	}

	// check interfaces for should weld
	if (Cast<ICVR__InternalGrabInterface>(GrabResult.InterfaceComponent)->__ShouldWeldOnGrab(_GrabParams))
	{
		_GrabParams.TranslationDelta = FVector::ZeroVector;
		_GrabParams.RotationDelta = FQuat::Identity;
		_GrabParams.bIsAttached = true;
		_GrabParams.bIsWelded = true;
	}

	// check whether this actors root component is simulating
	if (GrabResult.Actor->GetRootComponent()->IsA<UPrimitiveComponent>())
	{
		SetPhysicsConstraint(GrabResult.Actor->GetRootComponent(), _GrabParams);
	}

	// grabbed by another motion controller?
	for (auto& controller : GrabParams)
	{
		if (controller.Key != this &&
			controller.Value.GrabbedActor == _GrabParams.GrabbedActor &&
			controller.Value.GrabbedComponent == _GrabParams.GrabbedComponent &&
			controller.Value.InterfaceComponent == _GrabParams.InterfaceComponent
			)
		{
			// copy state to this motion controller
			_GrabParams.bWasSimulating = controller.Value.bWasSimulating;
			_GrabParams.bIsAttached = controller.Value.bIsAttached;
			_GrabParams.bIsWelded = controller.Value.bIsWelded;

			// tell other motion controller to let go of the grabbed actor
			controller.Key->ReleaseGrabbedActor(true);
		};
	}

	GrabParams[this] = _GrabParams;

	// call internal callback
	Cast<ICVR__InternalGrabInterface>(GrabResult.InterfaceComponent)->__OnGrab(_GrabParams);

	// trigger haptic feedback
	PlayHapticEffect(GrabHaptic, 1.f, false);

	// attach if required
	if (_GrabParams.bIsAttached)
	{
		_GrabParams.GrabbedActor->AttachToComponent(
			this,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			SnapSocketName
		);
	}
}

void UCVR_MotionControllerComponent::SetPhysicsConstraint(USceneComponent* component, FGrabParams& _GrabParams)
{
	UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(component);
	_GrabParams.bWasSimulating = PrimitiveComponent->IsSimulatingPhysics();

	if (_GrabParams.bIsAttached)
	{
		PrimitiveComponent->SetSimulatePhysics(false);
		return;
	}
	if (_GrabParams.bWasSimulating)
	{
		PhysicsHandleComponent->GrabComponentAtLocationWithRotation(PrimitiveComponent, NAME_None,
			PrimitiveComponent->GetComponentLocation(), PrimitiveComponent->GetComponentRotation());
	}
}

void UCVR_MotionControllerComponent::TriggerInteraction()
{
	// build FInteractionParams
	F__InternalInteractionParameters InteractionParams;
	InteractionParams.bIsGrabbed = true;
	InteractionParams.Outer = GetOwner();

	// get from static map
	FGrabParams _GrabParams = GrabParams[this];

	// callback
	if (_GrabParams.InterfaceComponent && _GrabParams.InterfaceComponent->Implements<UCVR__InternalInteractionInterface>())
	{
		Cast<ICVR__InternalInteractionInterface>(_GrabParams.InterfaceComponent)->__OnTriggerPress(InteractionParams);
	}
}

bool UCVR_MotionControllerComponent::StaticForceGrabRelease(AActor* actorToRelease)
{
	for (auto& controller : GrabParams)
	{
		if (controller.Value.GrabbedActor == actorToRelease)
		{
			controller.Key->ReleaseGrabbedActor(true, false);
			return true;
		};
	}
	return false;
}

void UCVR_MotionControllerComponent::StaticReleaseGrabbedActor(UCVR_MotionControllerComponent* motionController, AActor* actorToRelease)
{
	if (motionController->IsGrabbingActor())
	{
		motionController->ReleaseGrabbedActor(motionController->IsActorAlreadyGrabbed(GrabParams[motionController]));
	}
}

void UCVR_MotionControllerComponent::TrackGrabbedObjectMovementVelocity(FVector newLocation)
{
	// this function is far from optimised, but it only runs on motion controllers so may not need optimisation
	float frameTime = GetWorld()->GetTimeSeconds();
	int i = 0;
	while (i < trackedPositions.Num())
	{
		if (trackedPositions[i].Key + 0.2f < frameTime)
		{
			trackedPositions.RemoveAt(i);
		}
		else
		{
			i++;
		}
	}

	// track to location of the grabbed actor over time.
	trackedPositions.Add(TPair<float, FVector>(frameTime, newLocation));
}

void UCVR_MotionControllerComponent::TryMoveCompoundComponentRecursive(FGrabParams& _GrabParams)
{
	TryMoveGrabbedComponent(_GrabParams);

	FGrabParams _GrabParamsCopy = _GrabParams;
	if (GetCompoundComponent(_GrabParamsCopy) && _GrabParamsCopy.IsValid())
	{
		TryMoveCompoundComponentRecursive(_GrabParamsCopy);
	}
}

void UCVR_MotionControllerComponent::TryMoveGrabbedComponent(FGrabParams& _GrabParams)
{
	if (_GrabParams.IsValid())
	{
		MoveComponent(_GrabParams);
	}
}

void UCVR_MotionControllerComponent::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	Super::PostEditChangeProperty(e);
	UpdateSphereComponent();
}

void UCVR_MotionControllerComponent::PostLoad()
{
	Super::PostLoad();
	UpdateSphereComponent();
}

void UCVR_MotionControllerComponent::UpdateSphereComponent()
{
	GrabLocationComponent->SetSphereRadius(GrabRadius);
	GrabLocationComponent->SetRelativeLocation(GrabLocation);
}