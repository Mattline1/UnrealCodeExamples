// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGame.h"
#include "AITask_SetState.h"



UAITask_SetState::UAITask_SetState(const class FObjectInitializer& PCIP)
:Super(PCIP)
{
	NodeName = "SetState";
}

EBTNodeResult::Type UAITask_SetState::ExecuteTask(UBehaviorTreeComponent& OwnerTreeComponent, uint8* NodeMemory)
{
	AMobController* Mob_Controller = Cast<AMobController>(OwnerTreeComponent.GetOwner());
	if (Mob_Controller == NULL)	{ EBTNodeResult::Aborted; }
	AMob* Mob = Cast<AMob>(Mob_Controller->GetPawn());
	if (Mob == NULL)	{ EBTNodeResult::Aborted; }

	Mob->StageTransition();
	Mob->stage = newStage;

	return EBTNodeResult::Succeeded;
}
