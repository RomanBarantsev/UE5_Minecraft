#include "BreakableCube.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

ABreakableCube::ABreakableCube()
{
    PrimaryActorTick.bCanEverTick = false;
    RootPrim = Cast<UPrimitiveComponent>(GetRootComponent());
    GeometryCollectionComponent = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
    GeometryCollectionComponent->SetupAttachment(RootPrim);
    if (!RootPrim)
    {
        UE_LOG(LogTemp, Error, TEXT("[BreakableCube] Root is NOT UPrimitiveComponent!"));
        return;
    }
}

void ABreakableCube::FractureNow()
{
    
}

void ABreakableCube::BeginPlay()
{
    Super::BeginPlay();
    FractureNow();    
}
