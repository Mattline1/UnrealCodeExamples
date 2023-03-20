// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGame.h"
#include "AISpawnPoint.h"

// Sets default values
AAISpawnPoint::AAISpawnPoint(const class FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	Capsule = ObjectInitializer.CreateDefaultSubobject< UCapsuleComponent >(this, TEXT("HitBox"));
	//Capsule->SetCollisionProfileName(FName("NoCollision"));
	//Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RootComponent = Capsule;
}

AMob* AAISpawnPoint::SpawnMob(AActor* SpawnedBy)
{
	FActorSpawnParameters spawnParams;
	spawnParams.Owner = this;
	spawnParams.bNoCollisionFail = true;

	AMob* NewMob;
	NewMob = GetWorld()->SpawnActor<AMob>(MobClass, spawnParams);

	if (NewMob != NULL)
	{
		FVector heightAlteration= FVector(0.f, 0.f, NewMob->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 10);

		if (SpawnedBy != NULL) {
			NewMob->SetSummoner(SpawnedBy);
		}

		NewMob->SetActorLocation(GetActorLocation() + heightAlteration);
		NewMob->SetActorRotation(GetActorRotation());

		if (WeaponClass)
		{
			NewMob->DefaultWeaponClass = WeaponClass;
		}
	}
	return NewMob;
}
