// Fill out your copyright notice in the Description page of Project Settings.


#include "MC_Pawn.h"

#include "ChunkManagerSubsystem.h"
#include "FChunkBuildData.h"
#include "MinecraftProceduralMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/KismetSystemLibrary.h"


class UCharacterMovementComponent;
// Sets default values
AMC_Pawn::AMC_Pawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMC_Pawn::BeginPlay()
{
	Super::BeginPlay();	
	if (UFloatingPawnMovement* Move = FindComponentByClass<UFloatingPawnMovement>())
	{
		Move->MaxSpeed = 30000.f;       
		Move->Acceleration = 12000.f;   
		Move->Deceleration = 12000.f;
	}
	ChunkWorldSubsystem = GetWorld()->GetSubsystem<UChunkManagerSubsystem>();
	if (!ChunkWorldSubsystem)
	{
		UE_LOG(LogTemp,Error,TEXT("ChunkWorldSubsystem is nullptr"));
	}
	GetWorld()->GetTimerManager().SetTimer(WorldUpdateTimerHandle,this,&AMC_Pawn::WorldUpdate,1.0f,true,0);
}

void AMC_Pawn::WorldUpdate()
{
	if (ChunkWorldSubsystem)
	{
		ChunkWorldSubsystem->UpdateChunks(GetActorLocation());		
	}
}

void AMC_Pawn::Fire()
{
	FVector StartPos = GetActorLocation();
	FVector EndPos = StartPos + GetControlRotation().Vector() * 1000;
	TArray<AActor*> ActorsToIgnore;
	FHitResult Hit;
	if (UKismetSystemLibrary::LineTraceSingle(GetWorld(),StartPos,EndPos,TraceTypeQuery1,false,ActorsToIgnore,EDrawDebugTrace::None,Hit,true))
	{
		auto Mesh = Cast<UMinecraftProceduralMeshComponent>(Hit.GetComponent());
		if (!Mesh) return;
		if (ChunkWorldSubsystem)
		{
			ChunkWorldSubsystem->RemoveBlock(Hit, Mesh);
		}
	}		
}

FVector AMC_Pawn::GetPlayerVoxelPos()
{
	auto coord = GetActorLocation()/BLOCK_SIZE;
	coord.X = FMath::FloorToInt(coord.X);
	coord.Y = FMath::FloorToInt(coord.Y);
	coord.Z = FMath::FloorToInt(coord.Z);
	return coord;
}

// Called to bind functionality to input
void AMC_Pawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	InputComponent->BindAction("Fire",IE_Pressed,this,&ThisClass::Fire);
}

