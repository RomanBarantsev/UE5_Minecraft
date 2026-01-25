#include "BreakableCube.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

ABreakableCube::ABreakableCube()
{
    PrimaryActorTick.bCanEverTick = false;
    GeometryCollectionComponent = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
    SetRootComponent(GeometryCollectionComponent);
    RootPrim = Cast<UPrimitiveComponent>(GetRootComponent());
    if (!RootPrim)
    {
        UE_LOG(LogTemp, Error, TEXT("[BreakableCube] Root is NOT UPrimitiveComponent!"));
        return;
    }
}

void ABreakableCube::FractureNow(BlockType type)
{
    if (GeometryCollectionComponent)
    {
        auto Material = GeometryCollectionComponent->GetMaterial(0);
        if (Material)
        {
            if (auto DynamicMaterial = GeometryCollectionComponent->CreateAndSetMaterialInstanceDynamic(0))
            {
                // Устанавливаем индекс тайла для этого экземпляра
                DynamicMaterial->SetScalarParameterValue(FName("TileIndex"), type); // Пример значения
            }
        }
    }
}

void ABreakableCube::Reset()
{
   
}

void ABreakableCube::BeginPlay()
{
    Super::BeginPlay();
}
