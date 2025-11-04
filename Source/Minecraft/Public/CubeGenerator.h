// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Minecraft/PerlinNoise3D.h"
#include "CubeGenerator.generated.h"

class UDataTable;

USTRUCT(BlueprintType)
struct FPerlinNoiseBiom : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Perlin Noise")
	float Scale;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Perlin Noise")
	float Octaves;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Perlin Noise")
	float Persistence;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Perlin Noise")
	float Lacunarity;
};

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
	UPROPERTY(EditAnywhere)
	int Seed=0;
	UPROPERTY(EditAnywhere)
	UDataTable* PerlinNoiseTable;	
	
	float Threshold = 0.0f;  // порог плотности
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void Generation2D();
	void Generation3D();	
};
