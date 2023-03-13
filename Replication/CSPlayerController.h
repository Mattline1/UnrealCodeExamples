// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// engine
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Engine.h"
#include "UnrealNetwork.h"
#include "Engine/DataTable.h"
#include "Blueprint/UserWidget.h"

// game
#include "Public/MYSpectatorPawn.h"
#include "Public/SyncedTimeManager.h"

// gen
#include "MYPlayerController.generated.h"

/**
 *
 */
UCLASS()
class CITYSHOOTER_API AMYPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay, Replicated)
	uint8 Team = 0;

	UPROPERTY(BlueprintReadOnly, Category = Gameplay, Replicated)
	FName TeamName = "Green";

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	bool IsControllerReady = false;

protected:
	FSyncedTimer RespawnDelegateHandle;
	//TArray< FSyncedTimer > CardDrawDelegates;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	TSubclassOf<class AMYSpectatorPawn> DefaultSpectator = NULL;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	TSubclassOf<class AMYSpectatorPawn> InGameSpectator = NULL;

	// Lobby UI
	UPROPERTY()
	TSubclassOf<class UUserWidget> LobbyUIWidgetClass;

	UPROPERTY()
	TSubclassOf<class UUserWidget> GameUIWidgetClass;

public:
	UPROPERTY()
	UUserWidget* LobbyUIWidget = nullptr;

	UPROPERTY()
	UUserWidget* GameUIWidget = nullptr;

public:
	// overrides
	AMYPlayerController();
	virtual void BeginPlay();

	// pawn functions
	UFUNCTION(BlueprintCallable)
	void DoRespawn();

	UFUNCTION(BlueprintCallable)
	void OnPawnPostDeath();

public:
	UFUNCTION(Client, Reliable)
	void ClientOnInitialised();
	void ClientOnInitialised_Implementation();

	UFUNCTION(Reliable, Client)
	void ClientOnPawnPossessed(APawn* NewPawn);
	void ClientOnPawnPossessed_Implementation(APawn* NewPawn);

protected:
	// client
	UFUNCTION(Reliable, Client)
	void ClientOnReady();
	void ClientOnReady_Implementation();

	// server
	UFUNCTION(BlueprintCallable, Reliable, Server, WithValidation)
	void ServerSetReady();
	void ServerSetReady_Implementation();
	bool ServerSetReady_Validate();

	UFUNCTION(BlueprintCallable, Reliable, Server, WithValidation)
	void ServerLeaveReady();
	void ServerLeaveReady_Implementation();
	bool ServerLeaveReady_Validate();
};
