// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <map>

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
	float Scale=0;
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

struct ChunkCoord
{
	int x;
	int y;
	bool operator==(const ChunkCoord& rhs) const
	{
		return x == rhs.x && y == rhs.y;
	}
	bool operator<(const ChunkCoord& rhs) const
	{
		if (x != rhs.x) return x < rhs.x;
		return y < rhs.y;
	}
};

UCLASS()
class MINECRAFT_API ACubeGenerator : public AActor
{
	GENERATED_BODY()
private:
	int START_CHUNKS = 2;
public:
	// Sets default values for this actor's properties
	ACubeGenerator();
protected:
	void LoadLayers();
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	const int CubeSpacing = 0;
	
	FastNoiseLite CavesRoomNoise;
	FastNoiseLite CavesTunnelNoise;
	FastNoiseLite SurfaceNoise;
	FastNoiseLite ContNoise;
	FastNoiseLite PeakNoise;
	FastNoiseLite BedrockNoise;
	FNoisesParams CavesRoomParams;
	FNoisesParams CavesTunnelParams;
	FNoisesParams SurfaceParams;
	FNoisesParams ContParams;
	FNoisesParams PeakParams;
	FNoisesParams BedrockParams;
	
	UPROPERTY(EditAnywhere)
	UCurveFloat* ContinentalnessCurve;
	
	UPROPERTY(EditAnywhere)
	int Seed=1343;	
	UPROPERTY(EditAnywhere)
	UDataTable* PerlinNoiseTable;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Material")
	UMaterialInterface* Mat;
	UPROPERTY()
	TMap<UMinecraftProceduralMeshComponent*,UChunk*> MeshesMap;
	UPROPERTY()
	TArray<UGreedyMeshing*> GreedyMeshings;
	int64 Section=0;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> DestroyedBlockClass;
private:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void SetNoiseParams(FastNoiseLite& Noise, FNoisesParams params, FastNoiseLite::NoiseType noiseType);
	float GetHeightMask(int z, int minZ, int maxZ);
	int mapHeight(int x, int y);
	void CavesCreate(UChunk* chunk,int xChunk, int yChunk);
	void LoadNoiseParams(FastNoiseLite& noise, FNoisesParams& params);
	int  NormalizeNoise(float noise_value,int z,int z_min,int z_max,float threshold);
	void ChunksInit();
	std::map<ChunkCoord,UChunk*> Chunks;
public:
	int GetSurfaceHigh(FVector vec);
	void RemoveBlock(FVector hit,UMinecraftProceduralMeshComponent* mesh);
	void NewChunk(int xChunk, int yChunk);
	void Draw();
};
