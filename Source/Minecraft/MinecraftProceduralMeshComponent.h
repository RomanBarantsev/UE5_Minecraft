// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "MinecraftProceduralMeshComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MINECRAFT_API UMinecraftProceduralMeshComponent : public UProceduralMeshComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMinecraftProceduralMeshComponent(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	int64 Index;
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	void SetIndex(int i){Index=i;}
	int64 GetIndex() const {return Index;}
};
