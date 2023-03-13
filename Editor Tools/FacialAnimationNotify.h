// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "FacialAnimationNotify.generated.h"

/**
 *
 */
UCLASS()
class GLOBETHEATRE_API UFacialAnimationNotify : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "0.0", UIMax = "20.0"))
		float BlendSpeedModifier;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float AngryEyes;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float Angry;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float Shout;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float Distraught;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float Surprise;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float Bored;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float Happy;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float Drinking;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float Bite;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float Flirty;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float Worried;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float Wink;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float Cheer;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float SurprisedEyes;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float Smile;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float Beautiful;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float Chew;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float Drunk;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float Blink;

	//phonetics

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float mmmm;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float ffff;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float eeee;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float sshh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float oooo;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float ssss;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = "-1.0", UIMax = "1.0"))
		float llll;

	void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration);
	void NotifyTick(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float FrameDeltaTime);
	void NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation);

private:
	float blend;
	TMap<FName, float> expressions;

	void setMorphTargetValue(USkeletalMeshComponent* MeshComp, FName name, float value );
	void findModifiedMorphs(USkeletalMeshComponent* MeshComp, FName name, float value);
};
