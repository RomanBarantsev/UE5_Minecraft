// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>
#include "CoreMinimal.h"
#include "Minecraft/Chunk.h"
#include "UObject/Object.h"
#include "GreedyMeshing.generated.h"

class UProceduralMeshComponent;

enum class EFace
{
	PosX,NegX,PosY,NegY,PosZ,NegZ
};

CONSTEXPR int BLOCK_SIZE = 1000;
/**
 * 
 */
UCLASS()
class MINECRAFT_API UGreedyMeshing : public UObject
{
private:
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	bool IsAir(int x, int y, int z,const std::vector<std::vector<std::vector<BlockType>>>& Blocks);
	void AddFace(const FVector& BlockPos,EFace Face);
public:
	void BuildChunkMesh(const std::vector<std::vector<std::vector<BlockType>>>& Blocks);
	void CreateMesh(UProceduralMeshComponent& procMesh);
	GENERATED_BODY()
};
