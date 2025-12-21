// Fill out your copyright notice in the Description page of Project Settings.


#include "GreedyMeshing.h"

#include "ProceduralMeshComponent.h"


bool UGreedyMeshing::IsAir(int x, int y, int z, const std::vector<std::vector<std::vector<BlockType>>>& Blocks)
{
	if (x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || y >= CHUNK_SIZE || z >= CHUNK_Z)
		return true; // за границей = воздух
	return Blocks[x][y][z] == BlockType::Air;
}

void UGreedyMeshing::AddFace(const FVector& BlockPos, EFace Face)
{
	int StartIndex = Vertices.Num();
	FVector v0, v1, v2, v3;
	FVector normal;
	switch (Face)
	{
	case EFace::PosZ:
		v0 = BlockPos + FVector(0, 0, BLOCK_SIZE);
		v1 = BlockPos + FVector(0, BLOCK_SIZE, BLOCK_SIZE);
		v2 = BlockPos + FVector(BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);
		v3 = BlockPos + FVector(BLOCK_SIZE, 0, BLOCK_SIZE);
		normal = FVector::UpVector;
		break;
	case EFace::NegZ:
		v0 = BlockPos + FVector(0, 0, 0);
		v1 = BlockPos + FVector(BLOCK_SIZE, 0, 0);
		v2 = BlockPos + FVector(BLOCK_SIZE, BLOCK_SIZE, 0);
		v3 = BlockPos + FVector(0, BLOCK_SIZE, 0);
		normal = FVector::DownVector;
		break;
	case EFace::PosY:
		v0 = BlockPos + FVector(0, BLOCK_SIZE, 0);
		v1 = BlockPos + FVector(BLOCK_SIZE, BLOCK_SIZE, 0);
		v2 = BlockPos + FVector(BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);
		v3 = BlockPos + FVector(0, BLOCK_SIZE, BLOCK_SIZE);
		normal = FVector::RightVector;
		break;
	case EFace::NegY:
		v0 = BlockPos + FVector(BLOCK_SIZE, 0, 0);
		v1 = BlockPos + FVector(0, 0, 0);
		v2 = BlockPos + FVector(0, 0, BLOCK_SIZE);
		v3 = BlockPos + FVector(BLOCK_SIZE, 0, BLOCK_SIZE);
		normal = FVector::LeftVector;
		break;
	case EFace::PosX:
		v0 = BlockPos + FVector(BLOCK_SIZE, BLOCK_SIZE, 0);
		v1 = BlockPos + FVector(BLOCK_SIZE, 0, 0);
		v2 = BlockPos + FVector(BLOCK_SIZE, 0, BLOCK_SIZE);
		v3 = BlockPos + FVector(BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);
		normal = FVector::ForwardVector;
		break;
	case EFace::NegX:
		v0 = BlockPos + FVector(0, 0, 0);
		v1 = BlockPos + FVector(0, BLOCK_SIZE, 0);
		v2 = BlockPos + FVector(0, BLOCK_SIZE, BLOCK_SIZE);
		v3 = BlockPos + FVector(0, 0, BLOCK_SIZE);
		normal = FVector::BackwardVector;
		break;	
	default:
		return;
	}
	Vertices.Append({ v0, v1, v2, v3 });

	Triangles.Append({
		StartIndex + 0, StartIndex + 1, StartIndex + 2,
		StartIndex + 0, StartIndex + 2, StartIndex + 3
	});

	for (int i = 0; i < 4; i++)
		Normals.Add(normal);

	UVs.Append({
		FVector2D(0,0),
		FVector2D(1,0),
		FVector2D(1,1),
		FVector2D(0,1)
	});
}

void UGreedyMeshing::BuildChunkMesh(const std::vector<std::vector<std::vector<BlockType>>>& Blocks)
{
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int y = 0; y < CHUNK_SIZE; y++)
			for (int z = 0; z < CHUNK_Z; z++)
			{
				if (Blocks[x][y][z] == BlockType::Air)
					continue;

				FVector BlockPos(
					x * BLOCK_SIZE,
					y * BLOCK_SIZE,
					z * BLOCK_SIZE
				);

				if (IsAir(x + 1, y, z, Blocks)) AddFace(BlockPos, EFace::PosX);
				if (IsAir(x - 1, y, z, Blocks)) AddFace(BlockPos, EFace::NegX);
				if (IsAir(x, y + 1, z, Blocks)) AddFace(BlockPos, EFace::PosY);
				if (IsAir(x, y - 1, z, Blocks)) AddFace(BlockPos, EFace::NegY);
				if (IsAir(x, y, z + 1, Blocks)) AddFace(BlockPos, EFace::PosZ);
				if (IsAir(x, y, z - 1, Blocks)) AddFace(BlockPos, EFace::NegZ);
			}
}

void UGreedyMeshing::CreateMesh(UProceduralMeshComponent& procMesh)
{
	procMesh.CreateMeshSection(
	0,
	Vertices,
	Triangles,
	Normals,
	UVs,
	TArray<FColor>(),
	TArray<FProcMeshTangent>(),
	true
);
}
