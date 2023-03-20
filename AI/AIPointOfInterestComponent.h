// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SceneComponent.h"
#include "AIPointOfInterestComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYGAME_API UAIPointOfInterestComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAIPointOfInterestComponent();
	virtual void InitializeComponent() override;

	UPROPERTY(EditDefaultsOnly, Category = Channel)
		int32 Channel;
};
