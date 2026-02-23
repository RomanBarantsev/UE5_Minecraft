// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>
#include "CoreMinimal.h"
#include "Minecraft/Chunk.h"
#include "UObject/Object.h"
#include "GreedyMeshing.generated.h"

class UMinecraftProceduralMeshComponent;
class UProceduralMeshComponent;

enum class EFace
{
	PosX,NegX,PosY,NegY,PosZ,NegZ
};

struct FMaskCell
{
	bool bValid = false;
	BlockType Type;
};

/**
 * 
 */
UCLASS()
class MINECRAFT_API UGreedyMeshing : public UObject
{
	GENERATED_BODY()
private:
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FVector2D> UV1s; // Второй UV канал
	bool IsFaceVisible(int x, int y, int z, int dx, int dy, int dz);
	bool IsAir(int x, int y, int z);
	float GetTileIndex(BlockType Type);
	void GreedyZPos(bool bPositive);
	void AddQuadZ(int x, int y, int z, int w, int h, bool bPositive, BlockType Type);
	void GreedyXPos(bool bPositive);
	void AddQuadX(int x, int y, int z, int w, int h, bool bPositive, BlockType Type);
	void Tailing(float baseU, float baseV, int w, int h, bool bPositive);
	void GreedyYPos(bool bPositive);
	UPROPERTY()
	const UChunk* Chunk;
public:
	void BuildGreedyMesh(const UChunk* ch);
	void AddQuadY(int x, int y, int z, int w, int h, bool bPositive, BlockType Type);
public:
	FVector CreateMesh(UMinecraftProceduralMeshComponent& procMesh, UMaterialInterface* Mat);
	
};
