#include "FChunkBuildData.h"

FChunkBuildData::FChunkBuildData()
{
	Blocks.resize(TOTAL_BLOCKS, BlockType::Empty);
	SurfaceHeights.resize(CHUNK_X_SIZE * CHUNK_Y_SIZE, 0);
}

int FChunkBuildData::GetSurfaceHeight(int x, int y) const
{
	return SurfaceHeights[SurfaceIndex(x,y)];
}

void FChunkBuildData::SetSurfaceHeight(int x, int y, int height)
{
	SurfaceHeights[SurfaceIndex(x,y)]=height;
}
