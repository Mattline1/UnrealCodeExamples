// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "MyGame/Public/MobController.h"
#include "MyGame/Public/Mob.h"
#include "AITask_ScriptedAction.generated.h"

/**
 *
 */
UCLASS()
class MYGAME_API UAITask_ScriptedAction : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent&, uint8*) override;
	virtual void TickTask(UBehaviorTreeComponent&, uint8*, float) override;

	UPROPERTY(EditAnywhere, Category = AI)
	EAIStateEnum ActionToPerform;

	UPROPERTY(EditAnywhere, Category = AI)
	bool AlignToEnemy;

	UPROPERTY(EditAnywhere, Category = AI)
	bool InvertAlign;

protected:
	UAITask_ScriptedAction(const class FObjectInitializer& PCIP);
};
