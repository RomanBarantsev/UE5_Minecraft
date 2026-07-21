// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>

struct FChunkBuildData;
enum BlockType : uint8;
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
class FGreedyMeshing 
{
public:
	FGreedyMeshing();
private:
	static constexpr int32 AtlasTilesPerRow = 8;
	static constexpr int32 AtlasTileCount = AtlasTilesPerRow * AtlasTilesPerRow;
	static constexpr float TileSize = 1.0f / AtlasTilesPerRow;
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FVector2D> UV1s;
	bool IsFaceVisible(int x, int y, int z, int dx, int dy, int dz);
	bool IsAir(int x, int y, int z);
	float GetTileIndex(BlockType Type);
	void GreedyFace(EFace Face);
	void AddQuad(EFace Face, int x, int y, int z, int w, int h, BlockType Type);
	const FChunkBuildData* Chunk=nullptr;
public:
	void BuildGreedyMesh(const FChunkBuildData* Data);
public:
	FVector CreateMesh(UMinecraftProceduralMeshComponent& procMesh, UMaterialInterface* Mat);
	
};
