// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "MyGame/Public/MobController.h"
#include "MyGame/Public/Mob.h"

#include "AITask_Death.generated.h"

/**
 *
 */
UCLASS()
class MYGAME_API UAITask_Death : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent&, uint8*) override;
	virtual void TickTask(UBehaviorTreeComponent&, uint8*, float) override;

protected:
	UAITask_Death(const class FObjectInitializer& PCIP);

};
