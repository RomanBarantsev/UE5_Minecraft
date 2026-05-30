// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NoiseManagerSubSystem.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Minecraft/FChunkBuildData.h"
#include "CubeGenerator.generated.h"

class UBiomDataAsset;
class FGreedyMeshing;
class UMinecraftProceduralMeshComponent;
class UDataTable;

struct FInterpolatedBiomeData {
	float VerticalScale;
	float HeightOffset;
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
	const int chunkDeep=8;
	const float delimiterChunkHeight=0.05;
	UPROPERTY()
	UNoiseManagerSubSystem* NoiseManager;
	
public:
	// Sets default values for this actor's properties
	ACubeGenerator();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
	UPROPERTY(EditAnywhere)
	UCurveFloat* ContinentalnessCurve;
	UPROPERTY(EditAnywhere)
	UCurveFloat* PeaksValleysCurve;
	UPROPERTY(EditAnywhere)
	UCurveFloat* ErosionCurve;
	FFastNoises FastNoises;
	UPROPERTY()
	TArray<UBiomDataAsset*> BiomesArray;	
	UPROPERTY()
	TArray<FBiomLUTMap> BiomesLUTArray;	
	const int BiomesArraySize = 40000;
	
FBiomLUTMap& GetLUTData(float T, float H) 
	{
		float NormalizedT = (T + 1.0f) * 0.5f;
		float NormalizedH = (H + 1.0f) * 0.5f;

		int32 IndexT = FMath::Clamp(FMath::FloorToInt(NormalizedT * 200.0f), 0, 199);
		int32 IndexH = FMath::Clamp(FMath::FloorToInt(NormalizedH * 200.0f), 0, 199);

		int32 FinalIndex = (IndexT * 200) + IndexH;

		if (BiomesLUTArray.IsValidIndex(FinalIndex))
		{
			return BiomesLUTArray[FinalIndex];
		}

		return BiomesLUTArray[0]; 
	}
	
	void LoadAllBioms();
	void InitializeBiomeMap();
	FBiomLUTMap CalculateBiomWeights(int T, int H);
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Material")
	UMaterialInterface* Mat;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> DestroyedBlockClass;
private:
	
	float GetHeightMask(int z, int minZ, int maxZ);
	FInterpolatedBiomeData GetInterpolatedLUTData(float T, float H);
	int CalculateHeight(FNoises noises);
	void RemoveChunk(FChunkCoord coord);
	void GenerateChunkData(FChunkBuildData& Data);
	void GenerateCaves(FChunkBuildData& Data);
	void GenerateSurfaceLayer(int z, FNoises& noises, FChunkBuildData& Data, int x, int y);
	void FinalizeChunk(FChunkBuildData& Data, FGreedyMeshing& GreedyMeshing);
	
	struct FAsyncGenerationResult {
		FChunkCoord Coord;
		TSharedPtr<FChunkBuildData> BuildData;
		TSharedPtr<FGreedyMeshing> GreedyMeshing;
	};
	void AsyncChunkCreate(TArray<FChunkCoord>& GenerateArray,TArray<FAsyncGenerationResult>& Results);	
	TMultiMap<int32,FChunkCoord> CoordsToGenerate;
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
	int GetSurfaceHighInPos(FVector vec);
	void RemoveBlock(FHitResult hit,UMinecraftProceduralMeshComponent* mesh);
	float EPS = 0.1f;	
};
