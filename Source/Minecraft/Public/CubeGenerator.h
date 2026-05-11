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
	const int chunkDelimiter=4;
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
		// 1. Нормализуем входящие значения из [-1, 1] в [0, 1]
		// (Value + 1.0) * 0.5 даст нам диапазон от 0.0 до 1.0
		float NormalizedT = (T + 1.0f) * 0.5f;
		float NormalizedH = (H + 1.0f) * 0.5f;

		// 2. Масштабируем до размера сетки (0-199)
		// Используем FMath::Clamp, чтобы избежать вылета за пределы массива при T или H = 1.0
		int32 IndexT = FMath::Clamp(FMath::FloorToInt(NormalizedT * 200.0f), 0, 199);
		int32 IndexH = FMath::Clamp(FMath::FloorToInt(NormalizedH * 200.0f), 0, 199);

		// 3. Вычисляем финальный индекс в одномерном массиве
		// Формула для 2D сетки: Row * RowSize + Column
		int32 FinalIndex = (IndexT * 200) + IndexH;

		// Проверка на валидность массива перед возвратом (защита от краша)
		if (BiomesLUTArray.IsValidIndex(FinalIndex))
		{
			return BiomesLUTArray[FinalIndex];
		}

		// Возвращаем что-то по умолчанию, если индекс невалиден
		return BiomesLUTArray[0]; 
	}
	
	void LoadAllBioms();
	void InitializeBiomeMap();
	FBiomLUTMap CalculateBiomWeights(int T, int H);
	void VisualizeBiomeLUT();
	void SaveLUTToXml();	
	
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
	float EPS = 0.1f;	
};
