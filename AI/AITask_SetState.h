// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"

#include "MyGame/Public/MobController.h"
#include "MyGame/Public/Mob.h"


#include "AITask_SetState.generated.h"

/**
 *
 */
UCLASS()
class MYGAME_API UAITask_SetState : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent&, uint8*) override;

	UPROPERTY(EditAnywhere, Category = AI)
		uint8 newStage;

protected:
	UAITask_SetState(const class FObjectInitializer& PCIP);


};
