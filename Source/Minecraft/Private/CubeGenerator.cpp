// Fill out your copyright notice in the Description page of Project Settings.


#include "CubeGenerator.h"

#include "BreakableCube.h"
#include "GreedyMeshing.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Minecraft/BiomDataAsset.h"
#include "Minecraft/FastNoiseLite.h"
#include "Minecraft/FChunkBuildData.h"
#include "Minecraft/MinecraftProceduralMeshComponent.h"

class UProceduralMeshComponent;
// Sets default values

ACubeGenerator::ACubeGenerator()
{
	PrimaryActorTick.bCanEverTick = true;	
	// Создаём корневой компонент
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
		
}

void ACubeGenerator::LoadLayers()
{
	CavesRoomParams.rowName="CavesRoom";
	FastNoises.Add(&CavesRoomNoise,FText::FromName(CavesRoomParams.rowName));
	CavesTunnelParams.rowName="CavesTunnel";
	FastNoises.Add(&CavesTunnelNoise,FText::FromName(CavesTunnelParams.rowName));	
	
	ContParams.rowName="Continentalness";
	FastNoises.Add(&ContNoise,FText::FromName(ContParams.rowName));
	PeaksValleysParams.rowName="PeaksValleys";
	FastNoises.Add(&PeaksValleysNoise,FText::FromName(PeaksValleysParams.rowName));
	BedrockParams.rowName="Bedrock";
	FastNoises.Add(&BedrockNoise,FText::FromName(BedrockParams.rowName));
	BedrockParams.rowName="Erosion";
	FastNoises.Add(&ErosionNoise,FText::FromName(ErosionParams.rowName));
	
	LoadNoiseParams(CavesRoomNoise,CavesRoomParams);
	LoadNoiseParams(CavesTunnelNoise,CavesTunnelParams);
	
	LoadNoiseParams(ContNoise,ContParams);
	LoadNoiseParams(PeaksValleysNoise,PeaksValleysParams);
	LoadNoiseParams(ErosionNoise,ErosionParams);
	
	LoadNoiseParams(BedrockNoise,BedrockParams);
	
	SetNoiseParams(CavesRoomNoise,CavesRoomParams, FastNoiseLite::NoiseType_Perlin);
	SetNoiseParams(CavesTunnelNoise,CavesTunnelParams, FastNoiseLite::NoiseType_Perlin);	
	
	SetNoiseParams(ContNoise,ContParams, FastNoiseLite::NoiseType_Perlin);
	SetNoiseParams(PeaksValleysNoise,PeaksValleysParams, FastNoiseLite::NoiseType_Perlin);
	SetNoiseParams(ErosionNoise,ErosionParams, FastNoiseLite::NoiseType_Perlin);
	
	SetNoiseParams(BedrockNoise,BedrockParams, FastNoiseLite::NoiseType_Perlin);	
}

// Called when the game starts or when spawned
void ACubeGenerator::BeginPlay()
{
	Super::BeginPlay();
	LoadAllBioms();
	InitializeBiomeMap();
	LoadLayers();
	//time start
	UpdateChunks(FVector(0.0f,0.0f,0.0f));//start pos
	//time end
}

void ACubeGenerator::Tick(float DeltaSeconds)
{	
	Super::Tick(DeltaSeconds);	
	
	TArray<FAsyncGenerationResult> Results;
	Results.SetNum(CoordsToGenerate.Num());
	if (CoordsToGenerate.IsEmpty())
		return;
	
	TArray<FChunkCoord> GenerateArray;
	for (int i = 0; i < OperationPerTick; i++)
	{
		GenerateArray.Push(CoordsToGenerate.Pop());
	}
	AsyncChunkCreate(GenerateArray,Results);
	
}

void ACubeGenerator::LoadAllBioms()
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

void ACubeGenerator::InitializeBiomeMap()
{		
	BiomesLUTArray.SetNumUninitialized(BiomesArraySize);
	double StartTime = FPlatformTime::Seconds();
	for (int temp = -100; temp < 100; ++temp)
	{
		for (int hum = -100; hum < 100; ++hum)
		{				
			GetLUTData(temp,hum)=CalculateBiomWeights(temp,hum);
		}
	}
	double EndTime = FPlatformTime::Seconds();
	double TimePassedMs = (EndTime - StartTime) * 1000.0; // Переводим в миллисекунды

	// Выводим результат в Output Log
	UE_LOG(LogTemp, Warning, TEXT("InitializeBiomeMap (Parallel) took: %f ms"), TimePassedMs);
}

FBiomLUTMap ACubeGenerator::CalculateBiomWeights(int T, int H)
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
			return FBiomLUTMap{Biome->HeightModifier,Biome->VerticalScale,Biome};
		}		
		float W = 1.0f / (Weight * Weight);
		WeightedScaleSum += Biome->VerticalScale * W;
		WeightedOffsetSum += Biome->HeightModifier * W;
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

void ACubeGenerator::SetNoiseParams(FastNoiseLite& Noise,FNoisesParams params,FastNoiseLite::NoiseType noiseType)
{
	Noise.SetSeed(Seed);           // Seed для разнообразия
	Noise.SetFrequency(params.Scale);   // Масштаб шума (аналог Scale)
	Noise.SetFractalOctaves(params.Octaves);    // 4 октавы для детализации
	Noise.SetFractalGain(params.Persistence);   // Затухание амплитуды (Persistence)
	Noise.SetFractalLacunarity(params.Lacunarity); // Рост частоты (Lacunarity)
	Noise.SetNoiseType(noiseType); // Тип шума — Perlin
	Noise.SetFractalType(FastNoiseLite::FractalType_FBm);
}

float ACubeGenerator::GetHeightMask(int z, int minZ, int maxZ)
{
	if (z <= minZ || z >= maxZ) return 0.0f;

	float t = float(z - minZ) / float(maxZ - minZ);
	return 1.0f - t * t; // плавно затухает к поверхности
}

int ACubeGenerator::mapHeight(FNoises noises)
{		
	float cont = ContinentalnessCurve->GetFloatValue(noises.ContNoise);
	float baseVariation = noises.PeaksValleys * 4.0f;
	float height = cont + baseVariation;
	
	float erosion = ErosionCurve->GetFloatValue(noises.Erosion);
	float mountainHeight = noises.PeaksValleys * erosion * 32.0f;
	float mountainWeight = FMath::Max(0.0f, noises.ContNoise); 
	float finalHeight = height + (mountainHeight * FMath::Pow(mountainWeight, 2.0f));
	//float height = PeaksValleysCurve->GetFloatValue(noises.ContNoise+noises.PeaksValleys+noises.PeaksValleys);
	return  FMath::Clamp((int)finalHeight, 1, CHUNK_Z - 2);
}


void ACubeGenerator::LoadNoiseParams(FastNoiseLite& noise, FNoisesParams& params)
{
	auto Row = PerlinNoiseTable->FindRow<FPerlinNoiseBiom>(params.rowName,"name");	
	if (Row)
	{
		params.Scale = Row->Scale;
		params.Octaves = Row->Octaves;
		params.Persistence = Row->Persistence;
		params.Lacunarity = Row->Lacunarity;
	}
}


void ACubeGenerator::UpdateChunks(FVector coord)
{
	//PrintNoises(FMath::Abs(coord.X),FMath::Abs(coord.Y),FMath::Abs(coord.Z));
	double TStart = FPlatformTime::Seconds();
	if (!ChunksForRemote.IsEmpty())
	{
		for (auto Chunk : ChunksForRemote)
		{
			RemoveChunk(Chunk.Key);
		}
	}
	coord/=BLOCK_SIZE;
	FChunkCoord chunkCoord;
	chunkCoord.x = FMath::FloorToInt(coord.X/CHUNK_X);
	chunkCoord.y = FMath::FloorToInt(coord.Y/CHUNK_Y);
	UE_LOG(LogTemp, Warning, TEXT("chunkCoord x %d y %d"),chunkCoord.x,chunkCoord.y);
	
	if (FMath::Abs(currentChunkPosition.x - chunkCoord.x) > chunkDelimiter
	 || FMath::Abs(currentChunkPosition.y - chunkCoord.y) > chunkDelimiter
											|| currentChunkPosition.startPos)
	{
		currentChunkPosition.startPos=false;
		currentChunkPosition.x = FMath::FloorToInt((float)chunkCoord.x / chunkDelimiter) * chunkDelimiter;
		currentChunkPosition.y = FMath::FloorToInt((float)chunkCoord.y / chunkDelimiter) * chunkDelimiter;
		UE_LOG(LogTemp, Warning, TEXT("currentChunkPosition x %d y %d"),currentChunkPosition.x,currentChunkPosition.y);
		ChunksForRemote = Chunks;
		Chunks.Empty();		
		
		/*CoordsToGenerate.Add(FChunkCoord{0,0});				
		Chunks.Add(FChunkCoord{0,0},nullptr);*/
		
		for (int x  = chunkCoord.x-chunkDelimiter*2; x < chunkCoord.x+chunkDelimiter*2; ++x)
		{
			for (int y = chunkCoord.y-chunkDelimiter*2; y < chunkCoord.y+chunkDelimiter*2; ++y)
			{
				FChunkCoord newCoord{x,y};
				if (!ChunksForRemote.Contains(newCoord))
				{	
					CoordsToGenerate.Add(newCoord);				
					Chunks.Add(newCoord,nullptr);
				}
				else
				{
					Chunks.Add(newCoord,ChunksForRemote[newCoord]);
					ChunksForRemote.Remove(newCoord);
				}				
			}
		}	
	}
	double TEnd = FPlatformTime::Seconds();
//UE_LOG(LogTemp, Warning, TEXT("Building chunk took: %.2f ms"), (TEnd - TStart) * 1000.0f);
}

int ACubeGenerator::GetSurfaceHigh(FVector vec)
{
	int XChunkCoord = FMath::FloorToInt(vec.X / BLOCK_SIZE);
	int YChunkCoord = FMath::FloorToInt(vec.Y / BLOCK_SIZE);
	int XChunk = XChunkCoord/CHUNKSIZE_WIDE;
	int YChunk = YChunkCoord/CHUNKSIZE_WIDE;
	FChunkCoord Coord{XChunk,YChunk};
	if (!Chunks.Contains(Coord))
		return 0;
	auto chunk = Chunks[Coord];
	if (chunk==nullptr)
		return 0;
	return chunk->GetSurfaceHeight(XChunkCoord,YChunkCoord);
}

void ACubeGenerator::RemoveChunk(FChunkCoord coord)
{
	if (MeshesMap.Contains(coord))
	{
		UMinecraftProceduralMeshComponent* Mesh = MeshesMap[coord];
		Mesh->ClearAllMeshSections();
		MeshesMap.Remove(coord);
		FreeProcMeshes.Add(Mesh);
		MeshToChunkMap.Remove(Mesh);
		//FreeChunkцs.Add(Chunks[coord].Get());	
	}	
}

void ACubeGenerator::GenerateChunkData(FChunkBuildData& Data)
{
	for (int x = 0; x < CHUNK_X; x++)
	{
		for (int y = 0; y < CHUNK_Y; y++)
		{			
			int worldX = Data.Coord.x*CHUNK_X+x;
			int worldY = Data.Coord.y*CHUNK_Y+y;
			FNoises noises;
			noises.PeaksValleys = PeaksValleysNoise.GetNoise((float)worldX,(float)worldY);
			noises.ContNoise = ContNoise.GetNoise((float)worldX,(float)worldY);			
			noises.Bedrock = BedrockNoise.GetNoise((float)worldX,(float)worldY);
			noises.CavesRoom = CavesRoomNoise.GetNoise((float)worldX,(float)worldY);
			noises.CavesTunnel = CavesTunnelNoise.GetNoise((float)worldX,(float)worldY);
			noises.Erosion = CavesTunnelNoise.GetNoise((float)worldX,(float)worldY);
			int height = mapHeight(noises);
			//GenerateSurfaceLayer(height,noises,Data, x, y);
			Data.SetSurfaceHeight(x,y,height);
		}
	}
	GenerateCaves(Data);
	Data.Fill();
}

void ACubeGenerator::GenerateCaves(FChunkBuildData& Data)
{
	for (int xPerlin = Data.Coord.x*CHUNK_X, x =0; xPerlin <Data.Coord.x*CHUNK_X+CHUNK_X; xPerlin++,x++)
	{
		float fx = static_cast<float>(xPerlin);
		for (int yPerlin = Data.Coord.y*CHUNK_X, y=0; yPerlin <Data.Coord.y*CHUNK_X+CHUNK_X; yPerlin++,y++)
		{
			int index2D = x + y * CHUNK_X;
			float fy = static_cast<float>(yPerlin);
			float bedrockNoise = BedrockNoise.GetNoise(fx,fy);
			int bedrockTop = BEDROCK_BASE + static_cast<int>(((bedrockNoise + 1.0f) * 0.5f * BEDROCK_HEIGHT));
			int height = Data.GetSurfaceHeight(x,y);
			for (int z = 0; z < height; ++z)
			{	
				if (z<bedrockTop || z==0)
				{					
					Data.SetBlock(x,y,z,BlockType::Cobblestone);
					continue;
				}
				float Room = CavesRoomNoise.GetNoise(fx, fy, static_cast<float>(z));
				float Tunnel  = CavesTunnelNoise.GetNoise(fx, fy, static_cast<float>(z));
				float Mask = GetHeightMask(z, 1, height - 6);
				float Density =
					Tunnel * 1.2f +     // тоннели важнее
					Room * 0.8f;        // залы реже
				Density *= Mask;
				if (Density > 0.25f)
				{
					Data.SetBlock(x,y,z,BlockType::Air);
				}
			}
		}
	}
}

int ACubeGenerator::CalculateBlockHeight(float value)
{
	value=FMath::Abs(fmod(value,0.5));
	int val = static_cast<int>(value/delimiterChunkHeight);
	return val==0 ? 1 : val;
}

void ACubeGenerator::GenerateSurfaceLayer(int z, FNoises& noises,FChunkBuildData& Data,int x,int y)
{	
	
}

void ACubeGenerator::FinalizeChunk(FChunkBuildData& Data,FGreedyMeshing& GreedyMeshing)
{
	UMinecraftProceduralMeshComponent* ProcMesh;
	if (FreeProcMeshes.IsEmpty())
	{
		ProcMesh = NewObject<UMinecraftProceduralMeshComponent>(this);
		ProcMesh->RegisterComponent();
		ProcMesh->AttachToComponent(RootComponent,FAttachmentTransformRules::KeepRelativeTransform);
		ProcMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		ProcMesh->bUseAsyncCooking = true;
	}
	else
	{
		ProcMesh = FreeProcMeshes.Pop();		
	}
	ProcMesh->SetRelativeLocation(FVector(Data.Coord.x*CHUNK_X*BLOCK_SIZE, Data.Coord.y*CHUNK_X*BLOCK_SIZE, 0));
	GreedyMeshing.CreateMesh(*ProcMesh,Mat);
	MeshesMap.Add(Data.Coord,ProcMesh);
	MeshToChunkMap.Add(ProcMesh,&Data);
}

void ACubeGenerator::AsyncChunkCreate(TArray<FChunkCoord>& GenerateArray,TArray<FAsyncGenerationResult>& Results)
{	
	TWeakObjectPtr<ACubeGenerator> WeakThis = this;
	Async(EAsyncExecution::ThreadPool,[WeakThis,GenerateArray,Results]() mutable
	{
		ParallelFor(GenerateArray.Num(),[&](int32 i)
		{
			if (!WeakThis.IsValid())
				return;
			FChunkCoord CurrentCoord = GenerateArray[i];
			
			auto Data = MakeShared<FChunkBuildData>();
			Data->Coord = CurrentCoord;
			
			WeakThis->GenerateChunkData(*Data);
			
			auto Mesher = MakeShared<FGreedyMeshing>();
			Mesher->BuildGreedyMesh(&Data.Get());
			Results[i] = {CurrentCoord,Data,Mesher};
		});
		AsyncTask(ENamedThreads::GameThread, [WeakThis, Results]() {
		if (WeakThis.IsValid()) {
			for (const auto& Res : Results) {
				if (Res.BuildData.IsValid()) {
					if (WeakThis->Chunks.Contains(Res.Coord))
					{
						WeakThis->Chunks[Res.Coord] = Res.BuildData;
						WeakThis->FinalizeChunk(*Res.BuildData, *Res.GreedyMeshing);
					}					
				}
			}
		}
		});
	});
}

void ACubeGenerator::RemoveBlock(FHitResult Hit, UMinecraftProceduralMeshComponent* mesh)
{
	double TStart = FPlatformTime::Seconds();
	FVector CorrectWorldPos =Hit.ImpactPoint - Hit.ImpactNormal * EPS;
	FVector LocalPos =mesh->GetComponentTransform().InverseTransformPosition(CorrectWorldPos);
	int X = FMath::FloorToInt(LocalPos.X / BLOCK_SIZE);
	int Y = FMath::FloorToInt(LocalPos.Y / BLOCK_SIZE);
	int Z = FMath::FloorToInt(LocalPos.Z / BLOCK_SIZE);
	LocalPos/=BLOCK_SIZE;
	
	FChunkCoord chunkCoord;
	chunkCoord.x = FMath::FloorToInt(LocalPos.X/CHUNK_X);
	chunkCoord.y = FMath::FloorToInt(LocalPos.Y/CHUNK_Y);
	auto Chunk = MeshToChunkMap[mesh];
	
	BlockType CurrentBlockType = Chunk->GetBlock(X,Y,Z);
	Chunk->SetBlock(X,Y,Z,BlockType::Air);
	mesh->ClearAllMeshSections();
	mesh->bUseAsyncCooking = true; //????
	FGreedyMeshing GreedyMeshing;	
	GreedyMeshing.BuildGreedyMesh(Chunk);
	GreedyMeshing.CreateMesh(*mesh,Mat);
	FVector CubeLocation = FVector(mesh->GetComponentLocation().X+X*BLOCK_SIZE+BLOCK_SIZE/2,mesh->GetComponentLocation().Y+Y*BLOCK_SIZE+BLOCK_SIZE/2,mesh->GetComponentLocation().Z+Z*BLOCK_SIZE+BLOCK_SIZE/2);
	FActorSpawnParameters spawnParams;
	//TODO make a pool
	/*auto Actor = GetWorld()->SpawnActor<AActor>(DestroyedBlockClass,CubeLocation,FRotator::ZeroRotator,spawnParams);
	ABreakableCube* Cube = Cast<ABreakableCube>(Actor);
	if (Cube)
	{
		Cube->FractureNow(CurrentBlockType,Hit);
	}*/
	double TEnd = FPlatformTime::Seconds();
	//UE_LOG(LogTemp, Warning, TEXT("ReBuilding chunk took: %.2f ms"), (TEnd - TStart) * 1000.0f);
}

TMap<FastNoiseLite*, FText> ACubeGenerator::GetFastNoises()
{
	return FastNoises;
}

void ACubeGenerator::PrintNoises(int x,int y,int z)
{
	for (auto Noise : FastNoises)
	{
		UE_LOG(LogTemp, Warning, TEXT("Noise: %s value: %f"),*Noise.Value.ToString(),Noise.Key->GetNoise((float)x,(float)y,(float)z));
	}
}
