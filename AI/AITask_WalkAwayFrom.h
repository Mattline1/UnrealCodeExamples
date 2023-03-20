// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"

#include "MyGame/Public/MobController.h"
#include "MyGame/Public/Mob.h"

#include "AITask_WalkAwayFrom.generated.h"

/**
 *
 */
UCLASS()
class MYGAME_API UAITask_WalkAwayFrom : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UAITask_WalkAwayFrom(const class FObjectInitializer& Initializer);

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent&, uint8*) override;
	virtual void TickTask(UBehaviorTreeComponent&, uint8*, float) override;
};
