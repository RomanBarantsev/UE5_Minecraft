// Fill out your copyright notice in the Description page of Project Settings.


#include "CubeGenerator.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
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
	

	
}

// Called every frame
void ACubeGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACubeGenerator::Generation2D()
{
	auto Row = PerlinNoiseTable->FindRow<FPerlinNoiseBiom>("Default","name");
	int Scale;
	int Octaves;
	int Persistence;
	int Lacunarity;
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
	UPerlinNoise2D* Noise = NewObject<UPerlinNoise2D>();
	for (int x = 0; x < GridX; x++)
	{
		for (int y = 0; y < GridY; y++)
		{
			float highs  = Noise->Perlin2D(x, y, Scale, Octaves, Persistence, Lacunarity, Seed);
			FVector Location(x * CubeSize, y * CubeSize, highs * CubeSize);
			FTransform Transform(Location);
			HISM->AddInstance(Transform);		
		}
	}
}

void ACubeGenerator::Generation3D()
{
	UPerlinNoise3D* Noise = NewObject<UPerlinNoise3D>();
	
	for (int x = 0; x < GridX; x++)
	{
		for (int y = 0; y < GridY; y++)
		{
			for (int z = 0; z < GridZ; ++z)
			{
				float density  = Noise->Perlin3D(x, y, z, Scale, Octaves, Persistence, Lacunarity, Seed);
				if (density > Threshold)
				{
					FVector Location(x * CubeSize, y * CubeSize, z * CubeSize);
					FTransform Transform(Location);
					HISM->AddInstance(Transform);
				}
				
			}
			
		}
	}
	UE_LOG(LogTemp, Display, TEXT("✅ Сгенерировано %d кубов"), GridX * GridY);
}

