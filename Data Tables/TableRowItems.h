// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UObject/Object.h"

#include "TableRowItems.generated.h"

USTRUCT(BlueprintType)
struct FREEWORLD_API FTableRowItems : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	FTableRowItems() : Texture(NULL) {} //constructor
		
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName Type;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FString Description;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<class UObject> Texture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName Parent;

private:
	//FName Parent;
};
