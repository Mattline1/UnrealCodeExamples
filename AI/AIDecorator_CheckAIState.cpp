// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGame.h"
#include "AIDecorator_CheckAIState.h"

bool UAIDecorator_CheckAIState::CalculateRawConditionValue(class UBehaviorTreeComponent& OwnerComp, uint8 * NodeMemory) const
{
	AMobController* Mob_Controller = Cast<AMobController>(OwnerComp.GetOwner());
	if (Mob_Controller == NULL)	{ return false; }
	AMob* Mob = Cast<AMob>(Mob_Controller->GetPawn());
	if (Mob == NULL) { return false; }

	if (AI_State == Mob->AIState) { return true; }
	return false;
}


