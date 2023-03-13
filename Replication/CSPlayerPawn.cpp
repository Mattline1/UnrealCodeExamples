// Fill out your copyright notice in the Description page of Project Settings.

//
#include "Public/MYPlayerPawn.h"

//engine
#include "Engine.h"
#include "UnrealNetwork.h"
#include "Kismet/GameplayStatiMY.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/CharacterMovementComponent.h"

//game
#include "Public/MYGameState.h"
#include "Public/MYUserWidget.h"
#include "Public/MYAnimInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// init
AMYPlayerPawn::AMYPlayerPawn()
{
	bReplicates = true;
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	bIsFirstPerson = true;

	// hide 3rd person
	GetMesh()->SetOwnerNoSee(true);

	//
	HitBox = CreateDefaultSubobject<UCapsuleComponent>(TEXT("HitBoxComponent"));
	HitBox->SetupAttachment(GetCapsuleComponent());
	HitBox->SetCollisionProfileName("HitBox");

	LedgeTracePoint = CreateDefaultSubobject<USceneComponent>(TEXT("LedgeTracePoint"));
	LedgeTracePoint->SetupAttachment(GetCapsuleComponent());

	// Attach camera boom
	FirstPersonCameraBoom = CreateDefaultSubobject<USceneComponent>(TEXT("FirstPersonCameraBoom"));
	FirstPersonCameraBoom->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraBoom->SetRelativeLocation(FVector(0.0, 0.0, BaseEyeHeight));

	// Create a CameraComponent
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonCameraBoom);

	// Attach Muzzle
	MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePointComponent"));
	MuzzlePoint->SetupAttachment(FirstPersonCameraComponent);

	// third person components
	ThirdPersonSpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdPersonSpringArm"));
	ThirdPersonSpringArmComponent->SetupAttachment(FirstPersonCameraBoom);
	ThirdPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCameraComponent->SetupAttachment(ThirdPersonSpringArmComponent);

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterFirstPersonMesh"));
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->SetupAttachment(FirstPersonCameraComponent);
	FirstPersonMesh->bCastDynamiMYhadow = false;
	FirstPersonMesh->CastShadow = false;
	FirstPersonMesh->SetOwnerNoSee(false);

	LHandLocation = CreateDefaultSubobject<USceneComponent>(TEXT("LHandLocation"));
	LHandLocation->SetupAttachment(FirstPersonMesh);
	RHandLocation = CreateDefaultSubobject<USceneComponent>(TEXT("RHandLocation"));
	RHandLocation->SetupAttachment(FirstPersonMesh);
}

void AMYPlayerPawn::BeginPlay()
{
	r_Health = MaxHealth;

	// Call the base class
	Super::BeginPlay();

	// Spawn default weapon
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AMYWeaponBase* NewWeapon = GetWorld()->SpawnActor<AMYWeaponBase>(
		DefaultWeaponClass,
		RHandLocation->GetComponentLocation(),
		RHandLocation->GetComponentRotation(),
		ActorSpawnParams
		);

	NewWeapon->OnPickup(this);
	NewWeapon->SetVisibility(true);

	// fill weapon array - 7
	r_WeaponArray.Add(nullptr);
	r_WeaponArray.Add(NewWeapon);
	r_WeaponArray.Add(nullptr);
	r_WeaponArray.Add(nullptr);

	r_WeaponArray.Add(nullptr);
	r_WeaponArray.Add(nullptr);
	r_WeaponArray.Add(nullptr);
	r_WeaponArray.Add(nullptr);

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	LHandLocation->AttachToComponent(FirstPersonMesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("LHandLocation"));
	RHandLocation->AttachToComponent(FirstPersonMesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("RHandLocation"));

	//FirstPersonMesh->SetHiddenInGame(false, true);
	r_ActiveWeaponIndex = (uint8)EWeaponType::WT_PISTOL;

	// walk speed
	UCharacterMovementComponent* CharacterMovementComponentPtr = Cast<UCharacterMovementComponent>(GetCharacterMovement());
	CharacterMovementComponentPtr->MaxWalkSpeed = BaseWalkSpeed;

	CameraZ		= FirstPersonCameraBoom->GetComponentLocation().Z;
	CameraZBase = FirstPersonCameraBoom->GetRelativeLocation().Z;

	// start first person
	GoFirstPerson();
}

void AMYPlayerPawn::Tick(float DeltaTime) {

	FRotator PawnControlRotation = GetViewRotation();
	//Super::Tick(DeltaTime);

	// Control Rotations for components
	FirstPersonCameraBoom->SetWorldRotation(PawnControlRotation);

	// calculate difference between player look rotation and actor rotation
	float A = FMath::DegreesToRadians(FMath::UnwindDegrees(PawnControlRotation.Yaw));
	float B = FMath::DegreesToRadians(FMath::UnwindDegrees(GetActorRotation().Yaw));
	AngleDiff = FMath::Atan2(sin(A - B), cos(A - B));
	float AbsAngleDiff = FMath::Abs(AngleDiff);

	// send angle to animations
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance != nullptr && animInstance->IsA<UMYAnimInstance>())
	{
		UMYAnimInstance* animMYInstance = Cast<UMYAnimInstance>(animInstance);
		animMYInstance->PawnControlRotation.Pitch = PawnControlRotation.Pitch;
		animMYInstance->PawnControlRotation.Yaw	= FMath::RadiansToDegrees(AngleDiff);
	}

	if (r_bIsTakingCover)
	{
		if (AbsAngleDiff > 2.0f)
		{
			if (GetLocalRole() == ROLE_Authority)
			{
				ServerOnSwitchToCover();
			}
			TurnToView();
		}
	}
	else
	{
		// if greater than a threshold, reorient player
		if (AbsAngleDiff > 1.0f)
		{
			TurnToView();
		}

		// do extra movement modes
		// will skip unless needed, don't worry about performance
		HandleAdvancedMovement();
	}
};

void AMYPlayerPawn::HandleAdvancedMovement()
{
	ASyncedTimeManager* SyncedTimeManager = GetWorld()->GetGameState<AMYGameState>()->GetSyncedTimeManager();
	FHitResult OutHit;
	FVector FrontalTriggerOffset;
	float AdjustedDuration;
	float NormalisedDuration;
	float Rate;
	float Duration;
	float delta;
	float y;
	float z;

	//
	// const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EAlternativeMovementModes"), true);
	// FName name = EnumPtr->GetNameByValue((int64)r_AlternativeMovementMode);
	// GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White, name.ToString());

	// non-basic movement mode active?
	switch (r_AlternativeMovementMode)
	{
		////////////////////////
		// MOVE_Waiting
		////////////////////////
	case EAlternativeMovementModes::MOVE_Waiting:

		Duration = SyncedTimeManager->GetSyncedWorldTimeSeconds() - StartAlternativeMovementTimeStamp;

		if (Duration > WaitDuration)
		{
			if (DisableTimerOnWaitForClimbWhileFalling == false  // if waiting should cancel after duration
				|| GetCharacterMovement()->IsFalling() == false) // or if on the ground
			{
				r_AlternativeMovementMode = EAlternativeMovementModes::MOVE_Basic;
				GetCharacterMovement()->MovementMode = MOVE_Falling;
			}
		}

		// low trigger?
		FrontalTriggerOffset = FVector(0, GetCapsuleComponent()->GetScaledCapsuleRadius(), GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.5);

		// low trace
		if (DoSceneComponentTrace(OutHit, GetCapsuleComponent(), -FrontalTriggerOffset, 100.0) == false)
		{
			if (GetCharacterMovement()->IsFalling() == false)
			{
				r_AlternativeMovementMode = EAlternativeMovementModes::MOVE_Basic;
				GetCharacterMovement()->MovementMode = MOVE_Falling;
			}
			break; // early end
		}

		// the function will have exited by now if the low trace missed

		// high trace ?
		if (DoSceneComponentTrace(OutHit, GetCapsuleComponent(), FrontalTriggerOffset, 100.0))
		{
			// high and low? transition to high
			r_AlternativeMovementMode = EAlternativeMovementModes::MOVE_Climbing;
			GetCharacterMovement()->MovementMode = MOVE_Flying;
			StopJumping();

			// does colliding with a climbable wall reset the climbing timer?
			if (DisableTimerOnWaitForClimbWhileFalling)
			{
				StartAlternativeMovementTimeStamp = SyncedTimeManager->GetSyncedWorldTimeSeconds();
			}
		}
		else
		{
			StartAlternativeMovementTimeStamp = SyncedTimeManager->GetSyncedWorldTimeSeconds();
			r_AlternativeMovementMode = EAlternativeMovementModes::MOVE_Vaulting;
			GetCharacterMovement()->MovementMode = MOVE_Flying;
			StopJumping();
		}
		break;

		////////////////////////
		// MOVE_Climbing
		////////////////////////
	case EAlternativeMovementModes::MOVE_Climbing:

		//trigger vault at end of climb?
		if (DoSceneComponentTrace(OutHit, LedgeTracePoint, LedgeTracePoint->GetRelativeLocation().Z))
		{
			StartAlternativeMovementTimeStamp = SyncedTimeManager->GetSyncedWorldTimeSeconds(); // reset timer for pullup
			r_AlternativeMovementMode = EAlternativeMovementModes::MOVE_PullUp;
		}
		else
		{
			// climb rate
			Duration = SyncedTimeManager->GetSyncedWorldTimeSeconds() - StartAlternativeMovementTimeStamp;
			AdjustedDuration = FMath::Max(Duration - ClimbSpeedGracePeriod, 0.0f);
			NormalisedDuration = AdjustedDuration / ClimbFalloffDuration;
			Rate = 1 - FMath::Clamp(NormalisedDuration, 0.f, 1.f);

			// move component
			AddMovementInput(GetActorUpVector() + GetActorForwardVector()*0.2, Rate);

			// end movement
			if (Rate < ClimbSpeedCutoff)
			{
				r_AlternativeMovementMode = EAlternativeMovementModes::MOVE_Basic;
				GetCharacterMovement()->MovementMode = MOVE_Falling;
			}
		}
		break;

		////////////////////////
		// MOVE_PullUp
		////////////////////////
	case EAlternativeMovementModes::MOVE_PullUp:
		Duration = SyncedTimeManager->GetSyncedWorldTimeSeconds() - StartAlternativeMovementTimeStamp;
		delta = (Duration / PullUpDuration) * PI;

		y = sinf(delta);
		z = cosf(delta);

		AddMovementInput(GetActorUpVector() * z + GetActorForwardVector() * y);

		if (Duration > PullUpDuration)
		{
			r_AlternativeMovementMode = EAlternativeMovementModes::MOVE_Basic;
			GetCharacterMovement()->MovementMode = MOVE_Falling;
		}

		break;

		////////////////////////
		// MOVE_Vaulting
		////////////////////////
	case EAlternativeMovementModes::MOVE_Vaulting:
		Duration = SyncedTimeManager->GetSyncedWorldTimeSeconds() - StartAlternativeMovementTimeStamp;
		delta = (Duration / VaultDuration) * PI;

		y = sinf(delta);
		z = cosf(delta);

		AddMovementInput(GetActorUpVector() * z + GetActorForwardVector() * y);

		if (Duration > VaultDuration)
		{
			r_AlternativeMovementMode = EAlternativeMovementModes::MOVE_Basic;
			GetCharacterMovement()->MovementMode = MOVE_Falling;
		}

		break;

		////////////////////////
		// MOVE_Basic
		////////////////////////
	case EAlternativeMovementModes::MOVE_Basic:
		break;
	}
}

// Player movement
void AMYPlayerPawn::SetRemoteViewYaw(float NewRemoteViewYaw)
{
	// Compress pitch to 1 byte
	NewRemoteViewYaw = FRotator::ClampAxis(NewRemoteViewYaw);
	r_RemoteViewYaw = (uint8)(NewRemoteViewYaw * 255.f / 360.f);
}

FRotator AMYPlayerPawn::GetViewRotation() const
{
	if (Controller)
	{
		return Controller->GetControlRotation();
	}

	return FRotator( RemoteViewPitch / 255.f * 360.f, r_RemoteViewYaw / 255.f * 360.f, 0.f );
}

void AMYPlayerPawn::MoveForward( float Value )
{
	if (Value != 0.0f && r_bIsSliding == false)
	{
		if (r_bIsTakingCover)
		{
			if (FMath::Abs(AngleDiff) < 1.0f)
			{
				if (Value < 0.0f)
				{
					ServerOnSwitchToCover();
				}
			}
			else
			{
				MoveRight(Value * FMath::Sign(AngleDiff));
			}
		}
		else
		{
			bool IsInAlternativeMovementMode = r_AlternativeMovementMode != EAlternativeMovementModes::MOVE_Basic;

			FVector forward = GetActorForwardVector();
			AddMovementInput(forward, IsInAlternativeMovementMode ? 0.2 : Value);

			TurnToView();
		}
	}
}

void AMYPlayerPawn::MoveRight( float Value )
{
	bool IsInAlternativeMovementMode = r_AlternativeMovementMode != EAlternativeMovementModes::MOVE_Basic;

	if (Value != 0.0f && r_bIsSliding == false && IsInAlternativeMovementMode == false)
	{
		if (r_bIsTakingCover) // if moving back whilst in cover
		{
			if (FMath::Abs(AngleDiff) >= 1.0f)
			{
				if (FMath::Sign(Value) == FMath::Sign(AngleDiff))
				{
					ServerOnSwitchToCover();
				}
			}
			else
			{
				AddMovementInput(GetActorRightVector(), Value);
			}
		}
		else
		{
			AddMovementInput(GetActorRightVector(), Value);
			TurnToView();
		}
	}
}

void AMYPlayerPawn::TurnAtRate( float Rate )
{
	AddControllerYawInput( Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() );
}

void AMYPlayerPawn::LookUpAtRate( float Rate )
{
	AddControllerPitchInput( Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds() );
}

void AMYPlayerPawn::TurnToView()
{
	if (r_bIsTakingCover == false)
	{
		FRotator newRotator = FRotator::ZeroRotator;
		newRotator.Yaw = GetViewRotation().Yaw;
		SetActorRotation(newRotator);
	}
}

// events
void AMYPlayerPawn::OnSprint()
{
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerOnSprint();
	}

	UCharacterMovementComponent* CharacterMovementComponentPtr = GetCharacterMovement();
	CharacterMovementComponentPtr->MaxWalkSpeed = BaseSprintSpeed;
}

void AMYPlayerPawn::OnEndSprint()
{
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerOnEndSprint();
	}
	UCharacterMovementComponent* CharacterMovementComponentPtr = GetCharacterMovement();
	CharacterMovementComponentPtr->MaxWalkSpeed = BaseWalkSpeed;
}

/*
void AMYPlayerPawn::OnCrouch_Slide()
{
	if (r_bIsTakingCover == false)
	{
		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			ServerOnCrouch_Slide();
		}

		Crouch(); // replicates
		FirstPersonCameraBoom->GetRelativeLocation() = FVector(0.0, 0.0, CameraZBase / 2);

		//slide
		UCharacterMovementComponent* CharacterMovementComponentPtr = GetCharacterMovement();

		// is sprinting?
		if (CharacterMovementComponentPtr->MaxWalkSpeed >= BaseSprintSpeed - 10) // buffer to account for possible difference
		{
			// sliiiiide
			r_bIsSliding = true;
			CharacterMovementComponentPtr->BrakingFrictionFactor = BrakingFrictionFactorSlide;
			CharacterMovementComponentPtr->BrakingDecelerationWalking = BrakingDecelerationSlide;
			CharacterMovementComponentPtr->SetWalkableFloorAngle(15.0f);

			r_AlternativeMovementMode = EAlternativeMovementModes::MOVE_Basic; // slide cancels other movement

			FirstPersonCameraBoom->GetRelativeLocation() = FVector(0.0, 0.0, CameraZBase / 4); //lower on slide
		};

		bIsPlayersChoiceToCrouch = true;
	}
}

void AMYPlayerPawn::OnEndCrouch_Slide()
{
	if (r_bIsTakingCover == false)
	{
		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			ServerOnEndCrouch_Slide();
		}

		UnCrouch(); // replicates
		FirstPersonCameraBoom->GetRelativeLocation() = FVector(0.0, 0.0, CameraZBase);

		//slide
		UCharacterMovementComponent* CharacterMovementComponentPtr = GetCharacterMovement();
		CharacterMovementComponentPtr->BrakingFrictionFactor = 1.0f;
		CharacterMovementComponentPtr->BrakingDecelerationWalking = BrakingDecelerationWalking;
		CharacterMovementComponentPtr->SetWalkableFloorAngle(44.0f);

		r_bIsSliding = false;
		bIsPlayersChoiceToCrouch = false;
	}
}
*/

void AMYPlayerPawn::OnJump()
{
	Jump();

	if ( GetCharacterMovement()->IsFalling() == false
		&& r_bIsSliding == false)
	{
		r_AlternativeMovementMode = EAlternativeMovementModes::MOVE_Waiting; // may cancel jump

		ASyncedTimeManager* SyncedTimeManager = GetWorld()->GetGameState<AMYGameState>()->GetSyncedTimeManager();
		StartAlternativeMovementTimeStamp = SyncedTimeManager->GetSyncedWorldTimeSeconds();
	}

	// echo to server
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerOnJump();
	}
}

void AMYPlayerPawn::OnEndJump()
{
	r_AlternativeMovementMode = EAlternativeMovementModes::MOVE_Basic;
	GetCharacterMovement()->MovementMode = MOVE_Falling;

	// echo to server
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerOnEndJump();
	}
}

// view
void AMYPlayerPawn::GoThirdPerson()
{
	bIsFirstPerson = false;
	FirstPersonMesh->SetOwnerNoSee(true);
	GetMesh()->SetOwnerNoSee(false);
}

void AMYPlayerPawn::GoFirstPerson()
{
	bIsFirstPerson = true;
	FirstPersonMesh->SetOwnerNoSee(false);
	GetMesh()->SetOwnerNoSee(true);
}

void AMYPlayerPawn::SwitchViewModes()
{
	if (!r_bIsTakingCover)
	{
		if (bIsFirstPerson)
		{
			GoThirdPerson();
			bIsPlayersChoiceFirstPerson = false;
		}
		else
		{
			GoFirstPerson();
			bIsPlayersChoiceFirstPerson = true;
		}
	}
}

// weapons
void AMYPlayerPawn::OnReload()
{
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerOnReload();
		Animate("Reload");
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		// reload gun
		if (r_WeaponArray[r_ActiveWeaponIndex] != nullptr)
		{
			r_WeaponArray[r_ActiveWeaponIndex]->OnReload();
		}
	}
}

void AMYPlayerPawn::OnFirePressed()
{
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerOnFirePressed(); // send action to server
	}

	// Weapon replicates if available
	if (r_WeaponArray[r_ActiveWeaponIndex] != nullptr)
	{
		r_WeaponArray[r_ActiveWeaponIndex]->OnStartFire();
	}
}

void AMYPlayerPawn::OnFireReleased()
{
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerOnFireReleased(); // send action to server
	}

	if (GetLocalRole() == ROLE_Authority)
	{

	}

	// Weapon replicates if available
	if (r_WeaponArray[r_ActiveWeaponIndex] != nullptr)
	{
		r_WeaponArray[r_ActiveWeaponIndex]->OnEndFire();
	}
}

void AMYPlayerPawn::OnAltFirePressed()
{
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerOnAltFirePressed(); // send action to server
	}

	if (GetLocalRole() == ROLE_Authority)
	{

	}

	if (r_WeaponArray[r_ActiveWeaponIndex] != nullptr)
	{
		r_WeaponArray[r_ActiveWeaponIndex]->OnStartAltFire();
	}
}

void AMYPlayerPawn::OnAltFireReleased()
{
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerOnAltFireReleased(); // send action to server
	}

	if (GetLocalRole() == ROLE_Authority)
	{

	}

	if (r_WeaponArray[r_ActiveWeaponIndex] != nullptr)
	{
		r_WeaponArray[r_ActiveWeaponIndex]->OnEndAltFire();
	}
}

template<int val>
void AMYPlayerPawn::OnSwitchWeapon()
{
	ServerOnSwitchWeapon(val);
}


// interaction
void AMYPlayerPawn::OnInteractPressed()
{
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerOnInteractPressed(); // send action to server
	}

	if (GetLocalRole() == ROLE_Authority)
	{

	}
}

void AMYPlayerPawn::OnInteractReleased()
{
	//raycast into scene
	FHitResult OutHit;
	DoFaceTrace(OutHit);

	if (OutHit.Actor != nullptr)
	{
		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			ServerOnInteractReleased();
		}

		// weapon / player requires server permission
		if (GetLocalRole() == ROLE_Authority)
		{
			// is a weapon?
			if (OutHit.Actor->IsA<AMYWeaponBase>())
			{
				AMYWeaponBase* NewWeapon = Cast<AMYWeaponBase>(OutHit.Actor);

				// valid index?
				if (r_WeaponArray.IsValidIndex((uint8)NewWeapon->_WeaponType))
				{
					// get old weapon
					AMYWeaponBase* OldWeapon = r_WeaponArray[(uint8)NewWeapon->_WeaponType];
					bool wasDropped = false;

					// drop current if possible - skip if null
					if (OldWeapon != nullptr && OldWeapon->CanBeDropped)
					{
						OldWeapon->OnDrop();
						wasDropped = true;
					}

					// old weapon dropped?
					if (OldWeapon == nullptr || wasDropped)
					{
						// switch to new weapon
						NewWeapon->OnPickup(this);
						NewWeapon->SetVisibility((uint8)NewWeapon->_WeaponType == r_ActiveWeaponIndex);

						r_WeaponArray[(uint8)NewWeapon->_WeaponType] = NewWeapon; // triggers OnRep_WeaponArray for clients
					}
				}
			}
			// is a player?
			else if (OutHit.Actor->IsA<AMYPlayerPawn>())
			{

			}
		}
	}
}

void AMYPlayerPawn::OnAltInteractPressed()
{
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerOnAltInteractPressed();
	}

	if (GetLocalRole() == ROLE_Authority)
	{

	}
}

void AMYPlayerPawn::OnAltInteractReleased()
{
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerOnAltInteractReleased();
	}

	if (GetLocalRole() == ROLE_Authority)
	{

	}
}


void AMYPlayerPawn::DoDeath()
{
}

// server
void AMYPlayerPawn::DoPostDeathCleanup()
{
	AMYPlayerController* controller = GetController<AMYPlayerController>();

	if (controller == NULL)
	{
		return;
	}

	// no longer findable
	Tags.Remove(controller->TeamName);

	// tell attached spectators to sod off
	/*
	for (auto Child : Children)
	{
		if (Child->IsA(AMYSpectator::StaticClass()))
		{
			AMYSpectator* CastChild = Cast<AMYSpectator>(Child);

			if (CastChild)
			{
				CastChild->ServerSwitchTarget();
			}
		}
	}
	*/

	controller->OnPawnPostDeath();
	Destroy();
}

//////////////////////////////////////////////////////////////////////////
// external animation states imposed on the player

void AMYPlayerPawn::Animate(FString Name)
{
	// animate 1st person
	if (FirstPersonMesh->GetAnimInstance() != nullptr &&
		FirstPersonMesh->GetAnimInstance()->IsA<UMYAnimInstance>())
	{
		UMYAnimInstance* animInstance = Cast<UMYAnimInstance>(FirstPersonMesh->GetAnimInstance());
		animInstance->PlayAnimationMontage(Name);
	}

	// animate 3rd person
	if (GetMesh()->GetAnimInstance() != nullptr &&
		GetMesh()->GetAnimInstance()->IsA<UMYAnimInstance>())
	{
		UMYAnimInstance* animInstance = Cast<UMYAnimInstance>(GetMesh()->GetAnimInstance());
		animInstance->PlayAnimationMontage(Name);
	}
}

void AMYPlayerPawn::ForceRecoil()
{
	Animate("Recoil");
}


//////////////////////////////////////////////////////////////////////////
// utils

bool AMYPlayerPawn::DoFaceTrace(FHitResult& Hit) {

	FVector StartLocation = FirstPersonCameraComponent->GetComponentLocation();
	FVector EndLocation = StartLocation + (FirstPersonCameraComponent->GetForwardVector() * InteractionDistance);

	bool success = GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECollisionChannel::ECC_Visibility);
	return success;
}

bool AMYPlayerPawn::DoSceneComponentTrace(FHitResult& Hit, USceneComponent* component, float TraceDistance) {

	FVector StartLocation = component->GetComponentLocation();
	FVector EndLocation = StartLocation + (component->GetForwardVector() * TraceDistance);

	bool success = GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECollisionChannel::ECC_Visibility);
	return success;
}

bool AMYPlayerPawn::DoSceneComponentTrace(FHitResult& Hit, USceneComponent* component, FVector Offset, float TraceDistance) {

	FVector StartLocation = component->GetComponentLocation() + component->GetComponentRotation().RotateVector(Offset);
	FVector EndLocation = StartLocation + (component->GetForwardVector() * TraceDistance);

	bool success = GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECollisionChannel::ECC_Visibility);
	return success;
}

bool AMYPlayerPawn::DoSceneComponentTraceFlattened(FHitResult& Hit, USceneComponent* component, FVector Offset, float TraceDistance) {

	FVector StartLocation = component->GetComponentLocation() + component->GetComponentRotation().RotateVector(Offset);

	FVector ForwardVector = component->GetForwardVector();
	ForwardVector.Z = 0.0f;
	ForwardVector.Normalize();

	FVector EndLocation = StartLocation + (ForwardVector * TraceDistance);

	bool success = GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECollisionChannel::ECC_Visibility);
	return success;
}

bool AMYPlayerPawn::IsAbleToClimb()
{
	//DoSceneComponentTrace
	return false;
};


//////////////////////////////////////////////////////////////////////////
// on replication functions
void AMYPlayerPawn::OnRep_Health()
{
	if (r_Health <= 0)
	{
		GoThirdPerson();
	}
}

void AMYPlayerPawn::OnRep_WeaponArray()
{
	for (auto Wep : r_WeaponArray)
	{
		if (Wep != nullptr)
		{
			Wep->OnPickup(this);
			Wep->SetVisibility((uint8)Wep->_WeaponType == r_ActiveWeaponIndex);
		}
	}

	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);

	for (auto Obj : AttachedActors)
	{
		// is this the old weapon?
		if (Obj->IsA<AMYWeaponBase>() && r_WeaponArray.Contains(Obj) == false)
		{
			Cast<AMYWeaponBase>(Obj)->OnDrop();
		}
	}
}

void AMYPlayerPawn::OnRep_ActiveWeaponIndex()
{
	for (auto Wep : r_WeaponArray)
	{
		if (Wep != nullptr)
		{
			Wep->SetVisibility((uint8)Wep->_WeaponType == r_ActiveWeaponIndex);
		}
	}
}

void AMYPlayerPawn::OnRep_bIsTakingCover()
{
	if (r_bIsTakingCover)
	{
		GoThirdPerson();
	}
	else
	{
		if (bIsPlayersChoiceFirstPerson)
		{
			GoFirstPerson();
		}
	}
}

void AMYPlayerPawn::OnRep_bIsCrouchForced()
{
	if (r_bIsCrouchForced)
	{
		Crouch();
	}
	else if (!bIsPlayersChoiceToCrouch)
	{
		UnCrouch();
	}
}


//////////////////////////////////////////////////////////////////////////
// overrides
bool AMYPlayerPawn::CanCrouch() const
{
	return ( (!r_bIsTakingCover || r_bIsCrouchForced) && this->ACharacter::CanCrouch() );
}

void AMYPlayerPawn::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	FirstPersonCameraBoom->SetRelativeLocation(FVector(0.0, 0.0, CameraZBase / 2));

	//slide
	UCharacterMovementComponent* CharacterMovementComponentPtr = GetCharacterMovement();

	// is sprinting?
	if (CharacterMovementComponentPtr->MaxWalkSpeed >= BaseSprintSpeed - 10) // buffer to account for possible difference
	{
		// sliiiiide
		r_bIsSliding = true;
		CharacterMovementComponentPtr->BrakingFrictionFactor = BrakingFrictionFactorSlide;
		CharacterMovementComponentPtr->BrakingDecelerationWalking = BrakingDecelerationSlide;
		CharacterMovementComponentPtr->SetWalkableFloorAngle(15.0f);

		r_AlternativeMovementMode = EAlternativeMovementModes::MOVE_Basic; // slide cancels other movement

		FirstPersonCameraBoom->SetRelativeLocation(FVector(0.0, 0.0, CameraZBase / 4)); //lower on slide
	};
}

void AMYPlayerPawn::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	FirstPersonCameraBoom->SetRelativeLocation(FVector(0.0, 0.0, CameraZBase));

	//slide
	UCharacterMovementComponent* CharacterMovementComponentPtr = GetCharacterMovement();
	CharacterMovementComponentPtr->BrakingFrictionFactor = 1.0f;
	CharacterMovementComponentPtr->BrakingDecelerationWalking = BrakingDecelerationWalking;
	CharacterMovementComponentPtr->SetWalkableFloorAngle(44.0f);

	r_bIsSliding = false;
}



//////////////////////////////////////////////////////////////////////////
// Network Handling

// server sprint
void AMYPlayerPawn::ServerOnSprint_Implementation()
{
	OnSprint();
}

void AMYPlayerPawn::ServerOnEndSprint_Implementation()
{
	OnEndSprint();
}

// jump / climb / hang
void AMYPlayerPawn::ServerOnJump_Implementation()
{
	OnJump();
}

void AMYPlayerPawn::ServerOnEndJump_Implementation()
{
	OnEndJump();
}

// take cover
void AMYPlayerPawn::ServerOnSwitchToCover_Implementation()
{
	FHitResult OutHit;
	FVector FrontalTriggerOffset;

	// is a toggle
	if (r_bIsTakingCover)
	{
		r_bIsTakingCover  = false;
		r_bIsCrouchForced = false;
	}
	else
	{
		// low trigger?
		FrontalTriggerOffset = FVector(0.0f, 0.0f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.5);

		// low trace
		if (DoSceneComponentTraceFlattened(OutHit, FirstPersonCameraBoom, -FrontalTriggerOffset, 150.0))
		{
			FVector LowNormal = OutHit.Normal;
			// high trace
			if (DoSceneComponentTraceFlattened(OutHit, FirstPersonCameraBoom, FrontalTriggerOffset, 150.0) == false)
			{
				// crouch on high trace fail
				r_bIsCrouchForced = true;
			}

			// set rotation
			FRotator newRotator = (-LowNormal).ToOrientationRotator();
			SetActorRotation(newRotator);

			//on_rep client
			r_bIsTakingCover = true;
		}
	}

	// call replication functions on server
	OnRep_bIsTakingCover();
	OnRep_bIsCrouchForced();
}

// server reload
void AMYPlayerPawn::ServerOnReload_Implementation()
{
	OnReload();
}

// fire
void AMYPlayerPawn::ServerOnFirePressed_Implementation()
{
	OnFirePressed();
}

void AMYPlayerPawn::ServerOnFireReleased_Implementation()
{
	OnFireReleased();
}

// alt fire

void AMYPlayerPawn::ServerOnAltFirePressed_Implementation()
{
	OnAltFirePressed();
}

void AMYPlayerPawn::ServerOnAltFireReleased_Implementation()
{
	OnAltFireReleased();
}

// interact
void AMYPlayerPawn::ServerOnInteractPressed_Implementation()
{
	OnInteractPressed();
}

void AMYPlayerPawn::ServerOnInteractReleased_Implementation()
{
	OnInteractReleased();
}

// alt interact
void AMYPlayerPawn::ServerOnAltInteractPressed_Implementation()
{
	OnAltInteractPressed();
}

void AMYPlayerPawn::ServerOnAltInteractReleased_Implementation()
{
	OnAltInteractReleased();
}

// switch weapon
void AMYPlayerPawn::ServerOnSwitchWeapon_Implementation(uint8 index)
{
	// do a check for sensible
	r_ActiveWeaponIndex = index;
	OnRep_ActiveWeaponIndex();
}


////////////////////////////////////////////////////////
//// Validation

bool AMYPlayerPawn::ServerOnSprint_Validate()
{
	return true;
}

bool AMYPlayerPawn::ServerOnEndSprint_Validate()
{
	return true;
}

// crouch / slide
/*
bool AMYPlayerPawn::ServerOnCrouch_Slide_Validate()
{
	return true;
}

bool AMYPlayerPawn::ServerOnEndCrouch_Slide_Validate()
{
	return true;
}
*/

// jump / climb / hang
bool AMYPlayerPawn::ServerOnJump_Validate()
{
	return true;
}

bool AMYPlayerPawn::ServerOnEndJump_Validate()
{
	return true;
}

// cover
bool AMYPlayerPawn::ServerOnSwitchToCover_Validate()
{
	return true;
}

// reload
bool AMYPlayerPawn::ServerOnReload_Validate()
{
	return true;
}

// fire
bool AMYPlayerPawn::ServerOnFirePressed_Validate()
{
	return true;
}

bool AMYPlayerPawn::ServerOnFireReleased_Validate()
{
	return true;
}

// alt fire
bool AMYPlayerPawn::ServerOnAltFirePressed_Validate()
{
	return true;
}

bool AMYPlayerPawn::ServerOnAltFireReleased_Validate()
{
	return true;
}

// interact
bool AMYPlayerPawn::ServerOnInteractPressed_Validate()
{
	return true;
}

bool AMYPlayerPawn::ServerOnInteractReleased_Validate()
{
	return true;
}

//alt interact
bool AMYPlayerPawn::ServerOnAltInteractPressed_Validate()
{
	return true;
}

bool AMYPlayerPawn::ServerOnAltInteractReleased_Validate()
{
	return true;
}

// switch
bool AMYPlayerPawn::ServerOnSwitchWeapon_Validate(uint8 index)
{
	return true;
}

//// Client

void AMYPlayerPawn::ClientOnPossessed_Implementation()
{
	AMYPlayerController* controller = GetController<AMYPlayerController>();

	if (controller == NULL)
	{
		return;
	}

	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UUserWidget>(GetGameInstance(), HUDWidgetClass);
		UMYUserWidget* MYHUDWidget = Cast<UMYUserWidget>(HUDWidget);

		if (MYHUDWidget)
		{
			MYHUDWidget->AttachedController = controller;
			MYHUDWidget->AttachedPawn = this;
			MYHUDWidget->ReceivePostConstruction();
		}
	}
}

void AMYPlayerPawn::MulticastOnDeath_Implementation()
{
	ASyncedTimeManager* SyncedTimeManager = GetWorld()->GetGameState<AMYGameState>()->GetSyncedTimeManager();

	/*
	// stop CastingDelegateHandle timer
	if (CastingDelegateHandle.IsValid)
	{
		SyncedTimeManager->ClearTimer(CastingDelegateHandle);
	}

	// stop CastingLoopingDelegateHandle timer
	if (CastingLoopingDelegateHandle.IsValid)
	{
		SyncedTimeManager->ClearTimer(CastingLoopingDelegateHandle);
	}

	// stop LoopingDelegateHandle timer
	if (LoopingDelegateHandle.IsValid)
	{
		SyncedTimeManager->ClearTimer(LoopingDelegateHandle);
	}

	*/
	// play some death animation

	// clean up UI - if any
	if (HUDWidget)
	{
		HUDWidget->RemoveFromViewport();
		HUDWidget->Destruct();
	}
}

//////////////////////////////////////////////////////////////////////////
// Server Replication

void AMYPlayerPawn::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	if (GetLocalRole() == ROLE_Authority && GetController())
	{
		SetRemoteViewYaw(GetController()->GetControlRotation().Yaw);
	}
}

void AMYPlayerPawn::GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const {
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );

	DOREPLIFETIME(AMYPlayerPawn, r_Health);
	DOREPLIFETIME(AMYPlayerPawn, r_Team);
	DOREPLIFETIME(AMYPlayerPawn, r_bIsSliding);
	DOREPLIFETIME(AMYPlayerPawn, r_bIsTakingCover);
	DOREPLIFETIME(AMYPlayerPawn, r_AlternativeMovementMode);
	DOREPLIFETIME(AMYPlayerPawn, r_ActiveWeaponIndex);
	DOREPLIFETIME(AMYPlayerPawn, r_WeaponArray);

	DOREPLIFETIME_CONDITION(AMYPlayerPawn, r_RemoteViewYaw, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AMYPlayerPawn, r_bIsCrouchForced, COND_OwnerOnly);
}

//////////////////////////////////////////////////////////////////////////
// Overrides

// only runs on the server
void AMYPlayerPawn::PossessedBy(AController* NewController)
{
	AMYPlayerController* controller = Cast<AMYPlayerController>(NewController);

	if (controller != NULL)
	{
		r_Team = controller->Team;
		Tags.Add(controller->TeamName);

		r_ActiveWeaponIndex = 1;
	}

	Super::PossessedBy(NewController);

	// initialise UIs
	ClientOnPossessed();
	controller->ClientOnPawnPossessed(this);
}

void AMYPlayerPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// movement
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMYPlayerPawn::OnJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AMYPlayerPawn::OnEndJump);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMYPlayerPawn::DoCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AMYPlayerPawn::DoUnCrouch);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMYPlayerPawn::OnSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMYPlayerPawn::OnEndSprint);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMYPlayerPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMYPlayerPawn::MoveRight);

	// mouselook
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMYPlayerPawn::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMYPlayerPawn::LookUpAtRate);

	// actions
	PlayerInputComponent->BindAction("Reload", IE_Released, this, &AMYPlayerPawn::OnReload);
	PlayerInputComponent->BindAction("SwitchView", IE_Released, this, &AMYPlayerPawn::SwitchViewModes);
	PlayerInputComponent->BindAction("TakeCover", IE_Pressed, this, &AMYPlayerPawn::ServerOnSwitchToCover); // server function

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMYPlayerPawn::OnFirePressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AMYPlayerPawn::OnFireReleased);
	PlayerInputComponent->BindAction("AltFire", IE_Pressed, this, &AMYPlayerPawn::OnAltFirePressed);
	PlayerInputComponent->BindAction("AltFire", IE_Released, this, &AMYPlayerPawn::OnAltFireReleased);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMYPlayerPawn::OnInteractPressed);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &AMYPlayerPawn::OnInteractReleased);
	PlayerInputComponent->BindAction("AltInteract", IE_Pressed, this, &AMYPlayerPawn::OnAltInteractPressed);
	PlayerInputComponent->BindAction("AltInteract", IE_Released, this, &AMYPlayerPawn::OnAltInteractReleased);

	PlayerInputComponent->BindAction("Switch_0", IE_Released, this, &AMYPlayerPawn::OnSwitchWeapon<0>);
	PlayerInputComponent->BindAction("Switch_1", IE_Released, this, &AMYPlayerPawn::OnSwitchWeapon<1>);
	PlayerInputComponent->BindAction("Switch_2", IE_Released, this, &AMYPlayerPawn::OnSwitchWeapon<2>);
	PlayerInputComponent->BindAction("Switch_3", IE_Released, this, &AMYPlayerPawn::OnSwitchWeapon<3>);
	PlayerInputComponent->BindAction("Switch_4", IE_Released, this, &AMYPlayerPawn::OnSwitchWeapon<4>);
	PlayerInputComponent->BindAction("Switch_5", IE_Released, this, &AMYPlayerPawn::OnSwitchWeapon<5>);
	PlayerInputComponent->BindAction("Switch_6", IE_Released, this, &AMYPlayerPawn::OnSwitchWeapon<6>);
	PlayerInputComponent->BindAction("Switch_7", IE_Released, this, &AMYPlayerPawn::OnSwitchWeapon<7>);
}

float AMYPlayerPawn::TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	// if authority
	if (GetLocalRole() == ROLE_Authority) {

		// damage actor
		r_Health -= (int32)DamageAmount;

		// if dead
		if (r_Health <= 0) {

			// Run OnDeath everywhere
			//MulticastOnDeath();

			// hide/show meshes
			//GetMesh()->SetOwnerNoSee(false);
			//FirstPersonMesh->SetOwnerNoSee(true);

			// wait 2 seMY then destroy pawn
			ASyncedTimeManager* SyncedTimeManager = GetWorld()->GetGameState<AMYGameState>()->GetSyncedTimeManager();
			FSyncedTimer OnDeathDelegateHandle;

			SyncedTimeManager->SetTimer
			(
				OnDeathDelegateHandle,
				this,
				&AMYPlayerPawn::DoPostDeathCleanup,
				2.f,
				false,
				0.f
			);
		}
	}
	return 1.0f;
}

void AMYPlayerPawn::CalcCamera(float DeltaTime, struct FMinimalViewInfo & OutResult)
{
	if (bIsFirstPerson && FirstPersonCameraComponent && r_Health > 0.f) //&& !IsIdle
	{
		FirstPersonCameraComponent->GetCameraView(DeltaTime, OutResult);
		return;
	}
	else if (ThirdPersonCameraComponent)
	{
		ThirdPersonCameraComponent->GetCameraView(DeltaTime, OutResult);
		return;
	}

	GetActorEyesViewPoint(OutResult.Location, OutResult.Rotation);
}

