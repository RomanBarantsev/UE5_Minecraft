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
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FVector2D> UV1s;
	bool IsFaceVisible(int x, int y, int z, int dx, int dy, int dz);
	bool IsAir(int x, int y, int z);
	float GetTileIndex(BlockType Type);
	void GreedyZPos(bool bPositive);
	void AddQuadZ(int x, int y, int z, int w, int h, bool bPositive, BlockType Type);
	void GreedyXPos(bool bPositive);
	void AddQuadX(int x, int y, int z, int w, int h, bool bPositive, BlockType Type);
	void Tailing(float baseU, float baseV, int w, int h, bool bPositive);
	void GreedyYPos(bool bPositive);
	const FChunkBuildData* Chunk=nullptr;
public:
	void Clear();
	void BuildGreedyMesh(const FChunkBuildData* Data);
	void BuildMesh(const FChunkBuildData& Data);
	void AddQuadY(int x, int y, int z, int w, int h, bool bPositive, BlockType Type);
public:
	FVector CreateMesh(UMinecraftProceduralMeshComponent& procMesh, UMaterialInterface* Mat);
	
};
