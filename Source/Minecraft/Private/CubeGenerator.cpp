// Fill out your copyright notice in the Description page of Project Settings.


#include "CubeGenerator.h"

#include "GreedyMeshing.h"
#include "MovieSceneTracksComponentTypes.h"
#include "ProceduralMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "DSP/Osc.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"


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
		HISM->NumCustomDataFloats = 1;
		
		UE_LOG(LogTemp, Display, TEXT("✅ Cube mesh успешно загружен"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Не удалось загрузить StaticMesh: /Game/Cube_2.Cube_2"));
	}	
	
}

// Called when the game starts or when spawned
void ACubeGenerator::BeginPlay()
{
	Super::BeginPlay();
	HISM->NumCustomDataFloats = 2;
	HISM->bCastDynamicShadow = true;
	HISM->bAffectDistanceFieldLighting = false;
	if (Mat)
	{
		HISM->SetMaterial(0,Mat);
	}
	Surface = NewObject<UPerlinNoise2D>();
	Continentalness = NewObject<UPerlinNoise2D>();
	//time start
	LoadNoiseTemplate();
	NewChunk = NewObject<UChunk>();
	NewChunk->InitChunk();
	GenerateSurface();
	NewChunk->Fill();
	//DrawCall();
	UProceduralMeshComponent* ProcMesh;
	ProcMesh = NewObject<UProceduralMeshComponent>(this);
	ProcMesh->RegisterComponent();
	ProcMesh->AttachToComponent(RootComponent,FAttachmentTransformRules::KeepRelativeTransform);
	ProcMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);	
	UGreedyMeshing* GM = NewObject<UGreedyMeshing>();	
	GM->BuildChunkMesh(NewChunk->GetTerrain());
	GM->CreateMesh(*ProcMesh);
	//time end
}

// Called every frame
void ACubeGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACubeGenerator::GenerateSurface()
{
	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			NewChunk->Surface[x][y]  = mapHeight((Surface->Perlin2D(x, y,Scale,Octaves,Persistence,Lacunarity)),x,y);
			NewChunk->SetBlock(x,y,NewChunk->Surface[x][y],BlockType::Stone);			
		}
	}
}

void ACubeGenerator::Generation3D()
{
	auto terrain =  NewChunk->GetTerrain();
	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_SIZE; y++)
		{			
			for (int z = 0; z < NewChunk->Surface[x][y]; ++z)
			{
				/*int surf = NewChunk->Surface[x][y];
				
				int depth = surf - z;
				double heightInfluence = (z - depth) / 16.0;
				double density = noise - heightInfluence;*/
				//float noise  = Noise3D->Perlin3D(x, y, z, Scale, Octaves, Persistence, Lacunarity, Seed);
				//int normalized = NormalizeNoise(noise,z,0,NewChunk->Surface[x][y],Threshold);
				/*if (normalized > 0)
				{
					NewChunk->SetBlock(x,y,z,BlockType::Stone);				
				}
				else
				{
					NewChunk->SetBlock(x,y,z,BlockType::Empty);
				}*/
			}
		}
	}
	UE_LOG(LogTemp, Display, TEXT("✅ Сгенерировано %d кубов"), CHUNK_SIZE * CHUNK_SIZE);
}

int ACubeGenerator::mapHeight(double n,int x,int y)
{
	double norm = (n + 1.0) * 0.5;
	int h = (int)floor(MIN_HEIGHT + norm * (MAX_HEIGHT - MIN_HEIGHT));
	if (h < 1) h = 1;
	if (h >= CHUNK_Z-1) h = CHUNK_Z-2;
	float cont = Continentalness->Perlin2D( x, y,ContScale,ContOctaves,ContPersistence,ContLacunarity);
	cont=(cont+1.0f)/2;
	int contH=0;
	float Y=0;
	if (ContinentalnessCurve)
	{
		Y = ContinentalnessCurve->GetFloatValue(cont);
	}
	contH=static_cast<int>(Y);
	int finalHeight = (int)(h + contH);
	finalHeight = FMath::Clamp(finalHeight, 1, CHUNK_Z - 2);
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
	Row = PerlinNoiseTable->FindRow<FPerlinNoiseBiom>("Surface","name");	
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

void ACubeGenerator::DrawCall() const
{
	const auto& terrain = NewChunk->GetTerrain();
	HISM->PreAllocateInstancesMemory(CHUNK_SIZE*CHUNK_SIZE*CHUNK_Z);
	for (int x = 0; x < CHUNK_SIZE; x++)
	{
		for (int y = 0; y < CHUNK_SIZE; y++)
		{
			for (int z = 0; z < CHUNK_Z; z++)
			{
				if (terrain[x][y][z]!=-1)
				{
					FVector Location(x * CubeSize, y * CubeSize, z * CubeSize);
					FTransform Transform(Location);
					int32 InstID = HISM->AddInstance(Transform);
					//HISM->SetCustomDataValue(InstID,1,FMath::RandRange(0,16),false);
					//HISM->SetCustomDataValue(InstID,1,terrain[x][y][z],false);
				}				
			}
		}
	}
	HISM->MarkRenderStateDirty();
}

