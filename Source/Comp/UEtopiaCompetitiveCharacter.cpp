// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UEtopiaCompetitiveCharacter.h"
#include "Comp.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "UnrealNetwork.h"
#include "MyProjectile.h"
#include "MyPlayerController.h"
#include "MyPlayerState.h"
#include "MyGameInstance.h"


//////////////////////////////////////////////////////////////////////////
// AUEtopiaCompetitiveCharacter

AUEtopiaCompetitiveCharacter::AUEtopiaCompetitiveCharacter()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

												// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

												   // Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
												   // are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

												   // create a text render component so we can see player names
												   //Text = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Text Render"));
												   //Text->SetupAttachment(RootComponent);
												   //Text->SetRelativeLocation(FVector(0.f, 0, 90.f));
												   //Text->SetTextRenderColor(FColor::Red);
												   //Text->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
												   //Text->SetText(FText::FromString(TEXT("name")));

	Health = 1.0f;
	//HealthMax = 100.0f;
	ProjectileCount = 3;

	BaseFireRate = 1.0f;
	BaseFireCurrent = 1.0f;

	// Load our Sound Cue for the propeller sound we created in the editor... note your path may be different depending
	// on where you store the asset on disk.
	// SoundCue'/Game/Audio/Fire6.Fire6'
	static ConstructorHelpers::FObjectFinder<USoundCue> fireCue(TEXT("'/Game/Audio/Fire6.Fire6'"));

	// Store a reference to the Cue asset - we'll need it later.
	fireAudioCue = fireCue.Object;
	// Create an audio component, the audio component wraps the Cue, and allows us to ineract with
	// it, and its parameters from code.
	fireAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("FireAudioComp"));
	// I don't want the sound playing the moment it's created.
	fireAudioComponent->bAutoActivate = false; // don't play the sound immediately.
													// I want the sound to follow the pawn around, so I attach it to the Pawns root.
	fireAudioComponent->SetupAttachment(RootComponent);
		//AttachParent = RootComponent;
	// I want the sound to come from slighty in front of the pawn.
	fireAudioComponent->SetRelativeLocation(FVector(100.0f, 0.0f, 0.0f));
}

//////////////////////////////////////////////////////////////////////////
// Input

void AUEtopiaCompetitiveCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AUEtopiaCompetitiveCharacter::OnStartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AUEtopiaCompetitiveCharacter::OnStopFire);

	PlayerInputComponent->BindAxis("MoveForward", this, &AUEtopiaCompetitiveCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AUEtopiaCompetitiveCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AUEtopiaCompetitiveCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AUEtopiaCompetitiveCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AUEtopiaCompetitiveCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AUEtopiaCompetitiveCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AUEtopiaCompetitiveCharacter::OnResetVR);
}

void AUEtopiaCompetitiveCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (fireAudioCue->IsValidLowLevelFast()) {
		fireAudioComponent->SetSound(fireAudioCue);
	}
}

void AUEtopiaCompetitiveCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AUEtopiaCompetitiveCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	BaseFireCurrent = BaseFireCurrent + DeltaTime;
}

void AUEtopiaCompetitiveCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AUEtopiaCompetitiveCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	// jump, but only on the first touch
	if (FingerIndex == ETouchIndex::Touch1)
	{
		Jump();
	}
}

void AUEtopiaCompetitiveCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (FingerIndex == ETouchIndex::Touch1)
	{
		StopJumping();
	}
}

void AUEtopiaCompetitiveCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AUEtopiaCompetitiveCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AUEtopiaCompetitiveCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AUEtopiaCompetitiveCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}


void AUEtopiaCompetitiveCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME(AUEtopiaCompetitiveCharacter, Health);
	DOREPLIFETIME(AUEtopiaCompetitiveCharacter, ProjectileCount);
	DOREPLIFETIME(AUEtopiaCompetitiveCharacter, PlayerTitle);
}

void AUEtopiaCompetitiveCharacter::OnStartFire()
{
	UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AUEtopiaCompetitiveCharacter] [OnStartFire] "));
	if (CanFire()) {
		FVector ShootDir = GetActorForwardVector();
		FVector Origin = GetActorLocation();
		//  Tell the server to Spawn a bomb
		ServerAttemptSpawnProjectile();
		fireAudioComponent->Play();
	}

}

void AUEtopiaCompetitiveCharacter::OnStopFire()
{
	UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AUEtopiaCompetitiveCharacter] [OnStopFire] "));
	//StopWeaponFire();
}

float AUEtopiaCompetitiveCharacter::TakeDamageCustom(float Damage, struct FDamageEvent const& DamageEvent, class AActor* DamageCauser)
{
	UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AUEtopiaCompetitiveCharacter] [TakeDamageCustom] "));
	if (Role >= ROLE_Authority)
	{
		UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AUEtopiaCompetitiveCharacter] [TakeDamageCustom] SERVER "));

		Health -= Damage;
		if (Health <= 0)
		{
			Die(Damage, DamageEvent, DamageCauser);
			UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AUEtopiaCompetitiveCharacter] [TakeDamage] DIE "));
		}
		else
		{
			//PlayHit(ActualDamage, DamageEvent, EventInstigator ? EventInstigator->GetPawn() : NULL, DamageCauser);
			UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AUEtopiaCompetitiveCharacter] [TakeDamage] HIT "));

		}
	}

	return 0.f;
}

bool AUEtopiaCompetitiveCharacter::hasProjectiles()
{
	if (ProjectileCount > 0) {
		return true;
	}
	else {
		return false;
	}
}

void AUEtopiaCompetitiveCharacter::ServerAttemptSpawnProjectile_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AUEtopiaCompetitiveCharacter] [ServerAttemptSpawnProjectile_Implementation]  "));
	if (CanFire()) {
		UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AUEtopiaCompetitiveCharacter] [ServerAttemptSpawnProjectile_Implementation] CanFire "));
		float placementDistance = 150.0f;
		
		//FVector ShootDir = GetActorForwardVector();
		FVector ShootDir = FollowCamera->GetForwardVector();
		FVector Origin = GetActorLocation();
		FVector spawnlocationstart = ShootDir * placementDistance;
		FVector spawnlocation = spawnlocationstart + Origin;
		// Spawn the actor
		//Spawn a bomb
		FTransform const SpawnTransform(ShootDir.Rotation(), spawnlocation);
		AMyPlayerState* playerS = Cast<AMyPlayerState>(PlayerState);
		int32 playerID = playerS->PlayerId;
		AMyProjectile* const ProjectileActor = GetWorld()->SpawnActor<AMyProjectile>(AMyProjectile::StaticClass(), SpawnTransform);
		UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AUEtopiaCompetitiveCharacter] [ServerAttemptSpawnProjectile_Implementation] PlayerState->PlayerId %d "), playerS->PlayerId);
		UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AUEtopiaCompetitiveCharacter] [ServerAttemptSpawnProjectile_Implementation] PlayerState->TeamId %d "), playerS->TeamId);
		ProjectileActor->setPlayerID(playerID);
		ProjectileActor->setTeamID(playerS->TeamId);

		BaseFireCurrent = 0.0f;

		
	}


	// decrement bomb count
}

bool AUEtopiaCompetitiveCharacter::ServerAttemptSpawnProjectile_Validate()
{
	UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AUEtopiaCompetitiveCharacter] [ServerAttemptSpawnBomb_Validate]  "));
	return true;
}

bool AUEtopiaCompetitiveCharacter::Die(float KillingDamage, FDamageEvent const& DamageEvent, AActor* DamageCauser)
{
	if (Role >= ROLE_Authority)
	{
		UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AUEtopiaCompetitiveCharacter] [Die] SERVER "));

		// Record the kill and respawn.
		Health = 1.0f;

		AMyPlayerController* victim = Cast<AMyPlayerController>(GetController());
		AMyProjectile* projectile = Cast<AMyProjectile>(DamageCauser);


		UMyGameInstance* TheGameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());
		TheGameInstance->RecordKill(projectile->ownerPlayerID, victim->PlayerState->PlayerId);

		//Destroy();

		// this is not working.
		//GetWorld()->GetAuthGameMode()->RestartPlayer(this->GetController());

		// doing it manually.
		AMyPlayerState* playerS = Cast<AMyPlayerState>(PlayerState);

		if (playerS->TeamId == 1) {
			UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AUEtopiaCompetitiveCharacter] [Die] Team 1 "));
			FVector spawnlocation = FVector(-2990.0f, 2520.0f, 412.0f);
			SetActorLocation(spawnlocation);
			BaseFireCurrent = 1.0f;
		}
		else {
			UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AUEtopiaCompetitiveCharacter] [Die] Team 2 "));
			FVector spawnlocation = FVector(3099.0f, -2520.0f, 412.0f);
			SetActorLocation(spawnlocation);
			BaseFireCurrent = 1.0f;
		}

		
	}


	return true;
}

bool AUEtopiaCompetitiveCharacter::IsAlive() const
{
	return Health > 0;
}

bool AUEtopiaCompetitiveCharacter::CanFire() const
{
	if (IsAlive()) {
		if (BaseFireCurrent > BaseFireRate) {
			return true;
		}
		return false;
	}
	return false;
}
