// Fill out your copyright notice in the Description page of Project Settings.

#include "MyGameState.h"
#include "Comp.h"
#include "Net/UnrealNetwork.h"
#include "Engine.h"
//#include "MyServerPortalActor.h"



AMyGameState::AMyGameState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	UE_LOG(LogTemp, Log, TEXT("[UETOPIA] [AMyGameState] CONSTRUCT"));

	//GetWorld()->GetTimerManager().SetTimer(ServerPortalsTimerHandle, this, &AMyGameState::SpawnServerPortals, 20.0f, true);

	Team1Percentage = 0.0f;
	Team2Percentage = 0.0f;

	ClaimPercentageSpeed = 0.005f;
	CapturePercentageSpeed = 0.001f;

}

void AMyGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMyGameState, serverTitle);
	DOREPLIFETIME(AMyGameState, ServerLinks);
	DOREPLIFETIME(AMyGameState, TeamList);
	DOREPLIFETIME(AMyGameState, Team1Percentage);
	DOREPLIFETIME(AMyGameState, Team2Percentage);
	DOREPLIFETIME(AMyGameState, Team1Title);
	DOREPLIFETIME(AMyGameState, Team2Title);
	DOREPLIFETIME(AMyGameState, bTeam1HasControl);
	DOREPLIFETIME(AMyGameState, bTeam2HasControl);
	


}

void AMyGameState::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// populate the team names - skip if they don't exist yet.

	UMyGameInstance* TheGameInstance = Cast<UMyGameInstance>(GetWorld()->GetGameInstance());

	if (TheGameInstance->TeamList.teams.Num() > 1)
	{
		Team1Title = TheGameInstance->TeamList.teams[0].title;
		Team2Title = TheGameInstance->TeamList.teams[1].title;
	}
	


}

