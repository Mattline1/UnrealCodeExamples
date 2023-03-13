// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UObject/Object.h"

#include "TableRowSkills.generated.h"

USTRUCT(BlueprintType)
struct FREEWORLD_API FTableRowSkills : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	FTableRowSkills() : Texture(NULL) {} //constructor

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName DisplayName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int Cost; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int	CostCurrency;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int Gain;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int	GainCurrency;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSet<FName> PrerequisitesOr; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSet<FName> PrerequisitesAnd;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString Description;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<class UObject> Texture;
};
