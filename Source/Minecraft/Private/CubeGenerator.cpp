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

// Called when the game starts or when spawned
void ACubeGenerator::BeginPlay()
{
	Super::BeginPlay();	
	CavesParams.rowName="Caves";
	SurfaceParams.rowName="Surface";
	ContParams.rowName="Continentalness";
	LoadNoiseParams(CavesNoise,CavesParams);
	LoadNoiseParams(SurfaceNoise,SurfaceParams);
	LoadNoiseParams(ContNoise,ContParams);
	SetNoiseParams(CavesNoise,CavesParams, FastNoiseLite::NoiseType_Perlin);
	SetNoiseParams(SurfaceNoise,SurfaceParams, FastNoiseLite::NoiseType_Perlin);	
	SetNoiseParams(ContNoise,ContParams, FastNoiseLite::NoiseType_Perlin);	
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
}

int ACubeGenerator::mapHeight(double n,int x,int y)
{
	static int staticCont=0;
	double norm = (n + 1.0) * 0.5;
	int h = FMath::FloorToInt(floor(MIN_HEIGHT + norm * (MAX_HEIGHT - MIN_HEIGHT)));
	if (h < 1) h = 1;
	if (h >= CHUNK_Z-1) h = CHUNK_Z-2;
	
	float cont = ContNoise.GetNoise((float)x, (float)y);
	cont=(cont + 1.0) * 0.5;
	int contH=0;
	float Y=0;
	if (ContinentalnessCurve)
	{
		Y = ContinentalnessCurve->GetFloatValue(cont);
	}
	contH=static_cast<int>(Y);	
	int finalHeight = (int)(contH+h);
	//finalHeight = FMath::Clamp(finalHeight, 1, CHUNK_Z - 2);
	return finalHeight;
}

void ACubeGenerator::CavesCreate(UChunk* chunk,int xChunk, int yChunk) //TODO объеденить с Fill
{	
	
	float  delimiter=0.01;
	for (int xPerlin = xChunk*CHUNK_X, x =0; xPerlin <xChunk*CHUNK_X+CHUNK_X; xPerlin++,x++)
	{
		for (int yPerlin = yChunk*CHUNK_X, y=0; yPerlin <yChunk*CHUNK_X+CHUNK_X; yPerlin++,y++)
		{
			for (int z = 0; z < CHUNK_Z; ++z)
			{	
				float density = CavesNoise.GetNoise((float)xPerlin, (float)yPerlin, (float)z);
				if (density > -0.0f) {
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
			NewChunk->SetSurfaceHeight(x,y,mapHeight(SurfaceNoise.GetNoise((float)xPerlin,(float)yPerlin),xPerlin,yPerlin));
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

