// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnrealNetwork.h"
#include "SyncedTimeManager.generated.h"


// the synchronisation logic in this file is derived from this article:
// https://medium.com/@invicticide/accurately-syncing-unreals-network-clock-87a3f9262594
//
//
// The purpose of this class is to set up timers that are synchronised with the server time
// i.e. timers will execute at the same timestamp on server and client.
//

DECLARE_DELEGATE(FSyncedTimerDelegate);

USTRUCT()
struct FSyncedTimer
{
	GENERATED_BODY()

public:
	float Rate;
	float LastTimestamp;
	bool Loop = false;
	bool IsValid = false;
	unsigned int GUID;

	FSyncedTimerDelegate TimerMethod;
	FSyncedTimer* externalPtr;

	FORCEINLINE bool operator == (const FSyncedTimer& timer) const
	{
		if (GUID == timer.GUID)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};

UCLASS()
class CITYSHOOTER_API ASyncedTimeManager : public AActor
{
	GENERATED_BODY()

	TArray<int32> DirtyHandles;

	unsigned int timerGUID;

protected:
	float SyncedTime = 0.f;

	TArray<FSyncedTimer> SyncedTimerHandles;

public:
	ASyncedTimeManager();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Reliable, Client)
	void ClientReportServerTime(float requestWorldTime,	float serverTime);
	void ClientReportServerTime_Implementation(float requestWorldTime, float serverTime);

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerRequestServerTime(ASyncedTimeManager* requester, float requestWorldTime);
	void ServerRequestServerTime_Implementation(ASyncedTimeManager* requester, float requestWorldTime);
	bool ServerRequestServerTime_Validate(ASyncedTimeManager* requester, float requestWorldTime);

	float GetSyncedWorldTimeSeconds() const;
	void SyncClock();

	template< class UserClass >
	FORCEINLINE void SetTimer(FSyncedTimer& SyncedTimerHandle,
		UserClass* InObj,
		typename FSyncedTimerDelegate::TUObjectMethodDelegate< UserClass >::FMethodPtr InTimerMethod,
		float InRate, bool InbLoop, float InFirstDelay)
	{
		int index = SyncedTimerHandles.AddDefaulted();
		SyncedTimerHandles[index].Rate = InRate;
		SyncedTimerHandles[index].LastTimestamp = SyncedTime + InFirstDelay;
		SyncedTimerHandles[index].Loop = InbLoop;
		SyncedTimerHandles[index].IsValid = true;
		SyncedTimerHandles[index].GUID = timerGUID;
		SyncedTimerHandles[index].TimerMethod.BindUObject(InObj, InTimerMethod);
		SyncedTimerHandles[index].externalPtr = &SyncedTimerHandle;

		SyncedTimerHandle = SyncedTimerHandles[index];

		timerGUID++;
	}

	void ClearTimer(FSyncedTimer& SyncedTimerHandle);
};
