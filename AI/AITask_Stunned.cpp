// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGame.h"
#include "AITask_Stunned.h"

UAITask_Stunned::UAITask_Stunned(const class FObjectInitializer& PCIP)
		:Super(PCIP)
{
	NodeName = "Stunned";
	bNotifyTick = true;
}

EBTNodeResult::Type UAITask_Stunned::ExecuteTask(UBehaviorTreeComponent& OwnerTreeComponent, uint8* NodeMemory)
{
	AMobController* Mob_Controller = Cast<AMobController>(OwnerTreeComponent.GetOwner());
	if (Mob_Controller == NULL)	{ return EBTNodeResult::Failed; }
	AMob* Mob = Cast<AMob>(Mob_Controller->GetPawn());
	if (Mob == NULL)	{ return EBTNodeResult::Failed; }

	Mob->GetCharacterMovement()->bOrientRotationToMovement = false;

	Mob->TaskActive = true;
	Mob->Animate(Mob->Stunned, true);
	Mob->health = int(0);

	Mob_Controller->StopAttacking();
	Mob->DeactivateWeapons();

	if (Mob->stage <= 0) { Mob->AIState = EAIStateEnum::STT_DEAD; }
	else { Mob->AIState = EAIStateEnum::STT_STUNNED; }

	return EBTNodeResult::InProgress;
}

void UAITask_Stunned::TickTask(UBehaviorTreeComponent& OwnerTreeComponent, uint8* NodeMemory, float delta)
{
	AMobController* Mob_Controller = Cast<AMobController>(OwnerTreeComponent.GetOwner());
	if (Mob_Controller == NULL)	{ return; }
	AMob* Mob = Cast<AMob>(Mob_Controller->GetPawn());
	if (Mob == NULL)	{ return; }

	FName AnimSection = Mob->Anim_instance->Montage_GetCurrentSection();
	FString section = AnimSection.ToString();

	if (section.StartsWith("DealDamage"))
	{
		if (!Mob->WeaponsActive)
		{
			Mob->PerformAOE();
		}
	}

	else if (section.StartsWith("EndDamage"))
	{
		Mob->DeactivateWeapons();
	}

	else if (!Mob->Anim_instance->IsActiveSlotNode(FName("FullBody")) && Mob->TaskActive)
	{
		Mob->StageTransition();
		Mob->AIState = EAIStateEnum::STT_IDLE;
		FinishLatentTask(OwnerTreeComponent, EBTNodeResult::Succeeded);
	}
}

	/*
	Mob->StunTimer += GetWorld()->GetDeltaSeconds();

	if (Mob->StunTimer < Duration)
	{
		Mob->TaskActive = true;
		FName AnimSection = Mob->Anim_instance->Montage_GetCurrentSection();
		FString section = AnimSection.ToString();

		if (section.StartsWith("Recovery")) //loop if required
		{
			Mob->Animate(Mob->Stunned, true);
			Mob->Anim_instance->Montage_JumpToSection(FName("Loop"));
		}

		if (Mob->health < 0) // detect any hits
		{
			Mob->health = int(0);
			Mob->hits -= 1;
			Mob->Animate(Mob->StunnedFlinch, true);
		}

		if (Mob->hits <= 0)
		{
			if (Mob->stage <= 0) { Mob->AIState = EAIStateEnum::STT_DEAD; }
			else { Mob->StageTransition(); }
			FinishLatentTask(OwnerTreeComponent, EBTNodeResult::Succeeded);
		}
	}
	else if (!Mob->Anim_instance->IsActiveSlotNode(FName("FullBody")) && Mob->TaskActive)
	{
		Mob->AIState = EAIStateEnum::STT_IDLE;
		FinishLatentTask(OwnerTreeComponent, EBTNodeResult::Succeeded);
	}
	*/



