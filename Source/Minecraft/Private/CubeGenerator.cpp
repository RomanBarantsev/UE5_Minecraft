// Fill out your copyright notice in the Description page of Project Settings.


#include "CubeGenerator.h"
#include "GreedyMeshing.h"
#include "Minecraft/FastNoiseLite.h"
#include "Minecraft/MinecraftProceduralMeshComponent.h"

class UProceduralMeshComponent;
// Sets default values

ACubeGenerator::ACubeGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
	static ConstructorHelpers::FObjectFinder<UDataTable> DataTableRef(TEXT("DataTable'/Game/PerlinNoiseDataTable.PerlinNoiseDataTable'"));
	if (DataTableRef.Succeeded())
	{
		PerlinNoiseTable = DataTableRef.Object;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable: /Game/PerlinNoiseDataTable.PerlinNoiseDataTable"));
	}	
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
		for (int yPerlin = yChunk*CHUNK_X, y=0; yPerlin <yChunk*CHUNK_X+CHUNK_X; yPerlin++,y++)
		{
			float bedrockNoise = BedrockNoise.GetNoise((float)xPerlin,(float)yPerlin);
			int bedrockTop = BEDROCK_BASE + (int)((bedrockNoise + 1.0f) * 0.5f * BEDROCK_HEIGHT);
			for (int z = 0; z < CHUNK_Z; ++z)
			{	
				if (z<bedrockTop || z==0)
				{				
					chunk->SetBlock(x, y, z, BlockType::Cobblestone);
					continue;
				}
				float room = CavesRoomNoise.GetNoise((float)xPerlin, (float)yPerlin, (float)z);
				float tunnel  = CavesTunnelNoise.GetNoise((float)xPerlin, (float)yPerlin, (float)z);
				float mask = GetHeightMask(z, 1, chunk->GetSurfaceHeight(x,y) - 6);
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

void ACubeGenerator::ChunksInit()
{
	for (auto mesh : MeshesMap)
	{
		mesh.Value->ClearAllMeshSections();
	} 
	for (int x = 0; x < 5; x++)
	{
		for (int y = 0; y < 5; y++)
		{
			NewChunk(x,y);			
			Section++;
		}
	}	
}

void ACubeGenerator::RemoveBlock(int64 Index, FVector hit)
{
	
}

void ACubeGenerator::NewChunk(int xChunk, int yChunk)
{
	double TStart = FPlatformTime::Seconds();
	UChunk* NewChunk = NewObject<UChunk>();
	double T1 = FPlatformTime::Seconds();
	for (int xPerlin = xChunk*CHUNK_X, x =0; xPerlin <xChunk*CHUNK_X+CHUNK_X; xPerlin++,x++)
	{
		for (int yPerlin = yChunk*CHUNK_X, y=0; yPerlin <yChunk*CHUNK_X+CHUNK_X; yPerlin++,y++)
		{
			NewChunk->SetSurfaceHeight(x,y,mapHeight(xPerlin,yPerlin));
		}
	}	
	NewChunk->Fill();
	CavesCreate(NewChunk,xChunk,yChunk);
	
	ChunksMap.Add(Section,NewChunk);
	double T2 = FPlatformTime::Seconds();

	UMinecraftProceduralMeshComponent* ProcMesh = NewObject<UMinecraftProceduralMeshComponent>(this);
	ProcMesh->RegisterComponent();
	ProcMesh->AttachToComponent(RootComponent,FAttachmentTransformRules::KeepRelativeTransform);
	ProcMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProcMesh->SetRelativeLocation(FVector(xChunk*CHUNK_X*BLOCK_SIZE, yChunk*CHUNK_X*BLOCK_SIZE, 0));
	MeshesMap.Add(Section,ProcMesh);
	
	double T3 = FPlatformTime::Seconds();
	UGreedyMeshing* GM = NewObject<UGreedyMeshing>();	
	Async(EAsyncExecution::ThreadPool, [=]()
		{			
			GM->BuildChunkMesh(NewChunk);
			GreedyMeshingMap.Add(Section,GM);
			AsyncTask(ENamedThreads::GameThread, [=]()
			{				
				GM->CreateMesh(*ProcMesh,Mat,0);
			});
		});
	double T4 = FPlatformTime::Seconds();
	UE_LOG(LogTemp, Warning,
		TEXT("Chunk[%d,%d] Init: %.2f ms | Terrain: %.2f ms | Meshing: %.2f ms | MeshApply: %.2f ms | TOTAL: %.2f ms"),
		xChunk, yChunk,
		(T1-TStart)*1000,
		(T2-T1)*1000,
		(T3-T2)*1000,
		(T4-T3)*1000,
		(T4-TStart)*1000
	);
}

void ACubeGenerator::Draw()
{
	LoadLayers();
	ChunksInit();
}
