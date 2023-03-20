// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "MyGame/Public/MobController.h"
#include "AIService_FindPointOfInterest.generated.h"

/**
 *
 */
UCLASS()
class MYGAME_API UAIService_FindPointOfInterest : public UBTService_BlackboardBase
{
	GENERATED_BODY()
	virtual void TickNode(UBehaviorTreeComponent&, uint8*, float) override;

	UPROPERTY(EditAnywhere, Category = LookUp)
		EPOILookup lookup;
};
