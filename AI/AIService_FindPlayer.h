// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "AIService_FindPlayer.generated.h"

/**
 *
 */
UCLASS()
class MYGAME_API UAIService_FindPlayer : public UBTService_BlackboardBase
{
	GENERATED_BODY()
	virtual void TickNode(UBehaviorTreeComponent&, uint8*, float) override;

};
