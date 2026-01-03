#pragma once
#include <vector>

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Chunk.generated.h"

enum BlockType
{
	Empty=-1,
	Air   = 0,
	Grass = 9,
	Dirt  = 2,
	Stone = 3
};

constexpr int CHUNK_SIZE = 64;
constexpr int CHUNK_Z = 256;
constexpr int MIN_HEIGHT = 20;
constexpr int MAX_HEIGHT = 96;
constexpr int WATER_LEVEL = 62;
CONSTEXPR int BLOCK_SIZE = 256.0f;

UCLASS()
class MINECRAFT_API UChunk : public UObject
{
	GENERATED_BODY()
public:
	std::vector<std::vector<std::vector<BlockType>>> Terrain;
	std::vector<std::vector<int>> Surface;
	void InitChunk();
	void SetBlock(int x,int y,int z,BlockType type);
	void Fill();
	std::vector<std::vector<std::vector<BlockType>>>& GetTerrain();
};
