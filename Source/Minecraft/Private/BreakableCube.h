#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakableCube.generated.h"

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
	void FractureNow();
	virtual void BeginPlay() override;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
	UGeometryCollectionComponent * GeometryCollectionComponent;
private:
	
	UPROPERTY(VisibleAnywhere)
	UPrimitiveComponent* RootPrim;
};
