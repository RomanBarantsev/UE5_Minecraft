#include "ChunkGenerator.h"
#include "PerlinNoise3D.h"
#include "Async/Async.h"

AChunkGenerator::AChunkGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
    SetRootComponent(ProcMesh);
}

void AChunkGenerator::BeginPlay()
{
    Super::BeginPlay();
    GenerateChunk();
}

void AChunkGenerator::GenerateChunk()
{
    // Асинхронно, чтобы не лагало
    AsyncGenerate();
}

void AChunkGenerator::AsyncGenerate()
{
    const int32 Xs = ChunkSizeX;
    const int32 Ys = ChunkSizeY;
    const int32 Zs = ChunkSizeZ;
    const int32 Total = Xs * Ys * Zs;
    const float Scale = NoiseScale;
    const int32 LocalSeed = Seed;

    Async(EAsyncExecution::ThreadPool, [this, Xs, Ys, Zs, Total, Scale, LocalSeed]()
    {
        TArray<FVoxel> Voxels;
        Voxels.SetNum(Total);

        // Шумовая генерация высоты
        for (int32 x = 0; x < Xs; x++)
        {
            for (int32 y = 0; y < Ys; y++)
            {
                float n = UPerlinNoise3D::Perlin3D(x, y, 0, Scale, 3, 0.5f, 2.0f, LocalSeed);
                n = (n + 1) * 0.5f; // 0..1
                int32 height = FMath::Clamp((int32)(n * (Zs - 1)), 0, Zs - 1);

                for (int32 z = 0; z < Zs; z++)
                {
                    EVoxelType type = EVoxelType::Air;
                    if (z < height - 3) type = EVoxelType::Stone;
                    else if (z < height - 1) type = EVoxelType::Dirt;
                    else if (z == height - 1) type = EVoxelType::Grass;

                    Voxels[Index3D(x, y, z)] = { type };
                }
            }
        }

        // После генерации данных вызываем сборку меша в геймпотоке
        AsyncTask(ENamedThreads::GameThread, [this, Voxels]() mutable
        {
            BuildGreedyMesh(Voxels);
        });
    });
}

void AChunkGenerator::BuildGreedyMesh(const TArray<FVoxel>& Voxels)
{
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;

    const float S = VoxelSize;
    auto IsFilled = [&](int32 x, int32 y, int32 z) -> bool
    {
        if (x < 0 || y < 0 || z < 0 || x >= ChunkSizeX || y >= ChunkSizeY || z >= ChunkSizeZ)
            return false;
        return Voxels[Index3D(x, y, z)].Type != EVoxelType::Air;
    };

    // Мини-гриди: не соединяет диагонали, но объединяет соседние квадраты по XY.
    for (int32 z = 0; z < ChunkSizeZ; ++z)
    {
        for (int32 x = 0; x < ChunkSizeX; ++x)
        {
            for (int32 y = 0; y < ChunkSizeY; ++y)
            {
                EVoxelType Type = Voxels[Index3D(x, y, z)].Type;
                if (Type == EVoxelType::Air) continue;

                FVector Base = FVector(x * S, y * S, z * S);

                auto AddFace = [&](const FVector& A, const FVector& B, const FVector& C, const FVector& D, const FVector& Normal, FVector2D UVBase)
                {
                    int32 start = Vertices.Num();
                    Vertices.Add(A); Vertices.Add(B); Vertices.Add(C); Vertices.Add(D);
                    Triangles.Append({ start, start + 1, start + 2, start, start + 2, start + 3 });
                    Normals.Append({ Normal, Normal, Normal, Normal });

                    float uvS = 1.0f / 4.0f; // 4x4 атлас
                    UVs.Add(UVBase);
                    UVs.Add(UVBase + FVector2D(uvS, 0));
                    UVs.Add(UVBase + FVector2D(uvS, uvS));
                    UVs.Add(UVBase + FVector2D(0, uvS));
                };

                auto GetUV = [&](EVoxelType T)
                {
                    switch (T)
                    {
                    case EVoxelType::Grass: return FVector2D(0.0f, 0.0f);
                    case EVoxelType::Dirt:  return FVector2D(0.25f, 0.0f);
                    case EVoxelType::Stone: return FVector2D(0.5f, 0.0f);
                    case EVoxelType::Sand:  return FVector2D(0.75f, 0.0f);
                    default: return FVector2D(0.0f, 0.0f);
                    }
                };

                FVector2D UVBase = GetUV(Type);

                // Добавляем только видимые грани
                if (!IsFilled(x + 1, y, z)) AddFace(Base + FVector(S, 0, 0), Base + FVector(S, S, 0), Base + FVector(S, S, S), Base + FVector(S, 0, S), FVector(1, 0, 0), UVBase);
                if (!IsFilled(x - 1, y, z)) AddFace(Base + FVector(0, 0, 0), Base + FVector(0, 0, S), Base + FVector(0, S, S), Base + FVector(0, S, 0), FVector(-1, 0, 0), UVBase);
                if (!IsFilled(x, y + 1, z)) AddFace(Base + FVector(0, S, 0), Base + FVector(S, S, 0), Base + FVector(S, S, S), Base + FVector(0, S, S), FVector(0, 1, 0), UVBase);
                if (!IsFilled(x, y - 1, z)) AddFace(Base + FVector(0, 0, 0), Base + FVector(0, 0, S), Base + FVector(S, 0, S), Base + FVector(S, 0, 0), FVector(0, -1, 0), UVBase);
                if (!IsFilled(x, y, z + 1)) AddFace(Base + FVector(0, 0, S), Base + FVector(S, 0, S), Base + FVector(S, S, S), Base + FVector(0, S, S), FVector(0, 0, 1), UVBase);
                if (!IsFilled(x, y, z - 1)) AddFace(Base + FVector(0, 0, 0), Base + FVector(0, S, 0), Base + FVector(S, S, 0), Base + FVector(S, 0, 0), FVector(0, 0, -1), UVBase);
            }
        }
    }

    ProcMesh->ClearAllMeshSections();
    ProcMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, {}, {}, true);
    if (BlockMaterial)
        ProcMesh->SetMaterial(0, BlockMaterial);
}
