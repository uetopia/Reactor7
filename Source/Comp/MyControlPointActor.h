// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Core.h"
#include "Engine/StaticMeshActor.h"
#include "MyControlPointActor.generated.h"

/**
 * 
 */
UCLASS()
class COMP_API AMyControlPointActor : public AStaticMeshActor
{
	GENERATED_UCLASS_BODY()
	
public:

	/** sphere component */
	UPROPERTY(VisibleAnywhere, Category = "UEtopia")
		class USphereComponent* Sphere1;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	/** called when something enters the sphere component */
	UFUNCTION()
		void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnOverlapEnd(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// This is the function that gets called on the server
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerAttemptOverlapBegin(class AActor* OtherActor);

	// This is the function that gets called on the server
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerAttemptOverlapEnd(class AActor* OtherActor);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "UETOPIA")
		float ClaimPercentageSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "UETOPIA")
		float CapturePercentageSpeed;

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

	UPROPERTY(VisibleAnywhere,  Category = "UETOPIA")
		bool bRoundWinProcessing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "UETOPIA")
	bool bIsContested;
	
private:
	UPROPERTY(EditAnywhere)
		class UMaterial* Material;

	UPROPERTY(EditAnywhere)
		class UMaterialInstanceDynamic* MaterialInstance;

	/** Marks the properties we wish to replicate */
	//virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
