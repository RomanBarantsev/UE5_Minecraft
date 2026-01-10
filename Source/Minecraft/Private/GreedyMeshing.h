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
	bool IsFaceVisible(int x, int y, int z, int dx, int dy, int dz,
	                   const std::vector<std::vector<std::vector<BlockType>>>& Blocks);
	bool IsAir(int x, int y, int z,const std::vector<std::vector<std::vector<BlockType>>>& Blocks);
	float GetTileIndex(BlockType Type);
	void TestAtlasUVs();
	void GreedyZPos(const std::vector<std::vector<std::vector<BlockType>>>& Blocks, bool bPositive);
	void AddQuadZ(int x, int y, int z, int w, int h, bool bPositive, BlockType Type);
	void GreedyXPos(const std::vector<std::vector<std::vector<BlockType>>>& Blocks, bool bPositive);
	void AddQuadX(int x, int y, int z, int w, int h, bool bPositive, BlockType Type);
	void GreedyYPos(const std::vector<std::vector<std::vector<BlockType>>>& Blocks, bool bPositive);

public:
	void BuildChunkMesh(const std::vector<std::vector<std::vector<BlockType>>>& Blocks);
	void AddQuadY(int x, int y, int z, int w, int h, bool bPositive, BlockType Type);
public:
	FVector CreateMesh(UMinecraftProceduralMeshComponent& procMesh, UMaterialInterface* Mat, const int64& Section);
};
