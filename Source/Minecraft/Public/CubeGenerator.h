// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Chaos/Vector.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Minecraft/Chunk.h"
#include "Minecraft/PerlinNoise3D.h"
#include "CubeGenerator.generated.h"

class UDataTable;

USTRUCT(BlueprintType)
struct FPerlinNoiseBiom : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Perlin Noise")
	float Scale;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Perlin Noise")
	float Octaves;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Perlin Noise")
	float Persistence;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Perlin Noise")
	float Lacunarity;
};

UCLASS()
class MINECRAFT_API ACubeGenerator : public AActor
{
	GENERATED_BODY()
	UPROPERTY()
	UHierarchicalInstancedStaticMeshComponent* HISM;
public:
	// Sets default values for this actor's properties
	ACubeGenerator();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	const float CubeSize = 256.0f;
	const int CubeSpacing = 0;
	UPROPERTY()
	UPerlinNoise2D* Surface;
	UPROPERTY()
	UPerlinNoise2D* Continentalness;
	UPROPERTY()
	UPerlinNoise2D* Errosion;		
	UPROPERTY(EditAnywhere)
	UCurveFloat* ContinentalnessCurve;
	float Scale;
	float Octaves;
	float Persistence;
	float Lacunarity;
	float ContScale;
	float ContOctaves;
	float ContPersistence;
	float ContLacunarity;
	
	UPROPERTY(EditAnywhere)
	int Seed=1343;	
	UPROPERTY(EditAnywhere)
	UDataTable* PerlinNoiseTable;
	UPROPERTY()
	UChunk* NewChunk;	
	float Threshold = 0.1f;  // порог плотности
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Material")
	UMaterialInterface* Mat;
private:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION()
	void GenerateSurface();
	UFUNCTION()
	void Generation3D();	
	int mapHeight(double n,int x,int y);
	void LoadNoiseTemplate();
	int  NormalizeNoise(float noise_value,int z,int z_min,int z_max,float threshold);
	void DrawCall() const;
};
