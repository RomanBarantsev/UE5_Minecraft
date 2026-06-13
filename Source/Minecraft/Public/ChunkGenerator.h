// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NoiseManagerSubSystem.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Minecraft/FChunkBuildData.h"
#include "ChunkGenerator.generated.h"

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
class MINECRAFT_API AChunkGenerator : public AActor
{
	GENERATED_BODY()
private:
	const float delimiterChunkHeight=0.05;
	UPROPERTY()
	UNoiseManagerSubSystem* NoiseManager;
	
public:
	// Sets default values for this actor's properties
	AChunkGenerator();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
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
	
	
private:
	
	float GetHeightMask(int z, int minZ, int maxZ);
	FInterpolatedBiomeData GetInterpolatedLUTData(float T, float H);
	int CalculateHeight(FNoises noises);
	void GenerateCaves(FChunkBuildData& Data,int x,int y);
	void GenerateSurfaceLayer(int z, FNoises& noises, FChunkBuildData& Data, int x, int y);
	
	bool bIsGeneratingChunk = false;
public:
	void GenerateChunkData(FChunkBuildData& Data);
};
