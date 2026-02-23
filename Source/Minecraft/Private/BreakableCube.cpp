#include "BreakableCube.h"
#include "Components/PrimitiveComponent.h"
#include "Field/FieldSystemObjects.h"
#include "GameFramework/Actor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GeometryCollection/GeometryCollectionSimulationTypes.h"

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

void ABreakableCube::FractureNow(BlockType type, FHitResult hit)
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
            if (auto DynamicMaterial = GeometryCollectionComponent->CreateAndSetMaterialInstanceDynamic(1))
            {
                // Устанавливаем индекс тайла для этого экземпляра
                DynamicMaterial->SetScalarParameterValue(FName("TileIndex"), type); // Пример значения
            }
        }
        
        FVector ViewLocation;
        FRotator ViewRotation;

        GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
            ViewLocation,
            ViewRotation
        );

        FVector ViewDir = ViewRotation.Vector();
        /*GeometryCollectionComponent->ApplyPhysicsField(true,EGeometryCollectionPhysicsTypeEnum::Chaos_ExternalClusterStrain,nullptr,
     MakeShared<FRadialFalloff>(5000.f,0.f,1.f,0.f,300.f,hit.ImpactPoint,EFieldFalloffType::Field_Falloff_Linear));

        // Толкаем все куски
        GeometryCollectionComponent->ApplyPhysicsField(true,EGeometryCollectionPhysicsTypeEnum::Chaos_LinearVelocity,nullptr,
            MakeShared<FUniformVector>(ViewDir * 3000.f));*/
    }    
}

void ABreakableCube::Reset()
{
   
}

void ABreakableCube::BeginPlay()
{
    Super::BeginPlay();
}
