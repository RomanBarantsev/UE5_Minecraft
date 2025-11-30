#include "Chunk.h"

void UChunk::InitChunk()
{
	_xMax=CHUNK_SIZE;
	_yMax=CHUNK_SIZE;
	_zMax=CHUNK_Z;
	std::vector<BlockType> z(_zMax, BlockType::Empty);
	std::vector<std::vector<BlockType>> y(_yMax,z);
	std::vector<std::vector<std::vector<BlockType>>> x(_xMax,y);
	Terrain = x;
	std::vector<int> SurfX(_xMax, 0);
	std::vector<std::vector<int>> SurfY(_yMax,SurfX);
	Surface = SurfY;
	std::vector<char16_t> zFloor(_zMax, 0);
	std::vector<std::vector<char16_t>> yFloor(_yMax,zFloor);
}

void UChunk::SetBlock(int x, int y, int z, BlockType type)
{
	Terrain[x][y][z] = type;
}

void UChunk::Fill()
{
	for (int x = 0; x < _xMax; x++)
	{
		for (int y = 0; y < _yMax; y++)
		{
			for (int z = 0; z < MAX_HEIGHT; z++)
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
