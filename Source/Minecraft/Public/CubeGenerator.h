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
	const int GridX = 100;
	const int GridY = 100;
	const int GridZ = 100;
	const float CubeSize = 256.0f;
	const int CubeSpacing = 0;
	
	float Scale = 0.02f;         // Чем меньше, тем плавнее горы
	int32 Octaves = 4;
	float Persistence = 0.5f;
	float Lacunarity = 2.0f;
	int32 Seed = 1337;
	
	int MaxHeight=100;	
	float Threshold = 0.0f;  // порог плотности
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
