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

void UGreedyMeshing::AddFace(const FVector& BlockPos, EFace Face, BlockType Type)
{
	int StartIndex = Vertices.Num();
	FVector v0, v1, v2, v3;
	FVector normal;
	switch (Face)
	{
	case EFace::PosZ:
		v0 = BlockPos + FVector(0, 0, BLOCK_SIZE);
		v1 = BlockPos + FVector(0, BLOCK_SIZE, BLOCK_SIZE);
		v2 = BlockPos + FVector(BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);
		v3 = BlockPos + FVector(BLOCK_SIZE, 0, BLOCK_SIZE);
		normal = FVector::UpVector;
		break;
	case EFace::NegZ:
		v0 = BlockPos + FVector(0, 0, 0);
		v1 = BlockPos + FVector(BLOCK_SIZE, 0, 0);
		v2 = BlockPos + FVector(BLOCK_SIZE, BLOCK_SIZE, 0);
		v3 = BlockPos + FVector(0, BLOCK_SIZE, 0);
		normal = FVector::DownVector;
		break;
	case EFace::PosY:
		v0 = BlockPos + FVector(0, BLOCK_SIZE, 0);
		v1 = BlockPos + FVector(BLOCK_SIZE, BLOCK_SIZE, 0);
		v2 = BlockPos + FVector(BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);
		v3 = BlockPos + FVector(0, BLOCK_SIZE, BLOCK_SIZE);
		normal = FVector::RightVector;
		break;
	case EFace::NegY:
		v0 = BlockPos + FVector(BLOCK_SIZE, 0, 0);
		v1 = BlockPos + FVector(0, 0, 0);
		v2 = BlockPos + FVector(0, 0, BLOCK_SIZE);
		v3 = BlockPos + FVector(BLOCK_SIZE, 0, BLOCK_SIZE);
		normal = FVector::LeftVector;
		break;
	case EFace::PosX:
		v0 = BlockPos + FVector(BLOCK_SIZE, BLOCK_SIZE, 0);
		v1 = BlockPos + FVector(BLOCK_SIZE, 0, 0);
		v2 = BlockPos + FVector(BLOCK_SIZE, 0, BLOCK_SIZE);
		v3 = BlockPos + FVector(BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);
		normal = FVector::ForwardVector;
		break;
	case EFace::NegX:
		v0 = BlockPos + FVector(0, 0, 0);
		v1 = BlockPos + FVector(0, BLOCK_SIZE, 0);
		v2 = BlockPos + FVector(0, BLOCK_SIZE, BLOCK_SIZE);
		v3 = BlockPos + FVector(0, 0, BLOCK_SIZE);
		normal = FVector::BackwardVector;
		break;	
	default:
		return;
	}
	Vertices.Append({ v0, v1, v2, v3 });
	Triangles.Append({
		StartIndex + 0, StartIndex + 1, StartIndex + 2,
		StartIndex + 0, StartIndex + 2, StartIndex + 3
	});

	for (int i = 0; i < 4; i++)
		Normals.Add(normal);
	
	FVector4 UV = GetBlockUV(Type);
	UVs.Append({
	FVector2D(UV.X,          UV.Y),
	FVector2D(UV.X + UV.Z,   UV.Y),
	FVector2D(UV.X + UV.Z,   UV.Y + UV.W),
	FVector2D(UV.X,          UV.Y + UV.W)
	});
}

void UGreedyMeshing::BuildChunkMesh(const std::vector<std::vector<std::vector<BlockType>>>& Blocks)
{
	//GreedyZPos(Blocks, false);
	GreedyZPos(Blocks, true);
	GreedyXPos(Blocks, true);
	GreedyXPos(Blocks, false);
	GreedyYPos(Blocks, true);
	GreedyYPos(Blocks, false);
	for (int x = 0; x < CHUNK_SIZE; x++)
		for (int y = 0; y < CHUNK_SIZE; y++)
			for (int z = 0; z < CHUNK_Z; z++)
			{
				if (Blocks[x][y][z] == BlockType::Air)
					continue;

				FVector BlockPos(
					x * BLOCK_SIZE,
					y * BLOCK_SIZE,
					z * BLOCK_SIZE
				);
				BlockType type = Blocks[x][y][z];
				//if (IsAir(x + 1, y, z, Blocks)) AddFace(BlockPos, EFace::PosX,type);
				//if (IsAir(x - 1, y, z, Blocks)) AddFace(BlockPos, EFace::NegX,type);
				//if (IsAir(x, y + 1, z, Blocks)) AddFace(BlockPos, EFace::PosY,type);
				//if (IsAir(x, y - 1, z, Blocks)) AddFace(BlockPos, EFace::NegY,type);
				/*if (IsAir(x, y, z + 1, Blocks)) AddFace(BlockPos, EFace::PosZ,type);
				if (IsAir(x, y, z - 1, Blocks)) AddFace(BlockPos, EFace::NegZ,type);*/
			}
}


FVector4 UGreedyMeshing::GetBlockUV(BlockType Type)
{
	constexpr float T = 0.25;

	switch (Type)
	{
	case BlockType::Grass: return FVector4(0*T, 0*T, T, T);
	case BlockType::Dirt:  return FVector4(1*T, 0*T, T, T);
	case BlockType::Stone: return FVector4(2*T, 0*T, T, T);
	default:               return FVector4(0,   0,   T, T);
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


void UGreedyMeshing::AddQuadZ(int x, int y, int z,int w, int h,bool bPositive,BlockType Type)
{
	FVector base(
		x * BLOCK_SIZE,
		y * BLOCK_SIZE,
		z * BLOCK_SIZE + (bPositive ? BLOCK_SIZE : 0)
	);

	FVector dx(w * BLOCK_SIZE, 0, 0);
	FVector dy(0, h * BLOCK_SIZE, 0);

	int start = Vertices.Num();

	Vertices.Add(base);
	Vertices.Add(base + dy);
	Vertices.Add(base + dx + dy);
	Vertices.Add(base + dx);

	if (bPositive)
	{
		Triangles.Append({ start, start+1, start+2, start, start+2, start+3 });
		Normals.Append({ FVector::UpVector, FVector::UpVector, FVector::UpVector, FVector::UpVector });
	}
	else
	{
		Triangles.Append({ start, start+2, start+1, start, start+3, start+2 });
		Normals.Append({ FVector::DownVector, FVector::DownVector, FVector::DownVector, FVector::DownVector });
	}

	FVector4 UV = GetBlockUV(Type);
	UVs.Append({
		{UV.X, UV.Y},
		{UV.X, UV.Y + UV.W * h},
		{UV.X + UV.Z * w, UV.Y + UV.W * h},
		{UV.X + UV.Z * w, UV.Y}
	});
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
    // Для оси X:
    // w - размер по Y (width)
    // h - размер по Z (height)
    
    // Определяем координату плоскости X
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
        // Порядок вершин против часовой стрелки если смотреть снаружи
        Vertices.Add(FVector(xCoord, baseY, baseZ));                     // 0: ближний-нижний
        Vertices.Add(FVector(xCoord, baseY, baseZ + quadHeight));        // 1: ближний-верхний
        Vertices.Add(FVector(xCoord, baseY + quadWidth, baseZ + quadHeight)); // 2: дальний-верхний
        Vertices.Add(FVector(xCoord, baseY + quadWidth, baseZ));         // 3: дальний-нижний
        
        Triangles.Append({ start, start+1, start+2, start, start+2, start+3 });
    }
    else
    {
        // X-: нормаль влево (-X)
        // Порядок вершин по часовой стрелке если смотреть снаружи
        Vertices.Add(FVector(xCoord, baseY, baseZ));                     // 0: ближний-нижний
        Vertices.Add(FVector(xCoord, baseY + quadWidth, baseZ));         // 1: дальний-нижний
        Vertices.Add(FVector(xCoord, baseY + quadWidth, baseZ + quadHeight)); // 2: дальний-верхний
        Vertices.Add(FVector(xCoord, baseY, baseZ + quadHeight));        // 3: ближний-верхний
        
        Triangles.Append({ start, start+1, start+2, start, start+2, start+3 });
    }
    
    // Добавляем нормали
    for (int i = 0; i < 4; i++)
    {
        Normals.Add(Normal);
    }
    
    // UV координаты
    FVector4 UV = GetBlockUV(Type);
    
    // Масштабируем UV
    UVs.Append({
        {UV.X, UV.Y},
        {UV.X, UV.Y + UV.W * h},
        {UV.X + UV.Z * w, UV.Y + UV.W * h},
        {UV.X + UV.Z * w, UV.Y}
    });
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
        Normal = FVector(0, 1, 0); // Нормаль в сторону +Y
    }
    else
    {
        // Y- (грань смотрит в сторону -Y)
        yCoord = y * BLOCK_SIZE;
        Normal = FVector(0, -1, 0); // Нормаль в сторону -Y
    }
    
    // Базовые координаты
    float baseX = x * BLOCK_SIZE;
    float baseZ = z * BLOCK_SIZE;
    
    // Размеры квада
    float quadWidth = w * BLOCK_SIZE;  // по X
    float quadHeight = h * BLOCK_SIZE; // по Z
    
    int start = Vertices.Num();

    // Ключевое исправление: правильный порядок вершин
    // Чтобы текстуры не смотрели внутрь, нужно обеспечить
    // правильный порядок обхода вершин
    
    if (bPositive)
    {
        // Y+: грань смотрит в сторону +Y
        // Порядок вершин должен быть ПО ЧАСОВОЙ СТРЕЛКЕ если смотреть изнутри блока
        // или ПРОТИВ ЧАСОВОЙ если смотреть снаружи
        
        // Правильный порядок для +Y (снаружи):
        Vertices.Add(FVector(baseX, yCoord, baseZ));                     // 0: левый-нижний
        Vertices.Add(FVector(baseX + quadWidth, yCoord, baseZ));         // 1: правый-нижний
        Vertices.Add(FVector(baseX + quadWidth, yCoord, baseZ + quadHeight)); // 2: правый-верхний
        Vertices.Add(FVector(baseX, yCoord, baseZ + quadHeight));        // 3: левый-верхний
        
        // Треугольники: 0-1-2 и 0-2-3
        Triangles.Append({ start, start+1, start+2, start, start+2, start+3 });
    }
    else
    {
        // Y-: грань смотрит в сторону -Y
        // Порядок должен быть обратным
        
        Vertices.Add(FVector(baseX, yCoord, baseZ));                     // 0: левый-нижний
        Vertices.Add(FVector(baseX, yCoord, baseZ + quadHeight));        // 1: левый-верхний
        Vertices.Add(FVector(baseX + quadWidth, yCoord, baseZ + quadHeight)); // 2: правый-верхний
        Vertices.Add(FVector(baseX + quadWidth, yCoord, baseZ));         // 3: правый-нижний
        
        Triangles.Append({ start, start+1, start+2, start, start+2, start+3 });
    }
    
    // Добавляем нормали (уже правильные)
    for (int i = 0; i < 4; i++)
    {
        Normals.Add(Normal);
    }
    
    // UV координаты
    FVector4 UV = GetBlockUV(Type);
    
    // Правильные UV для каждой грани
    if (bPositive)
    {
        // +Y
        UVs.Append({
            {UV.X, UV.Y},                           // 0: левый-нижний
            {UV.X + UV.Z * w, UV.Y},                // 1: правый-нижний
            {UV.X + UV.Z * w, UV.Y + UV.W * h},     // 2: правый-верхний
            {UV.X, UV.Y + UV.W * h}                 // 3: левый-верхний
        });
    }
    else
    {
        // -Y (возможно, нужно другую текстуру или зеркально)
        UVs.Append({
            {UV.X, UV.Y},                           // 0: левый-нижний
            {UV.X, UV.Y + UV.W * h},                // 1: левый-верхний
            {UV.X + UV.Z * w, UV.Y + UV.W * h},     // 2: правый-верхний
            {UV.X + UV.Z * w, UV.Y}                 // 3: правый-нижний
        });
    }
}

FVector UGreedyMeshing::CreateMesh(UMinecraftProceduralMeshComponent& procMesh,UMaterialInterface* Mat, const int64& Section)
{
	procMesh.CreateMeshSection(
	0,
	Vertices,
	Triangles,
	Normals,
	UVs,
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
	return procMesh.GetComponentLocation();
}

