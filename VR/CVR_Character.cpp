// Fill out your copyright notice in the Description page of Project Settings.

#include "CVR_Character.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/CVR__InternalInputBindingsInterface.h"
#include "Interfaces/CVR__InternalInteractionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h"

// Sets default values
ACVR_Character::ACVR_Character()
{
	// Set this character to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(38.f, 96.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_PhysicsBody, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Vehicle, ECollisionResponse::ECR_Ignore);

	// VolumeComponent
	VolumeComponent = CreateDefaultSubobject<UCVR_VolumeComponent>(TEXT("VolumeComponent"));
	VolumeComponent->SetupAttachment(RootComponent);

	// CameraComponent
	HMDComponent = CreateDefaultSubobject<UCVR_HMDComponent>(TEXT("HMDComponent"));
	HMDComponent->SetupAttachment(VolumeComponent);

	// Right Controller
	MotionControllerRight = CreateDefaultSubobject<UCVR_MotionControllerComponent>(TEXT("R_MotionController"));
	MotionControllerRight->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	MotionControllerRight->SetShowDeviceModel(true);
	MotionControllerRight->SetupAttachment(VolumeComponent);

	// Left Controller
	MotionControllerLeft = CreateDefaultSubobject<UCVR_MotionControllerComponent>(TEXT("L_MotionController"));
	MotionControllerLeft->MotionSource = FXRMotionControllerBase::LeftHandSourceId;
	MotionControllerLeft->SetShowDeviceModel(true);
	MotionControllerLeft->SetupAttachment(VolumeComponent);

	// Interaction
	WidgetInteractionComponent = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("R_WidgetInteractionComponent"));
	WidgetInteractionComponent->SetupAttachment(MotionControllerRight); // <-- default to right

	Handedness = EHandedness::HAND_Variable;

	// set defaults
	bUseControllerRotationYaw = true;
	Cast<UCharacterMovementComponent>(GetMovementComponent())->bUseControllerDesiredRotation = true;
}

// Called when the game starts or when spawned
void ACVR_Character::BeginPlay()
{
	Super::BeginPlay();

	if (UHeadMountedDisplayFunctionLibrary::GetHMDDeviceName() != "None")
	{
		OnEnterVR();
	}
	else
	{
		OnExitVR();
	}
}

void ACVR_Character::OnEnterVR()
{
	bUseControllerRotationYaw = false;
	ResetHandedness();

	VolumeComponent->OnEnterVR();
}

void ACVR_Character::OnExitVR()
{
	bUseControllerRotationYaw = true;
	WidgetInteractionComponent->AttachToComponent(VolumeComponent->GetCameraComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	VolumeComponent->OnExitVR();
}

void ACVR_Character::ResetHandedness()
{
	switch (Handedness)
	{
	case EHandedness::HAND_Left:
		WidgetInteractionComponent->AttachToComponent(MotionControllerLeft, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		break;
	case EHandedness::HAND_Variable:
	case EHandedness::HAND_Right:
		WidgetInteractionComponent->AttachToComponent(MotionControllerRight, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		break;
	}
}

void ACVR_Character::ResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ACVR_Character::PressPointerRight()
{
	switch (Handedness)
	{
	case EHandedness::HAND_Variable:
		if (WidgetInteractionComponent->GetAttachParent() == MotionControllerRight)
		{
			WidgetInteractionComponent->PressPointerKey(EKeys::LeftMouseButton);
		}
		else
		{
			WidgetInteractionComponent->AttachToComponent(MotionControllerRight, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}
		break;

	case EHandedness::HAND_Right:
		WidgetInteractionComponent->PressPointerKey(EKeys::LeftMouseButton);
		break;

	default:
		break;
	}
}

void ACVR_Character::PressPointerLeft()
{
	switch (Handedness)
	{
	case EHandedness::HAND_Variable:
		if (WidgetInteractionComponent->GetAttachParent() == MotionControllerLeft)
		{
			WidgetInteractionComponent->PressPointerKey(EKeys::LeftMouseButton);
		}
		else
		{
			WidgetInteractionComponent->AttachToComponent(MotionControllerLeft, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}
		break;

	case EHandedness::HAND_Left:
		WidgetInteractionComponent->PressPointerKey(EKeys::LeftMouseButton);
		break;

	default:
		break;
	}
}

void ACVR_Character::ReleasePointerRight()
{
	if (Handedness == EHandedness::HAND_Right || Handedness == EHandedness::HAND_Variable)
	{
		WidgetInteractionComponent->ReleasePointerKey(EKeys::LeftMouseButton);
	}
}

void ACVR_Character::ReleasePointerLeft()
{
	if (Handedness == EHandedness::HAND_Left || Handedness == EHandedness::HAND_Variable)
	{
		WidgetInteractionComponent->ReleasePointerKey(EKeys::LeftMouseButton);
	}
}

// Called to bind functionality to input
void ACVR_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	// Bind VR events
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ACVR_Character::ResetVR);

	// Bind motion controller events
	if (IsValid(MotionControllerLeft))
	{
		PlayerInputComponent->BindAxis("GrabLeft", MotionControllerLeft, &UCVR_MotionControllerComponent::GrabAxis);
		PlayerInputComponent->BindAction("PointerLeft", IE_Pressed, this, &ACVR_Character::PressPointerLeft);
		PlayerInputComponent->BindAction("PointerLeft", IE_Released, this, &ACVR_Character::ReleasePointerLeft);
		PlayerInputComponent->BindAction("TriggerLeft", IE_Pressed, MotionControllerLeft, &UCVR_MotionControllerComponent::TriggerInteraction);
		PlayerInputComponent->BindAction("TriggerLeft", IE_Released, MotionControllerLeft, &UCVR_MotionControllerComponent::ReleaseInteraction);
	}

	if (IsValid(MotionControllerRight))
	{
		PlayerInputComponent->BindAxis("GrabRight", MotionControllerRight, &UCVR_MotionControllerComponent::GrabAxis);
		PlayerInputComponent->BindAction("PointerRight", IE_Pressed, this, &ACVR_Character::PressPointerRight);
		PlayerInputComponent->BindAction("PointerRight", IE_Released, this, &ACVR_Character::ReleasePointerRight);
		PlayerInputComponent->BindAction("TriggerRight", IE_Pressed, MotionControllerRight, &UCVR_MotionControllerComponent::TriggerInteraction);
		PlayerInputComponent->BindAction("TriggerRight", IE_Released, MotionControllerRight, &UCVR_MotionControllerComponent::ReleaseInteraction);
	}

	//
	for (auto child : GetComponents())
	{
		ICVR__InternalInputBindingsInterface* in = Cast<ICVR__InternalInputBindingsInterface>(child);

		if (in && in->__UseDefaultActions())
		{
			in->__BindDefaultActionsToInputComponent(PlayerInputComponent);
		}
	}

	/*
	* Bind inputs for dev only
	*/
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	// Bind VR events
	PlayerInputComponent->BindAction("DEV_ResetVR", IE_Pressed, this, &ACVR_Character::ResetVR);
#endif
}