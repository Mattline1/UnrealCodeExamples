// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "MyGame/Public/Mob.h"
#include "AISpawnPoint.generated.h"

UCLASS()
class MYGAME_API AAISpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	AAISpawnPoint(const class FObjectInitializer& ObjectInitializer);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent * Capsule;

	UPROPERTY(EditAnywhere, Category = SpawnVariables)
	bool TriggeredSpawn;

	UPROPERTY(EditAnywhere, Category = SpawnVariables)
	UClass* MobClass;

	UPROPERTY(EditAnywhere, Category = SpawnVariables)
	UClass* WeaponClass;

	UFUNCTION(BlueprintCallable, Category = SpawnFunction)
		AMob* SpawnMob(AActor* SpawnedBy);

};
