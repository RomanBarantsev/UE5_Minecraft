// Fill out your copyright notice in the Description page of Project Settings.


#include "Minecraft/Public/Cube.h"

#include "Preferences/PersonaOptions.h"


// Sets default values
ACube::ACube()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);
	UStaticMesh* NewMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Script/Engine.StaticMesh'/Game/Cube.Cube'"));
	if (NewMesh)
	{
		StaticMesh->SetStaticMesh(NewMesh);
	}
	StaticMesh->BodyInstance.bLockXRotation = true;
	StaticMesh->BodyInstance.bLockYRotation = true;
	StaticMesh->BodyInstance.bLockZRotation = true;
	StaticMesh->BodyInstance.bLockYTranslation = true;
	StaticMesh->BodyInstance.bLockXTranslation = true;
	StaticMesh->bIgnoreRadialForce = true;
	StaticMesh->bIgnoreRadialForce = true;
	StaticMesh->SetSimulatePhysics(false);
}

void ACube::Fall(bool state)
{
	StaticMesh->SetSimulatePhysics(true);
}

void ACube::CheckFloor()
{
	FHitResult HitResult;
	FVector StartLocation = GetActorLocation()*100;
	FVector EndLocation = StartLocation + -GetActorUpVector();
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult,StartLocation,EndLocation,ECC_Visibility,FCollisionQueryParams());
}

