// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameState.h"
#include "MyGameInstance.h"
//#include "MyServerPortalActor.h"
#include "MyGameState.generated.h"

/**
*
*/
UCLASS()
class COMP_API AMyGameState : public AGameState
{
	GENERATED_UCLASS_BODY()

		//UMyGameInstance gameInstance;

		virtual void BeginPlay();

	//void LoadLevel();

	/* Handle to manage the timer */
	FTimerHandle ServerPortalsTimerHandle;

	//void SpawnServerPortals();

public:

	UPROPERTY(Replicated, BlueprintReadOnly)
		FMyServerLinks ServerLinks;

	UPROPERTY(Replicated, BlueprintReadOnly)
		FString serverTitle;

	UPROPERTY(Replicated, BlueprintReadOnly)
		FString MatchTitle;

	UPROPERTY(Replicated, BlueprintReadOnly)
		FMyTeamList TeamList;
	//TArray<AMyServerPortalActor*> ServerPortalActorReference;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "UETOPIA")
		float ClaimPercentageSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "UETOPIA")
		float CapturePercentageSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "UETOPIA")
		FString Team1Title;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "UETOPIA")
		FString Team2Title;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "UETOPIA")
		float Team1Percentage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "UETOPIA")
		float Team2Percentage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "UETOPIA")
		float Team1ClaimPercentage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "UETOPIA")
		float Team2ClaimPercentage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "UETOPIA")
		bool bTeam1HasControl;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "UETOPIA")
		bool bTeam2HasControl;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "UETOPIA")
		bool bTeam1Claiming;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "UETOPIA")
		bool bTeam2Claiming;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "UETOPIA")
		bool bIsContested;
};
