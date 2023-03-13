// Fill out your copyright notice in the Description page of Project Settings.

#include "InvokableAnimInstance.h"

void UInvokableAnimInstance::PlayAnimationMontage(FString name)
{
	if (AnimationMontageMap.Contains(name))
	{
		Montage_Play(AnimationMontageMap[name]);
	}
}