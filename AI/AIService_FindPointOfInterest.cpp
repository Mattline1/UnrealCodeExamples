// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGame.h"
#include "AIService_FindPointOfInterest.h"

void UAIService_FindPointOfInterest::TickNode(UBehaviorTreeComponent & OwnerTreeComponent, uint8* NodeMemory, float delta)
{
	Super::TickNode(OwnerTreeComponent, NodeMemory, delta);
	AMobController* Mob_Controller = Cast<AMobController>(OwnerTreeComponent.GetOwner());
	Mob_Controller->FindPointOfInterest(lookup, GetSelectedBlackboardKey());
}


