#include "Chunk.h"

void UChunk::InitChunk()
{
	std::vector<BlockType> z(CHUNK_Z, BlockType::Empty);
	std::vector<std::vector<BlockType>> y(CHUNK_SIZE,z);
	std::vector<std::vector<std::vector<BlockType>>> x(CHUNK_SIZE,y);
	Terrain = x;
	std::vector<int> SurfX(CHUNK_SIZE, 0);
	std::vector<std::vector<int>> SurfY(CHUNK_SIZE,SurfX);
	Surface = SurfY;
	std::vector<char16_t> zFloor(CHUNK_Z, 0);
	std::vector<std::vector<char16_t>> yFloor(CHUNK_SIZE,zFloor);
}

void UChunk::SetBlock(int x, int y, int z, BlockType type)
{
	Terrain[x][y][z] = type;
}

void UChunk::Fill()
{
	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			for (int z = 0; z < CHUNK_SIZE; z++)
			{
				if (Terrain[x][y][z] == BlockType::Empty)
				{
					Terrain[x][y][z] = BlockType::Stone;
				}
				else
				{
					z=MAX_HEIGHT;
				}
			}
		}
	}
}

std::vector<std::vector<std::vector<BlockType>>>& UChunk::GetTerrain()
{
	return Terrain;
}
