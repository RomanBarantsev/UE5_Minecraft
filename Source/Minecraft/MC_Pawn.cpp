// Fill out your copyright notice in the Description page of Project Settings.


#include "MC_Pawn.h"

#include "CubeGenerator.h"
#include "FChunkBuildData.h"
#include "MinecraftProceduralMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"
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
		Move->MaxSpeed = 600000.f;       
		Move->Acceleration = 120000.f;   
		Move->Deceleration = 120000.f;
	}
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(),ACubeGenerator::StaticClass(),OutActors);
	CubeGenerator = Cast<ACubeGenerator>(OutActors[0]);
	if (!CubeGenerator)
	{
		UE_LOG(LogTemp,Error,TEXT("CubeGenerator is nullptr"));
	}
	//PlaceAboveSurface();
	GetWorld()->GetTimerManager().SetTimer(WorldUpdateTimerHandle,this,&AMC_Pawn::WorldUpdate,2.0f,true,0);
}

void AMC_Pawn::WorldUpdate()
{
	if (CubeGenerator)
	{
	CubeGenerator->UpdateChunks(GetActorLocation());		
	}
}

// Called every frame
void AMC_Pawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMC_Pawn::PlaceAboveSurface()
{
	auto height = CubeGenerator->GetSurfaceHigh(GetActorLocation());
	SetActorLocation(GetActorLocation() + FVector(0,0,height*BLOCK_SIZE+PawnSize));
	UE_LOG(LogTemp,Warning,TEXT("height %d"),height);
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
		CubeGenerator->RemoveBlock(Hit, Mesh);
	}		
}

// Called to bind functionality to input
void AMC_Pawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	InputComponent->BindAction("Fire",IE_Pressed,this,&ThisClass::Fire);
}

