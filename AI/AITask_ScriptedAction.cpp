// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGame.h"
#include "AITask_ScriptedAction.h"

UAITask_ScriptedAction::UAITask_ScriptedAction(const class FObjectInitializer& PCIP)
:Super(PCIP)
{
	NodeName = "ScriptedAction";
	bNotifyTick = true;
}

EBTNodeResult::Type UAITask_ScriptedAction::ExecuteTask(UBehaviorTreeComponent& OwnerTreeComponent, uint8* NodeMemory)
{
	AMobController* Mob_Controller = Cast<AMobController>(OwnerTreeComponent.GetOwner());
	if (Mob_Controller == NULL)	{ return EBTNodeResult::Failed; }
	AMob* Mob = Cast<AMob>(Mob_Controller->GetPawn());
	if (Mob == NULL) { return EBTNodeResult::Failed; }

	Mob_Controller->StopAttacking();
	Mob->GetCharacterMovement()->bOrientRotationToMovement = false;

	if (ActionToPerform == EAIStateEnum::STT_FLINCHED) {
		if (Mob->MobType == EMobTypeEnum::BOSS)
		{
			if (Mob->AIState != EAIStateEnum::STT_ATTACKING) {
				Mob->Animate(Mob->Flinched, true);
			}
			else
			{
				Mob->AIState = EAIStateEnum::STT_IDLE;
				return EBTNodeResult::Failed;
			}
		}
		else
		{
			Mob->Animate(Mob->Flinched, true);
		}
	}

	if (ActionToPerform == EAIStateEnum::STT_KNOCKED) {
		Mob->Animate(Mob->KnockedDown, true);
	}

	if (ActionToPerform == EAIStateEnum::STT_STRAFING) {
		int32 randnum = FMath::RandRange(1, 5);

		if (randnum == 1)      { Mob->Animate(Mob->SideStepL, true); }
		else if (randnum == 2) { Mob->Animate(Mob->SideStepR, true); }
		else {
			Mob->AIState = EAIStateEnum::STT_IDLE;
			Mob_Controller->SetBBState(EAIStateEnum::STT_IDLE);
			return EBTNodeResult::Failed;
		}
	}

	if (ActionToPerform == EAIStateEnum::STT_TAUNTING) {
		int32 randnum = FMath::RandRange(1, 10);

		if (randnum == 1)	   { Mob->Animate(Mob->Taunt_1, true); }
		else if (randnum == 2) { Mob->Animate(Mob->Taunt_2, true); }
		else if (randnum == 3) { Mob->Animate(Mob->Taunt_3, true); }
		else {
			Mob->AIState = EAIStateEnum::STT_IDLE;
			Mob_Controller->SetBBState(EAIStateEnum::STT_IDLE);
			return EBTNodeResult::Failed;
		}
	}

	Mob->AIState = ActionToPerform;
	Mob_Controller->SetBBState(ActionToPerform);
	return EBTNodeResult::InProgress;
}

void UAITask_ScriptedAction::TickTask(UBehaviorTreeComponent& OwnerTreeComponent, uint8* NodeMemory, float delta)
{
	AMobController* Mob_Controller = Cast<AMobController>(OwnerTreeComponent.GetOwner());
	if (Mob_Controller == NULL)	{ return; }
	AMob* Mob = Cast<AMob>(Mob_Controller->GetPawn());
	if (Mob == NULL) { return; }

	if (AlignToEnemy){
		if (InvertAlign) { Mob->AlignToEnemy(10.f, true); }
		else { Mob->AlignToEnemy(10.f); }
	}


	if (!Mob->TaskActive) { Mob->TaskActive = true; }
	else if ( !Mob->Anim_instance->IsActiveSlotNode(FName("FullBody"))) {
		Mob->TaskActive = false;
		Mob->AIState = EAIStateEnum::STT_IDLE;
		Mob_Controller->SetBBState(EAIStateEnum::STT_IDLE);

		FinishLatentTask(OwnerTreeComponent, EBTNodeResult::Succeeded);
	}
}


