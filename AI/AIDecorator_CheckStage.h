// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "MyGame/Public/AIDecorator_CheckHealth.h"
#include "MyGame/Public/MobController.h"
#include "MyGame/Public/Mob.h"

#include "AIDecorator_CheckStage.generated.h"

/**
 *
 */
UCLASS()
class MYGAME_API UAIDecorator_CheckStage : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = AI)
	EComparison Comparison;

	UPROPERTY(EditAnywhere, Category = AI)
	uint8 CheckedValue;

protected:
	bool CalculateRawConditionValue(class UBehaviorTreeComponent& OwnerComp, uint8 * NodeMemory) const;
};
