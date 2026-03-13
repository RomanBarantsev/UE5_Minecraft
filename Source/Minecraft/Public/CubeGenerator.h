// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Minecraft/FastNoiseLite.h"
#include "CubeGenerator.generated.h"

struct FChunkBuildData;
class FGreedyMeshing;
class UMinecraftProceduralMeshComponent;
class UDataTable;

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

USTRUCT(BlueprintType)
struct FChunkCoord
{
	GENERATED_BODY()
	int x;
	int y;
	bool startPos=true;
	bool operator==(const FChunkCoord& rhs) const
	{
		return x == rhs.x && y == rhs.y;
	}
	bool operator<(const FChunkCoord& rhs) const
	{
		if (x != rhs.x) return x < rhs.x;
		return y < rhs.y;
	}
};

FORCEINLINE uint32 GetTypeHash(const FChunkCoord& Key)
{
	// Простой способ: скомбинировать хеши полей
	uint32 Hash = GetTypeHash(Key.x);
	Hash = HashCombine(Hash, GetTypeHash(Key.y));
	return Hash;
}

UCLASS()
class MINECRAFT_API ACubeGenerator : public AActor
{
	GENERATED_BODY()
private:
	int START_CHUNKS = 2;
	const int chunkDelimiter=2;

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
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> DestroyedBlockClass;
private:
	void SetNoiseParams(FastNoiseLite& Noise, FNoisesParams params, FastNoiseLite::NoiseType noiseType);
	float GetHeightMask(int z, int minZ, int maxZ);
	int mapHeight(int x, int y);
	void LoadNoiseParams(FastNoiseLite& noise, FNoisesParams& params);
	void RemoveChunk(FChunkCoord coord);
	void StartAsyncGeneration(const FChunkCoord& coord);
	void GenerateChunkData(FChunkBuildData& Data);
	void GenerateCaves(FChunkBuildData& Data);
	void FinalizeChunk(FChunkBuildData& Data,FGreedyMeshing& GreedyMeshing);
	TMap<FChunkCoord,TSharedPtr<FChunkBuildData>> Chunks;
	TMap<FChunkCoord,TSharedPtr<FChunkBuildData>> ChunksForRemote;
	TArray<FChunkBuildData*> FreeChunks;
	TArray<FGreedyMeshing*> FreeGreedyMeshings;	
	UPROPERTY()
	TMap<FChunkCoord,UMinecraftProceduralMeshComponent*>MeshesMap;
	UPROPERTY()
	TArray<UMinecraftProceduralMeshComponent*> FreeProcMeshes;
	TMap<UMinecraftProceduralMeshComponent*,FChunkBuildData*> MeshToChunkMap;
	FChunkCoord currentChunkPosition;
	TQueue<FChunkCoord,EQueueMode::Spsc> ChunkGenerationQueue;
	bool bIsGeneratingChunk = false;
public:
	void ProcessQueue();
	UFUNCTION()
	void UpdateChunks(FVector coord);
public:
	int GetSurfaceHigh(FVector vec);
	void RemoveBlock(FHitResult hit,UMinecraftProceduralMeshComponent* mesh);
	float EPS = 0.1f;	
};
