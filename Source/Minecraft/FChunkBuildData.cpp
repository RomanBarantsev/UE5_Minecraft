#include "FChunkBuildData.h"

FChunkBuildData::FChunkBuildData()
{
	Blocks.resize(TOTAL_BLOCKS, BlockType::Empty);
	SurfaceHeights.resize(CHUNK_X_SIZE * CHUNK_Y_SIZE, 0);
}

void FChunkBuildData::Fill()
{
	for (int x = 0; x < CHUNK_X_SIZE; x++)
	{
		const int xOff = x * X_STRIDE;

		for (int y = 0; y < CHUNK_Y_SIZE; y++)
		{
			const int yOff = xOff + y * Y_STRIDE;
			const int surface = SurfaceHeights[SurfaceIndex(x,y)];

			for (int z = 0; z < CHUNK_Z_SIZE; z++)
			{
				const int idx = yOff + z;

				if (z > surface)
				{
					Blocks[idx] = Air;
				}
				else if (Blocks[idx] == Empty)
				{
					Blocks[idx] = Stone;
				}
			}
		}
	}
}

int FChunkBuildData::GetSurfaceHeight(int x, int y) const
{
	return SurfaceHeights[SurfaceIndex(x,y)];
}

void FChunkBuildData::SetSurfaceHeight(int x, int y, int height)
{
	SurfaceHeights[SurfaceIndex(x,y)]=height;
}
