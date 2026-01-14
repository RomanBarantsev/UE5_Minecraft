// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Minecraft/Chunk.h"
#include "Minecraft/FastNoiseLite.h"
#include "CubeGenerator.generated.h"

class UMinecraftProceduralMeshComponent;
class UDataTable;
class UGreedyMeshing;

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

USTRUCT(BlueprintType)
struct FNoisesParams
{
	GENERATED_BODY()
public:	
	float Scale;
	float Octaves;
	float Persistence;
	float Lacunarity;
	FName rowName;
};

UCLASS()
class MINECRAFT_API ACubeGenerator : public AActor
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	ACubeGenerator();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	const int CubeSpacing = 0;
	
	FastNoiseLite CavesNoise;
	FastNoiseLite SurfaceNoise;
	FastNoiseLite ContNoise;
	FNoisesParams CavesParams;
	FNoisesParams SurfaceParams;
	FNoisesParams ContParams;
	
	UPROPERTY(EditAnywhere)
	UCurveFloat* ContinentalnessCurve;
	
	UPROPERTY(EditAnywhere)
	int Seed=1343;	
	UPROPERTY(EditAnywhere)
	UDataTable* PerlinNoiseTable;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Material")
	UMaterialInterface* Mat;
	UPROPERTY()
	TMap<int64,UChunk*> ChunksMap;
	UPROPERTY()
	TMap<int64,UGreedyMeshing*> GreedyMeshingMap;
	UPROPERTY()
	TMap<int64,UMinecraftProceduralMeshComponent*> MeshesMap;
	int64 Section=0;

private:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void SetNoiseParams(FastNoiseLite& Noise, FNoisesParams params, FastNoiseLite::NoiseType noiseType);
	int mapHeight(double n,int x,int y);
	void CavesCreate(UChunk* chunk,int xChunk, int yChunk);
	void LoadNoiseParams(FastNoiseLite& noise, FNoisesParams& params);
	int  NormalizeNoise(float noise_value,int z,int z_min,int z_max,float threshold);
	void ChunksInit();
public:
	void RemoveBlock(int64 Index, FVector hit);
	void NewChunk(int xChunk, int yChunk);
	
};
