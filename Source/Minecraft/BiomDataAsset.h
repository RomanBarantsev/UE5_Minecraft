// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FChunkBuildData.h"
#include "Engine/DataAsset.h"
#include "BiomDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class MINECRAFT_API UBiomDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Climate")
	float TargetTemperature;

	UPROPERTY(EditAnywhere, Category = "Climate")
	float TargetHumidity;
	
	UPROPERTY(EditAnywhere, Category = "Terrain")
	FText BiomName;
	
	UPROPERTY(EditAnywhere, Category = "Terrain")
	float Offset = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Terrain")
	float VerticalScale = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Terrain")
	TMap<int32,TEnumAsByte<BlockType>> SurfaceLayers;
};
