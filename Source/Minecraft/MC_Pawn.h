// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DefaultPawn.h"
#include "MC_Pawn.generated.h"

class ACubeGenerator;
class UGreedyMeshing;

UCLASS()
class MINECRAFT_API AMC_Pawn : public ADefaultPawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMC_Pawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void PlaceAboveSurface();
	void Fire();
	UPROPERTY()
	ACubeGenerator* CubeGenerator;
	void Redraw();
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
