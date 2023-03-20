// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGame.h"
#include "AITask_Death.h"


UAITask_Death::UAITask_Death(const class FObjectInitializer& PCIP)
		:Super(PCIP)
{
	NodeName = "Death";
	bNotifyTick = true;
}

EBTNodeResult::Type UAITask_Death::ExecuteTask(UBehaviorTreeComponent& OwnerTreeComponent, uint8* NodeMemory)
{
	AMobController* Mob_Controller = Cast<AMobController>(OwnerTreeComponent.GetOwner());
	if (Mob_Controller == NULL)	{ return EBTNodeResult::Failed; }
	AMob* Mob = Cast<AMob>(Mob_Controller->GetPawn());
	if (Mob == NULL)	{ return EBTNodeResult::Failed; }

	Mob->GetCharacterMovement()->bOrientRotationToMovement = false;
	Mob->Animate(Mob->Death, true);
	Mob->AIState = EAIStateEnum::STT_DEAD;
	Mob->PerformDeath();

	Mob_Controller->StopAttacking();
	return EBTNodeResult::InProgress;
}

void UAITask_Death::TickTask(UBehaviorTreeComponent& OwnerTreeComponent, uint8* NodeMemory, float delta)
{
	//bNotifyTick = false;
	//FinishLatentTask(OwnerTreeComponent, EBTNodeResult::Succeeded);
}
