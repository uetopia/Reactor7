// Fill out your copyright notice in the Description page of Project Settings.

#include "MyControlPointActor.h"
#include "Engine.h"
#include "UEtopiaCompetitiveCharacter.h"
#include "MyPlayerState.h"
#include "MyGameState.h"
#include "MyGameInstance.h"



AMyControlPointActor::AMyControlPointActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereVisualAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	// StaticMesh'/Engine/BasicShapes/Cylinder.Cylinder'
	GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
	GetStaticMeshComponent()->SetStaticMesh(SphereVisualAsset.Object);
	GetStaticMeshComponent()->SetRelativeLocation(FVector(1.0f, 1.0f, -3.0f));

	FVector relativeScale = FVector(7.0f, 7.0f, 0.1f);
	GetStaticMeshComponent()->SetRelativeScale3D(relativeScale);

	RootComponent = GetStaticMeshComponent();

	Sphere1 = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere1"));
	Sphere1->InitSphereRadius(3500.0f);
	Sphere1->SetupAttachment(RootComponent);
	Sphere1->OnComponentBeginOverlap.AddDynamic(this, &AMyControlPointActor::OnOverlapBegin);       // set up a notification for when this component overlaps something
	Sphere1->OnComponentEndOverlap.AddDynamic(this, &AMyControlPointActor::OnOverlapEnd);


	if (!IsRunningDedicatedServer()) {
		UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AMyControlPointActor] construct - Not a Dedicated server."));
		// For some reason the dedicated server can't load the material, but it does not need it.

		static ConstructorHelpers::FObjectFinder<UMaterial> MatFinder(TEXT("Material'/Game/ControlPointMaterial.ControlPointMaterial'"));

		if (MatFinder.Succeeded())
		{
			UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AMyControlPointActor] [Construct]  "));
			Material = MatFinder.Object;
			GetStaticMeshComponent()->SetMaterial(0, Material);
		}


	}
	Team1Percentage = 0.0f;
	Team2Percentage = 0.0f;

	ClaimPercentageSpeed = 0.05f;
	CapturePercentageSpeed = 0.001f;
	bRoundWinProcessing = false;
}

// Called every frame
void AMyControlPointActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// this should only run on server

	if (IsRunningDedicatedServer()) {
		//UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AMyControlPointActor] Running on Dedicated server"));

		bool Team1OnPoint = false;
		bool Team2OnPoint = false;
		bool Team1OnPointAlone = false;
		bool Team2OnPointAlone = false;

		// inject values into game state
		AMyGameState* const MyGameState = GetWorld() != NULL ? GetWorld()->GetGameState<AMyGameState>() : NULL;

		TArray<AActor*> FoundActors;

		Sphere1->GetOverlappingActors(FoundActors, TSubclassOf<AUEtopiaCompetitiveCharacter>());

		for (int32 b = 0; b < FoundActors.Num(); b++)
		{
			AActor* const thisCharacter = FoundActors[b];

			AUEtopiaCompetitiveCharacter* const UEtopiaCharacter = Cast<AUEtopiaCompetitiveCharacter>(thisCharacter);
			if (UEtopiaCharacter) {
				AMyPlayerState* const UEtopiaPlayerState = Cast<AMyPlayerState>(UEtopiaCharacter->PlayerState);

				// these teamIDs are team Index.

				if (UEtopiaPlayerState->TeamId == 0) {
					//UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AMyControlPointActor] Team 0 is on point"));
					Team1OnPoint = true;
				}
				else if (UEtopiaPlayerState->TeamId == 1) {
					//UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AMyControlPointActor] Team 1 is on point"));
					Team2OnPoint = true;
				}
			}
			
		}

		if (Team1OnPoint & !Team2OnPoint) {
			//UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AMyControlPointActor] Team 0 is on point ALONE"));
			Team1OnPointAlone = true;
			// reset claiming
			bTeam2Claiming = false;
			Team2ClaimPercentage = 0.0f;
			if (Team1ClaimPercentage < 1.0f) {
				Team1ClaimPercentage = Team1ClaimPercentage + ClaimPercentageSpeed;
				MyGameState->Team1ClaimPercentage = Team1ClaimPercentage;
			}
			else if (Team1Percentage < 1.0f) {
				Team1Percentage = Team1Percentage + CapturePercentageSpeed;
				MyGameState->Team1Percentage = Team1Percentage;

				bTeam2HasControl = false;
				bTeam1HasControl = true;
				MyGameState->bTeam2HasControl = false;
				MyGameState->bTeam1HasControl = true;
			}
			else {
				UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AMyControlPointActor] Team 0 WINS"));
				// prevent duplicate results - check the bool
				if (!bRoundWinProcessing) {
					bRoundWinProcessing = true;
					UWorld* const World = GetWorld();
					if (World) {
						UMyGameInstance* gameInstance = Cast<UMyGameInstance>(World->GetGameInstance());
						gameInstance->RecordRoundWin(0);

						bTeam2HasControl = false;
						bTeam1HasControl = false;
						MyGameState->bTeam2HasControl = false;
						MyGameState->bTeam1HasControl = false;
					}
				}
			}
		}

		else if (Team2OnPoint & !Team1OnPoint) {
			//UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AMyControlPointActor] Team 1 is on point ALONE"));
			Team2OnPointAlone = true;
			// reset claiming
			bTeam1Claiming = false;
			Team1ClaimPercentage = 0.0f;
			if (Team2ClaimPercentage < 1.0f) {
				Team2ClaimPercentage = Team2ClaimPercentage + ClaimPercentageSpeed;
				MyGameState->Team2ClaimPercentage = Team2ClaimPercentage;
			}
			else if (Team2Percentage < 1.0f) {
				Team2Percentage = Team2Percentage + CapturePercentageSpeed;
				MyGameState->Team2Percentage = Team2Percentage;

				bTeam2HasControl = true;
				bTeam1HasControl = false;
				MyGameState->bTeam2HasControl = true;
				MyGameState->bTeam1HasControl = false;
			}
			else {
				UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AMyControlPointActor] Team 1 WINS"));
				// prevent duplicate results - check the bool
				if (!bRoundWinProcessing) {
					bRoundWinProcessing = true;
					UWorld* const World = GetWorld();
					if (World) {
						UMyGameInstance* gameInstance = Cast<UMyGameInstance>(World->GetGameInstance());
						gameInstance->RecordRoundWin(1);

						bTeam2HasControl = false;
						bTeam1HasControl = false;
						MyGameState->bTeam2HasControl = false;
						MyGameState->bTeam1HasControl = false;
					}
				}
				
			}
		}
	}

}

void AMyControlPointActor::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		if (OtherActor && (OtherActor != this) && OtherComp)
		{
			UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AMyRewardItemActor] OnOverlapBegin"));
			//ToggleLight();
			// TODO tell the server via gameinstance to attempt the pickup.
			ServerAttemptOverlapBegin(OtherActor);
		}
	}
}

void AMyControlPointActor::ServerAttemptOverlapBegin_Implementation(class AActor* OtherActor)
{
	UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AMyRewardItemActor] [ServerAttemptOverlapBegin_Implementation]  "));


}

bool AMyControlPointActor::ServerAttemptOverlapBegin_Validate(class AActor* OtherActor)
{
	//UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AUEtopiaPersistCharacter] [ServerAttemptPickUp_Validate]  "));
	return true;
}

void AMyControlPointActor::OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (HasAuthority())
	{
		if (OtherActor && (OtherActor != this) && OtherComp)
		{
			UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AMyRewardItemActor] OnOverlapEnd"));
			//ToggleLight();
			// TODO tell the server via gameinstance to attempt the pickup.
			ServerAttemptOverlapEnd(OtherActor);
		}
	}
}

void AMyControlPointActor::ServerAttemptOverlapEnd_Implementation(class AActor* OtherActor)
{
	UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AMyRewardItemActor] [ServerAttemptOverlapEnd_Implementation]  "));


}

bool AMyControlPointActor::ServerAttemptOverlapEnd_Validate(class AActor* OtherActor)
{
	//UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AUEtopiaPersistCharacter] [ServerAttemptPickUp_Validate]  "));
	return true;
}

