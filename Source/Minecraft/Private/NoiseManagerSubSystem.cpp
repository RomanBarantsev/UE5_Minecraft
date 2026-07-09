// Fill out your copyright notice in the Description page of Project Settings.


#include "NoiseManagerSubSystem.h"

#include "MinecrafteDataBaseSettings.h"


void UNoiseManagerSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	const UMinecraftDataBaseSettings* Settings = GetDefault<UMinecraftDataBaseSettings>();
	if (Settings)
	{
		PerlinNoiseTable = Cast<UDataTable>(Settings->PerlinNoiseTablePath.TryLoad());;
		if (!PerlinNoiseTable)
			UE_LOG(LogTemp, Warning, TEXT("PerlinNoiseTablePath not found, set it in the Project Settings"));
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
	InitializeNoise(NS.CopperOreNoise, CopperOreParams, TEXT("CopperOre"), FastNoiseLite::NoiseType_Perlin, 0.075f, 3.0f, 0.5f, 2.0f);
	InitializeNoise(NS.IronOreNoise, IronOreParams, TEXT("IronOre"), FastNoiseLite::NoiseType_Perlin, 0.07f, 3.0f, 0.5f, 2.0f);
	InitializeNoise(NS.GoldOreNoise, GoldOreParams, TEXT("GoldOre"), FastNoiseLite::NoiseType_Perlin, 0.065f, 3.0f, 0.5f, 2.0f);
	InitializeNoise(NS.RedstoneOreNoise, RedstoneOreParams, TEXT("RedstoneOre"), FastNoiseLite::NoiseType_Perlin, 0.06f, 3.0f, 0.5f, 2.0f);
	InitializeNoise(NS.LapisOreNoise, LapisOreParams, TEXT("LapisOre"), FastNoiseLite::NoiseType_Perlin, 0.055f, 3.0f, 0.5f, 2.0f);
	InitializeNoise(NS.DiamondOreNoise, DiamondOreParams, TEXT("DiamondOre"), FastNoiseLite::NoiseType_Perlin, 0.05f, 3.0f, 0.5f, 2.0f);
	InitializeNoise(NS.EmeraldOreNoise, EmeraldOreParams, TEXT("EmeraldOre"), FastNoiseLite::NoiseType_Perlin, 0.045f, 3.0f, 0.5f, 2.0f);
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
	LoadNoiseParams(Noise, Params);
	SetNoiseParams(Noise, Params, NoiseType);
}

void UNoiseManagerSubSystem::LoadNoiseParams(FastNoiseLite& noise, FNoisesParams& params)
{
	if (!PerlinNoiseTable)
	{
		return;
	}
	
	auto Row = PerlinNoiseTable->FindRow<FPerlinNoiseBiom>(params.rowName,"name");	
	if (Row)
	{
		params.Scale = Row->Scale;
		params.Octaves = Row->Octaves;
		params.Persistence = Row->Persistence;
		params.Lacunarity = Row->Lacunarity;
	}
}

TMap<FastNoiseLite*, FText>& UNoiseManagerSubSystem::GetNoisesMap()
{
	return FastNoises;
}

FFastNoises& UNoiseManagerSubSystem::GetNoises()
{
	return NS;
}
