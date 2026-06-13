// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cube.generated.h"

UCLASS(Abstract)
class MINECRAFT_API ACube : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACube();	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StaticMesh;
	
public:
	UFUNCTION()
	virtual void Fall(bool state);
	UFUNCTION()
	virtual void CheckFloor();
};
