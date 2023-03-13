// Fill out your copyright notice in the Description page of Project Settings.

//
#include "Public/MyPlayerController.h"

// engine
#include "UnrealMathUtility.h"
#include "Engine/DataTable.h"

// game
#include "Public/MyGameState.h"
#include "Public/MyUserWidget.h"
#include "Public/AssetReference.h"

//////////////////////////////////////////////////////////////////////////
// Overrides
AMYPlayerController::AMYPlayerController()
{
	bReplicates = true;
	IsControllerReady = false;

	static ConstructorHelpers::FObjectFinder<UDataTable> AssetLUT(TEXT("DataTable'/Game/Game/AssetsReferenceCpp.AssetsReferenceCpp'"));
	if (AssetLUT.Object != NULL)
	{
		DefaultSpectator 	= AssetLUT.Object->FindRow<FAssetReference>(FName("DefaultPawnClass"), FString(""))->Value;
		InGameSpectator 	= AssetLUT.Object->FindRow<FAssetReference>(FName("DefaultSpectatorClass"), FString(""))->Value;
		LobbyUIWidgetClass 	= AssetLUT.Object->FindRow<FAssetReference>(FName("PreGameLobbyWidgetClass"), FString(""))->Value;y
	}
}

void AMYPlayerController::BeginPlay()
{
	IsControllerReady = false;
	Super::BeginPlay();

	if (GetLocalRole() == ROLE_Authority)
	{
		if (Team == 0)
		{
			TeamName = "Green";
		}
		else
		{
			TeamName = "Purple";
		}
	}

	ServerSetReady();
}

//
void AMYPlayerController::DoRespawn()
{
	//destroy old pawn
	GetPawn()->Destroy();

	// valid game mode?
	AGameModeBase* gameModeBase = GetWorld()->GetAuthGameMode();
	if (gameModeBase == NULL)
	{
		return;
	}

	// find a spot to spawn the player
	AActor* startSpot = gameModeBase->ChoosePlayerStart(this);

	// spot is sensible?
	if (startSpot != nullptr && startSpot != NULL)
	{
		// spawn a new pawn
		gameModeBase->RestartPlayerAtPlayerStart(this, startSpot);
	}
}

// server
void AMYPlayerController::OnPawnPostDeath()
{
	AMYGameState* gameState = GetWorld()->GetGameState<AMYGameState>();
	ASyncedTimeManager* SyncedTimeManager = gameState->GetSyncedTimeManager();

	SyncedTimeManager->SetTimer
	(
		RespawnDelegateHandle,
		this,
		&AMYPlayerController::DoRespawn,
		10,
		false,
		0
	);

	// spectate
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	//AMYSpectator* NewSpectatorPawn = GetWorld()->SpawnActor<AMYSpectator>(InGameSpectator, ActorSpawnParams);
	//Possess(NewSpectatorPawn);
}

//////////////////////////////////////////////////////////////////////////
// Network Handling

void AMYPlayerController::ClientOnReady_Implementation()
{
	if ( LobbyUIWidget )
	{
		UMYUserWidget* MYReadyUIWidget = Cast<UMYUserWidget>(LobbyUIWidget);
		MYReadyUIWidget->ReceiveOnReady();
	}
}

void AMYPlayerController::ClientOnInitialised_Implementation()
{
	if (LobbyUIWidgetClass)
	{
		LobbyUIWidget = CreateWidget<UUserWidget>(GetGameInstance(), LobbyUIWidgetClass);
		UMYUserWidget* MYLobbyUIWidget = Cast<UMYUserWidget>(LobbyUIWidget);

		if (MYLobbyUIWidget)
		{
			MYLobbyUIWidget->AttachedController = this;
			MYLobbyUIWidget->AttachedPawn = nullptr;

			MYLobbyUIWidget->ReceivePostConstruction();
			MYLobbyUIWidget->AddToViewport(0);
		}
	}

	if (GameUIWidgetClass)
	{
		GameUIWidget = CreateWidget<UUserWidget>(GetGameInstance(), GameUIWidgetClass);
		UMYUserWidget* MYGameUIWidget = Cast<UMYUserWidget>(GameUIWidget);

		if (MYGameUIWidget)
		{
			MYGameUIWidget->AttachedController = this;
			MYGameUIWidget->AttachedPawn = nullptr;

			MYGameUIWidget->ReceivePostConstruction();
		}
	}
}

void AMYPlayerController::ClientOnPawnPossessed_Implementation(APawn* NewPawn)
{
	// remove lobby UI
	if (LobbyUIWidget)
	{
		LobbyUIWidget->RemoveFromViewport();
	}

	// add game UI
	if (GameUIWidget)
	{
		// is the new pawn a playerpawn?
		AMYPlayerPawn* playerPawn = Cast<AMYPlayerPawn>(NewPawn);
		if (playerPawn)
		{
			UMYUserWidget* MYGameUIWidget = Cast<UMYUserWidget>(GameUIWidget);
			if (MYGameUIWidget)
			{
				MYGameUIWidget->AttachedController = this;
				MYGameUIWidget->AttachedPawn = playerPawn;
				MYGameUIWidget->ReceivePostConstruction();
			}
		}

		// show game ui
		GameUIWidget->AddToViewport(1);
	}
}

//// Server
void AMYPlayerController::ServerSetReady_Implementation()
{
	if ( !IsControllerReady )
	{
		IsControllerReady = true;
		ClientOnReady();
	}
}

void AMYPlayerController::ServerLeaveReady_Implementation()
{
	if ( IsControllerReady )
	{
		IsControllerReady = false;
	}
}

//////////////////////////////////////////////////////////////////////////
// Server Replication

void AMYPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMYPlayerController, IsControllerReady);
}

//////////////////////////////////////////////////////////////////////////
// Validation
bool AMYPlayerController::ServerSetReady_Validate()
{
	return true;
}

bool AMYPlayerController::ServerLeaveReady_Validate()
{
	return true;
}