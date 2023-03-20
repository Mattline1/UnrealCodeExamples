// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "MyGame/Public/MobController.h"
#include "MyGame/Public/Mob.h"
#include "AITask_DeclareAttacking.generated.h"

/**
 *
 */
UCLASS()
class MYGAME_API UAITask_DeclareAttacking : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent&, uint8*) override;

protected:
	UAITask_DeclareAttacking(const class FObjectInitializer& PCIP);
};
