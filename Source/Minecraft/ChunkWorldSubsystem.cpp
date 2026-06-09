// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkWorldSubsystem.h"

#include "ChunkGenerator.h"
#include "GreedyMeshing.h"
#include "BreakableCube.h"
#include "MinecraftProceduralMeshComponent.h"
#include "MinecrafteDataBaseSettings.h"

void UChunkWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
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
	GetWorld()->GetTimerManager().SetTimer(	ChunkUpdateTimer,this,&UChunkWorldSubsystem::Tick,0.2f,true);
}

void UChunkWorldSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ChunkUpdateTimer);
	}
	ChunkGenerator = nullptr;
	Super::Deinitialize();
}

void UChunkWorldSubsystem::SetChunkGenerator(AChunkGenerator* InChunkGenerator)
{
	ChunkGenerator = InChunkGenerator;
}

void UChunkWorldSubsystem::UpdateChunks(FVector coord)
{
	if (!ChunksForRemote.IsEmpty())
	{
		int RemovedCount = 0;
		for (auto It = ChunksForRemote.CreateIterator(); It && RemovedCount < OperationPerTick; ++It, ++RemovedCount)
		{
			RemoveChunk(It.Key());
			It.RemoveCurrent();
		}
		return;
	}
	coord/=BLOCK_SIZE;
	FChunkCoord chunkCoord;
	chunkCoord.x = FMath::FloorToInt(coord.X/CHUNK_X);
	chunkCoord.y = FMath::FloorToInt(coord.Y/CHUNK_Y);
	UE_LOG(LogTemp, Warning, TEXT("chunkCoord x %d y %d"),chunkCoord.x,chunkCoord.y);
	
	if (FMath::Abs(currentChunkPosition.x - chunkCoord.x) > chunkDeep/2
	 || FMath::Abs(currentChunkPosition.y - chunkCoord.y) > chunkDeep/2
											|| currentChunkPosition.startPos)
	{
		currentChunkPosition.startPos=false;
		currentChunkPosition.x = FMath::FloorToInt((float)chunkCoord.x / chunkDeep) * chunkDeep;
		currentChunkPosition.y = FMath::FloorToInt((float)chunkCoord.y / chunkDeep) * chunkDeep;
		UE_LOG(LogTemp, Warning, TEXT("currentChunkPosition x %d y %d"),currentChunkPosition.x,currentChunkPosition.y);
		for (auto& ChunkPair : Chunks)
		{
			if (!ChunksForRemote.Contains(ChunkPair.Key))
			{
				ChunksForRemote.Add(ChunkPair.Key, ChunkPair.Value);
			}
		}
		Chunks.Empty();				
		
		for (int x  = chunkCoord.x-chunkDeep; x < chunkCoord.x+chunkDeep; ++x)
		{
			for (int y = chunkCoord.y-chunkDeep; y < chunkCoord.y+chunkDeep; ++y)
			{
				FChunkCoord newCoord{x,y};
				int weight = FMath::Max(FMath::Abs(chunkCoord.x-x),FMath::Abs(chunkCoord.y-y));
				if (!ChunksForRemote.Contains(newCoord))
				{
					CoordsToGenerate.Add(weight,newCoord);				
					Chunks.Add(newCoord,nullptr);
				}
				else
				{
					Chunks.Add(newCoord,ChunksForRemote[newCoord]);
					ChunksForRemote.Remove(newCoord);
				}				
			}
		}	
	}
}

void UChunkWorldSubsystem::RemoveChunk(FChunkCoord coord)
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

void UChunkWorldSubsystem::FinalizeChunk(FChunkBuildData& Data, FGreedyMeshing& GreedyMeshing)
{
	UMinecraftProceduralMeshComponent* ProcMesh;
	if (FreeProcMeshes.IsEmpty())
	{
		ProcMesh = NewObject<UMinecraftProceduralMeshComponent>(ChunksContainer);
		ProcMesh->RegisterComponent();
		ProcMesh->AttachToComponent(ChunksContainer->GetRootComponent(),FAttachmentTransformRules::KeepRelativeTransform);
		ProcMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		ProcMesh->bUseAsyncCooking = true;
	}
	else
	{
		ProcMesh = FreeProcMeshes.Pop();		
	}
	ProcMesh->SetRelativeLocation(FVector(Data.Coord.x*CHUNK_X*BLOCK_SIZE, Data.Coord.y*CHUNK_Y*BLOCK_SIZE, 0));
	GreedyMeshing.CreateMesh(*ProcMesh,BlocksMatertial);
	MeshesMap.Add(Data.Coord,ProcMesh);
	if (Chunks.Contains(Data.Coord))
	{
		MeshToChunkMap.Add(ProcMesh, Chunks[Data.Coord].Get());
	}
}

void UChunkWorldSubsystem::AsyncChunkCreate(const TArray<FChunkCoord>& GenerateArray)
{
	if (GenerateArray.IsEmpty() || !ChunkGenerator)
	{
		return;
	}
	
	TArray<FAsyncGenerationResult> Results;
	Results.SetNum(GenerateArray.Num());
	
	TWeakObjectPtr<UChunkWorldSubsystem> WeakSubsystem = this;
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
			Data->Coord = CurrentCoord;
			
			WeakGenerator->GenerateChunkData(*Data);
			
			auto Mesher = MakeShared<FGreedyMeshing>();
			Mesher->BuildGreedyMesh(Data.Get());
			(*ResultsPtr)[i] = {CurrentCoord,Data,Mesher};
		});
		AsyncTask(ENamedThreads::GameThread, [WeakSubsystem, ResultsPtr]() {
		if (WeakSubsystem.IsValid()) {
			for (const auto& Res : *ResultsPtr) {
				if (Res.BuildData.IsValid()) {
					if (WeakSubsystem->Chunks.Contains(Res.Coord))
					{
						WeakSubsystem->Chunks[Res.Coord] = Res.BuildData;
						WeakSubsystem->FinalizeChunk(*Res.BuildData, *Res.GreedyMeshing);
					}					
				}
			}
		}
		delete ResultsPtr;
		});
	});
}

void UChunkWorldSubsystem::Tick()
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

int UChunkWorldSubsystem::GetSurfaceHighInPos(FVector vec)
{
	int XChunkCoord = FMath::FloorToInt(vec.X / BLOCK_SIZE);
	int YChunkCoord = FMath::FloorToInt(vec.Y / BLOCK_SIZE);
	int XChunk = XChunkCoord/CHUNKSIZE_WIDE;
	int YChunk = YChunkCoord/CHUNKSIZE_WIDE;
	FChunkCoord Coord{XChunk,YChunk};
	if (!Chunks.Contains(Coord))
		return 0;
	auto chunk = Chunks[Coord];
	if (!chunk.IsValid())
		return 0;
	return chunk->GetSurfaceHeight(XChunkCoord,YChunkCoord);
}

void UChunkWorldSubsystem::RemoveBlock(FHitResult Hit, UMinecraftProceduralMeshComponent* mesh)
{
	FVector CorrectWorldPos =Hit.ImpactPoint - Hit.ImpactNormal * EPS;
	FVector LocalPos =mesh->GetComponentTransform().InverseTransformPosition(CorrectWorldPos);
	int X = FMath::FloorToInt(LocalPos.X / BLOCK_SIZE);
	int Y = FMath::FloorToInt(LocalPos.Y / BLOCK_SIZE);
	int Z = FMath::FloorToInt(LocalPos.Z / BLOCK_SIZE);
	LocalPos/=BLOCK_SIZE;
	
	FChunkCoord chunkCoord;
	chunkCoord.x = FMath::FloorToInt(LocalPos.X/CHUNK_X);
	chunkCoord.y = FMath::FloorToInt(LocalPos.Y/CHUNK_Y);
	if (!MeshToChunkMap.Contains(mesh))
	{
		UE_LOG(LogTemp, Error, TEXT("Mesh not found in MeshToChunkMap"));
		return;
	}
	auto Chunk = MeshToChunkMap[mesh];

	BlockType CurrentBlockType = Chunk->GetBlock(X,Y,Z);
	Chunk->SetBlock(X,Y,Z,BlockType::Air);
	mesh->ClearAllMeshSections();
	mesh->bUseAsyncCooking = true; 
	FGreedyMeshing GreedyMeshing;	
	GreedyMeshing.BuildGreedyMesh(Chunk);
	GreedyMeshing.CreateMesh(*mesh,BlocksMatertial);
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
