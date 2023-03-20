// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "AIPointOfInterest.generated.h"

UCLASS()
class MYGAME_API AAIPointOfInterest : public AActor
{
	GENERATED_BODY()

public:
	AAIPointOfInterest();
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = Channel)
	int32 Channel;

};
