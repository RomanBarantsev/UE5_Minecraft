#pragma once
#include <vector>
#include "CoreMinimal.h"

struct FChunkCoord
{
	int x;
	int y;
	bool startPos=true;
	bool operator==(const FChunkCoord& rhs) const
	{
		return x == rhs.x && y == rhs.y;
	}
	bool operator<(const FChunkCoord& rhs) const
	{
		if (x != rhs.x) return x < rhs.x;
		return y < rhs.y;
	}
};

FORCEINLINE uint32 GetTypeHash(const FChunkCoord& Key)
{
	// Простой способ: скомбинировать хеши полей
	uint32 Hash = GetTypeHash(Key.x);
	Hash = HashCombine(Hash, GetTypeHash(Key.y));
	return Hash;
}

UENUM()
enum BlockType : uint8
{
	Empty = 0,
	Air = 5,
	Grass = 9,
	Dirt = 3,
	Stone = 2,
	Snow = 1,
	Leaves = 8,
	Sand = 7,
	Gravel = 6,
	Cobblestone = 4,
	Bricks= 12,
	Glass = 11,
	Water = 10,
	Lava = 13,
	Bedrock = 14,
	IronBlock = 15,
	GoldBlock = 16,
	Count = 16
};

constexpr int BLOCK_SIZE = 256;

constexpr int BEDROCK_BASE = 0;
constexpr int BEDROCK_HEIGHT = 5;

constexpr int CHUNK_X = 64;
constexpr int CHUNK_Y = 64;
constexpr int CHUNK_Z = 256;
constexpr uint8_t MIN_HEIGHT = 20;
constexpr uint8_t MAX_HEIGHT = 96;
constexpr int Z_STRIDE = 1;
constexpr int Y_STRIDE = CHUNK_Z;
constexpr int X_STRIDE = CHUNK_Z * CHUNK_Y;

constexpr int TOTAL_BLOCKS = CHUNK_X * CHUNK_Y * CHUNK_Z;
constexpr float CHUNKSIZE_WIDE = BLOCK_SIZE*CHUNK_X;

struct FChunkBuildData
{
private:
	std::vector<uint8> Blocks;
	std::vector<uint8> SurfaceHeights;
	
public:
	FChunkCoord Coord;
	FChunkBuildData();

	FORCEINLINE int Index(int x, int y, int z) const
	{
		return z + y * Y_STRIDE + x * X_STRIDE;
	}
	FORCEINLINE int SurfaceIndex(int x, int y) const
	{
		return x + y * CHUNK_X;
	}
	FORCEINLINE BlockType GetBlock(int x, int y, int z) const
	{
		return (BlockType)Blocks[Index(x,y,z)];
	}
	FORCEINLINE void SetBlock(int x, int y, int z, BlockType type)
	{
		Blocks[Index(x,y,z)] = (uint8)type;
	}
	void SetSurfaceHeight(int x, int y, int height);
	int GetSurfaceHeight(int x, int y) const;
	void Fill();
};
