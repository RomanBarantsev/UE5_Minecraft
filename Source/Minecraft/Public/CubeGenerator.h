// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Minecraft/FastNoiseLite.h"
#include "Minecraft/FChunkBuildData.h"
#include "CubeGenerator.generated.h"

class UBiomDataAsset;
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

struct FNoises
{
	float CavesRoom;
	float CavesTunnel;
	float ContNoise;
	float PeaksValleys;
	float Bedrock;
	float Erosion;
};

USTRUCT()
struct FBiomLUTMap
{
	GENERATED_BODY()
	
	UPROPERTY()
	UBiomDataAsset* Biome;
	UPROPERTY()
	float VerticalScale;
	
	UPROPERTY()
	float HeightOffset;
	FBiomLUTMap(float W,float V,UBiomDataAsset* DataAsset) : Biome(DataAsset),VerticalScale(V),HeightOffset(W){};
	FBiomLUTMap() : Biome(nullptr), VerticalScale(0.0f), HeightOffset(0.0f){};
};

UCLASS()
class MINECRAFT_API ACubeGenerator : public AActor
{
	GENERATED_BODY()
private:
	const int chunkDelimiter=4;
	const float delimiterChunkHeight=0.05;

public:
	// Sets default values for this actor's properties
	ACubeGenerator();
protected:
	void LoadLayers();
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
	UPROPERTY()
	TArray<UBiomDataAsset*> BiomesArray;	
	UPROPERTY()
	TArray<FBiomLUTMap> BiomesLUTArray;	
	const int BiomesArraySize = 40000;
	
	FBiomLUTMap& GetLUTData(int T, int H) {
		return BiomesLUTArray[(T + 100) * 200 + (H + 100)];
	}
	
	void LoadAllBioms();
	void InitializeBiomeMap();
	FBiomLUTMap CalculateBiomWeights(int T, int H);
	
	FastNoiseLite CavesRoomNoise;
	FastNoiseLite CavesTunnelNoise;
	FastNoiseLite ContNoise;
	FastNoiseLite PeaksValleysNoise;
	FastNoiseLite BedrockNoise;
	FastNoiseLite ErosionNoise;
	TMap<FastNoiseLite*,FText> FastNoises;
	FNoisesParams CavesRoomParams;
	FNoisesParams CavesTunnelParams;
	FNoisesParams ContParams;
	FNoisesParams PeaksValleysParams;
	FNoisesParams BedrockParams;
	FNoisesParams ErosionParams;
		
	UPROPERTY(EditAnywhere)
	UCurveFloat* ContinentalnessCurve;
	UPROPERTY(EditAnywhere)
	UCurveFloat* PeaksValleysCurve;
	UPROPERTY(EditAnywhere)
	UCurveFloat* ErosionCurve;
	
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
	int mapHeight(FNoises noises);
	void LoadNoiseParams(FastNoiseLite& noise, FNoisesParams& params);
	void RemoveChunk(FChunkCoord coord);
	void GenerateChunkData(FChunkBuildData& Data);
	void GenerateCaves(FChunkBuildData& Data);
	int CalculateBlockHeight(float value);	
	void GenerateSurfaceLayer(int z, FNoises& noises, FChunkBuildData& Data, int x, int y);
	void FinalizeChunk(FChunkBuildData& Data, FGreedyMeshing& GreedyMeshing);
	
	struct FAsyncGenerationResult {
		FChunkCoord Coord;
		TSharedPtr<FChunkBuildData> BuildData;
		TSharedPtr<FGreedyMeshing> GreedyMeshing;
	};
	void AsyncChunkCreate(TArray<FChunkCoord>& GenerateArray,TArray<FAsyncGenerationResult>& Results);	
	TArray<FChunkCoord> CoordsToGenerate;
	size_t OperationPerTick=1;
	
	TMap<FChunkCoord,TSharedPtr<FChunkBuildData>> Chunks;
	TMap<FChunkCoord,TSharedPtr<FChunkBuildData>> ChunksForRemote;
	
	TArray<FChunkBuildData*> FreeChunks;
	TArray<FGreedyMeshing*> FreeGreedyMeshings;	
	UPROPERTY()
	TArray<UMinecraftProceduralMeshComponent*> FreeProcMeshes;
	
	TMap<FChunkCoord,UMinecraftProceduralMeshComponent*>MeshesMap;
	TMap<UMinecraftProceduralMeshComponent*,FChunkBuildData*> MeshToChunkMap;
	FChunkCoord currentChunkPosition;
	bool bIsGeneratingChunk = false;
public:
	UFUNCTION()
	void UpdateChunks(FVector coord);
public:
	int GetSurfaceHigh(FVector vec);
	void RemoveBlock(FHitResult hit,UMinecraftProceduralMeshComponent* mesh);
	TMap<FastNoiseLite*,FText> GetFastNoises();
	float EPS = 0.1f;	
private:
	void PrintNoises(int x, int y, int z);
};
