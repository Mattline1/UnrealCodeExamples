// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "MyGame/Public/MobController.h"
#include "MyGame/Public/Mob.h"

#include "AIDecorator_CheckHealth.generated.h"

/**
 *
 */
UENUM(BlueprintType)
enum class EComparison : uint8
{
	GREATERTHAN	UMETA(DisplayName = "GREATERTHAN"),
	LESSTHAN 	UMETA(DisplayName = "LESSTHAN"),
	EQUALTO 		UMETA(DisplayName = "EQUALTO")
};

UCLASS()
class MYGAME_API UAIDecorator_CheckHealth : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = AI)
	EComparison Comparison;

	UPROPERTY(EditAnywhere, Category = AI)
	int32 CheckedValue;

protected:
	bool CalculateRawConditionValue(class UBehaviorTreeComponent& OwnerComp, uint8 * NodeMemory) const;
};
