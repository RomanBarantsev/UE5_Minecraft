// Fill out your copyright notice in the Description page of Project Settings.


#include "GreedyMeshing.h"

#include "CubeGenerator.h"
#include "ProceduralMeshComponent.h"
#include "Minecraft/MinecraftProceduralMeshComponent.h"

bool UGreedyMeshing::IsFaceVisible(	int x, int y, int z,int dx, int dy, int dz,	const std::vector<std::vector<std::vector<BlockType>>>& Blocks)
{
	BlockType a = Blocks[x][y][z];
	BlockType b = IsAir(x + dx, y + dy, z + dz, Blocks)
				  ? BlockType::Air
				  : Blocks[x + dx][y + dy][z + dz];

	return a != BlockType::Air && b == BlockType::Air;
}

bool UGreedyMeshing::IsAir(int x, int y, int z, const std::vector<std::vector<std::vector<BlockType>>>& Blocks)
{
	if (x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || y >= CHUNK_SIZE || z >= CHUNK_Z)
		return true; // за границей = воздух
	return Blocks[x][y][z] == BlockType::Air;
}


void UGreedyMeshing::BuildChunkMesh(const std::vector<std::vector<std::vector<BlockType>>>& Blocks)
{
	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			for (int z = 0; z < CHUNK_Z; z++)
			{
				BlockType type = Blocks[x][y][z];
				if (type == BlockType::Air) continue;
                
				// Проверяем каждую грань
				if (IsFaceVisible(x, y, z, 0, 0, 1, Blocks))  // Z+
					AddQuadZ(x, y, z, 1, 1, true, type);
				if (IsFaceVisible(x, y, z, 0, 0, -1, Blocks)) // Z-
					AddQuadZ(x, y, z, 1, 1, false, type);
				if (IsFaceVisible(x, y, z, 1, 0, 0, Blocks))  // X+
					AddQuadX(x, y, z, 1, 1, true, type);
				if (IsFaceVisible(x, y, z, -1, 0, 0, Blocks)) // X-
					AddQuadX(x, y, z, 1, 1, false, type);
				if (IsFaceVisible(x, y, z, 0, 1, 0, Blocks))  // Y+
					AddQuadY(x, y, z, 1, 1, true, type);
				if (IsFaceVisible(x, y, z, 0, -1, 0, Blocks)) // Y-
					AddQuadY(x, y, z, 1, 1, false, type);
			}
		}
	}
	//TestAtlasUVs();	    
	/*GreedyZPos(Blocks, true);
	GreedyZPos(Blocks, false);
	GreedyXPos(Blocks, true);
	GreedyXPos(Blocks, false);
	GreedyYPos(Blocks, true);
	GreedyYPos(Blocks, false);*/
}

float UGreedyMeshing::GetTileIndex(BlockType Type)
{
	// Предполагая что BlockType начинается с:
	// Empty=0, Air=1, Grass=2, Dirt=3, Stone=4, ...
    
	switch (Type)
	{
	case BlockType::Grass:        return 0.0f;  // (0,0) - первая текстура в атласе
	case BlockType::Dirt:         return 1.0f;  // (1,0)
	case BlockType::Stone:        return 2.0f;  // (2,0)
	case BlockType::Wood:         return 3.0f;  // (3,0)
	case BlockType::Leaves:       return 4.0f;  // (0,1) - вторая строка
	case BlockType::Sand:         return 5.0f;  // (1,1)
	case BlockType::Gravel:       return 6.0f;  // (2,1)
	case BlockType::Cobblestone:  return 7.0f;  // (3,1)
	case BlockType::Bricks:       return 8.0f;  // (0,2)
	case BlockType::Glass:        return 9.0f;  // (1,2)
	case BlockType::Water:        return 10.0f; // (2,2)
	case BlockType::Lava:         return 11.0f; // (3,2)
	case BlockType::Bedrock:      return 12.0f; // (0,3)
	case BlockType::IronBlock:    return 13.0f; // (1,3)
	case BlockType::GoldBlock:    return 14.0f; // (2,3)
	case BlockType::Empty:
	case BlockType::Air:
	default:
		return 0.0f;  // Grass по умолчанию для воздуха/пустоты
	}
}

void UGreedyMeshing::TestAtlasUVs()
{
	UE_LOG(LogTemp, Warning, TEXT("=== TESTING ATLAS UVs ==="));
    
	// Очищаем все
	Vertices.Empty();
	Triangles.Empty();
	Normals.Empty();
	UVs.Empty();
	UV1s.Empty();
    
	// Создаем 6 граней с разными типами блоков
	// Грани Y+
	AddQuadY(0, 0, 0, 1, 1, true, BlockType::Grass);      // тайл 0,0
	AddQuadY(1, 0, 0, 1, 1, true, BlockType::Dirt);       // тайл 1,0
	AddQuadY(2, 0, 0, 1, 1, true, BlockType::Stone);      // тайл 2,0
	AddQuadY(3, 0, 0, 1, 1, true, BlockType::Wood);       // тайл 3,0
    
	// Грани X+
	AddQuadX(0, 1, 0, 1, 1, true, BlockType::Leaves);     // тайл 0,1
	AddQuadX(1, 1, 0, 1, 1, true, BlockType::Sand);       // тайл 1,1
	AddQuadX(2, 1, 0, 1, 1, true, BlockType::Gravel);     // тайл 2,1
	AddQuadX(3, 1, 0, 1, 1, true, BlockType::Cobblestone);// тайл 3,1
    
	UE_LOG(LogTemp, Warning, TEXT("Test created: vertices=%d, uvs=%d"), 
		Vertices.Num(), UVs.Num());
    
	// Проверяем UV
	for (int i = 0; i < FMath::Min(UVs.Num(), 16); i += 4)
	{
		FVector2D uv = UVs[i];
		UE_LOG(LogTemp, Warning, TEXT("Quad %d: first UV = (%f, %f)"), 
			i/4, uv.X, uv.Y);
	}
}

void UGreedyMeshing::GreedyZPos(const std::vector<std::vector<std::vector<BlockType>>>& Blocks,bool bPositive)
{
	
	FMaskCell Mask[CHUNK_SIZE][CHUNK_SIZE];
	int dz = bPositive ? 1 : -1;

	for (int z = 0; z < CHUNK_Z; z++)
	{
		//clear mask
		for (int x = 0; x < CHUNK_SIZE; x++)
		{
			for (int y = 0; y < CHUNK_SIZE; y++)
			{
				Mask[x][y].bValid = false;
			}
		}
		// 1. build mask
		for (int x = 0; x < CHUNK_SIZE; x++)
			for (int y = 0; y < CHUNK_SIZE; y++)
			{
				int nz = z + dz;
				if (nz < 0 || nz >= CHUNK_Z) continue;

				if (IsFaceVisible(x, y, z, 0, 0, dz, Blocks))
				{
					Mask[x][y].bValid = true;
					Mask[x][y].Type = Blocks[x][y][z];
				}
			}

		// 2. greedy merge mask
		for (int x = 0; x < CHUNK_SIZE; x++)
			for (int y = 0; y < CHUNK_SIZE; y++)
			{
				if (!Mask[x][y].bValid)
					continue;

				BlockType Type = Mask[x][y].Type;

				int width = 1;
				while (x + width < CHUNK_SIZE &&
					   Mask[x + width][y].bValid &&
					   Mask[x + width][y].Type == Type)
				{
					width++;
				}

				int height = 1;
				bool done = false;
				while (y + height < CHUNK_SIZE && !done)
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
    
    // КЛЮЧЕВОЕ ИЗМЕНЕНИЕ: передаем информацию о тайле через UV
    // Формат: (localU + tileX, localV + tileY)
    // Где localU = 0..w, localV = 0..h (для тайлинга)
    // А tileX, tileY = позиция в атласе (0-3 для 4x4)
    
    const float AtlasSize = 4.0f;
    float tileIndex = GetTileIndex(Type);
    int tileX = FMath::FloorToInt(tileIndex) % (int)AtlasSize;
    int tileY = FMath::FloorToInt(tileIndex) / (int)AtlasSize;
    
    // Добавляем смещение тайла к UV координатам
    // В материале потом умножим на 2, возьмем Frac и умножим на 0.25
    
    if (bPositive)
    {
        // Вершина 0: (0, 0) -> (tileX + 0, tileY + 0)
        UVs.Add(FVector2D((float)tileX + 0.0f, (float)tileY + 0.0f));
        
        // Вершина 1: (0, h) -> (tileX + 0, tileY + h)
        UVs.Add(FVector2D((float)tileX + 0.0f, (float)tileY + (float)h));
        
        // Вершина 2: (w, h) -> (tileX + w, tileY + h)
        UVs.Add(FVector2D((float)tileX + (float)w, (float)tileY + (float)h));
        
        // Вершина 3: (w, 0) -> (tileX + w, tileY + 0)
        UVs.Add(FVector2D((float)tileX + (float)w, (float)tileY + 0.0f));
    }
    else
    {
        // Задняя грань (зеркально)
        UVs.Add(FVector2D((float)tileX + (float)w, (float)tileY + 0.0f));
        UVs.Add(FVector2D((float)tileX + 0.0f, (float)tileY + 0.0f));
        UVs.Add(FVector2D((float)tileX + 0.0f, (float)tileY + (float)h));
        UVs.Add(FVector2D((float)tileX + (float)w, (float)tileY + (float)h));
    }
}

void UGreedyMeshing::GreedyXPos(const std::vector<std::vector<std::vector<BlockType>>>& Blocks, bool bPositive)
{
    int dx = bPositive ? 1 : -1;

    // НУЖНО ИЗМЕНИТЬ РАЗМЕР МАСКИ ДЛЯ ОСИ X!
    // Для оси X: маска должна быть размером [CHUNK_SIZE][CHUNK_Z]
    
    // Создаем маску с правильными размерами
    std::vector<std::vector<FMaskCell>> Mask(CHUNK_SIZE, std::vector<FMaskCell>(CHUNK_Z));

    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        // 1. Очистить маску для текущего слоя X
        for (int y = 0; y < CHUNK_SIZE; y++)
            for (int z = 0; z < CHUNK_Z; z++)
                Mask[y][z].bValid = false;

        // 2. Построить маску видимых граней
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
            for (int z = 0; z < CHUNK_Z; z++)
            {
                // Проверяем грани по оси X
                if (IsFaceVisible(x, y, z, dx, 0, 0, Blocks))
                {
                    Mask[y][z].bValid = true;
                    Mask[y][z].Type = Blocks[x][y][z];
                }
            }
        }

        // 3. Объединение видимых граней (Greedy алгоритм)
        for (int y = 0; y < CHUNK_SIZE; y++)
        {
            for (int z = 0; z < CHUNK_Z; z++)
            {
                if (!Mask[y][z].bValid)
                    continue;

                BlockType CurrentType = Mask[y][z].Type;

                // Находим ширину (по оси Y)
                int width = 1;
                while (y + width < CHUNK_SIZE &&
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
    
    // ====== ИСПРАВЛЕННЫЕ UV (как в AddQuadZ) ======
    const float AtlasSize = 4.0f;
    float tileIndex = GetTileIndex(Type);
    int tileX = FMath::FloorToInt(tileIndex) % (int)AtlasSize;
    int tileY = FMath::FloorToInt(tileIndex) / (int)AtlasSize;
    
    if (bPositive)
    {
        // X+ грань
        // UV для 4 вершин: (tileX + localU, tileY + localV)
        // где localU = 0..w, localV = 0..h
        UVs.Add(FVector2D((float)tileX + 0.0f, (float)tileY + 0.0f));      // 0
        UVs.Add(FVector2D((float)tileX + 0.0f, (float)tileY + (float)h));  // 1
        UVs.Add(FVector2D((float)tileX + (float)w, (float)tileY + (float)h)); // 2
        UVs.Add(FVector2D((float)tileX + (float)w, (float)tileY + 0.0f));  // 3
    }
    else
    {
        // X- грань
        UVs.Add(FVector2D((float)tileX + (float)w, (float)tileY + 0.0f));  // 0
        UVs.Add(FVector2D((float)tileX + 0.0f, (float)tileY + 0.0f));      // 1
        UVs.Add(FVector2D((float)tileX + 0.0f, (float)tileY + (float)h));  // 2
        UVs.Add(FVector2D((float)tileX + (float)w, (float)tileY + (float)h)); // 3
    }
    
    // UV1 — TileIndex (для альтернативного подхода)
    for (int i = 0; i < 4; i++)
    {
        UV1s.Add(FVector2D(tileIndex, 0));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AddQuadX: pos(%d,%d,%d) w=%d h=%d type=%d UV0:(%f,%f)"), 
        x, y, z, w, h, (int)Type, UVs[UVs.Num()-4].X, UVs[UVs.Num()-4].Y);
}

void UGreedyMeshing::GreedyYPos(const std::vector<std::vector<std::vector<BlockType>>>& Blocks, bool bPositive)
{
    int dy = bPositive ? 1 : -1;

    // Для оси Y: маска должна быть размером [CHUNK_SIZE][CHUNK_Z]
    std::vector<std::vector<FMaskCell>> Mask(CHUNK_SIZE, std::vector<FMaskCell>(CHUNK_Z));

    for (int y = 0; y < CHUNK_SIZE; y++)
    {
        // Очистить маску
        for (int x = 0; x < CHUNK_SIZE; x++)
            for (int z = 0; z < CHUNK_Z; z++)
                Mask[x][z].bValid = false;

        // Построить маску
        for (int x = 0; x < CHUNK_SIZE; x++)
        {
            for (int z = 0; z < CHUNK_Z; z++)
            {
                if (IsFaceVisible(x, y, z, 0, dy, 0, Blocks))
                {
                    Mask[x][z].bValid = true;
                    Mask[x][z].Type = Blocks[x][y][z];
                }
            }
        }

        // Объединение
        for (int x = 0; x < CHUNK_SIZE; x++)
        {
            for (int z = 0; z < CHUNK_Z; z++)
            {
                if (!Mask[x][z].bValid)
                    continue;

                BlockType CurrentType = Mask[x][z].Type;

                int width = 1;
                while (x + width < CHUNK_SIZE &&
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
        Vertices.Add(FVector(baseX, yCoord, baseZ));                     // 0
        Vertices.Add(FVector(baseX + quadWidth, yCoord, baseZ));         // 1
        Vertices.Add(FVector(baseX + quadWidth, yCoord, baseZ + quadHeight)); // 2
        Vertices.Add(FVector(baseX, yCoord, baseZ + quadHeight));        // 3
        
        Triangles.Append({ start, start+1, start+2, start, start+2, start+3 });
    }
    else
    {
        // Y- грань
        Vertices.Add(FVector(baseX, yCoord, baseZ));                     // 0
        Vertices.Add(FVector(baseX, yCoord, baseZ + quadHeight));        // 1
        Vertices.Add(FVector(baseX + quadWidth, yCoord, baseZ + quadHeight)); // 2
        Vertices.Add(FVector(baseX + quadWidth, yCoord, baseZ));         // 3
        
        Triangles.Append({ start, start+1, start+2, start, start+2, start+3 });
    }
    
    // Добавляем нормали
    for (int i = 0; i < 4; i++)
    {
        Normals.Add(Normal);
    }
    
    // ====== ИСПРАВЛЕННЫЕ UV (как в AddQuadZ) ======
    const float AtlasSize = 4.0f;
    float tileIndex = GetTileIndex(Type);
    int tileX = FMath::FloorToInt(tileIndex) % (int)AtlasSize;
    int tileY = FMath::FloorToInt(tileIndex) / (int)AtlasSize;
    
    if (bPositive)
    {
        // Y+ грань
        UVs.Add(FVector2D((float)tileX + 0.0f, (float)tileY + 0.0f));      // 0
        UVs.Add(FVector2D((float)tileX + (float)w, (float)tileY + 0.0f));  // 1
        UVs.Add(FVector2D((float)tileX + (float)w, (float)tileY + (float)h)); // 2
        UVs.Add(FVector2D((float)tileX + 0.0f, (float)tileY + (float)h));  // 3
    }
    else
    {
        // Y- грань
        UVs.Add(FVector2D((float)tileX + 0.0f, (float)tileY + 0.0f));      // 0
        UVs.Add(FVector2D((float)tileX + 0.0f, (float)tileY + (float)h));  // 1
        UVs.Add(FVector2D((float)tileX + (float)w, (float)tileY + (float)h)); // 2
        UVs.Add(FVector2D((float)tileX + (float)w, (float)tileY + 0.0f));  // 3
    }
    
    // UV1 — TileIndex
    for (int i = 0; i < 4; i++)
    {
        UV1s.Add(FVector2D(tileIndex, 0));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("AddQuadY: pos(%d,%d,%d) w=%d h=%d type=%d UV0:(%f,%f)"), 
        x, y, z, w, h, (int)Type, UVs[UVs.Num()-4].X, UVs[UVs.Num()-4].Y);
}

FVector UGreedyMeshing::CreateMesh(UMinecraftProceduralMeshComponent& procMesh,UMaterialInterface* Mat, const int64& Section)
{
	procMesh.CreateMeshSection(
	Section,
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
	procMesh.SetIndex(Section);
	procMesh.SetMaterial(Section, Mat);
	Vertices.Reset();
	Triangles.Reset();
	Normals.Reset();
	UVs.Reset();
	UV1s.Reset();
	return procMesh.GetComponentLocation();
}
