// Fill out your copyright notice in the Description page of Project Settings.


#include "GreedyMeshing.h"
#include "ProceduralMeshComponent.h"
#include "Minecraft/FChunkBuildData.h"
#include "Minecraft/MinecraftProceduralMeshComponent.h"

namespace
{
	struct FFaceConfig
	{
		int NormalAxis;
		int UAxis;
		int VAxis;
		int NormalSign;
		FVector Normal;
	};

	int GetAxisSize(int Axis)
	{
		switch (Axis)
		{
		case 0: return CHUNK_X_SIZE;
		case 1: return CHUNK_Y_SIZE;
		default: return CHUNK_Z_SIZE;
		}
	}

	FFaceConfig GetFaceConfig(EFace Face)
	{
		switch (Face)
		{
		case EFace::PosX:
			return { 0, 1, 2, 1, FVector(1, 0, 0) };
		case EFace::NegX:
			return { 0, 1, 2, -1, FVector(-1, 0, 0) };
		case EFace::PosY:
			return { 1, 0, 2, 1, FVector(0, 1, 0) };
		case EFace::NegY:
			return { 1, 0, 2, -1, FVector(0, -1, 0) };
		case EFace::PosZ:
			return { 2, 0, 1, 1, FVector::UpVector };
		case EFace::NegZ:
			return { 2, 0, 1, -1, FVector::DownVector };
		default:
			return { 2, 0, 1, 1, FVector::UpVector };
		}
	}

	bool UsesVFirstVertexOrder(EFace Face)
	{
		return Face == EFace::PosX || Face == EFace::NegY || Face == EFace::PosZ;
	}

	void SetVectorAxis(FVector& Vector, int Axis, float Value)
	{
		switch (Axis)
		{
		case 0:
			Vector.X = Value;
			break;
		case 1:
			Vector.Y = Value;
			break;
		default:
			Vector.Z = Value;
			break;
		}
	}
}

FGreedyMeshing::FGreedyMeshing()
{
}

bool FGreedyMeshing::IsFaceVisible(	int x, int y, int z,int dx, int dy, int dz)
{
	BlockType a = Chunk->GetBlock(x,y,z);
	BlockType b = IsAir(x + dx, y + dy, z + dz)
				  ? BlockType::Air
				  : Chunk->GetBlock(x + dx,y + dy,z + dz);

	return a != BlockType::Air && b == BlockType::Air;
}

bool FGreedyMeshing::IsAir(int x, int y, int z)
{
	if (x < 0 || y < 0 || z < 0 || x >= CHUNK_X_SIZE || y >= CHUNK_Y_SIZE || z >= CHUNK_Z_SIZE)
		return true; 
	return  Chunk->GetBlock(x,y,z) == BlockType::Air;
}

void FGreedyMeshing::BuildGreedyMesh(const FChunkBuildData* Data)
{	
	Chunk = Data;
	GreedyFace(EFace::PosZ);
	GreedyFace(EFace::NegZ);
	GreedyFace(EFace::PosX);
	GreedyFace(EFace::NegX);
	GreedyFace(EFace::PosY);
	GreedyFace(EFace::NegY);
}

float FGreedyMeshing::GetTileIndex(BlockType Type)
{
	return (float)Type;
}

void FGreedyMeshing::GreedyFace(EFace Face)
{
	const FFaceConfig Config = GetFaceConfig(Face);
	const int SliceSize = GetAxisSize(Config.NormalAxis);
	const int USize = GetAxisSize(Config.UAxis);
	const int VSize = GetAxisSize(Config.VAxis);

	std::vector<FMaskCell> Mask(USize * VSize);
	auto MaskAt = [&](int U, int V) -> FMaskCell&
	{
		return Mask[U + V * USize];
	};

	for (int Slice = 0; Slice < SliceSize; Slice++)
	{
		for (FMaskCell& Cell : Mask)
		{
			Cell.bValid = false;
		}

		for (int U = 0; U < USize; U++)
		{
			for (int V = 0; V < VSize; V++)
			{
				int Coords[3] = {};
				int Delta[3] = {};
				Coords[Config.NormalAxis] = Slice;
				Coords[Config.UAxis] = U;
				Coords[Config.VAxis] = V;
				Delta[Config.NormalAxis] = Config.NormalSign;

				const int NeighborSlice = Slice + Config.NormalSign;
				if (Config.NormalAxis == 2 && (NeighborSlice < 0 || NeighborSlice >= SliceSize))
				{
					continue;
				}

				if (IsFaceVisible(Coords[0], Coords[1], Coords[2], Delta[0], Delta[1], Delta[2]))
				{
					MaskAt(U, V).bValid = true;
					MaskAt(U, V).Type = Chunk->GetBlock(Coords[0], Coords[1], Coords[2]);
				}
			}
		}

		for (int U = 0; U < USize; U++)
		{
			for (int V = 0; V < VSize; V++)
			{
				if (!MaskAt(U, V).bValid)
				{
					continue;
				}

				BlockType CurrentType = MaskAt(U, V).Type;

				int Width = 1;
				while (U + Width < USize &&
					MaskAt(U + Width, V).bValid &&
					MaskAt(U + Width, V).Type == CurrentType)
				{
					Width++;
				}

				int Height = 1;
				bool bDone = false;
				while (V + Height < VSize && !bDone)
				{
					for (int i = 0; i < Width; i++)
					{
						if (!MaskAt(U + i, V + Height).bValid ||
							MaskAt(U + i, V + Height).Type != CurrentType)
						{
							bDone = true;
							break;
						}
					}
					if (!bDone)
					{
						Height++;
					}
				}

				int Coords[3] = {};
				Coords[Config.NormalAxis] = Slice;
				Coords[Config.UAxis] = U;
				Coords[Config.VAxis] = V;
				AddQuad(Face, Coords[0], Coords[1], Coords[2], Width, Height, CurrentType);

				for (int ClearU = 0; ClearU < Width; ClearU++)
				{
					for (int ClearV = 0; ClearV < Height; ClearV++)
					{
						MaskAt(U + ClearU, V + ClearV).bValid = false;
					}
				}
			}
		}
	}
}

void FGreedyMeshing::AddQuad(EFace Face, int x, int y, int z, int w, int h, BlockType Type)
{
	const FFaceConfig Config = GetFaceConfig(Face);
	const int CoordValues[3] = { x, y, z };
	float BaseCoords[3] = {
		static_cast<float>(x * BLOCK_SIZE),
		static_cast<float>(y * BLOCK_SIZE),
		static_cast<float>(z * BLOCK_SIZE)
	};
	BaseCoords[Config.NormalAxis] = static_cast<float>(
		(CoordValues[Config.NormalAxis] + (Config.NormalSign > 0 ? 1 : 0)) * BLOCK_SIZE);

	FVector Base(BaseCoords[0], BaseCoords[1], BaseCoords[2]);
	FVector U = FVector::ZeroVector;
	FVector V = FVector::ZeroVector;
	SetVectorAxis(U, Config.UAxis, static_cast<float>(w * BLOCK_SIZE));
	SetVectorAxis(V, Config.VAxis, static_cast<float>(h * BLOCK_SIZE));

	const int Start = Vertices.Num();
	if (UsesVFirstVertexOrder(Face))
	{
		Vertices.Add(Base);
		Vertices.Add(Base + V);
		Vertices.Add(Base + U + V);
		Vertices.Add(Base + U);
	}
	else
	{
		Vertices.Add(Base);
		Vertices.Add(Base + U);
		Vertices.Add(Base + U + V);
		Vertices.Add(Base + V);
	}

	Triangles.Append({ Start, Start + 1, Start + 2, Start, Start + 2, Start + 3 });
	for (int i = 0; i < 4; i++)
	{
		Normals.Add(Config.Normal);
	}

	const float AtlasSize = 8.0f;
	const float TileSize = 1.0f / AtlasSize;
	float TileIndex = GetTileIndex(Type);
	int TileX = FMath::FloorToInt(TileIndex) % static_cast<int>(AtlasSize);
	int TileY = FMath::FloorToInt(TileIndex) / static_cast<int>(AtlasSize);
	float BaseU = TileX * TileSize;
	float BaseV = TileY * TileSize;

	switch (Face)
	{
	case EFace::PosY:
		UVs.Add(FVector2D(0.0f, 0.0f));
		UVs.Add(FVector2D(static_cast<float>(w), 0.0f));
		UVs.Add(FVector2D(static_cast<float>(w), static_cast<float>(h)));
		UVs.Add(FVector2D(0.0f, static_cast<float>(h)));
		break;
	case EFace::NegY:
		UVs.Add(FVector2D(0.0f, 0.0f));
		UVs.Add(FVector2D(0.0f, static_cast<float>(h)));
		UVs.Add(FVector2D(static_cast<float>(w), static_cast<float>(h)));
		UVs.Add(FVector2D(static_cast<float>(w), 0.0f));
		break;
	case EFace::NegX:
	case EFace::NegZ:
		UVs.Add(FVector2D(static_cast<float>(w), 0.0f));
		UVs.Add(FVector2D(0.0f, 0.0f));
		UVs.Add(FVector2D(0.0f, static_cast<float>(h)));
		UVs.Add(FVector2D(static_cast<float>(w), static_cast<float>(h)));
		break;
	default:
		UVs.Add(FVector2D(0.0f, 0.0f));
		UVs.Add(FVector2D(0.0f, static_cast<float>(h)));
		UVs.Add(FVector2D(static_cast<float>(w), static_cast<float>(h)));
		UVs.Add(FVector2D(static_cast<float>(w), 0.0f));
		break;
	}

	for (int i = 0; i < 4; i++)
	{
		UV1s.Add(FVector2D(BaseU, BaseV));
	}
}

FVector FGreedyMeshing::CreateMesh(UMinecraftProceduralMeshComponent& procMesh,UMaterialInterface* Mat)
{
	procMesh.CreateMeshSection(
	0,
	Vertices,
	Triangles,
	Normals,
	UVs,     
	UV1s,    
	TArray<FVector2D>(), 
	TArray<FVector2D>(),
	TArray<FColor>(),
	TArray<FProcMeshTangent>(),
	true
);
	procMesh.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	procMesh.SetCollisionObjectType(ECC_WorldDynamic);
	procMesh.SetCollisionResponseToAllChannels(ECR_Block);
	
	procMesh.SetMaterial(0, Mat);
	Vertices.Reset();
	Triangles.Reset();
	Normals.Reset();
	UVs.Reset();
	UV1s.Reset();
	return procMesh.GetComponentLocation();
}
