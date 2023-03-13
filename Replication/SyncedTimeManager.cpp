// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/SyncedTimeManager.h"
#include "Engine.h"

ASyncedTimeManager::ASyncedTimeManager()
{
	//bReplicates = true;
	//NetPriority = 0.2f;
	//NetUpdateFrequency = 1.0f;

	timerGUID = 0;

	UWorld* const World = GetWorld();
	if (World != NULL)
	{
		SyncedTime = World->GetTimeSeconds();
	}

	PrimaryActorTick.bCanEverTick = true;
}

float ASyncedTimeManager::GetSyncedWorldTimeSeconds() const
{
	return SyncedTime;
}

void ASyncedTimeManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() == ROLE_Authority) {
		SyncedTime = GetWorld()->GetTimeSeconds();
	}
	else {
		SyncedTime += DeltaTime;
	}

	for (int i = 0; i < SyncedTimerHandles.Num(); i++)
	{
		FSyncedTimer* Timer = &SyncedTimerHandles[i];

		if ((SyncedTime - Timer->LastTimestamp) > Timer->Rate)
		{
			Timer->LastTimestamp = SyncedTime;
			Timer->TimerMethod.ExecuteIfBound();

			if (!Timer->Loop)
			{
				DirtyHandles.Add(i);
			}
		}
	}

	// indices are implicitly sorted during the execution loop above
	// this means the index to delete will never be a lower value than those that
	// have already been deleted.
	// so...
	// we can safely decrement the indices as the array shrinks
	// and the decremented index will refer to the same element in the array

	if (DirtyHandles.Num() > 0)
	{
		int counter = 0;
		for (auto& index : DirtyHandles)
		{
			if (SyncedTimerHandles.IsValidIndex(index))
			{
				//falsify external object
				SyncedTimerHandles[index].externalPtr->IsValid = false;
				SyncedTimerHandles.RemoveAt(index - counter, 1, false);
			}
			counter++;
		}
		DirtyHandles.Empty();
		SyncedTimerHandles.Shrink();
	}
}

void ASyncedTimeManager::SyncClock()
{
	ServerRequestServerTime(this, GetWorld()->GetTimeSeconds());
}

void ASyncedTimeManager::ServerRequestServerTime_Implementation(ASyncedTimeManager* requester, float requestWorldTime )
{
	float serverTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	ClientReportServerTime(requestWorldTime, serverTime);
}

bool ASyncedTimeManager::ServerRequestServerTime_Validate(ASyncedTimeManager* requester, float requestWorldTime)
{
	return true;
}

void ASyncedTimeManager::ClientReportServerTime_Implementation(	float requestWorldTime,	float serverTime )
{
	// Apply the round-trip request time to the server's
	// reported time to get the up-to-date server time
	float roundTripTime = GetWorld()->GetTimeSeconds() - requestWorldTime;
	float adjustedTime = serverTime + (roundTripTime * 0.5f);
	SyncedTime = adjustedTime;
}

void ASyncedTimeManager::ClearTimer(FSyncedTimer& SyncedTimerHandle)
{
	int index = SyncedTimerHandles.Find(SyncedTimerHandle);
	if (index != INDEX_NONE)
	{
		SyncedTimerHandles.RemoveAt(index);
	}
	SyncedTimerHandle.IsValid = false;
}
