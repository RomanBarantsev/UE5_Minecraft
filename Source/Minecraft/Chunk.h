#pragma once
#include <vector>

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Chunk.generated.h"

enum BlockType
{
	Empty=-1,
	Stone=2,
	Dirt=2,
	Grass=9,
};

constexpr int CHUNK_SIZE = 512;
constexpr int CHUNK_Z = 512;
constexpr int MIN_HEIGHT = 20;
constexpr int MAX_HEIGHT = 96;
constexpr int WATER_LEVEL = 62;

UCLASS()
class MINECRAFT_API UChunk : public UObject
{
	GENERATED_BODY()
public:
	int _xMax;
	int _yMax;
	int _zMax;
	std::vector<std::vector<std::vector<BlockType>>> Terrain;
	std::vector<std::vector<int>> Surface;
	void InitChunk();
	void SetBlock(int x,int y,int z,BlockType type);
	void Fill();
	std::vector<std::vector<std::vector<BlockType>>>& GetTerrain();
};
