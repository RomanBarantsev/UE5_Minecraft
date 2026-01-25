#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakableCube.generated.h"

enum BlockType : uint8;
class UGeometryCollectionComponent;
class UGeometryCollection;
class UProceduralMeshComponent;

UCLASS()
class MINECRAFT_API ABreakableCube : public AActor
{
	GENERATED_BODY()

public:
	ABreakableCube();

protected:
	virtual void BeginPlay() override;
public:		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	UGeometryCollectionComponent * GeometryCollectionComponent;
	void FractureNow(BlockType type);
	void Reset();
private:
	UPROPERTY(EditAnywhere)
	UMaterialInterface* AtlasMaterial;
	UPROPERTY(VisibleAnywhere)
	UPrimitiveComponent* RootPrim;
};
