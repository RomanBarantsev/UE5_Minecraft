// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FChunkBuildData.h"
#include "Subsystems/WorldSubsystem.h"
#include "ChunkManagerSubsystem.generated.h"

class UMinecraftProceduralMeshComponent;
class FGreedyMeshing;
class AChunkGenerator;
/**
 * 
 */
UCLASS()
class MINECRAFT_API UChunkManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
	struct FAsyncGenerationResult {
		FChunkCoord Coord;
		TSharedPtr<FChunkBuildData> BuildData;
		TSharedPtr<FGreedyMeshing> GreedyMeshing;
	};	
	UPROPERTY()
	AActor* ChunksContainer;
	UPROPERTY()
	AChunkGenerator* ChunkGenerator = nullptr;
	const int chunkDeep=8;
	size_t OperationPerTick=3;
	float EPS = 0.1f;	
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;	
	void RemoveChunk(FChunkCoord coord);
	void FinalizeChunk(FChunkBuildData& Data, FGreedyMeshing& GreedyMeshing);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Material", meta=(AllowPrivateAccess="true"))
	UMaterialInterface* BlocksMatertial; // TODO get from Menu class
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> DestroyedBlockClass;
	
	void AsyncChunkCreate(const TArray<FChunkCoord>& GenerateArray);	
	
	FTimerHandle ChunkUpdateTimer;
	void Tick();
	
	TMap<FChunkCoord,TSharedPtr<FChunkBuildData>> Chunks;
	TMap<FChunkCoord,TSharedPtr<FChunkBuildData>> ChunksForRemote;
	UPROPERTY()
	TArray<UMinecraftProceduralMeshComponent*> FreeProcMeshes;
	TMap<UMinecraftProceduralMeshComponent*,FChunkBuildData*> MeshToChunkMap;
	TMultiMap<int32,FChunkCoord> CoordsToGenerate;
	
	FChunkCoord currentChunkPosition;	
	
	TMap<FChunkCoord,UMinecraftProceduralMeshComponent*>MeshesMap;	
public:	
	void SetChunkGenerator(AChunkGenerator* InChunkGenerator);
	void UpdateChunks(FVector coord);
	void RemoveBlock(FHitResult Hit, UMinecraftProceduralMeshComponent* mesh);
};
