// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGame.h"
#include "AITask_Attack.h"


UAITask_Attack::UAITask_Attack(const class FObjectInitializer& PCIP)
		:Super(PCIP)
{
	NodeName = "Attack";
	AttackType = EAttackTypeEnum::AT_Light;
	bNotifyTick = true;
}

EBTNodeResult::Type UAITask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerTreeComponent, uint8* NodeMemory)
{
	AMobController* Mob_Controller = Cast<AMobController>(OwnerTreeComponent.GetOwner());
	if (Mob_Controller == NULL)	{ return EBTNodeResult::Failed; }
	AMob* Mob = Cast<AMob>(Mob_Controller->GetPawn());
	if (Mob == NULL) { return EBTNodeResult::Failed; }

	if (!Mob_Controller->PerformAttack(AttackType, true)){ return EBTNodeResult::Failed; }
	Mob->AlignToEnemy(Mob->AttackTurnSpeed);

	return EBTNodeResult::InProgress;
}


void UAITask_Attack::TickTask(UBehaviorTreeComponent& OwnerTreeComponent, uint8* NodeMemory, float delta)
{
	AMobController* Mob_Controller = Cast<AMobController>(OwnerTreeComponent.GetOwner());
	if (Mob_Controller == NULL)	{ return; }
	AMob* Mob = Cast<AMob>(Mob_Controller->GetPawn());
	if (Mob == NULL) { return; }


	if ( !Mob_Controller->PerformAttack(AttackType, false) )
	{
		Mob->AIState = EAIStateEnum::STT_IDLE;
		Mob_Controller->StopAttacking();
		FinishLatentTask(OwnerTreeComponent, EBTNodeResult::Succeeded);
	}
}
