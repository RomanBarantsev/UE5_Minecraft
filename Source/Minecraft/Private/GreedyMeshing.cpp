// Fill out your copyright notice in the Description page of Project Settings.


#include "GreedyMeshing.h"
#include "ProceduralMeshComponent.h"
#include "Minecraft/FChunkBuildData.h"
#include "Minecraft/MinecraftProceduralMeshComponent.h"

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
	if (x < 0 || y < 0 || z < 0 || x >= CHUNK_X || y >= CHUNK_X || z >= CHUNK_Z)
		return true; 
	return  Chunk->GetBlock(x,y,z) == BlockType::Air;
}

void FGreedyMeshing::Clear()
{
	Vertices.Empty();
	Triangles.Empty();
	Normals.Empty();
	UVs.Empty();
	UV1s.Empty();
}

void FGreedyMeshing::BuildGreedyMesh(const FChunkBuildData* Data)
{	
	Chunk = Data;
	GreedyZPos( true);
	GreedyZPos( false);
	GreedyXPos( true);
	GreedyXPos( false);
	GreedyYPos( true);
	GreedyYPos( false);
}

void FGreedyMeshing::BuildMesh(const FChunkBuildData& Data)
{
	
}

float FGreedyMeshing::GetTileIndex(BlockType Type)
{
	return (float)Type;
}

void FGreedyMeshing::GreedyZPos(bool bPositive)
{
	
	FMaskCell Mask[CHUNK_X][CHUNK_X];
	int dz = bPositive ? 1 : -1;

	for (int z = 0; z < CHUNK_Z; z++)
	{
		for (int x = 0; x < CHUNK_X; x++)
		{
			for (int y = 0; y < CHUNK_X; y++)
			{
				Mask[x][y].bValid = false;
			}
		}
		for (int x = 0; x < CHUNK_X; x++)
			for (int y = 0; y < CHUNK_X; y++)
			{
				int nz = z + dz;
				if (nz < 0 || nz >= CHUNK_Z) continue;

				if (IsFaceVisible(x, y, z, 0, 0, dz))
				{
					Mask[x][y].bValid = true;
					Mask[x][y].Type =  Chunk->GetBlock(x,y,z);
				}
			}
		for (int x = 0; x < CHUNK_X; x++)
			for (int y = 0; y < CHUNK_X; y++)
			{
				if (!Mask[x][y].bValid)
					continue;

				BlockType Type = Mask[x][y].Type;

				int width = 1;
				while (x + width < CHUNK_X &&
					   Mask[x + width][y].bValid &&
					   Mask[x + width][y].Type == Type)
				{
					width++;
				}

				int height = 1;
				bool done = false;
				while (y + height < CHUNK_X && !done)
				{
					for (int i = 0; i < width; i++)
					{
						if (!Mask[x + i][y + height].bValid ||
							Mask[x + i][y + height].Type != Type)
						{
							done = true;
							break;
						}
					}
					if (!done) height++;
				}
				AddQuadZ(x, y, z, width, height, bPositive, Type);
				for (int dx = 0; dx < width; dx++)
					for (int dy = 0; dy < height; dy++)
						Mask[x + dx][y + dy].bValid = false;
			}
	}
}

void FGreedyMeshing::AddQuadZ(int x, int y, int z, int w, int h, bool bPositive, BlockType Type)
{
    float zPos = bPositive ? (z + 1) * BLOCK_SIZE : z * BLOCK_SIZE;

    FVector base(
        x * BLOCK_SIZE,
        y * BLOCK_SIZE,
        zPos
    );

    FVector dx(w * BLOCK_SIZE, 0, 0);
    FVector dy(0, h * BLOCK_SIZE, 0);

    int start = Vertices.Num();

    if (bPositive)
    {
        Vertices.Add(base);                     // 0: нижний-левый
        Vertices.Add(base + dy);                // 1: верхний-левый  
        Vertices.Add(base + dx + dy);           // 2: верхний-правый
        Vertices.Add(base + dx);                // 3: нижний-правый
        
        Triangles.Append({ start, start + 1, start + 2,
                          start, start + 2, start + 3 });
        Normals.Append({ FVector::UpVector, FVector::UpVector,
                        FVector::UpVector, FVector::UpVector });
    }
    else
    {
        Vertices.Add(base);                     // 0: нижний-левый
        Vertices.Add(base + dx);                // 1: нижний-правый
        Vertices.Add(base + dx + dy);           // 2: верхний-правый
        Vertices.Add(base + dy);                // 3: верхний-левый
        
        Triangles.Append({ start, start + 1, start + 2,
                          start, start + 2, start + 3 });
        Normals.Append({ FVector::DownVector, FVector::DownVector,
                        FVector::DownVector, FVector::DownVector });
    }
    
    const float AtlasSize = 4.0f;
	const float TileSize = 1.0f / AtlasSize;

	float tileIndex = GetTileIndex(Type);
	int tileX = FMath::FloorToInt(tileIndex) % (int)AtlasSize;
	int tileY = FMath::FloorToInt(tileIndex) / (int)AtlasSize;

	float baseU = tileX * TileSize;
	float baseV = tileY * TileSize;

	if (bPositive)
	{
		UVs.Add({0, 0});
		UVs.Add({0, (float)h});
		UVs.Add({(float)w, (float)h});
		UVs.Add({(float)w, 0});
	}
	else
	{
		UVs.Add({(float)w, 0});
		UVs.Add({0, 0});
		UVs.Add({0, (float)h});
		UVs.Add({(float)w, (float)h});
	}

	for (int i = 0; i < 4; i++)
		UV1s.Add({ baseU, baseV });
}

void FGreedyMeshing::GreedyXPos(bool bPositive)
{
    int dx = bPositive ? 1 : -1;

    std::vector<std::vector<FMaskCell>> Mask(CHUNK_X, std::vector<FMaskCell>(CHUNK_Z));

    for (int x = 0; x < CHUNK_X; x++)
    {
        for (int y = 0; y < CHUNK_X; y++)
            for (int z = 0; z < CHUNK_Z; z++)
                Mask[y][z].bValid = false;

        for (int y = 0; y < CHUNK_X; y++)
        {
            for (int z = 0; z < CHUNK_Z; z++)
            {
                if (IsFaceVisible(x, y, z, dx, 0, 0))
                {
                    Mask[y][z].bValid = true;
                    Mask[y][z].Type =  Chunk->GetBlock(x,y,z);
                }
            }
        }

        for (int y = 0; y < CHUNK_X; y++)
        {
            for (int z = 0; z < CHUNK_Z; z++)
            {
                if (!Mask[y][z].bValid)
                    continue;

                BlockType CurrentType = Mask[y][z].Type;

                int width = 1;
                while (y + width < CHUNK_X &&
                       Mask[y + width][z].bValid &&
                       Mask[y + width][z].Type == CurrentType)
                {
                    width++;
                }

                int height = 1;
                bool done = false;
                while (z + height < CHUNK_Z && !done)
                {
                    for (int i = 0; i < width; i++)
                    {
                        if (!Mask[y + i][z + height].bValid ||
                            Mask[y + i][z + height].Type != CurrentType)
                        {
                            done = true;
                            break;
                        }
                    }
                    if (!done) height++;
                }

                AddQuadX(x, y, z, width, height, bPositive, CurrentType);

                for (int dy = 0; dy < width; dy++)
                {
                    for (int dz = 0; dz < height; dz++)
                    {
                        Mask[y + dy][z + dz].bValid = false;
                    }
                }
            }
        }
    }
}

void FGreedyMeshing::AddQuadX(int x, int y, int z, int w, int h, bool bPositive, BlockType Type)
{
    float xCoord;
    FVector Normal;
    
    if (bPositive)
    {
        xCoord = (x + 1) * BLOCK_SIZE;
        Normal = FVector(1, 0, 0);
    }
    else
    {
        xCoord = x * BLOCK_SIZE;
        Normal = FVector(-1, 0, 0); 
    }
    
    float baseY = y * BLOCK_SIZE;
    float baseZ = z * BLOCK_SIZE;
    
    float quadWidth = w * BLOCK_SIZE;  
    float quadHeight = h * BLOCK_SIZE;
    
    int start = Vertices.Num();

    if (bPositive)
    {
        Vertices.Add(FVector(xCoord, baseY, baseZ));                     
        Vertices.Add(FVector(xCoord, baseY, baseZ + quadHeight));        
        Vertices.Add(FVector(xCoord, baseY + quadWidth, baseZ + quadHeight)); 
        Vertices.Add(FVector(xCoord, baseY + quadWidth, baseZ));        
        
        Triangles.Append({ start, start+1, start+2, start, start+2, start+3 });
    }
    else
    {
        // X-: нормаль влево (-X)
        Vertices.Add(FVector(xCoord, baseY, baseZ));                    
        Vertices.Add(FVector(xCoord, baseY + quadWidth, baseZ));         
        Vertices.Add(FVector(xCoord, baseY + quadWidth, baseZ + quadHeight)); 
        Vertices.Add(FVector(xCoord, baseY, baseZ + quadHeight));       
        
        Triangles.Append({ start, start+1, start+2, start, start+2, start+3 });
    }
    
    for (int i = 0; i < 4; i++)
    {
        Normals.Add(Normal);
    }

	const float AtlasSize = 4.0f;
	const float TileSize = 1.0f / AtlasSize;

	float tileIndex = GetTileIndex(Type);
	int tileX = FMath::FloorToInt(tileIndex) % (int)AtlasSize;
	int tileY = FMath::FloorToInt(tileIndex) / (int)AtlasSize;

	float baseU = tileX * TileSize;
	float baseV = tileY * TileSize;

	Tailing(baseU,baseV,w,h,bPositive);   
}

void FGreedyMeshing::Tailing(float baseU, float baseV, int w,int h, bool bPositive)
{
	if (bPositive)
	{
		UVs.Add({0, 0});
		UVs.Add({0, (float)h});
		UVs.Add({(float)w, (float)h});
		UVs.Add({(float)w, 0});
	}
	else
	{
		UVs.Add({(float)w, 0});
		UVs.Add({0, 0});
		UVs.Add({0, (float)h});
		UVs.Add({(float)w, (float)h});
	}


	for (int i = 0; i < 4; i++)
		UV1s.Add(FVector2D(baseU, baseV ));
}

void FGreedyMeshing::GreedyYPos(bool bPositive)
{
    int dy = bPositive ? 1 : -1;

    std::vector<std::vector<FMaskCell>> Mask(CHUNK_X, std::vector<FMaskCell>(CHUNK_Z));

    for (int y = 0; y < CHUNK_X; y++)
    {
        for (int x = 0; x < CHUNK_X; x++)
            for (int z = 0; z < CHUNK_Z; z++)
                Mask[x][z].bValid = false;

        for (int x = 0; x < CHUNK_X; x++)
        {
            for (int z = 0; z < CHUNK_Z; z++)
            {
                if (IsFaceVisible(x, y, z, 0, dy, 0))
                {
                    Mask[x][z].bValid = true;
                    Mask[x][z].Type =  Chunk->GetBlock(x,y,z);
                }
            }
        }

        for (int x = 0; x < CHUNK_X; x++)
        {
            for (int z = 0; z < CHUNK_Z; z++)
            {
                if (!Mask[x][z].bValid)
                    continue;

                BlockType CurrentType = Mask[x][z].Type;

                int width = 1;
                while (x + width < CHUNK_X &&
                       Mask[x + width][z].bValid &&
                       Mask[x + width][z].Type == CurrentType)
                {
                    width++;
                }

                int height = 1;
                bool done = false;
                while (z + height < CHUNK_Z && !done)
                {
                    for (int i = 0; i < width; i++)
                    {
                        if (!Mask[x + i][z + height].bValid ||
                            Mask[x + i][z + height].Type != CurrentType)
                        {
                            done = true;
                            break;
                        }
                    }
                    if (!done) height++;
                }

                AddQuadY(x, y, z, width, height, bPositive, CurrentType);

                for (int dx = 0; dx < width; dx++)
                {
                    for (int dz = 0; dz < height; dz++)
                    {
                        Mask[x + dx][z + dz].bValid = false;
                    }
                }
            }
        }
    }
}
void FGreedyMeshing::AddQuadY(int x, int y, int z, int w, int h, bool bPositive, BlockType Type)
{   
    float yCoord;
    FVector Normal;
    
    if (bPositive)
    {
        yCoord = (y + 1) * BLOCK_SIZE;
        Normal = FVector(0, 1, 0);
    }
    else
    {
        yCoord = y * BLOCK_SIZE;
        Normal = FVector(0, -1, 0);
    }
    
    float baseX = x * BLOCK_SIZE;
    float baseZ = z * BLOCK_SIZE;
    
    float quadWidth = w * BLOCK_SIZE;  
    float quadHeight = h * BLOCK_SIZE; 
    
    int start = Vertices.Num();

    if (bPositive)
    {
        Vertices.Add(FVector(baseX, yCoord, baseZ));                    
        Vertices.Add(FVector(baseX + quadWidth, yCoord, baseZ));       
        Vertices.Add(FVector(baseX + quadWidth, yCoord, baseZ + quadHeight)); 
        Vertices.Add(FVector(baseX, yCoord, baseZ + quadHeight));
        Triangles.Append({ start, start+1, start+2, start, start+2, start+3 });
    }
    else
    {
        // Y- грань
        Vertices.Add(FVector(baseX, yCoord, baseZ));                     
        Vertices.Add(FVector(baseX, yCoord, baseZ + quadHeight));        
        Vertices.Add(FVector(baseX + quadWidth, yCoord, baseZ + quadHeight)); 
        Vertices.Add(FVector(baseX + quadWidth, yCoord, baseZ));        
        
        Triangles.Append({ start, start+1, start+2, start, start+2, start+3 });
    }
    
    for (int i = 0; i < 4; i++)
    {
        Normals.Add(Normal);
    }
    
    const float AtlasSize = 4.0f;
    const float TileSize = 1.0f / AtlasSize;

    float tileIndex = GetTileIndex(Type);
    int tileX = FMath::FloorToInt(tileIndex) % (int)AtlasSize;
    int tileY = FMath::FloorToInt(tileIndex) / (int)AtlasSize;

    float baseU = tileX * TileSize;
    float baseV = tileY * TileSize;

    if (bPositive)
    {
        UVs.Add(FVector2D(0.0f, 0.0f));          
        UVs.Add(FVector2D((float)w, 0.0f));      
        UVs.Add(FVector2D((float)w, (float)h));   
        UVs.Add(FVector2D(0.0f, (float)h));       
    }
    else
    {
        UVs.Add(FVector2D(0.0f, 0.0f));          
        UVs.Add(FVector2D(0.0f, (float)h));      
        UVs.Add(FVector2D((float)w, (float)h));  
        UVs.Add(FVector2D((float)w, 0.0f));      
    }

    for (int i = 0; i < 4; i++)
        UV1s.Add(FVector2D(baseU, baseV));   
 
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
	
	procMesh.SetIndex(0);
	procMesh.SetMaterial(0, Mat);
	Vertices.Reset();
	Triangles.Reset();
	Normals.Reset();
	UVs.Reset();
	UV1s.Reset();
	return procMesh.GetComponentLocation();
}
