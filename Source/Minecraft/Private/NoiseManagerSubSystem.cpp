// Fill out your copyright notice in the Description page of Project Settings.


#include "NoiseManagerSubSystem.h"

#include "MinecrafteDataBaseSettings.h"


void UNoiseManagerSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	const UMinecraftDataBaseSettings* Settings = GetDefault<UMinecraftDataBaseSettings>();
	if (Settings)
	{
		TerrainNoiseTablePath = Cast<UDataTable>(Settings->PerlinNoiseTablePath.TryLoad());;
		if (!TerrainNoiseTablePath)
			UE_LOG(LogTemp, Warning, TEXT("PerlinNoiseTablePath not found, set it in the Project Settings"));
	}
	if (Settings)
	{
		OreGenerationTablePath = Cast<UDataTable>(Settings->OreGenerationTablePath.TryLoad());;
		if (!OreGenerationTablePath)
			UE_LOG(LogTemp, Warning, TEXT("OreGenerationTablePath not found, set it in the Project Settings"));
	}
	LoadLayers();	
	Super::Initialize(Collection);
}


void UNoiseManagerSubSystem::LoadLayers()
{
	FastNoises.Empty();
	
	InitializeNoise(NS.CavesRoomNoise, CavesRoomParams, TEXT("CavesRoom"), FastNoiseLite::NoiseType_Perlin);
	InitializeNoise(NS.CavesTunnelNoise, CavesTunnelParams, TEXT("CavesTunnel"), FastNoiseLite::NoiseType_Perlin);
	InitializeNoise(NS.ContinentalnessNoise, ContinentalnessParams, TEXT("Continentalness"), FastNoiseLite::NoiseType_Perlin);
	InitializeNoise(NS.PeaksValleysNoise, PeaksValleysParams, TEXT("PeaksValleys"), FastNoiseLite::NoiseType_Perlin);
	InitializeNoise(NS.BedrockNoise, BedrockParams, TEXT("Bedrock"), FastNoiseLite::NoiseType_Perlin);
	InitializeNoise(NS.ErosionNoise, ErosionParams, TEXT("Erosion"), FastNoiseLite::NoiseType_Perlin);
	InitializeNoise(NS.HumidityNoise, HumidityParams, TEXT("Humidity"), FastNoiseLite::NoiseType_Perlin);
	InitializeNoise(NS.TemperatureNoise, TemperatureParams, TEXT("Temperature"), FastNoiseLite::NoiseType_Perlin);
	
	InitializeNoise(NS.CoalOreNoise, CoalOreParams, TEXT("CoalOre"), FastNoiseLite::NoiseType_Perlin, 0.085f, 3.0f, 0.5f, 2.0f);
	OresAddParamList.CoalOre.NoisesParams = &CoalOreParams;
	InitializeNoise(NS.CopperOreNoise, CopperOreParams, TEXT("CopperOre"), FastNoiseLite::NoiseType_Perlin, 0.075f, 3.0f, 0.5f, 2.0f);
	OresAddParamList.CopperOre.NoisesParams = &CopperOreParams;
	InitializeNoise(NS.IronOreNoise, IronOreParams, TEXT("IronOre"), FastNoiseLite::NoiseType_Perlin, 0.07f, 3.0f, 0.5f, 2.0f);
	OresAddParamList.IronOre.NoisesParams = &IronOreParams;
	InitializeNoise(NS.GoldOreNoise, GoldOreParams, TEXT("GoldOre"), FastNoiseLite::NoiseType_Perlin, 0.065f, 3.0f, 0.5f, 2.0f);
	OresAddParamList.GoldOre.NoisesParams = &GoldOreParams;
	InitializeNoise(NS.RedstoneOreNoise, RedstoneOreParams, TEXT("RedstoneOre"), FastNoiseLite::NoiseType_Perlin, 0.06f, 3.0f, 0.5f, 2.0f);
	OresAddParamList.RedstoneOre.NoisesParams = &RedstoneOreParams;
	InitializeNoise(NS.LapisOreNoise, LapisOreParams, TEXT("LapisOre"), FastNoiseLite::NoiseType_Perlin, 0.055f, 3.0f, 0.5f, 2.0f);
	OresAddParamList.LapisOre.NoisesParams = &LapisOreParams;
	InitializeNoise(NS.DiamondOreNoise, DiamondOreParams, TEXT("DiamondOre"), FastNoiseLite::NoiseType_Perlin, 0.05f, 3.0f, 0.5f, 2.0f);
	OresAddParamList.DiamondOre.NoisesParams = &DiamondOreParams;
	InitializeNoise(NS.EmeraldOreNoise, EmeraldOreParams, TEXT("EmeraldOre"), FastNoiseLite::NoiseType_Perlin, 0.045f, 3.0f, 0.5f, 2.0f);
	OresAddParamList.EmeraldOre.NoisesParams = &EmeraldOreParams;
	InitializeOreNoiseAdditionParams(OresAddParamList.CoalOre);
	InitializeOreNoiseAdditionParams(OresAddParamList.DiamondOre);
	InitializeOreNoiseAdditionParams(OresAddParamList.CopperOre);
	InitializeOreNoiseAdditionParams(OresAddParamList.EmeraldOre);
	InitializeOreNoiseAdditionParams(OresAddParamList.GoldOre);
	InitializeOreNoiseAdditionParams(OresAddParamList.IronOre);
	InitializeOreNoiseAdditionParams(OresAddParamList.LapisOre);
	InitializeOreNoiseAdditionParams(OresAddParamList.RedstoneOre);
	
}

void UNoiseManagerSubSystem::SetNoiseParams(FastNoiseLite& Noise, FNoisesParams params,	FastNoiseLite::NoiseType noiseType)
{
	Noise.SetSeed(Seed);           // Seed 
	Noise.SetFrequency(params.Scale);   // Scale
	Noise.SetFractalOctaves(params.Octaves);    
	Noise.SetFractalGain(params.Persistence);   // Persistence
	Noise.SetFractalLacunarity(params.Lacunarity); // Lacunarity
	Noise.SetNoiseType(noiseType); //  Perlin
	Noise.SetFractalType(FastNoiseLite::FractalType_FBm);
}

void UNoiseManagerSubSystem::InitializeNoise(FastNoiseLite& Noise, FNoisesParams& Params, FName RowName, FastNoiseLite::NoiseType NoiseType, float DefaultScale, float DefaultOctaves, float DefaultPersistence, float DefaultLacunarity)
{
	Params.rowName = RowName;
	Params.Scale = DefaultScale;
	Params.Octaves = DefaultOctaves;
	Params.Persistence = DefaultPersistence;
	Params.Lacunarity = DefaultLacunarity;
	
	FastNoises.Add(&Noise, FText::FromName(Params.rowName));
	LoadNoiseParamsFromTable(Noise, Params);
	SetNoiseParams(Noise, Params, NoiseType);
}

void UNoiseManagerSubSystem::InitializeOreNoiseAdditionParams(FOreAdditionalParameters& Ore)
{
	if (OreGenerationTablePath)
	{
		
		auto RowOreNoise = OreGenerationTablePath->FindRow<FOreGenerationRow>(Ore.NoisesParams->rowName,"name");	
		Ore.MaxZ = RowOreNoise->MaxZ;
		Ore.MinZ = RowOreNoise->MinZ;
		Ore.OreBlock = RowOreNoise->OreBlock;
		Ore.Threshold = RowOreNoise->Threshold;
	}	
}

void UNoiseManagerSubSystem::LoadNoiseParamsFromTable(FastNoiseLite& noise, FNoisesParams& params)
{
	if (!OreGenerationTablePath || !TerrainNoiseTablePath)
	{
		return;
	}
	auto RowTerrainNoise = TerrainNoiseTablePath->FindRow<FPerlinNoiseRow>(params.rowName,"name");	
	if (RowTerrainNoise)
	{
		params.Scale = RowTerrainNoise->Scale;
		params.Octaves = RowTerrainNoise->Octaves;
		params.Persistence = RowTerrainNoise->Persistence;
		params.Lacunarity = RowTerrainNoise->Lacunarity;
	}
	auto RowOreNoise = OreGenerationTablePath->FindRow<FOreGenerationRow>(params.rowName,"name");	
	if (RowOreNoise)
	{
		params.Scale = RowOreNoise->Scale;
		params.Octaves = RowOreNoise->Octaves;
		params.Persistence = RowOreNoise->Persistence;
		params.Lacunarity = RowOreNoise->Lacunarity;
	}
}

TMap<FastNoiseLite*, FText>& UNoiseManagerSubSystem::GetNoisesMap()
{
	return FastNoises;
}

FFastNoises& UNoiseManagerSubSystem::GetFastNoises()
{
	return NS;
}

FOresAddParamsList& UNoiseManagerSubSystem::GetOreAdditionalParameters()
{
	return OresAddParamList;
}
