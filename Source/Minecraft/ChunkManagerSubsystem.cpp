// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkManagerSubsystem.h"

#include "ChunkGenerator.h"
#include "GreedyMeshing.h"
#include "BreakableCube.h"
#include "MinecraftProceduralMeshComponent.h"
#include "MinecrafteDataBaseSettings.h"

void UChunkManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	const UMinecraftDataBaseSettings* Settings = GetDefault<UMinecraftDataBaseSettings>();
	if (Settings)
	{
		BlocksMatertial = Cast<UMaterialInterface>(Settings->BlocksMaterial.TryLoad());
		if (!BlocksMatertial)
			UE_LOG(LogTemp, Warning, TEXT("BlocksMatertial not found, set it in the Project Settings"));
	}
	FActorSpawnParameters params;
	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;
	ChunksContainer = GetWorld()->SpawnActor(AActor::StaticClass(), &SpawnLocation, &SpawnRotation, params);
	if (!ChunksContainer)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn ChunksContainer"));
		return;
	}
	USceneComponent* RootComponent = NewObject<USceneComponent>(ChunksContainer, TEXT("Root"));
	RootComponent->RegisterComponent();
	ChunksContainer->SetRootComponent(RootComponent);
	GetWorld()->GetTimerManager().SetTimer(	ChunkUpdateTimer,this,&UChunkManagerSubsystem::Tick,0.2f,true);
}

void UChunkManagerSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ChunkUpdateTimer);
	}
	ChunkGenerator = nullptr;
	Super::Deinitialize();
}

FChunkCoord UChunkManagerSubsystem::GetChunkCoordFromVectorPos(FVector pos)
{
	FChunkCoord chunkCoord;
	chunkCoord.x = FMath::FloorToInt(pos.X/CHUNK_X_SIZE);
	chunkCoord.y = FMath::FloorToInt(pos.Y/CHUNK_Y_SIZE);
	return chunkCoord;
}

void UChunkManagerSubsystem::SetChunkGenerator(AChunkGenerator* InChunkGenerator)
{
	ChunkGenerator = InChunkGenerator;
}

void UChunkManagerSubsystem::UpdateChunks(FVector coord)
{
	coord/=BLOCK_SIZE;
	FChunkCoord chunkCoord = GetChunkCoordFromVectorPos(coord);
	if (!bHasCurrentChunkPosition ||
		FMath::Abs(currentChunkPosition.x - chunkCoord.x) >= chunkGenerationBorder ||
		FMath::Abs(currentChunkPosition.y - chunkCoord.y) >= chunkGenerationBorder )
	{ //we are in new square
		currentChunkPosition.x = FMath::FloorToInt((float)chunkCoord.x / chunkDeep) * chunkDeep;
		currentChunkPosition.y = FMath::FloorToInt((float)chunkCoord.y / chunkDeep) * chunkDeep;
		UE_LOG(LogTemp, Warning, TEXT("currentChunkPosition x %d y %d"),currentChunkPosition.x,currentChunkPosition.y);
		currentChunkPosition=chunkCoord;
		bHasCurrentChunkPosition = true;
		//new coordinate to generate
		for (int x  = chunkCoord.x-chunkDeep; x < chunkCoord.x+chunkDeep; ++x)
		{
			for (int y = chunkCoord.y-chunkDeep; y < chunkCoord.y+chunkDeep; ++y)
			{
				FChunkCoord newCoord{x,y};
				ChunksDesiredCoord.Add(newCoord);
			}
		}
		//coordinate to remove
		for (auto ChunkCurrent : ChunksCurrentCoord)
		{
			if (!ChunksDesiredCoord.Contains(ChunkCurrent))
			{
				ChunksForRemoveCoord.Add(ChunkCurrent);
			}
		}
		//weight for generation
		for (const auto Chunk : ChunksDesiredCoord)
		{
			if (!ChunksCurrentCoord.Contains(Chunk))
			{
				int Weight = FMath::Max(FMath::Abs(chunkCoord.x-Chunk.x),FMath::Abs(chunkCoord.y-Chunk.y));
				CoordsToGenerate.Add(Weight,Chunk);
				ChunksBuildData.Add(Chunk,nullptr);
			}
		}
		ChunksCurrentCoord=ChunksDesiredCoord;
		for (auto& Chunk : ChunksForRemoveCoord)
		{
			RemoveChunk(Chunk);
		}
		ChunksDesiredCoord.Empty();
		ChunksForRemoveCoord.Empty();
	}
}

void UChunkManagerSubsystem::RemoveChunk(FChunkCoord coord)
{
	if (MeshesMap.Contains(coord))
	{
		UMinecraftProceduralMeshComponent* Mesh = MeshesMap[coord];
		Mesh->ClearAllMeshSections();
		MeshesMap.Remove(coord);
		FreeProcMeshes.Add(Mesh);
		MeshToChunkMap.Remove(Mesh);
	}
}

void UChunkManagerSubsystem::FinalizeChunk(FChunkBuildData& Data, FGreedyMeshing& GreedyMeshing)
{
	//proc mesh gets from the pool or creates a new one
	UMinecraftProceduralMeshComponent* ProcMesh;
	if (FreeProcMeshes.IsEmpty())
	{
		ProcMesh = NewObject<UMinecraftProceduralMeshComponent>(ChunksContainer);
		ProcMesh->RegisterComponent();
		ProcMesh->AttachToComponent(ChunksContainer->GetRootComponent(),FAttachmentTransformRules::KeepRelativeTransform);
		ProcMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		ProcMesh->bUseAsyncCooking = true; //TODO for far chunk don't do collision
	}
	else
	{
		ProcMesh = FreeProcMeshes.Pop();
	}
	ProcMesh->SetRelativeLocation(FVector(Data.ChunkCoord.x*CHUNK_X_SIZE*BLOCK_SIZE, Data.ChunkCoord.y*CHUNK_Y_SIZE*BLOCK_SIZE, 0));

	//send proc mesh to greedy mesh for assembly chunk
	GreedyMeshing.CreateMesh(*ProcMesh,BlocksMatertial);
	MeshesMap.Add(Data.ChunkCoord,ProcMesh);
	if (ChunksBuildData.Contains(Data.ChunkCoord))
	{
		MeshToChunkMap.Add(ProcMesh, ChunksBuildData[Data.ChunkCoord]);
	}
}

void UChunkManagerSubsystem::AsyncChunkCreate(const TArray<FChunkCoord>& GenerateArray)
{
	if (GenerateArray.IsEmpty() || !ChunkGenerator)
	{
		return;
	}

	TArray<FAsyncGenerationResult> Results;
	Results.SetNum(GenerateArray.Num());

	TWeakObjectPtr<UChunkManagerSubsystem> WeakSubsystem = this;
	TWeakObjectPtr<AChunkGenerator> WeakGenerator = ChunkGenerator;
	TArray<FAsyncGenerationResult>* ResultsPtr = new TArray<FAsyncGenerationResult>(Results);
	Async(EAsyncExecution::ThreadPool,[WeakSubsystem,WeakGenerator,GenerateArray,ResultsPtr]()
	{
		ParallelFor(GenerateArray.Num(),[&](int32 i)
		{
			if (!WeakGenerator.IsValid())
				return;
			FChunkCoord CurrentCoord = GenerateArray[i];

			TSharedPtr<FChunkBuildData> Data = MakeShared<FChunkBuildData>();
			Data->ChunkCoord = CurrentCoord;

			WeakGenerator->GenerateChunkData(*Data);

			auto Mesher = MakeShared<FGreedyMeshing>();
			Mesher->BuildGreedyMesh(Data.Get());
			(*ResultsPtr)[i] = {CurrentCoord,Data,Mesher};
		});
		AsyncTask(ENamedThreads::GameThread, [WeakSubsystem, ResultsPtr]() {
		if (WeakSubsystem.IsValid()) {
			for (const auto& Res : *ResultsPtr) {
				if (Res.BuildData.IsValid()) {
					if (WeakSubsystem->ChunksBuildData.Contains(Res.Coord))
					{
						WeakSubsystem->ChunksBuildData[Res.Coord] = Res.BuildData;
						WeakSubsystem->FinalizeChunk(*Res.BuildData, *Res.GreedyMeshing);
					}
				}
			}
		}
		delete ResultsPtr;
		});
	});
}

void UChunkManagerSubsystem::Tick()
{
	if (CoordsToGenerate.IsEmpty() || !ChunkGenerator)
		return;
	TArray<FChunkCoord> GenerateArray;
	for (int i = 0; i < OperationPerTick; i++)
	{
		for (int j = 0; j < chunkDeep*2; ++j)
		{
			auto It = CoordsToGenerate.CreateKeyIterator(j);
			if(It)
			{
				FChunkCoord OutCoord = It.Value();
				It.RemoveCurrent();
				GenerateArray.Push(OutCoord);
				break;
			}

		}
	}
	AsyncChunkCreate(GenerateArray);
}

void UChunkManagerSubsystem::RemoveBlock(FHitResult Hit, UMinecraftProceduralMeshComponent* mesh)
{
	FVector CorrectWorldPos =Hit.ImpactPoint - Hit.ImpactNormal * EPS;
	FVector LocalPos =mesh->GetComponentTransform().InverseTransformPosition(CorrectWorldPos);
	int X = FMath::FloorToInt(LocalPos.X / BLOCK_SIZE);
	int Y = FMath::FloorToInt(LocalPos.Y / BLOCK_SIZE);
	int Z = FMath::FloorToInt(LocalPos.Z / BLOCK_SIZE);
	LocalPos/=BLOCK_SIZE;

	if (!MeshToChunkMap.Contains(mesh))
	{
		UE_LOG(LogTemp, Error, TEXT("Mesh not found in MeshToChunkMap"));
		return;
	}
	TSharedPtr<FChunkBuildData> Chunk = MeshToChunkMap[mesh];
	if (!Chunk.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Chunk data is invalid"));
		return;
	}

	BlockType CurrentBlockType = Chunk->GetBlock(X,Y,Z);
	Chunk->SetBlock(X,Y,Z,BlockType::Air);
	mesh->bUseAsyncCooking = true;

	TWeakObjectPtr<UMinecraftProceduralMeshComponent> WeakMesh = mesh;
	TWeakObjectPtr<UChunkManagerSubsystem> WeakSubsystem = this;
	Async(EAsyncExecution::ThreadPool,[WeakSubsystem, WeakMesh, Chunk]()
	{
		TSharedPtr<FGreedyMeshing> GreedyMeshing = MakeShared<FGreedyMeshing>();
		GreedyMeshing->BuildGreedyMesh(Chunk.Get());

		AsyncTask(ENamedThreads::GameThread, [WeakSubsystem, WeakMesh, GreedyMeshing]()
		{
			if (!WeakSubsystem.IsValid() || !WeakMesh.IsValid())
			{
				return;
			}

			GreedyMeshing->CreateMesh(*WeakMesh.Get(), WeakSubsystem->BlocksMatertial);
		});
	});

	FVector CubeLocation = FVector(mesh->GetComponentLocation().X+X*BLOCK_SIZE+BLOCK_SIZE/2,mesh->GetComponentLocation().Y+Y*BLOCK_SIZE+BLOCK_SIZE/2,mesh->GetComponentLocation().Z+Z*BLOCK_SIZE+BLOCK_SIZE/2);
	FActorSpawnParameters spawnParams;
	//TODO make a pool
	auto Actor = GetWorld()->SpawnActor<AActor>(DestroyedBlockClass,CubeLocation,FRotator::ZeroRotator,spawnParams);
	ABreakableCube* Cube = Cast<ABreakableCube>(Actor);
	if (Cube)
	{
		Cube->FractureNow(CurrentBlockType,Hit);
	}
}
