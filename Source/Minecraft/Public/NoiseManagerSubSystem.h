// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

USTRUCT()
struct FOreAdditionalParameters 
{
	GENERATED_BODY()
	FNoisesParams* NoisesParams = nullptr;
	BlockType OreBlock = BlockType::CoalOre;
	int32 MinZ = 0;
	int32 MaxZ = CHUNK_Z_SIZE - 1;
	float Threshold = 0.5f;
};


USTRUCT()
struct FOresAddParamsList 
{
	GENERATED_BODY()
public:	 
	FOreAdditionalParameters CoalOre;
	FOreAdditionalParameters CopperOre;
	FOreAdditionalParameters IronOre;
	FOreAdditionalParameters GoldOre;
	FOreAdditionalParameters RedstoneOre;
	FOreAdditionalParameters LapisOre;
	FOreAdditionalParameters DiamondOre;
	FOreAdditionalParameters EmeraldOre;
};

struct FNoisesRunTime
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
struct FPerlinNoiseRow : public FTableRowBase
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
struct FOreGenerationRow : public FPerlinNoiseRow
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ore")
	TEnumAsByte<BlockType> OreBlock = BlockType::CoalOre;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ore")
	int32 MinZ = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ore")
	int32 MaxZ = CHUNK_Z_SIZE - 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ore")
	float Threshold = 0.5f;
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

struct FOreAdditionalNoises
{
	
};

// TODO need to rework
/*struct FNoiseRegistration
{
	FastNoiseLite* Noise = nullptr;
	FNoisesParams* Params = nullptr;
	FName RowName;
	FastNoiseLite::NoiseType NoiseType = FastNoiseLite::NoiseType_Perlin;
};

struct FNoiseOreRegistaion : public FNoiseRegistration
{	
	FOreGenerationParam* OreParams = nullptr;
};*/

UCLASS()
class MINECRAFT_API UNoiseManagerSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere)
	int Seed=1343;	//default value
	FFastNoises NS;
	FOresAddParamsList OresAddParamList;
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
	UDataTable* TerrainNoiseTablePath;
	UPROPERTY()
	UDataTable* OreGenerationTablePath;
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	void SetNoiseParams(FastNoiseLite& Noise, FNoisesParams params, FastNoiseLite::NoiseType noiseType);
	void InitializeNoise(FastNoiseLite& Noise, FNoisesParams& Params, FName RowName, FastNoiseLite::NoiseType NoiseType, float DefaultScale = 0.05f, float DefaultOctaves = 3.0f, float DefaultPersistence = 0.5f, float DefaultLacunarity = 2.0f);
	void InitializeOreNoiseAdditionParams(FOreAdditionalParameters& CoalOre);
	void LoadLayers();
	void LoadNoiseParamsFromTable(FastNoiseLite& noise, FNoisesParams& params);
public:
	TMap<FastNoiseLite*,FText>& GetNoisesMap();
	FFastNoises& GetFastNoises();
	FOresAddParamsList& GetOreAdditionalParameters();
};
