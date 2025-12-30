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
	bool bVisible;
	BlockType Type;
	bool bBackFace;
};

constexpr int MAX_DIM = FMath::Max(CHUNK_SIZE, CHUNK_Z);

constexpr int ATLAS_SIZE = 4;          // 4x4
constexpr float TILE = 1.0f / 4.0f;    // 0.25
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
	bool IsAir(int x, int y, int z,const std::vector<std::vector<std::vector<BlockType>>>& Blocks);
	void AddFace(const FVector& BlockPos, EFace Face, BlockType Type);
	FVector4 GetBlockUV(BlockType Type);
	FIntPoint AtlasFromIndex(int Index);
public:
	void BuildChunkMesh(const std::vector<std::vector<std::vector<BlockType>>>& Blocks);
	FVector CreateMesh(UMinecraftProceduralMeshComponent& procMesh,UMaterialInterface* Mat, const int64& Section);
};
