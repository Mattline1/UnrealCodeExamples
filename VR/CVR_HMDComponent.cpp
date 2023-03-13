// Fill out your copyright notice in the Description page of Project Settings.

#include "CVR_HMDComponent.h"
#include "Camera/CameraComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"

UCVR_HMDComponent::UCVR_HMDComponent()
{
	SetRelativeLocation(FVector(-0.f, 0.f, 64.f));
	bUsePawnControlRotation = false;
}

void UCVR_HMDComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCVR_HMDComponent::OnEnterVR()
{
	bUsePawnControlRotation = false;
	bLockToHmd = true;
	ResetVR();
}

void UCVR_HMDComponent::OnExitVR()
{
	bUsePawnControlRotation = true;
	bLockToHmd = false;
}

void UCVR_HMDComponent::ResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}