// Fill out your copyright notice in the Description page of Project Settings.


#include "CubeGenerator.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
ACubeGenerator::ACubeGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

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
	
	const int GridX = 1000;
	const int GridY = 1000;
	const float CubeSize = 100.0f;
	const int CubeSpacing = 5;
	for (int x = 0; x < GridX; x++)
	{
		for (int y = 0; y < GridY; y++)
		{
			FVector Location(x * CubeSize*CubeSpacing, y * CubeSize*CubeSpacing, /*UKismetMathLibrary::RandomInteger64InRange(0,100)*/0);
			FTransform Transform(Location);
			HISM->AddInstance(Transform);
		}
	}

	UE_LOG(LogTemp, Display, TEXT("✅ Сгенерировано %d кубов"), GridX * GridY);
}

// Called every frame
void ACubeGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

