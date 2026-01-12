#include "Chunk.h"

#include "DSP/AudioDebuggingUtilities.h"

int UChunk::Index(uint8_t x, uint8_t y, uint16_t z) const
{
	return x+y*CHUNK_SIZE+z*SLICE_SIZE;
}

int UChunk::SurfaceIndex(uint8_t x, uint8_t y) const
{
	return x + y * CHUNK_SIZE;
}

void UChunk::SetBlock(uint8_t x, uint8_t y, uint16_t z, BlockType type)
{
	Cubes[Index(x,y,z)]=type;
}

BlockType UChunk::GetBlock(uint8_t x, uint8_t y, uint16_t z) const
{
	return Cubes[Index(x,y,z)];
}

void UChunk::SetSurfaceHeight(uint8_t x,uint8_t y,uint16_t height)
{
	Surface[SurfaceIndex(x,y)]=height;
}

void UChunk::Fill()
{
	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			for (int z = 0; z < CHUNK_Z; z++)
			{
				if (z>Surface[SurfaceIndex(x,y)])
				{
					Cubes[Index(x,y,z)] = BlockType::Air;
				}
				if (Cubes[Index(x,y,z)] == BlockType::Empty)
				{
					Cubes[Index(x,y,z)] = BlockType::Stone;
				}
				if (z==Surface[SurfaceIndex(x,y)])
				{
					Cubes[Index(x,y,z)] = BlockType::Bricks;
				}
			}
		}
	}
}

