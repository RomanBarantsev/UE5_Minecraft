// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MinecrafteDataBaseSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Voxel World Generation"))
class MINECRAFT_API UMinecraftDataBaseSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(Config, EditAnywhere, Category = "Noise")
	FSoftObjectPath PerlinNoiseTablePath;
	UPROPERTY(Config, EditAnywhere, Category = "Ore")
	FSoftObjectPath OreGenerationTablePath;
	UPROPERTY(Config, EditAnywhere, Category = "Material")
	FSoftObjectPath BlocksMaterial;
};
