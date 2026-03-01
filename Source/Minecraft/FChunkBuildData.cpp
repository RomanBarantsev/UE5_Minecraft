#include "FChunkBuildData.h"

FChunkBuildData::FChunkBuildData()
{
	Blocks.resize(TOTAL_BLOCKS, BlockType::Empty);
	SurfaceHeights.resize(CHUNK_X * CHUNK_Y, 0);
}

void FChunkBuildData::Fill()
{
	for (int x = 0; x < CHUNK_X; x++)
	{
		const int xOff = x * X_STRIDE;

		for (int y = 0; y < CHUNK_Y; y++)
		{
			const int yOff = xOff + y * Y_STRIDE;
			const int surface = SurfaceHeights[SurfaceIndex(x,y)];

			for (int z = 0; z < CHUNK_Z; z++)
			{
				const int idx = yOff + z;

				if (z > surface)
				{
					Blocks[idx] = Air;
				}
				else if (z == surface)
				{
					Blocks[idx] = Bricks;
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
