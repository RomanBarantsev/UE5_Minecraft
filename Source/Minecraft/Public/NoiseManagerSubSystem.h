// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GreedyMeshing.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Minecraft/FastNoiseLite.h"
#include "Minecraft/FChunkBuildData.h"
#include "NoiseManagerSubSystem.generated.h"

/**
 * 
 */

USTRUCT()
struct FNoisesParams
{
	GENERATED_BODY()
public:	
	float Scale = 0.05f;
	float Octaves = 3.0f;
	float Persistence = 0.5f;
	float Lacunarity = 2.0f;
	FName rowName;
};

struct FNoises
{
	float CavesRoom;
	float CavesTunnel;
	float Continentalness;
	float PeaksValleys;
	float Bedrock;
	float Erosion;
	float Humidity;
	float Temperature;
	float CoalOre;
	float CopperOre;
	float IronOre;
	float GoldOre;
	float RedstoneOre;
	float LapisOre;
	float DiamondOre;
	float EmeraldOre;
};

USTRUCT()
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
struct FOreGenerationRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ore")
	TEnumAsByte<BlockType> OreBlock = BlockType::CoalOre;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ore")
	int32 MinZ = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ore")
	int32 MaxZ = CHUNK_Z_SIZE - 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ore")
	float Scale = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ore")
	int32 Octaves = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ore")
	float Persistence = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ore")
	float Lacunarity = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ore")
	float Threshold = 0.72f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ore")
	int32 Seed = 1337;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ore")
	int32 Priority = 0;
};

struct FFastNoises
{
	FastNoiseLite CavesRoomNoise;
	FastNoiseLite CavesTunnelNoise;
	FastNoiseLite ContinentalnessNoise;
	FastNoiseLite PeaksValleysNoise;
	FastNoiseLite BedrockNoise;
	FastNoiseLite ErosionNoise;
	FastNoiseLite TemperatureNoise;
	FastNoiseLite HumidityNoise;
	
	FastNoiseLite CoalOreNoise;
	FastNoiseLite CopperOreNoise;
	FastNoiseLite IronOreNoise;
	FastNoiseLite GoldOreNoise;
	FastNoiseLite RedstoneOreNoise;
	FastNoiseLite LapisOreNoise;
	FastNoiseLite DiamondOreNoise;
	FastNoiseLite EmeraldOreNoise;
};

UCLASS()
class MINECRAFT_API UNoiseManagerSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere)
	int Seed=1343;	//default value
	FFastNoises NS;
	FNoisesParams CavesRoomParams;
	FNoisesParams CavesTunnelParams;
	FNoisesParams ContinentalnessParams;
	FNoisesParams PeaksValleysParams;
	FNoisesParams HumidityParams;
	FNoisesParams TemperatureParams;
	FNoisesParams BedrockParams;
	FNoisesParams ErosionParams;
	
	FNoisesParams CoalOreParams;
	FNoisesParams CopperOreParams;
	FNoisesParams IronOreParams;
	FNoisesParams GoldOreParams;
	FNoisesParams RedstoneOreParams;
	FNoisesParams LapisOreParams;
	FNoisesParams DiamondOreParams;
	FNoisesParams EmeraldOreParams;
	
	TMap<FastNoiseLite*,FText> FastNoises; //for UI	
	
	UPROPERTY()
	UDataTable* PerlinNoiseTable;
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;	
	void SetNoiseParams(FastNoiseLite& Noise, FNoisesParams params, FastNoiseLite::NoiseType noiseType);
	void InitializeNoise(FastNoiseLite& Noise, FNoisesParams& Params, FName RowName, FastNoiseLite::NoiseType NoiseType, float DefaultScale = 0.05f, float DefaultOctaves = 3.0f, float DefaultPersistence = 0.5f, float DefaultLacunarity = 2.0f);
	void LoadLayers();
	void LoadNoiseParams(FastNoiseLite& noise, FNoisesParams& params);
public:
	TMap<FastNoiseLite*,FText>& GetNoisesMap();
	FFastNoises& GetNoises();
};
