#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ChunkGenerator.generated.h"

UENUM(BlueprintType)
enum class EVoxelType : uint8
{
    Air,
    Dirt,
    Grass,
    Stone,
    Sand
};

USTRUCT()
struct FVoxel
{
    GENERATED_BODY()
    EVoxelType Type = EVoxelType::Air;
};

UCLASS()
class MINECRAFT_API AChunkGenerator : public AActor
{
    GENERATED_BODY()

public:
    AChunkGenerator();

    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chunk")
    int32 ChunkSizeX = 16;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chunk")
    int32 ChunkSizeY = 16;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chunk")
    int32 ChunkSizeZ = 64;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chunk")
    float VoxelSize = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Noise")
    float NoiseScale = 0.03f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Noise")
    int32 Seed = 1337;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Material")
    UMaterialInterface* BlockMaterial;

    UPROPERTY(VisibleAnywhere, Category="Components")
    UProceduralMeshComponent* ProcMesh;

    UFUNCTION(CallInEditor, BlueprintCallable, Category="Chunk")
    void GenerateChunk();

protected:
    void AsyncGenerate();
    void BuildGreedyMesh(const TArray<FVoxel>& Voxels);

    FORCEINLINE int32 Index3D(int32 X, int32 Y, int32 Z) const { return (Z * ChunkSizeY * ChunkSizeX) + (Y * ChunkSizeX) + X; }
};
