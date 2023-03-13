// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UObject/Object.h"

#include "TableRowSkillTrees.generated.h"

USTRUCT(BlueprintType)
struct FREEWORLD_API FTableRowSkillTrees : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName DisplayName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString Description;	
};
