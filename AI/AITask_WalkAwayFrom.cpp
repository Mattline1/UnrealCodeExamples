// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGame.h"
#include "AITask_WalkAwayFrom.h"


UAITask_WalkAwayFrom::UAITask_WalkAwayFrom(const class FObjectInitializer& Initializer)
:Super(Initializer)
{
	NodeName = "Walk Away";
	bNotifyTick = true;
}

EBTNodeResult::Type UAITask_WalkAwayFrom::ExecuteTask(UBehaviorTreeComponent& OwnerTreeComponent, uint8* NodeMemory)
{
	AMobController* Mob_Controller = Cast<AMobController>(OwnerTreeComponent.GetOwner());
	if (Mob_Controller == NULL)	{ return EBTNodeResult::Failed; }
	AMob* Mob = Cast<AMob>(Mob_Controller->GetPawn());
	if (Mob == NULL) { return EBTNodeResult::Failed; }

	Mob->GetCharacterMovement()->bOrientRotationToMovement = true;

	return EBTNodeResult::InProgress;
}

void UAITask_WalkAwayFrom::TickTask(UBehaviorTreeComponent& OwnerTreeComponent, uint8* NodeMemory, float delta)
{
	AMobController* Mob_Controller = Cast<AMobController>(OwnerTreeComponent.GetOwner());
	if (Mob_Controller == NULL)	{ return; }
	AMob* Mob = Cast<AMob>(Mob_Controller->GetPawn());
	if (Mob == NULL) { return; }

	if (Mob->SpawnedBy == NULL) { return; }

	FVector Direction = Mob->GetActorLocation() - Mob->SpawnedBy->GetActorLocation();
	Mob->AddMovementInput(Direction, Mob->GetMovementComponent()->GetMaxSpeed());

	if (Mob->DistanceToSpawner > 400) {
		Mob->GetCharacterMovement()->bOrientRotationToMovement = false;
		FinishLatentTask(OwnerTreeComponent, EBTNodeResult::Succeeded);
	};
}