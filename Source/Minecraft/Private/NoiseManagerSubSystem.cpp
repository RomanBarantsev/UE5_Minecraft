// Fill out your copyright notice in the Description page of Project Settings.


#include "NoiseManagerSubSystem.h"

#include "NoiseDataBaseSettings.h"


void UNoiseManagerSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	const UNoiseDataBaseSettings* Settings = GetDefault<UNoiseDataBaseSettings>();
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
	CavesRoomParams.rowName="CavesRoom";
	FastNoises.Add(&NS.CavesRoomNoise,FText::FromName(CavesRoomParams.rowName));
	CavesTunnelParams.rowName="CavesTunnel";
	FastNoises.Add(&NS.CavesTunnelNoise,FText::FromName(CavesTunnelParams.rowName));	
	
	ContinentalnessParams.rowName="Continentalness";
	FastNoises.Add(&NS.ContinentalnessNoise,FText::FromName(ContinentalnessParams.rowName));
	PeaksValleysParams.rowName="PeaksValleys";
	FastNoises.Add(&NS.PeaksValleysNoise,FText::FromName(PeaksValleysParams.rowName));
	BedrockParams.rowName="Bedrock";
	FastNoises.Add(&NS.BedrockNoise,FText::FromName(BedrockParams.rowName));
	ErosionParams.rowName="Erosion";
	FastNoises.Add(&NS.ErosionNoise,FText::FromName(ErosionParams.rowName));
	
	HumidityParams.rowName="Humidity";
	FastNoises.Add(&NS.HumidityNoise,FText::FromName(HumidityParams.rowName));
	TemperatureParams.rowName="Temperature";
	FastNoises.Add(&NS.TemperatureNoise,FText::FromName(TemperatureParams.rowName));
	
	LoadNoiseParams(NS.CavesRoomNoise,CavesRoomParams);
	LoadNoiseParams(NS.CavesTunnelNoise,CavesTunnelParams);
	
	LoadNoiseParams(NS.HumidityNoise,HumidityParams);
	LoadNoiseParams(NS.TemperatureNoise,TemperatureParams);
	
	LoadNoiseParams(NS.ContinentalnessNoise,ContinentalnessParams);
	LoadNoiseParams(NS.PeaksValleysNoise,PeaksValleysParams);
	LoadNoiseParams(NS.ErosionNoise,ErosionParams);
	
	LoadNoiseParams(NS.BedrockNoise,BedrockParams);
	
	SetNoiseParams(NS.CavesRoomNoise,CavesRoomParams, FastNoiseLite::NoiseType_Perlin);
	SetNoiseParams(NS.CavesTunnelNoise,CavesTunnelParams, FastNoiseLite::NoiseType_Perlin);
	
	SetNoiseParams(NS.HumidityNoise,HumidityParams, FastNoiseLite::NoiseType_Perlin);
	SetNoiseParams(NS.TemperatureNoise,TemperatureParams, FastNoiseLite::NoiseType_Perlin);	
	
	SetNoiseParams(NS.ContinentalnessNoise,ContinentalnessParams, FastNoiseLite::NoiseType_Perlin);
	SetNoiseParams(NS.PeaksValleysNoise,PeaksValleysParams, FastNoiseLite::NoiseType_Perlin);
	SetNoiseParams(NS.ErosionNoise,ErosionParams, FastNoiseLite::NoiseType_Perlin);
	
	SetNoiseParams(NS.BedrockNoise,BedrockParams, FastNoiseLite::NoiseType_Perlin);
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

void UNoiseManagerSubSystem::LoadNoiseParams(FastNoiseLite& noise, FNoisesParams& params)
{
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
