// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "MyGame/Public/MobController.h"
#include "MyGame/Public/Mob.h"

#include "AIDecorator_CheckAIState.generated.h"

/**
 *
 */
UCLASS()
class MYGAME_API UAIDecorator_CheckAIState : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = AI)
	EAIStateEnum AI_State;

protected:
	bool CalculateRawConditionValue(class UBehaviorTreeComponent& OwnerComp, uint8 * NodeMemory) const;
};
