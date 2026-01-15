// Fill out your copyright notice in the Description page of Project Settings.


#include "MC_Pawn.h"

#include "CubeGenerator.h"
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
		Move->MaxSpeed = 600000.f;        // было ~1200
		Move->Acceleration = 120000.f;   // быстрее разгон
		Move->Deceleration = 120000.f;
	}	
}

// Called every frame
void AMC_Pawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AMC_Pawn::Fire()
{
	FVector StartPos=GetActorLocation();
	FVector EndPos=StartPos+GetControlRotation().Vector()*1000;
	TArray<AActor*> ActorsToIgnore;
	FHitResult OutHit;	
	UKismetSystemLibrary::LineTraceSingle(GetWorld(),StartPos,EndPos,TraceTypeQuery1,false,ActorsToIgnore,EDrawDebugTrace::ForDuration,OutHit,true);	
	if (const auto& CubeGenerator = Cast<ACubeGenerator>(OutHit.GetActor()))
	{
		if (const auto& Chunk = Cast<UMinecraftProceduralMeshComponent> (OutHit.GetComponent()))
		{
			FVector WorldHitLocation = OutHit.Location;
			FVector LocalHitLocation = Chunk->GetComponentTransform().InverseTransformPosition(WorldHitLocation);
			UE_LOG(LogTemp,Display,TEXT("%f %f %f"),LocalHitLocation.X/BLOCK_SIZE,LocalHitLocation.Y/BLOCK_SIZE,LocalHitLocation.Z/BLOCK_SIZE);
		}
	}
		
}

void AMC_Pawn::Redraw()
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(),ACubeGenerator::StaticClass(),OutActors);
	auto CubeGenerator = Cast<ACubeGenerator>(OutActors[0]);
	if (CubeGenerator)
	{
		CubeGenerator->Draw();
	}
}

// Called to bind functionality to input
void AMC_Pawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	InputComponent->BindAction("Fire",IE_Pressed,this,&ThisClass::Fire);
	InputComponent->BindAction("Redraw",IE_Pressed,this,&ThisClass::Redraw);
}

