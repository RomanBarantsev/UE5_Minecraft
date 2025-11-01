// Fill out your copyright notice in the Description page of Project Settings.


#include "TerrainGenerator.h"

#include "Constraint.h"
#include "Cube.h"
#include "Minecraft/PerlinNoise.h"
#include "UObject/FastReferenceCollector.h"


// Sets default values
ATerrainGenerator::ATerrainGenerator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ATerrainGenerator::BeginPlay()
{
	Super::BeginPlay();
	FActorSpawnParameters spawnParameters;
	FRotator rotation(0,0,0);
	PerlinNoise PN(10,10);
	auto grid = PN.GetMatrix();
	FVector pos(100,0,0);	
	for (auto vec : grid)
	{
		for (auto z : vec)
		{
			pos+=FVector(0,0,z);
			GetWorld()->SpawnActor(ACube::StaticClass(), &pos,&rotation, spawnParameters);		
			pos+=FVector(100.0f,0,0);
		}	
		pos+=FVector(-pos.X+100,100.0f,0);
	}
}

// Called every frame
void ATerrainGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

