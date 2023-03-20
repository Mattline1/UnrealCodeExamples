// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGame.h"
#include "AIDecorator_CheckStage.h"

bool UAIDecorator_CheckStage::CalculateRawConditionValue(class UBehaviorTreeComponent& OwnerComp, uint8 * NodeMemory) const
{
	AMobController* Mob_Controller = Cast<AMobController>(OwnerComp.GetOwner());
	if (Mob_Controller == NULL)	{ return false; }
	AMob* Mob = Cast<AMob>(Mob_Controller->GetPawn());
	if (Mob == NULL) { return false; }

	if (Comparison == EComparison::EQUALTO)
	{
		if (Mob->stage == CheckedValue) { return true; }
	}

	if (Comparison == EComparison::GREATERTHAN)
	{
		if (Mob->stage > CheckedValue) { return true; }
	}

	if (Comparison == EComparison::LESSTHAN)
	{
		if (Mob->stage < CheckedValue) { return true; }
	}

	return false;
}


