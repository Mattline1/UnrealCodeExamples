// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "MyGame/Public/MobController.h"
#include "MyGame/Public/Mob.h"
#include "AITask_TeleportTo.generated.h"

/**
 *
 */
UCLASS()
class MYGAME_API UAITask_TeleportTo : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Node)
	float AcceptableRadius;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent&, uint8*) override;

};
