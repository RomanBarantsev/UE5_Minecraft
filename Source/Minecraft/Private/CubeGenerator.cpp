// Fill out your copyright notice in the Description page of Project Settings.


#include "CubeGenerator.h"

#include "BreakableCube.h"
#include "GreedyMeshing.h"
#include "GeometryCollection/GeometryCollectionActor.h"
#include "Minecraft/FastNoiseLite.h"
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
	ChunksInit();
	//time end
}

void ACubeGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
}

// Called every frame
void ACubeGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	
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

void ACubeGenerator::CavesCreate(UChunk* chunk,int xChunk, int yChunk) //TODO объеденить с Fill
{	
	
	float  delimiter=0.01;
	for (int xPerlin = xChunk*CHUNK_X, x =0; xPerlin <xChunk*CHUNK_X+CHUNK_X; xPerlin++,x++)
	{
		float fx = (float)xPerlin;
		for (int yPerlin = yChunk*CHUNK_X, y=0; yPerlin <yChunk*CHUNK_X+CHUNK_X; yPerlin++,y++)
		{
			float fy = (float)xPerlin;
			float bedrockNoise = BedrockNoise.GetNoise(fx,fy);
			int bedrockTop = BEDROCK_BASE + (int)((bedrockNoise + 1.0f) * 0.5f * BEDROCK_HEIGHT);
			int height = chunk->GetSurfaceHeight(x,y);
			for (int z = 0; z < height; ++z)
			{	
				if (z<bedrockTop || z==0)
				{				
					chunk->SetBlock(x, y, z, BlockType::Cobblestone);
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
					chunk->SetBlock(x, y, z, BlockType::Air);
				}
			}
		}
	}
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

int  ACubeGenerator::NormalizeNoise(float noise_value,int z,int z_min,int z_max,float threshold)
{
	float denom = static_cast<float>(z_max - z_min);
	float y_norm = denom <= 0.0001f ? 0.0f : (z - z_min) / denom;
	// base density: чем глубже — тем больше плотность
	float base = (static_cast<float>(z_max) - static_cast<float>(z)) / static_cast<float>(z_max); // 0..1
	// convert noise to 0..1
	float n = (noise_value + 1.0f) * 0.5f;
	// keep vertical influence but milder
	float vertical_weight = 1.0f - fabs( (y_norm - 0.5f) * 2.0f ); // 1 in middle, 0 on edges
	float cave_strength = 0.6f; // настроить
	float density = base + (n - 0.5f) * cave_strength * vertical_weight;
	return (density > threshold) ? 1 : 0;
}

void ACubeGenerator::ChunkMeshesGenerator()
{
	FChunkCoord Coord;
	if (ChunkGenerationQueue.Dequeue(Coord))
	{
		ChunkToProcMesh(Coord);
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
				NewChunk(x,y);
				UE_LOG(LogTemp, Warning, TEXT("NewChunk x %d y %d"),x,y);
			}
		}
		
		Chunks = MoveTemp(NewChunks);
		NewChunks.Empty();
		for (auto chunk : Chunks)
		{
			ChunkGenerationQueue.Enqueue(chunk.Key);
		}
		GetWorld()->GetTimerManager().SetTimer(ChunkMeshesGenerationTimerHandle,this,&ThisClass::ChunkMeshesGenerator,0.01,true,false);
		
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
	UChunk* chunk = Chunks[Coord];
	if (chunk==nullptr)
		return 0;
	return chunk->GetSurfaceHeight(XChunkCoord,YChunkCoord);
}

void ACubeGenerator::ChunksInit()
{
	UpdateChunks(FVector(0.0f,0.0f,0.0f));//start pos
}

void ACubeGenerator::NewChunk(int xChunk, int yChunk)
{	
	FChunkCoord coord{xChunk, yChunk};
	if (Chunks.Contains(coord))
	{
		NewChunks.Add(coord, Chunks[coord]);
	}
	else
	{
		NewChunks.Add(coord, nullptr);
	}
}

void ACubeGenerator::ChunkRemove(FChunkCoord coord)
{
	UChunk* chunk = Chunks[coord];
	UMinecraftProceduralMeshComponent* Mesh = MeshesMap[coord];
	Mesh->ClearAllMeshSections();
	ChunkMap.Remove(coord);
	MeshesMap.Remove(coord);
	FreeMeshes.Add(Mesh);
	MeshToChunkMap.Remove(Mesh);
}

void ACubeGenerator::ChunkToProcMesh(const FChunkCoord& coord)
{
	UChunk* NewChunk = NewObject<UChunk>(this);
	ChunkMap.Add(coord,NewChunk);
	static float PerlinDelimiter = 0.1;
	for (int xPerlin = coord.x*CHUNK_X, x =0; x<CHUNK_X; xPerlin++,x++) //
	{
		for (int yPerlin = coord.y*CHUNK_Y, y=0; y<CHUNK_Y; yPerlin++,y++)
		{
			NewChunk->SetSurfaceHeight(x,y,mapHeight(xPerlin,yPerlin));
		}
	}
	if (!Chunks.Contains(coord))
		return;
	Chunks[coord]=NewChunk;
	NewChunk->Fill();	
	CavesCreate(NewChunk,coord.x,coord.y);
	
	UMinecraftProceduralMeshComponent* ProcMesh;
	if (!FreeMeshes.IsEmpty())
	{
		ProcMesh =FreeMeshes.Last();
		FreeMeshes.Pop();
	}
	else
	{
		ProcMesh = NewObject<UMinecraftProceduralMeshComponent>(this);
		ProcMesh->RegisterComponent();
		ProcMesh->AttachToComponent(RootComponent,FAttachmentTransformRules::KeepRelativeTransform);
		ProcMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	MeshToChunkMap.Add(ProcMesh,NewChunk);
	
	ProcMesh->SetRelativeLocation(FVector(coord.x*CHUNK_X*BLOCK_SIZE, coord.y*CHUNK_X*BLOCK_SIZE, 0));
	MeshesMap.Add(coord,ProcMesh);
	UGreedyMeshing* GM = NewObject<UGreedyMeshing>();
	
	TWeakObjectPtr<UChunk> WeakChunk = NewChunk;
	TWeakObjectPtr<UMinecraftProceduralMeshComponent> WeakProcMesh = ProcMesh;
	TWeakObjectPtr<ACubeGenerator> WeakThis = this;
	Async(EAsyncExecution::ThreadPool, [WeakChunk, WeakProcMesh, WeakThis, GM]()
		{
			if (!WeakChunk.IsValid())
				return;
			
			GM->BuildGreedyMesh(WeakChunk.Get());
			AsyncTask(ENamedThreads::GameThread, [WeakChunk, WeakProcMesh, WeakThis, GM]()
			{
				if (!WeakProcMesh.IsValid() || !WeakChunk.IsValid())
					return;
				GM->CreateMesh(*WeakProcMesh,WeakThis->Mat);
			});
		});
	
	
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
	UChunk* Chunk = MeshToChunkMap[mesh];
	
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


void ACubeGenerator::Draw()
{
	LoadLayers();
	ChunksInit();
}
