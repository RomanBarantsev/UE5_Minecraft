// Fill out your copyright notice in the Description page of Project Settings.


#include "CubeGenerator.h"

#include <iostream>

#include "GreedyMeshing.h"
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
	Surface = NewObject<UPerlinNoise2D>();
	Continentalness = NewObject<UPerlinNoise2D>();
	Caves = NewObject<UPerlinNoise3D>();
	//time start
	LoadNoiseTemplate();
	ChunksInit();
	//time end
}

// Called every frame
void ACubeGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

int ACubeGenerator::mapHeight(double n,int x,int y)
{
	static int staticCont=0;
	double norm = (n + 1.0) * 0.5;
	int h = FMath::FloorToInt(floor(MIN_HEIGHT + norm * (MAX_HEIGHT - MIN_HEIGHT)));
	if (h < 1) h = 1;
	if (h >= CHUNK_Z-1) h = CHUNK_Z-2;
	
	float cont = Continentalness->Perlin2D( x, y,ContScale,ContOctaves,ContPersistence,ContLacunarity);
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

void ACubeGenerator::LoadNoiseTemplate()
{
	auto Row = PerlinNoiseTable->FindRow<FPerlinNoiseBiom>("Surface","name");	
	if (Row)
	{
		Scale = Row->Scale;
		Octaves = Row->Octaves;
		Persistence = Row->Persistence;
		Lacunarity = Row->Lacunarity;
	}
	Row = PerlinNoiseTable->FindRow<FPerlinNoiseBiom>("Continentalness","name");	
	if (Row)
	{
		ContScale = Row->Scale;
		ContOctaves = Row->Octaves;
		ContPersistence = Row->Persistence;
		ContLacunarity = Row->Lacunarity;
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
			NewChunk->SetSurfaceHeight(x,y,mapHeight((Surface->Perlin2D(xPerlin, yPerlin,Scale,Octaves,Persistence,Lacunarity)),xPerlin,yPerlin));
		}
	}	
	NewChunk->Fill();
	
	
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

