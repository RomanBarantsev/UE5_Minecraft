// Fill out your copyright notice in the Description page of Project Settings.


#include "CubeGenerator.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "DSP/Osc.h"
#include "Kismet/KismetMathLibrary.h"


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

	// Создаём HISM-компонент (лучше, чем обычный ISM, т.к. поддерживает LOD и culling)
	HISM = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("CubeHISM"));
	HISM->SetupAttachment(RootComponent);

	// Загружаем меш через ConstructorHelpers — это безопасно и быстро в конструкторе
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Game/Cube_2.Cube_2"));

	if (CubeMesh.Succeeded())
	{
		HISM->SetStaticMesh(CubeMesh.Object);
		HISM->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HISM->SetMobility(EComponentMobility::Movable);
		UE_LOG(LogTemp, Display, TEXT("✅ Cube mesh успешно загружен"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Не удалось загрузить StaticMesh: /Game/Cube_2.Cube_2"));
	}	
	// Устанавливаем оптимальные параметры
	HISM->NumCustomDataFloats = 0;
	HISM->bCastDynamicShadow = true;
	HISM->bAffectDistanceFieldLighting = false;
}

// Called when the game starts or when spawned
void ACubeGenerator::BeginPlay()
{
	Super::BeginPlay();
	Noise2D = NewObject<UPerlinNoise2D>();
	Noise3D = NewObject<UPerlinNoise3D>();
	//time start
	LoadNoiseTemplate("Default");
	NewChunk = NewObject<UChunk>();
	NewChunk->InitChunk();
	Generation2D();
	LoadNoiseTemplate("Cave");
	//Generation3D();
	NewChunk->Fill();
	DrawCall();
	//time end
}

// Called every frame
void ACubeGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACubeGenerator::Generation2D()
{
	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			int floor  = mapHeight((Noise2D->Perlin2D(x, y,Scale,Octaves,Persistence,Lacunarity)));
			NewChunk->SetBlock(x,y,floor,BlockType::Stone);
		}
	}
}

void ACubeGenerator::Generation3D()
{
	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			for (int z = 0; z < CHUNK_Z; ++z)
			{
				float density  = Noise3D->Perlin3D(x, y, z, Scale, Octaves, Persistence, Lacunarity, Seed);
				
				int normalized = NormalizeNoise(density,z,0,UpperLayer3d,Threshold);
				if (normalized > 0)
				{
					NewChunk->SetBlock(x,y,z,BlockType::Dirt);				
				}				
			}
		}
	}
	UE_LOG(LogTemp, Display, TEXT("✅ Сгенерировано %d кубов"), CHUNK_SIZE * CHUNK_SIZE);
}

int ACubeGenerator::mapHeight(double n)
{
	double norm = (n + 1.0) * 0.5;
	int h = (int)floor(MIN_HEIGHT + norm * (MAX_HEIGHT - MIN_HEIGHT));
	if (h < 1) h = 1;
	if (h >= CHUNK_Z-1) h = CHUNK_Z-2;
	return h;
}

void ACubeGenerator::LoadNoiseTemplate(FName name)
{
	auto Row = PerlinNoiseTable->FindRow<FPerlinNoiseBiom>(name,"name");	
	if (Row)
	{
		Scale = Row->Scale;
		Octaves = Row->Octaves;
		Persistence = Row->Persistence;
		Lacunarity = Row->Lacunarity;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Data table FPerlinNoiseBiom read error"));
		return;
	}
}

int  ACubeGenerator::NormalizeNoise(float noise_value,int z,int z_min,int z_max,float threshold)
{
	// Нормализуем Y в [0; 1]
	float y_norm = static_cast<float>(z - z_min) / (z_max - z_min);
    
	// Параболический фактор: максимум в центре слоя
	float vertical_factor = 4.0f * y_norm * (1.0f - y_norm);
        
	// Применяем вертикальный фактор
	float adjusted_noise = noise_value * vertical_factor;
    
	// Проверяем порог: если шум > threshold → воздух (0), иначе — твёрдый блок (1)
	return (adjusted_noise > threshold) ? 0 : 1;
}

void ACubeGenerator::DrawCall() const
{
	auto terrain = NewChunk->GetTerrain();
	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			for (int z = 0; z < CHUNK_Z; z++)
			{
				if (terrain[x][y][z]!=0)
				{
					FVector Location(x * CubeSize, y * CubeSize, z * CubeSize);
					FTransform Transform(Location);
					HISM->AddInstance(Transform);
				}				
			}
		}
	}
}

