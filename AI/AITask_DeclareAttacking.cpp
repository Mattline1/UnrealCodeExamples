// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGame.h"
#include "AITask_DeclareAttacking.h"


UAITask_DeclareAttacking::UAITask_DeclareAttacking(const class FObjectInitializer& PCIP)
:Super(PCIP)
{
	NodeName = "DeclareAttacking";
}

EBTNodeResult::Type UAITask_DeclareAttacking::ExecuteTask(UBehaviorTreeComponent& OwnerTreeComponent, uint8* NodeMemory)
{
	AMobController* Mob_Controller = Cast<AMobController>(OwnerTreeComponent.GetOwner());
	if (Mob_Controller == NULL)	{ return EBTNodeResult::Failed; }
	AMob* Mob = Cast<AMob>(Mob_Controller->GetPawn());
	if (Mob == NULL) { return EBTNodeResult::Failed; }

	uint8 AttackersTemp = Mob_Controller->GetCurrentNumOfAttackers();

	if (AttackersTemp < 5 || Mob->MobType == EMobTypeEnum::BOSS) {
		AttackersTemp += 1;
		Mob_Controller->SetCurrentNumOfAttackers(AttackersTemp);
		Mob_Controller->IsAttacking = true;

		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}

