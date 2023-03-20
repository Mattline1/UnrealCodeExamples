// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "MyGame/Public/MobController.h"
#include "MyGame/Public/Mob.h"

#include "AITask_Attack.generated.h"

/**
 *
 */
UCLASS()
class MYGAME_API UAITask_Attack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& , uint8*) override;
	virtual void TickTask(UBehaviorTreeComponent&, uint8*, float) override;

	UPROPERTY(EditAnywhere, Category = AI)
	EAttackTypeEnum AttackType;

protected:
	UAITask_Attack(const class FObjectInitializer& PCIP);


};
