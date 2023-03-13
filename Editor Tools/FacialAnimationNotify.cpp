// Fill out your copyright notice in the Description page of Project Settings.

#include "FacialAnimationNotify.h"

void UFacialAnimationNotify::NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration)
{
	findModifiedMorphs(MeshComp, FName("AngryEyes"), AngryEyes);
	findModifiedMorphs(MeshComp, FName("Angry"), Angry);
	findModifiedMorphs(MeshComp, FName("Shout"), Shout);
	findModifiedMorphs(MeshComp, FName("Distraught"), Distraught);
	findModifiedMorphs(MeshComp, FName("Surprise"), Surprise);
	findModifiedMorphs(MeshComp, FName("Bored"), Bored);
	findModifiedMorphs(MeshComp, FName("Happy"), Happy);
	findModifiedMorphs(MeshComp, FName("Drinking"), Drinking);
	findModifiedMorphs(MeshComp, FName("Bite"), Bite);
	findModifiedMorphs(MeshComp, FName("Flirty"), Flirty);
	findModifiedMorphs(MeshComp, FName("Worried"), Worried);
	findModifiedMorphs(MeshComp, FName("Wink"), Wink);
	findModifiedMorphs(MeshComp, FName("Cheer"), Cheer);
	findModifiedMorphs(MeshComp, FName("SurprisedEyes"), SurprisedEyes);
	findModifiedMorphs(MeshComp, FName("Smile"), Smile);
	findModifiedMorphs(MeshComp, FName("Beautiful"), Beautiful);
	findModifiedMorphs(MeshComp, FName("Chew"), Chew);
	findModifiedMorphs(MeshComp, FName("Drunk"), Drunk);
	findModifiedMorphs(MeshComp, FName("Blink"), Blink);

	findModifiedMorphs(MeshComp, FName("mmmm"), mmmm);
	findModifiedMorphs(MeshComp, FName("ffff"), ffff);
	findModifiedMorphs(MeshComp, FName("eeee"), eeee);
	findModifiedMorphs(MeshComp, FName("sshh"), sshh);
	findModifiedMorphs(MeshComp, FName("oooo"), oooo);
	findModifiedMorphs(MeshComp, FName("ssss"), ssss);
	findModifiedMorphs(MeshComp, FName("llll"), llll);
}

void UFacialAnimationNotify::NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
	expressions.Empty();
}

void UFacialAnimationNotify::NotifyTick(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float FrameDeltaTime)
{
	for (auto It = expressions.CreateConstIterator(); It; ++It)
	{
		float lerped = FMath::LerpStable(MeshComp->GetMorphTarget(It.Key()), It.Value(), FrameDeltaTime*10*BlendSpeedModifier);
		setMorphTargetValue(MeshComp, It.Key(), lerped);

		if (lerped < 0.0001)
		{
			expressions.Remove(It.Key());
		}
	}
}

void UFacialAnimationNotify::setMorphTargetValue(USkeletalMeshComponent* MeshComp, FName name, float value)
{
	MeshComp->SetMorphTarget(name, value);
}


void UFacialAnimationNotify::findModifiedMorphs(USkeletalMeshComponent* MeshComp, FName name, float value)
{
	// find if an animation  needs to change, if so add it to the TMap

	float foundValue = MeshComp->GetMorphTarget(name);

	if (fabs(foundValue - value) > 0.000001)
	{
		expressions.Add(name, value);
	}

	//expressions
}