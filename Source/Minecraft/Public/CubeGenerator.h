// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Minecraft/PerlinNoise3D.h"
#include "CubeGenerator.generated.h"

UCLASS()
class MINECRAFT_API ACubeGenerator : public AActor
{
	GENERATED_BODY()
	UPROPERTY()
	UHierarchicalInstancedStaticMeshComponent* HISM;
public:
	// Sets default values for this actor's properties
	ACubeGenerator();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
