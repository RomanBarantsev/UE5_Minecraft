// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkGenerator.h"
#include "Minecraft/ChunkManagerSubsystem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Minecraft/BiomDataAsset.h"
#include "Minecraft/FastNoiseLite.h"
#include "Minecraft/FChunkBuildData.h"
#include "NoiseManagerSubSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

class UProceduralMeshComponent;
// Sets default values

AChunkGenerator::AChunkGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
	// Создаём корневой компонент
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
		
}


// Called when the game starts or when spawned
void AChunkGenerator::BeginPlay()
{
	Super::BeginPlay();
	NoiseManager = GetGameInstance()->GetSubsystem<UNoiseManagerSubSystem>();
	if (!NoiseManager)
		UKismetSystemLibrary::QuitGame(GetWorld(), UGameplayStatics::GetPlayerController(GetWorld(),0), EQuitPreference::Quit, false);	
	FastNoises = NoiseManager->GetNoises();
	LoadAllBioms();
	InitializeBiomeMap();

	if (UChunkManagerSubsystem* ChunkWorldSubsystem = GetWorld()->GetSubsystem<UChunkManagerSubsystem>())
	{
		ChunkWorldSubsystem->SetChunkGenerator(this);
		ChunkWorldSubsystem->UpdateChunks(FVector::ZeroVector);
	}
}

void AChunkGenerator::LoadAllBioms()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	
	TArray<FAssetData> AssetDataList;
	FARFilter Filter;
	Filter.ClassPaths.Add(UBiomDataAsset::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Add("/Game/Data");
	Filter.bRecursivePaths = true;
	
	AssetRegistry.GetAssets(Filter, AssetDataList);
	
	BiomesArray.Empty();
	for (const auto& AssetData : AssetDataList)
	{
		UBiomDataAsset* BiomeData = Cast<UBiomDataAsset>(AssetData.GetAsset());
		if (BiomeData)
		{
			BiomesArray.Add(BiomeData);
		}
	}
}

void AChunkGenerator::InitializeBiomeMap()
{		
	BiomesLUTArray.SetNumUninitialized(BiomesArraySize);
	double StartTime = FPlatformTime::Seconds();
	ParallelFor(BiomesArraySize, [&](int32 i)
	{
		int32 tempIndex = i / 200;
		int32 humIndex = i % 200;
		int32 T = tempIndex - 100;
		int32 H = humIndex - 100;
		BiomesLUTArray[i]=CalculateBiomWeights(T,H);
	});
	double EndTime = FPlatformTime::Seconds();
	double TimePassedMs = (EndTime - StartTime) * 1000.0; 
	UE_LOG(LogTemp, Warning, TEXT("InitializeBiomeMap (Parallel) took: %f ms"), TimePassedMs);
}

FBiomLUTMap AChunkGenerator::CalculateBiomWeights(int T, int H)
{	
	float fTemp=T/100.0f;
	float fHum=H/100.0f;
	float TotalWeight=0;
	float MaxWeight=0;
	float WeightedScaleSum = 0.0f;
	float WeightedOffsetSum = 0.0f;
	UBiomDataAsset* WinnerBiome=nullptr;
	for (const auto& Biome : BiomesArray)
	{
		float Weight =FMath::Square(Biome->TargetTemperature-fTemp)+FMath::Square(Biome->TargetHumidity-fHum);
		if (Weight<0.01f)
		{
			return FBiomLUTMap{Biome->Offset,Biome->VerticalScale,Biome};
		}		
		float W = 1.0f / (Weight * Weight);
		WeightedScaleSum += Biome->VerticalScale * W;
		WeightedOffsetSum += Biome->Offset * W;
		TotalWeight+=W;
		if (W>MaxWeight)
		{
			MaxWeight=W;
			WinnerBiome=Biome;
		}
	}
	FBiomLUTMap BlendedBiomeData;
	BlendedBiomeData.VerticalScale = WeightedScaleSum / TotalWeight;
	BlendedBiomeData.HeightOffset = WeightedOffsetSum / TotalWeight;
	BlendedBiomeData.Biome = WinnerBiome;
	return BlendedBiomeData; 
}

float AChunkGenerator::GetHeightMask(int z, int minZ, int maxZ)
{
	if (z <= minZ || z >= maxZ) return 0.0f;

	float t = float(z - minZ) / float(maxZ - minZ);
	return 1.0f - t * t; // плавно затухает к поверхности
}

FInterpolatedBiomeData AChunkGenerator::GetInterpolatedLUTData(float T, float H) 
{
	// 1. Переводим [-1, 1] в координаты сетки [0, 199]
	float GridT = (T + 1.0f) * 0.5f * 199.0f;
	float GridH = (H + 1.0f) * 0.5f * 199.0f;

	// 2. Находим индексы четырех соседних ячеек
	int32 T0 = FMath::FloorToInt(GridT);
	int32 T1 = FMath::Min(T0 + 1, 199);
	int32 H0 = FMath::FloorToInt(GridH);
	int32 H1 = FMath::Min(H0 + 1, 199);

	// 3. Вычисляем веса смешивания (дробная часть)
	float FracT = GridT - T0;
	float FracH = GridH - H0;

	// 4. Берем данные из 4-х точек	
	auto& D00 = BiomesLUTArray[T0 * 200 + H0];
	auto& D01 = BiomesLUTArray[T0 * 200 + H1];
	auto& D10 = BiomesLUTArray[T1 * 200 + H0];
	auto& D11 = BiomesLUTArray[T1 * 200 + H1];

	// 5. Билинейная интерполяция Scale
	float S0 = FMath::Lerp(D00.VerticalScale, D01.VerticalScale, FracH);
	float S1 = FMath::Lerp(D10.VerticalScale, D11.VerticalScale, FracH);
	float FinalScale = FMath::Lerp(S0, S1, FracT);

	// 6. Билинейная интерполяция Offset
	float O0 = FMath::Lerp(D00.HeightOffset, D01.HeightOffset, FracH);
	float O1 = FMath::Lerp(D10.HeightOffset, D11.HeightOffset, FracH);
	float FinalOffset = FMath::Lerp(O0, O1, FracT);

	return { FinalScale, FinalOffset };
}

int AChunkGenerator::CalculateHeight(FNoises noises)
{
	
	float BaseHeight = ContinentalnessCurve->GetFloatValue(noises.Continentalness);
	float Erosion = (noises.Erosion + 1.0f) * 0.5; // 0-1
	float Peaks = PeaksValleysCurve->GetFloatValue(noises.PeaksValleys);
	float Height = BaseHeight +	Peaks*Erosion;
	FInterpolatedBiomeData Biome = GetInterpolatedLUTData(noises.Temperature, noises.Humidity);
	Height =Height * Biome.VerticalScale +Biome.HeightOffset;	

	return FMath::Clamp(FMath::FloorToInt(Height), 1, CHUNK_Z_SIZE - 2);
}

void AChunkGenerator::GenerateChunkData(FChunkBuildData& Data)
{
 // Measure total generation time and time for each stage
 double TotalStart = FPlatformTime::Seconds();

 const int TotalCells = CHUNK_X_SIZE * CHUNK_Y_SIZE;
 TArray<float> Temps; Temps.SetNumUninitialized(TotalCells);
 TArray<float> Humids; Humids.SetNumUninitialized(TotalCells);

 // 1) Height calculation (also sample and store temperature/humidity for later)
 double HeightStart = FPlatformTime::Seconds();
 for (int x = 0; x < CHUNK_X_SIZE; x++)
 {
  for (int y = 0; y < CHUNK_Y_SIZE; y++)
  {
   int worldX = Data.ChunkCoord.x*CHUNK_X_SIZE+x;
   int worldY = Data.ChunkCoord.y*CHUNK_Y_SIZE+y;
   FNoises noises;

   noises.PeaksValleys = FastNoises.PeaksValleysNoise.GetNoise((float)worldX,(float)worldY);
   noises.Continentalness = FastNoises.ContinentalnessNoise.GetNoise((float)worldX,(float)worldY);
   noises.Bedrock = FastNoises.BedrockNoise.GetNoise((float)worldX,(float)worldY);
   noises.CavesRoom = FastNoises.CavesRoomNoise.GetNoise((float)worldX,(float)worldY);
   noises.CavesTunnel = FastNoises.CavesTunnelNoise.GetNoise((float)worldX,(float)worldY);
   noises.Erosion = FastNoises.ErosionNoise.GetNoise((float)worldX,(float)worldY);
   noises.Humidity = FastNoises.HumidityNoise.GetNoise((float)worldX,(float)worldY);
   noises.Temperature = FastNoises.TemperatureNoise.GetNoise((float)worldX,(float)worldY);

   // store temp/humidity for surface generation stage
   int idx = x * CHUNK_Y_SIZE + y;
   Temps[idx] = noises.Temperature;
   Humids[idx] = noises.Humidity;

   int height = CalculateHeight(noises);
   Data.SetSurfaceHeight(x,y,height);
  }
 }
 double HeightEnd = FPlatformTime::Seconds();

 // 2) Surface generation (use stored temperature/humidity)
 double SurfaceStart = FPlatformTime::Seconds();
double CavesStart=FPlatformTime::Seconds();
double CavesEnd=FPlatformTime::Seconds();
double CavesMs = 0.0;
 if (!BiomesArray.IsEmpty())
 {
  for (int x = 0; x < CHUNK_X_SIZE; x++)
  {
   for (int y = 0; y < CHUNK_Y_SIZE; y++)
   {
	int idx = x * CHUNK_Y_SIZE + y;
	FNoises noises;
	noises.Temperature = Temps[idx];
	noises.Humidity = Humids[idx];
	int height = Data.GetSurfaceHeight(x,y);   	
   	// 3) Cave generation
   	CavesStart = FPlatformTime::Seconds();
   	GenerateCaves(Data,x,y);	
   	CavesEnd = FPlatformTime::Seconds();
   	CavesMs += (CavesEnd - CavesStart) * 1000.0;
	GenerateSurfaceLayer(height, noises, Data, x, y);
   }
  }
 }
 double SurfaceEnd = FPlatformTime::Seconds();

 // 4) Finalize / Fill
 double FillStart = FPlatformTime::Seconds();
 Data.Fill();
 double FillEnd = FPlatformTime::Seconds();

 double TotalEnd = FPlatformTime::Seconds();

 // Logging timings (milliseconds)
 double HeightMs = (HeightEnd - HeightStart) * 1000.0;
 double SurfaceMs = (SurfaceEnd - SurfaceStart) * 1000.0;
 double FillMs = (FillEnd - FillStart) * 1000.0;
 double TotalMs = (TotalEnd - TotalStart) * 1000.0;

 UE_LOG(LogTemp, Warning, TEXT("GenerateChunkData timings for chunk (%d,%d): Height=%.3fms (avg %.6fms/cell), Surface=%.3fms, Caves=%.3fms, Fill=%.3fms, Total=%.3fms"),
  Data.ChunkCoord.x, Data.ChunkCoord.y,
  HeightMs, (HeightMs / (double)TotalCells),
  SurfaceMs, CavesMs, FillMs, TotalMs);
}

void AChunkGenerator::GenerateCaves(FChunkBuildData& Data,int x,int y)
{
	int xPerlin = Data.ChunkCoord.x * CHUNK_X_SIZE + x;
	int yPerlin = Data.ChunkCoord.y * CHUNK_Y_SIZE + y;
	float fx = static_cast<float>(xPerlin);
	float fy = static_cast<float>(yPerlin);
	float bedrockNoise = FastNoises.BedrockNoise.GetNoise(fx,fy);
	int bedrockTop = BEDROCK_BASE + static_cast<int>(((bedrockNoise + 1.0f) * 0.5f * BEDROCK_HEIGHT));
	int height = Data.GetSurfaceHeight(x,y);
	for (int z = 0; z < height; ++z)
	{	
		if (z<bedrockTop || z==0)
		{					
			Data.SetBlock(x,y,z,BlockType::Cobblestone);
			continue;
		}
		float Room = FastNoises.CavesRoomNoise.GetNoise(fx, fy, static_cast<float>(z));
		float Tunnel  = FastNoises.CavesTunnelNoise.GetNoise(fx, fy, static_cast<float>(z));
		float Mask = GetHeightMask(z, 1, height);
		float Density =
			Tunnel * 1.2f +     // tunnels are more important
			Room * 0.8f;        // rooms are not so frequently
		Density *= Mask;
		if (Density > 0.25f)
		{
			Data.SetBlock(x,y,z,BlockType::Air);
		}
	}
}

void AChunkGenerator::GenerateSurfaceLayer(int z, FNoises& noises,FChunkBuildData& Data,int x,int y)
{	
	auto LUTData = GetLUTData(noises.Temperature,noises.Humidity);
	auto BiomeLayers = LUTData.Biome->SurfaceLayers;
	float bedrockNoise = FastNoises.BedrockNoise.GetNoise(static_cast<float>(x)* 0.1f,static_cast<float>(y)* 0.1f);
	int ThicknessOffset = FMath::RoundToInt(bedrockNoise * 2.5f);
	for (auto Layer : BiomeLayers)
	{		
		int baseThickness = Layer.Key;	
		int DynamicThickness = baseThickness + ThicknessOffset;
		if (DynamicThickness <= 0)
		{
			continue; 
		}
		for (int i = z; i > z-DynamicThickness; --i)
		{
			Data.SetBlock(x,y,i,Layer.Value);
		}
		z-=DynamicThickness;
	}	
}

