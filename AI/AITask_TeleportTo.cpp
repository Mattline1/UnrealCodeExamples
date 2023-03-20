// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGame.h"
#include "AITask_TeleportTo.h"


EBTNodeResult::Type UAITask_TeleportTo::ExecuteTask(UBehaviorTreeComponent& OwnerTreeComponent, uint8* NodeMemory)
{
	AMobController* Mob_Controller = Cast<AMobController>(OwnerTreeComponent.GetOwner());
	if (Mob_Controller == NULL)	{ return EBTNodeResult::Failed; }
	AMob* Mob = Cast<AMob>(Mob_Controller->GetPawn());
	if (Mob == NULL) { return EBTNodeResult::Failed; }

	AActor* TeleportActor = Cast<AActor>(OwnerTreeComponent.GetBlackboardComponent()->GetValueAsObject(GetSelectedBlackboardKey()));
	float distance = Mob->GetDistanceTo(TeleportActor);

	if (Mob->GetDistanceTo(TeleportActor) - Mob->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() > AcceptableRadius)
	{
		bool WasTeleported = Mob->TeleportTo(TeleportActor->GetActorLocation(), TeleportActor->GetActorRotation(), false, false);
		if (WasTeleported) { return EBTNodeResult::Succeeded; }
	}
	return EBTNodeResult::Failed;
}