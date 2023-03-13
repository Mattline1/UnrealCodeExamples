// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MyCharacter.h"
#include "MyController.h"
#include "InvokableAnimInstance.generated.h"

/**
 *
 */
UCLASS()
class FREEWORLD_API UInvokableAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		AFreeWorldCharacter* AttachedCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		AFreeWorldPlayerController* AttachedController;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TMap<FString, UAnimMontage*> AnimationMontageMap;

public:
	UFUNCTION()
	void PlayAnimationMontage(FString name);
};

