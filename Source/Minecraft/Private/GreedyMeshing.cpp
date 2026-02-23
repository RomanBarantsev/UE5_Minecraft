// Fill out your copyright notice in the Description page of Project Settings.


#include "GreedyMeshing.h"
#include "CubeGenerator.h"
#include "ProceduralMeshComponent.h"
#include "Minecraft/MinecraftProceduralMeshComponent.h"

bool UGreedyMeshing::IsFaceVisible(	int x, int y, int z,int dx, int dy, int dz)
{
	BlockType a = Chunk->GetBlock(x,y,z);
	BlockType b = IsAir(x + dx, y + dy, z + dz)
				  ? BlockType::Air
				  : Chunk->GetBlock(x + dx,y + dy,z + dz);

	return a != BlockType::Air && b == BlockType::Air;
}

bool UGreedyMeshing::IsAir(int x, int y, int z)
{
	if (x < 0 || y < 0 || z < 0 || x >= CHUNK_X || y >= CHUNK_X || z >= CHUNK_Z)
		return true; // за границей = воздух
	return  Chunk->GetBlock(x,y,z) == BlockType::Air;
}


void UGreedyMeshing::BuildGreedyMesh(const UChunk* ch)
{	
	Chunk = ch;
	GreedyZPos( true);
	GreedyZPos( false);
	GreedyXPos( true);
	GreedyXPos( false);
	GreedyYPos( true);
	GreedyYPos( false);
}

float UGreedyMeshing::GetTileIndex(BlockType Type)
{
	return (float)Type;
}

void UGreedyMeshing::GreedyZPos(bool bPositive)
{
	
	FMaskCell Mask[CHUNK_X][CHUNK_X];
	int dz = bPositive ? 1 : -1;

	for (int z = 0; z < CHUNK_Z; z++)
	{
		//clear mask
		for (int x = 0; x < CHUNK_X; x++)
		{
			for (int y = 0; y < CHUNK_X; y++)
			{
				Mask[x][y].bValid = false;
			}
		}
		// 1. build mask
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

		// 2. greedy merge mask
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

				// 3. добавить ОДНУ большую грань
				AddQuadZ(x, y, z, width, height, bPositive, Type);

				// 4. "съесть" mask
				for (int dx = 0; dx < width; dx++)
					for (int dy = 0; dy < height; dy++)
						Mask[x + dx][y + dy].bValid = false;
			}
	}
}

void UGreedyMeshing::AddQuadZ(int x, int y, int z, int w, int h, bool bPositive, BlockType Type)
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
    
    
	// ===== UV система для greedy + атлас =====

	const float AtlasSize = 4.0f;
	const float TileSize = 1.0f / AtlasSize;

	float tileIndex = GetTileIndex(Type);
	int tileX = FMath::FloorToInt(tileIndex) % (int)AtlasSize;
	int tileY = FMath::FloorToInt(tileIndex) / (int)AtlasSize;

	float baseU = tileX * TileSize;
	float baseV = tileY * TileSize;

	// ---- UV0 : тайлинг (0..w, 0..h)

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

	// ---- UV1 : смещение тайла в атласе

	for (int i = 0; i < 4; i++)
		UV1s.Add({ baseU, baseV });
}

void UGreedyMeshing::GreedyXPos(bool bPositive)
{
    int dx = bPositive ? 1 : -1;

    // НУЖНО ИЗМЕНИТЬ РАЗМЕР МАСКИ ДЛЯ ОСИ X!
    // Для оси X: маска должна быть размером [CHUNK_X][CHUNK_Z]
    
    // Создаем маску с правильными размерами
    std::vector<std::vector<FMaskCell>> Mask(CHUNK_X, std::vector<FMaskCell>(CHUNK_Z));

    for (int x = 0; x < CHUNK_X; x++)
    {
        // 1. Очистить маску для текущего слоя X
        for (int y = 0; y < CHUNK_X; y++)
            for (int z = 0; z < CHUNK_Z; z++)
                Mask[y][z].bValid = false;

        // 2. Построить маску видимых граней
        for (int y = 0; y < CHUNK_X; y++)
        {
            for (int z = 0; z < CHUNK_Z; z++)
            {
                // Проверяем грани по оси X
                if (IsFaceVisible(x, y, z, dx, 0, 0))
                {
                    Mask[y][z].bValid = true;
                    Mask[y][z].Type =  Chunk->GetBlock(x,y,z);
                }
            }
        }

        // 3. Объединение видимых граней (Greedy алгоритм)
        for (int y = 0; y < CHUNK_X; y++)
        {
            for (int z = 0; z < CHUNK_Z; z++)
            {
                if (!Mask[y][z].bValid)
                    continue;

                BlockType CurrentType = Mask[y][z].Type;

                // Находим ширину (по оси Y)
                int width = 1;
                while (y + width < CHUNK_X &&
                       Mask[y + width][z].bValid &&
                       Mask[y + width][z].Type == CurrentType)
                {
                    width++;
                }

                // Находим высоту (по оси Z)
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

                // 4. Добавить объединенный квад
                AddQuadX(x, y, z, width, height, bPositive, CurrentType);

                // 5. Пометить использованные ячейки как обработанные
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

void UGreedyMeshing::AddQuadX(int x, int y, int z, int w, int h, bool bPositive, BlockType Type)
{
    float xCoord;
    FVector Normal;
    
    if (bPositive)
    {
        // X+ (правая грань)
        xCoord = (x + 1) * BLOCK_SIZE;
        Normal = FVector(1, 0, 0); // Нормаль вправо
    }
    else
    {
        // X- (левая грань)
        xCoord = x * BLOCK_SIZE;
        Normal = FVector(-1, 0, 0); // Нормаль влево
    }
    
    // Базовые координаты
    float baseY = y * BLOCK_SIZE;
    float baseZ = z * BLOCK_SIZE;
    
    // Размеры квада
    float quadWidth = w * BLOCK_SIZE;  // по Y
    float quadHeight = h * BLOCK_SIZE; // по Z
    
    int start = Vertices.Num();

    if (bPositive)
    {
        // X+: нормаль вправо (+X)
        Vertices.Add(FVector(xCoord, baseY, baseZ));                     // 0
        Vertices.Add(FVector(xCoord, baseY, baseZ + quadHeight));        // 1
        Vertices.Add(FVector(xCoord, baseY + quadWidth, baseZ + quadHeight)); // 2
        Vertices.Add(FVector(xCoord, baseY + quadWidth, baseZ));         // 3
        
        Triangles.Append({ start, start+1, start+2, start, start+2, start+3 });
    }
    else
    {
        // X-: нормаль влево (-X)
        Vertices.Add(FVector(xCoord, baseY, baseZ));                     // 0
        Vertices.Add(FVector(xCoord, baseY + quadWidth, baseZ));         // 1
        Vertices.Add(FVector(xCoord, baseY + quadWidth, baseZ + quadHeight)); // 2
        Vertices.Add(FVector(xCoord, baseY, baseZ + quadHeight));        // 3
        
        Triangles.Append({ start, start+1, start+2, start, start+2, start+3 });
    }
    
    // Добавляем нормали
    for (int i = 0; i < 4; i++)
    {
        Normals.Add(Normal);
    }
	// ===== UV система для greedy + атлас =====

	const float AtlasSize = 4.0f;
	const float TileSize = 1.0f / AtlasSize;

	float tileIndex = GetTileIndex(Type);
	int tileX = FMath::FloorToInt(tileIndex) % (int)AtlasSize;
	int tileY = FMath::FloorToInt(tileIndex) / (int)AtlasSize;

	float baseU = tileX * TileSize;
	float baseV = tileY * TileSize;

	// ---- UV0 : тайлинг (0..w, 0..h)
	Tailing(baseU,baseV,w,h,bPositive);   
}

void UGreedyMeshing::Tailing(float baseU, float baseV, int w,int h, bool bPositive)
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

	// ---- UV1 : смещение тайла в атласе

	for (int i = 0; i < 4; i++)
		UV1s.Add(FVector2D(baseU, baseV ));
}

void UGreedyMeshing::GreedyYPos(bool bPositive)
{
    int dy = bPositive ? 1 : -1;

    // Для оси Y: маска должна быть размером [CHUNK_X][CHUNK_Z]
    std::vector<std::vector<FMaskCell>> Mask(CHUNK_X, std::vector<FMaskCell>(CHUNK_Z));

    for (int y = 0; y < CHUNK_X; y++)
    {
        // Очистить маску
        for (int x = 0; x < CHUNK_X; x++)
            for (int z = 0; z < CHUNK_Z; z++)
                Mask[x][z].bValid = false;

        // Построить маску
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

        // Объединение
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
void UGreedyMeshing::AddQuadY(int x, int y, int z, int w, int h, bool bPositive, BlockType Type)
{
    // Для оси Y:
    // w - размер по X (width)
    // h - размер по Z (height)
    
    // Определяем координату плоскости Y
    float yCoord;
    FVector Normal;
    
    if (bPositive)
    {
        // Y+ (грань смотрит в сторону +Y)
        yCoord = (y + 1) * BLOCK_SIZE;
        Normal = FVector(0, 1, 0);
    }
    else
    {
        // Y- (грань смотрит в сторону -Y)
        yCoord = y * BLOCK_SIZE;
        Normal = FVector(0, -1, 0);
    }
    
    // Базовые координаты
    float baseX = x * BLOCK_SIZE;
    float baseZ = z * BLOCK_SIZE;
    
    // Размеры квада
    float quadWidth = w * BLOCK_SIZE;  // по X
    float quadHeight = h * BLOCK_SIZE; // по Z
    
    int start = Vertices.Num();

    if (bPositive)
    {
        // Y+ грань
        Vertices.Add(FVector(baseX, yCoord, baseZ));                     // 0: нижний-левый
        Vertices.Add(FVector(baseX + quadWidth, yCoord, baseZ));         // 1: нижний-правый
        Vertices.Add(FVector(baseX + quadWidth, yCoord, baseZ + quadHeight)); // 2: верхний-правый
        Vertices.Add(FVector(baseX, yCoord, baseZ + quadHeight));        // 3: верхний-левый
        
        Triangles.Append({ start, start+1, start+2, start, start+2, start+3 });
    }
    else
    {
        // Y- грань
        Vertices.Add(FVector(baseX, yCoord, baseZ));                     // 0: нижний-левый
        Vertices.Add(FVector(baseX, yCoord, baseZ + quadHeight));        // 1: верхний-левый
        Vertices.Add(FVector(baseX + quadWidth, yCoord, baseZ + quadHeight)); // 2: верхний-правый
        Vertices.Add(FVector(baseX + quadWidth, yCoord, baseZ));         // 3: нижний-правый
        
        Triangles.Append({ start, start+1, start+2, start, start+2, start+3 });
    }
    
    // Добавляем нормали
    for (int i = 0; i < 4; i++)
    {
        Normals.Add(Normal);
    }
    
    // ===== UV система для greedy + атлас =====
    const float AtlasSize = 4.0f;
    const float TileSize = 1.0f / AtlasSize;

    float tileIndex = GetTileIndex(Type);
    int tileX = FMath::FloorToInt(tileIndex) % (int)AtlasSize;
    int tileY = FMath::FloorToInt(tileIndex) / (int)AtlasSize;

    float baseU = tileX * TileSize;
    float baseV = tileY * TileSize;

    // ---- UV0 : тайлинг (0..w, 0..h)
    // Для Y+ грани: вершины идут по часовой стрелке снизу-слева
    // Для Y- грани: вершины идут против часовой стрелки

    if (bPositive)
    {
        // Y+: 0-1-2-3: нижний-левый -> нижний-правый -> верхний-правый -> верхний-левый
        UVs.Add(FVector2D(0.0f, 0.0f));           // v0: нижний-левый (U=0, V=0)
        UVs.Add(FVector2D((float)w, 0.0f));       // v1: нижний-правый (U=w, V=0)
        UVs.Add(FVector2D((float)w, (float)h));   // v2: верхний-правый (U=w, V=h)
        UVs.Add(FVector2D(0.0f, (float)h));       // v3: верхний-левый (U=0, V=h)
    }
    else
    {
        // Y-: 0-1-2-3: нижний-левый -> верхний-левый -> верхний-правый -> нижний-правый
        UVs.Add(FVector2D(0.0f, 0.0f));           // v0: нижний-левый (U=0, V=0)
        UVs.Add(FVector2D(0.0f, (float)h));       // v1: верхний-левый (U=0, V=h)
        UVs.Add(FVector2D((float)w, (float)h));   // v2: верхний-правый (U=w, V=h)
        UVs.Add(FVector2D((float)w, 0.0f));       // v3: нижний-правый (U=w, V=0)
    }

    // ---- UV1 : смещение тайла в атласе
    for (int i = 0; i < 4; i++)
        UV1s.Add(FVector2D(baseU, baseV));   
 
}

FVector UGreedyMeshing::CreateMesh(UMinecraftProceduralMeshComponent& procMesh,UMaterialInterface* Mat)
{
	procMesh.CreateMeshSection(
	0,
	Vertices,
	Triangles,
	Normals,
	UVs,     // UV0
	UV1s,    // UV1
	TArray<FVector2D>(), // UV2
	TArray<FVector2D>(), // UV3
	TArray<FColor>(),
	TArray<FProcMeshTangent>(),
	true
);
	procMesh.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	procMesh.SetCollisionObjectType(ECC_WorldDynamic);
	procMesh.SetCollisionResponseToAllChannels(ECR_Block);

	/*procMesh.SetSimulatePhysics(true);
	procMesh.SetEnableGravity(true);
	procMesh.ContainsPhysicsTriMeshData(true);
	procMesh.RecreatePhysicsState();
	procMesh.bUseComplexAsSimpleCollision = false;*/

	
	procMesh.SetIndex(0);
	procMesh.SetMaterial(0, Mat);
	Vertices.Reset();
	Triangles.Reset();
	Normals.Reset();
	UVs.Reset();
	UV1s.Reset();
	return procMesh.GetComponentLocation();
}
