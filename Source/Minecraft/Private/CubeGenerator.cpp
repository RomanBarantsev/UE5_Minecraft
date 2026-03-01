// Fill out your copyright notice in the Description page of Project Settings.


#include "CubeGenerator.h"

#include "BreakableCube.h"
#include "GreedyMeshing.h"
#include "Minecraft/FastNoiseLite.h"
#include "Minecraft/FChunkBuildData.h"
#include "Minecraft/MinecraftProceduralMeshComponent.h"

class UProceduralMeshComponent;
// Sets default values

ACubeGenerator::ACubeGenerator()
{
	PrimaryActorTick.bCanEverTick = false;	
	// Создаём корневой компонент
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	
}

void ACubeGenerator::LoadLayers()
{
	CavesRoomParams.rowName="CavesRoom";
	CavesTunnelParams.rowName="CavesTunnel";
	
	SurfaceParams.rowName="Surface";
	ContParams.rowName="Continentalness";	
	PeakParams.rowName="Peak";
	
	BedrockParams.rowName="Bedrock";
	
	LoadNoiseParams(CavesRoomNoise,CavesRoomParams);
	LoadNoiseParams(CavesTunnelNoise,CavesTunnelParams);
	
	LoadNoiseParams(SurfaceNoise,SurfaceParams);
	LoadNoiseParams(ContNoise,ContParams);
	LoadNoiseParams(PeakNoise,PeakParams);
	
	LoadNoiseParams(BedrockNoise,BedrockParams);
	
	SetNoiseParams(CavesRoomNoise,CavesRoomParams, FastNoiseLite::NoiseType_Perlin);
	SetNoiseParams(CavesTunnelNoise,CavesTunnelParams, FastNoiseLite::NoiseType_Perlin);	
	
	SetNoiseParams(SurfaceNoise,SurfaceParams, FastNoiseLite::NoiseType_Perlin);	
	SetNoiseParams(ContNoise,ContParams, FastNoiseLite::NoiseType_Perlin);
	SetNoiseParams(PeakNoise,PeakParams, FastNoiseLite::NoiseType_Perlin);
	
	SetNoiseParams(BedrockNoise,BedrockParams, FastNoiseLite::NoiseType_Perlin);	
}

// Called when the game starts or when spawned
void ACubeGenerator::BeginPlay()
{
	Super::BeginPlay();
	LoadLayers();
	//time start
	UpdateChunks(FVector(0.0f,0.0f,0.0f));//start pos
	//time end
}


void ACubeGenerator::SetNoiseParams(FastNoiseLite& Noise,FNoisesParams params,FastNoiseLite::NoiseType noiseType)
{
	Noise.SetSeed(Seed);           // Seed для разнообразия
	Noise.SetFrequency(params.Scale);   // Масштаб шума (аналог Scale)
	Noise.SetFractalOctaves(params.Octaves);    // 4 октавы для детализации
	Noise.SetFractalGain(params.Persistence);   // Затухание амплитуды (Persistence)
	Noise.SetFractalLacunarity(params.Lacunarity); // Рост частоты (Lacunarity)
	Noise.SetNoiseType(noiseType); // Тип шума — Perlin
	Noise.SetFractalType(FastNoiseLite::FractalType_FBm);
}

float ACubeGenerator::GetHeightMask(int z, int minZ, int maxZ)
{
	if (z <= minZ || z >= maxZ) return 0.0f;

	float t = float(z - minZ) / float(maxZ - minZ);
	return 1.0f - t * t; // плавно затухает к поверхности
}

int ACubeGenerator::mapHeight(int x, int y)
{
	float cont = ContNoise.GetNoise((float)x, (float)y);     // [-1..1]
	float surf = SurfaceNoise.GetNoise((float)x, (float)y);
	float peaks = PeakNoise.GetNoise((float)x, (float)y);

	cont = (cont + 1) * 0.5f;
	surf = (surf + 1) * 0.5f;
	peaks = fabs(peaks);

	float baseHeight = FMath::Lerp(50.f, 85.f, cont);

	float mountain = pow(peaks, 1.6f) * 45.f;

	float detail = (surf - 0.5f) * 10.f;

	float height = baseHeight + mountain + detail;

	return FMath::Clamp((int)height, 1, CHUNK_Z - 2);
}


void ACubeGenerator::LoadNoiseParams(FastNoiseLite& noise, FNoisesParams& params)
{
	auto Row = PerlinNoiseTable->FindRow<FPerlinNoiseBiom>(params.rowName,"name");	
	if (Row)
	{
		params.Scale = Row->Scale;
		params.Octaves = Row->Octaves;
		params.Persistence = Row->Persistence;
		params.Lacunarity = Row->Lacunarity;
	}
}


void ACubeGenerator::UpdateChunks(FVector coord)
{	
	double TStart = FPlatformTime::Seconds();
	coord/=BLOCK_SIZE;
	FChunkCoord chunkCoord;
	chunkCoord.x = FMath::FloorToInt(coord.X/CHUNK_X);
	chunkCoord.y = FMath::FloorToInt(coord.Y/CHUNK_Y);
	UE_LOG(LogTemp, Warning, TEXT("chunkCoord x %d y %d"),chunkCoord.x,chunkCoord.y);
	if (FMath::Abs(currentChunkPosition.x - chunkCoord.x) > chunkDelimiter
	 || FMath::Abs(currentChunkPosition.y - chunkCoord.y) > chunkDelimiter
											|| currentChunkPosition.startPos)
	{
		currentChunkPosition.startPos=false;
		currentChunkPosition.x = FMath::FloorToInt((float)chunkCoord.x / chunkDelimiter) * chunkDelimiter;
		currentChunkPosition.y = FMath::FloorToInt((float)chunkCoord.y / chunkDelimiter) * chunkDelimiter;
		UE_LOG(LogTemp, Warning, TEXT("currentChunkPosition x %d y %d"),currentChunkPosition.x,currentChunkPosition.y);	
		
		for (auto& Pair : Chunks)
		{
			ChunkRemove(Pair.Key);
		}
		
		for (int x  = chunkCoord.x-chunkDelimiter*2; x < chunkCoord.x+chunkDelimiter*2; ++x)
		{
			for (int y = chunkCoord.y-chunkDelimiter*2; y < chunkCoord.y+chunkDelimiter*2; ++y)
			{				
				ChunkToProcMesh(FChunkCoord{x,y});
			}
		}
		//ChunkToProcMesh(FChunkCoord{0,0});
		Chunks = MoveTemp(NewChunks);
		NewChunks.Empty();
		for (auto chunk : Chunks)
		{
			ChunkGenerationQueue.Enqueue(chunk.Key);
		}
		
		UE_LOG(LogTemp,Warning, TEXT("FreeMeshes %d"),FreeMeshes.Num());
		double T1 = FPlatformTime::Seconds();
		UE_LOG(LogTemp, Warning,
			TEXT("Chunks Init: %.2f ms"),
			(T1-TStart)*1000
		);
	}
	UE_LOG(LogTemp, Log, TEXT("%d, %d"),chunkCoord.x,chunkCoord.y);		
}

int ACubeGenerator::GetSurfaceHigh(FVector vec)
{
	int XChunkCoord = FMath::FloorToInt(vec.X / BLOCK_SIZE);
	int YChunkCoord = FMath::FloorToInt(vec.Y / BLOCK_SIZE);
	int XChunk = XChunkCoord/CHUNKSIZE_WIDE;
	int YChunk = YChunkCoord/CHUNKSIZE_WIDE;
	FChunkCoord Coord{XChunk,YChunk};
	if (!Chunks.Contains(Coord))
		return 0;
	FChunkBuildData* chunk = Chunks[Coord];
	if (chunk==nullptr)
		return 0;
	return chunk->GetSurfaceHeight(XChunkCoord,YChunkCoord);
}

void ACubeGenerator::ChunkRemove(FChunkCoord coord)
{
	UMinecraftProceduralMeshComponent* Mesh = MeshesMap[coord];
	Mesh->ClearAllMeshSections();
	ChunkMap.Remove(coord);
	MeshesMap.Remove(coord);
	FreeMeshes.Add(Mesh);
	MeshToChunkMap.Remove(Mesh);
}

void ACubeGenerator::ChunkToProcMesh(const FChunkCoord& coord)
{
	TWeakObjectPtr<ACubeGenerator> WeakThis = this;
	Async(EAsyncExecution::ThreadPool, [WeakThis,coord]()
	{
		if (!WeakThis.IsValid())
			return;
		
		UE_LOG(LogTemp, Warning, TEXT("NewChunk x %d y %d"),coord.x,coord.y);
		FChunkBuildData BuildData;
		BuildData.Coord = coord;
		
		WeakThis->GenerateChunkData(BuildData);
		AsyncTask(ENamedThreads::GameThread, [WeakThis, BuildData = MoveTemp(BuildData)]() mutable  
		{
			if (!WeakThis.IsValid())
				return;			
			WeakThis->Chunks.Add(BuildData.Coord, &BuildData);
			WeakThis->FinalizeChunk(BuildData);
		});	
	});
}

void ACubeGenerator::GenerateChunkData(FChunkBuildData& Data)
{
	for (int x = 0; x < CHUNK_X; x++)
	{
		for (int y = 0; y < CHUNK_Y; y++)
		{			
			int worldX = Data.Coord.x*CHUNK_X+x;
			int worldY = Data.Coord.y*CHUNK_Y+y;
			int height = mapHeight(worldX, worldY);
			Data.SetSurfaceHeight(x,y,height);
		}
	}
	Data.Fill();
	//GenerateBlocksAndCaves(Data);
}

void ACubeGenerator::GenerateBlocksAndCaves(FChunkBuildData& Data)
{
	float  delimiter=0.01;
	for (int xPerlin = Data.Coord.x*CHUNK_X, x =0; xPerlin <Data.Coord.x*CHUNK_X+CHUNK_X; xPerlin++,x++)
	{
		float fx = (float)xPerlin;
		for (int yPerlin = Data.Coord.y*CHUNK_X, y=0; yPerlin <Data.Coord.y*CHUNK_X+CHUNK_X; yPerlin++,y++)
		{
			int index2D = x + y * CHUNK_X;
			float fy = (float)xPerlin;
			float bedrockNoise = BedrockNoise.GetNoise(fx,fy);
			int bedrockTop = BEDROCK_BASE + (int)((bedrockNoise + 1.0f) * 0.5f * BEDROCK_HEIGHT);
			int height = Data.GetSurfaceHeight(x,y);
			for (int z = 0; z < height; ++z)
			{	
				int index = x + y * CHUNK_X + z * CHUNK_X * CHUNK_Y;
				if (z<bedrockTop || z==0)
				{				
					Data.SetBlock(x,y,z,BlockType::Cobblestone);
					continue;
				}
				float room = CavesRoomNoise.GetNoise(fx, fy, (float)z);
				float tunnel  = CavesTunnelNoise.GetNoise(fx, fy, (float)z);
				float mask = GetHeightMask(z, 1, height - 6);
				float density =
					tunnel * 1.2f +     // тоннели важнее
					room * 0.8f;        // залы реже
				density *= mask;
				if (density > 0.25f)
				{
					Data.SetBlock(x,y,z,BlockType::Air);
				}
			}
		}
	}
}

void ACubeGenerator::FinalizeChunk(const FChunkBuildData& Data)
{
	UMinecraftProceduralMeshComponent* ProcMesh;
	if (FreeMeshes.IsEmpty())
	{
		ProcMesh = NewObject<UMinecraftProceduralMeshComponent>(this);
		ProcMesh->RegisterComponent();
		ProcMesh->AttachToComponent(RootComponent,FAttachmentTransformRules::KeepRelativeTransform);
		ProcMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	else
	{
		ProcMesh = FreeMeshes.Pop();		
	}
	ProcMesh->SetRelativeLocation(FVector(Data.Coord.x*CHUNK_X*BLOCK_SIZE, Data.Coord.y*CHUNK_X*BLOCK_SIZE, 0));
	UGreedyMeshing* GreedyMeshing = NewObject<UGreedyMeshing>(this);
	GreedyMeshing->BuildGreedyMesh(&Data);
	GreedyMeshing->CreateMesh(*ProcMesh,Mat);
}

void ACubeGenerator::RemoveBlock(FHitResult Hit, UMinecraftProceduralMeshComponent* mesh)
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
	FChunkBuildData* Chunk = MeshToChunkMap[mesh];
	
	BlockType CurrentBlockType = Chunk->GetBlock(X,Y,Z);
	Chunk->SetBlock(X,Y,Z,BlockType::Air);
	mesh->ClearAllMeshSections();
	UGreedyMeshing* GM = NewObject<UGreedyMeshing>();
	GM->BuildGreedyMesh(Chunk);
	GM->CreateMesh(*mesh,Mat);	
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

