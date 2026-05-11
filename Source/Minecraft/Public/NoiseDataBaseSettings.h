// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "NoiseDataBaseSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Voxel World Generation"))
class MINECRAFT_API UNoiseDataBaseSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(Config, EditAnywhere, Category = "Noise")
	FSoftObjectPath PerlinNoiseTablePath;
};
