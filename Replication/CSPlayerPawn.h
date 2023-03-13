// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blueprint/UserWidget.h"

#include "Public/SyncedTimeManager.h"
#include "Public/MyWeaponBase.h"

#include "MyPlayerPawn.generated.h"

class UInputComponent;

UENUM(BlueprintType)
enum class EAlternativeMovementModes : uint8
{
	MOVE_Basic     UMETA(DisplayName = "Base"),
	MOVE_Climbing  UMETA(DisplayName = "Climbing"),
	MOVE_Vaulting  UMETA(DisplayName = "Vaulting"),
	MOVE_PullUp	   UMETA(DisplayName = "PullingUp"),
	MOVE_Waiting   UMETA(DisplayName = "WaitingForSurface")
};

UCLASS()
class MYSHOOTER_API AMyPlayerPawn: public ACharacter
{
	GENERATED_BODY()

public:
	// components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UCapsuleComponent* HitBox;

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* MuzzlePoint;

	// visual
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* FirstPersonMesh;

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* LHandLocation;

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh )
	class USceneComponent* RHandLocation;

	// movement
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	class USceneComponent* LedgeTracePoint;

	// movement
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	class USceneComponent* VaultTracePoint;

	// camera
	UPROPERTY(VisibleDefaultsOnly, Category = Camera)
	class USceneComponent* FirstPersonCameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* ThirdPersonSpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* ThirdPersonCameraComponent;

	//defaults
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	uint8 MaxHealth = 100;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera)
	float BaseZSmoothRate = 50.f;

	// UI
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = UI)
	TSubclassOf<class UUserWidget> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = UI)
	TSubclassOf<class AMYWeaponBase> DefaultWeaponClass;

protected:
	UPROPERTY()
	UUserWidget* HUDWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	float BaseWalkSpeed = 250.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	float BaseSprintSpeed = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	float SlideVelocityThreshold = 150.f;

	//advanced
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay, AdvancedDisplay)
	float ClimbSpeedGracePeriod = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay, AdvancedDisplay)
	float ClimbFalloffDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay, AdvancedDisplay)
	float ClimbSpeedCutoff = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay, AdvancedDisplay)
	float PullUpDuration = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay, AdvancedDisplay)
	float VaultDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay, AdvancedDisplay)
	float WaitDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay, AdvancedDisplay)
	bool DisableTimerOnWaitForClimbWhileFalling = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay, AdvancedDisplay)
	float BrakingFrictionFactorSlide = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay, AdvancedDisplay)
	float BrakingDecelerationSlide = 128.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay, AdvancedDisplay)
	float BrakingDecelerationWalking = 2048.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay, AdvancedDisplay)
	float InteractionDistance = 150.f;

// replicated variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_Health)
	uint8 r_Health = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	uint8 r_Team = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_ActiveWeaponIndex)
	uint8 r_ActiveWeaponIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_WeaponArray)
	TArray<AMYWeaponBase*> r_WeaponArray;

	UPROPERTY(Replicated)
	bool r_bIsSliding = false;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_bIsTakingCover)
	bool r_bIsTakingCover = false;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_bIsCrouchForced)
	bool r_bIsCrouchForced = false;

	UPROPERTY(Replicated)
	EAlternativeMovementModes r_AlternativeMovementMode = EAlternativeMovementModes::MOVE_Basic;

	UPROPERTY(replicated)
	uint8 r_RemoteViewYaw;

// non-replicated stateful variables
	UPROPERTY()
	float StartAlternativeMovementTimeStamp;

	UPROPERTY()
	float CameraZ = 0.f;

	UPROPERTY()
	float CameraZBase = 0.f;

	UPROPERTY()
	float AngleDiff = 0.0f;

	UPROPERTY()
	bool bIsFirstPerson = true;

	UPROPERTY()
	bool bIsPlayersChoiceFirstPerson = true;

	UPROPERTY()
	bool bIsPlayersChoiceToCrouch = false;

	UPROPERTY()
	bool bCoverStrafeOverridden = false;

public:
	AMYPlayerPawn();

	// overrides
	virtual void Tick( float DeltaTime ) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo & OutResult) override;
	virtual float TakeDamage(float DamageAmount,
		struct FDamageEvent const & DamageEvent,
		AController * EventInstigator,
		AActor * DamageCauser) override;

protected:
	virtual void SetupPlayerInputComponent( UInputComponent* InputComponent ) override;
	virtual void BeginPlay();

	// utils
	bool DoFaceTrace(FHitResult& Hit);
	bool DoSceneComponentTrace(FHitResult& Hit, USceneComponent* component, float TraceDistance);
	bool DoSceneComponentTrace(FHitResult& Hit, USceneComponent* component, FVector Offset, float TraceDistance);
	bool DoSceneComponentTraceFlattened(FHitResult& Hit, USceneComponent* component, FVector Offset, float TraceDistance);
	bool IsAbleToClimb();

	void Animate(FString Name);
	void GoThirdPerson();
	void GoFirstPerson();

	// input handling
	UFUNCTION()
	void MoveForward( float Value );

	UFUNCTION()
	void MoveRight( float Value);

	UFUNCTION()
	void TurnAtRate( float Rate );

	UFUNCTION()
	void LookUpAtRate( float Rate );

	UFUNCTION()
	void TurnToView();

	UFUNCTION()
	void HandleAdvancedMovement();

	//
	UFUNCTION()
	void OnSprint();

	UFUNCTION()
	void OnEndSprint();

	//UFUNCTION()
	//void OnCrouch_Slide();

	//UFUNCTION()
	//void OnEndCrouch_Slide();

	UFUNCTION()
	void OnJump();

	UFUNCTION()
	void OnEndJump();

	UFUNCTION()
	void SwitchViewModes();

	// weapons
	UFUNCTION()
	void OnFirePressed();

	UFUNCTION()
	void OnFireReleased();

	UFUNCTION()
	void OnAltFirePressed();

	UFUNCTION()
	void OnAltFireReleased();

	UFUNCTION()
	void OnReload();

	// interaction
	UFUNCTION()
	void OnInteractPressed();

	UFUNCTION()
	void OnInteractReleased();

	UFUNCTION()
	void OnAltInteractPressed();

	UFUNCTION()
	void OnAltInteractReleased();

	template<int val>
	void OnSwitchWeapon();

	// utils
	UFUNCTION()
	void DoDeath();

	UFUNCTION()
	void DoPostDeathCleanup();

	// get
	UFUNCTION()
	FRotator GetViewRotation() const override;

	UFUNCTION()
	void SetRemoteViewYaw(float NewRemoteViewPitch);

	// on rep

	UFUNCTION()
	void OnRep_ActiveWeaponIndex();

	UFUNCTION()
	void OnRep_Health();

	UFUNCTION()
	void OnRep_bIsTakingCover();

	UFUNCTION()
	void OnRep_bIsCrouchForced();

	UFUNCTION()
		void OnRep_WeaponArray();

	// overrides
	bool CanCrouch() const override;

	UFUNCTION()
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	UFUNCTION()
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	UFUNCTION()
	void DoCrouch()		{ bIsPlayersChoiceToCrouch = true; Crouch(); };

	UFUNCTION()
	void DoUnCrouch()	{ bIsPlayersChoiceToCrouch = false; UnCrouch(); };

public:
	// external animation states imposed on the player
	//UFUNCTION()
	//void ForceFlinch();

	UFUNCTION()
	void ForceRecoil();

//////////////////////////////////////////////////////////////////////////
// networked
public:
	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;

	// Client
	UFUNCTION( Client, Reliable )
	void ClientOnPossessed();
	void ClientOnPossessed_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnDeath();
	void MulticastOnDeath_Implementation();

	//UFUNCTION(NetMulticast, Reliable)
	//void MulticastOnDamaged(int32 _Health, int32 DamageAmount, TSubclassOf<UDamageType> DamageType);
	//void MulticastOnDamaged_Implementation(int32 _Health, int32 DamageAmount, TSubclassOf<UDamageType> DamageType);

	// movement functions
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerOnSprint();
	void ServerOnSprint_Implementation();
	bool ServerOnSprint_Validate();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerOnEndSprint();
	void ServerOnEndSprint_Implementation();
	bool ServerOnEndSprint_Validate();

	//UFUNCTION(Reliable, Server, WithValidation)
	//void ServerOnCrouch_Slide();
	//void ServerOnCrouch_Slide_Implementation();
	//bool ServerOnCrouch_Slide_Validate();

	//UFUNCTION(Reliable, Server, WithValidation)
	//void ServerOnEndCrouch_Slide();
	//void ServerOnEndCrouch_Slide_Implementation();
	//bool ServerOnEndCrouch_Slide_Validate();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerOnJump();
	void ServerOnJump_Implementation();
	bool ServerOnJump_Validate();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerOnEndJump();
	void ServerOnEndJump_Implementation();
	bool ServerOnEndJump_Validate();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerOnSwitchToCover();
	void ServerOnSwitchToCover_Implementation();
	bool ServerOnSwitchToCover_Validate();

	// server weapon functions
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerOnReload();
	void ServerOnReload_Implementation();
	bool ServerOnReload_Validate();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerOnFirePressed();
	void ServerOnFirePressed_Implementation();
	bool ServerOnFirePressed_Validate();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerOnFireReleased();
	void ServerOnFireReleased_Implementation();
	bool ServerOnFireReleased_Validate();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerOnAltFirePressed();
	void ServerOnAltFirePressed_Implementation();
	bool ServerOnAltFirePressed_Validate();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerOnAltFireReleased();
	void ServerOnAltFireReleased_Implementation();
	bool ServerOnAltFireReleased_Validate();

	// server interaction
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerOnInteractPressed();
	void ServerOnInteractPressed_Implementation();
	bool ServerOnInteractPressed_Validate();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerOnInteractReleased();
	void ServerOnInteractReleased_Implementation();
	bool ServerOnInteractReleased_Validate();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerOnAltInteractPressed();
	void ServerOnAltInteractPressed_Implementation();
	bool ServerOnAltInteractPressed_Validate();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerOnAltInteractReleased();
	void ServerOnAltInteractReleased_Implementation();
	bool ServerOnAltInteractReleased_Validate();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerOnSwitchWeapon(uint8 index);
	void ServerOnSwitchWeapon_Implementation(uint8 index);
	bool ServerOnSwitchWeapon_Validate(uint8 index);


private:
	//UFUNCTION()
	//void DoInteractionTrace();

};

