// Fill out your copyright notice in the Description page of Project Settings.


#include "CubeGenerator.h"

#include "GreedyMeshing.h"
#include "IImageWrapper.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Minecraft/BiomDataAsset.h"
#include "Minecraft/FastNoiseLite.h"
#include "Minecraft/FChunkBuildData.h"
#include "IImageWrapperModule.h"
#include "NoiseManagerSubSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Minecraft/MinecraftProceduralMeshComponent.h"

class UProceduralMeshComponent;
// Sets default values

ACubeGenerator::ACubeGenerator()
{
	PrimaryActorTick.bCanEverTick = true;	
	// Создаём корневой компонент
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
		
}


// Called when the game starts or when spawned
void ACubeGenerator::BeginPlay()
{
	Super::BeginPlay();
	NoiseManager = GetGameInstance()->GetSubsystem<UNoiseManagerSubSystem>();
	if (!NoiseManager)
		UKismetSystemLibrary::QuitGame(GetWorld(), UGameplayStatics::GetPlayerController(GetWorld(),0), EQuitPreference::Quit, false);	
	FastNoises = NoiseManager->GetNoises();
	LoadAllBioms();
	InitializeBiomeMap();
#ifdef  UE_EDITOR
	SaveLUTToXml();
	VisualizeBiomeLUT();
#endif
	
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
	ParallelFor(BiomesArraySize, [&](int32 i)
	{
		int32 tempIndex = i / 200;
		int32 humIndex = i % 200;
		int32 T = tempIndex - 100;
		int32 H = humIndex - 100;
		BiomesLUTArray[i]=CalculateBiomWeights(T,H);
	});
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

void ACubeGenerator::VisualizeBiomeLUT()
{
    int32 Size = 200;
    
    // 1. Создаем текстуру (для отображения в движке, если нужно)
    UTexture2D* DebugTexture = UTexture2D::CreateTransient(Size, Size, PF_B8G8R8A8);
    if (!DebugTexture) return;

    // Подготавливаем массив цветов для сохранения в файл
    TArray<FColor> OutPixels;
    OutPixels.SetNum(BiomesArraySize);

    // 2. Проходим по данным и формируем цвета
    for (int32 i = 0; i < BiomesArraySize; ++i)
    {
        FBiomLUTMap& LUTData = BiomesLUTArray[i];
        FColor PixelColor = FColor::Black;

        if (LUTData.Biome)
        {
            FString Name = LUTData.Biome->GetName();
            
            // Логика раскраски (можно расширить под твои биомы)
            if (Name.Contains(TEXT("Desert")))      PixelColor = FColor::Yellow;
            else if (Name.Contains(TEXT("Tundra")))  PixelColor = FColor::Blue;
            else if (Name.Contains(TEXT("Plains")))  PixelColor = FColor::Cyan;
            else if (Name.Contains(TEXT("Forest")))  PixelColor = FColor::Green;
            else if (Name.Contains(TEXT("Mountain"))) PixelColor = FColor::White;
            else PixelColor = FColor::Orange; // Для неизвестных биомов
        }
        else
        {
            // Если биом не определен, пусть будет серым
            PixelColor = FColor(50, 50, 50);
        }

        OutPixels[i] = PixelColor;
    }

    // 3. Записываем данные в Transient текстуру (для GPU)
    FTexture2DMipMap& Mip = DebugTexture->GetPlatformData()->Mips[0];
    void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
    FMemory::Memcpy(TextureData, OutPixels.GetData(), OutPixels.Num() * sizeof(FColor));
    Mip.BulkData.Unlock();
    DebugTexture->UpdateResource();

    // 4. Сохранение в PNG файл
    FString FilePath = FPaths::ProjectSavedDir() + TEXT("Screenshots/BiomeLUT.png");

    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

    // PNG ожидает RGBA или BGRA. FColor — это обычно BGRA в памяти Windows.
    if (ImageWrapper.IsValid() && ImageWrapper->SetRaw(OutPixels.GetData(), OutPixels.Num() * sizeof(FColor), Size, Size, ERGBFormat::BGRA, 8))
    {
        if (FFileHelper::SaveArrayToFile(ImageWrapper->GetCompressed(), *FilePath))
        {
            UE_LOG(LogTemp, Warning, TEXT("Картинка успешно сохранена в: %s"), *FilePath);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Не удалось записать файл на диск!"));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Biome LUT Visualization Complete. Generated 200x200 map."));
}

void ACubeGenerator::SaveLUTToXml()
{
	double StartTime = FPlatformTime::Seconds();

	// 1. Заголовок XML
	FString XmlContent = TEXT("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	XmlContent += TEXT("<BiomeLUT>\n");

	// Резервируем память, чтобы избежать частых переаллокаций строки (примерно 150 байт на запись)
	XmlContent.Reserve(BiomesArraySize * 150);

	for (int32 i = 0; i < BiomesArraySize; ++i)
	{
		const FBiomLUTMap& Data = BiomesLUTArray[i];
        
		int32 T = (i / 200) - 100;
		int32 H = (i % 200) - 100;
		FString BiomeName = Data.Biome ? Data.Biome->GetName() : TEXT("None");

		// Формируем узел для каждой ячейки
		XmlContent += FString::Printf(TEXT("  <Entry i=\"%d\">\n"), i);
		XmlContent += FString::Printf(TEXT("    <Temp>%d</Temp>\n"), T);
		XmlContent += FString::Printf(TEXT("    <Hum>%d</Hum>\n"), H);
		XmlContent += FString::Printf(TEXT("    <Biome>%s</Biome>\n"), *BiomeName);
		XmlContent += FString::Printf(TEXT("    <Scale>%.4f</Scale>\n"), Data.VerticalScale);
		XmlContent += FString::Printf(TEXT("    <Offset>%.4f</Offset>\n"), Data.HeightOffset);
		XmlContent += TEXT("  </Entry>\n");
	}

	XmlContent += TEXT("</BiomeLUT>");

	// 2. Сохраняем в папку Saved/Logs
	FString FilePath = FPaths::ProjectLogDir() + TEXT("BiomeLUT_Data.xml");
    
	if (FFileHelper::SaveStringToFile(XmlContent, *FilePath))
	{
		double EndTime = FPlatformTime::Seconds();
		UE_LOG(LogTemp, Warning, TEXT("XML saved (%.2f ms): %s"), (EndTime - StartTime) * 1000.0, *FilePath);
	}
}


float ACubeGenerator::GetHeightMask(int z, int minZ, int maxZ)
{
	if (z <= minZ || z >= maxZ) return 0.0f;

	float t = float(z - minZ) / float(maxZ - minZ);
	return 1.0f - t * t; // плавно затухает к поверхности
}

FInterpolatedBiomeData ACubeGenerator::GetInterpolatedLUTData(float T, float H) 
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

int ACubeGenerator::CalculateHeight(FNoises noises)
{
	
	float BaseHeight = ContinentalnessCurve->GetFloatValue(noises.Continentalness);
	float Erosion = (noises.Erosion + 1.0f) * 0.5;
	float Peaks = PeaksValleysCurve->GetFloatValue(noises.PeaksValleys);
	//Peaks = FMath::Pow(Peaks, Erosion);
	float Height = BaseHeight +	Peaks*Erosion;
	FInterpolatedBiomeData Biome = GetInterpolatedLUTData(noises.Temperature, noises.Humidity);
	Height =Height * Biome.VerticalScale +Biome.HeightOffset;	

	return FMath::Clamp(FMath::FloorToInt(Height), 1, CHUNK_Z - 2);
}

void ACubeGenerator::UpdateChunks(FVector coord)
{
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
			
			noises.PeaksValleys = FastNoises.PeaksValleysNoise.GetNoise((float)worldX,(float)worldY);
			noises.Continentalness = FastNoises.ContinentalnessNoise.GetNoise((float)worldX,(float)worldY);			
			noises.Bedrock = FastNoises.BedrockNoise.GetNoise((float)worldX,(float)worldY);
			noises.CavesRoom = FastNoises.CavesRoomNoise.GetNoise((float)worldX,(float)worldY);
			noises.CavesTunnel = FastNoises.CavesTunnelNoise.GetNoise((float)worldX,(float)worldY);
			noises.Erosion = FastNoises.ErosionNoise.GetNoise((float)worldX,(float)worldY);
			noises.Humidity = FastNoises.HumidityNoise.GetNoise((float)worldX,(float)worldY);
			noises.Temperature = FastNoises.TemperatureNoise.GetNoise((float)worldX,(float)worldY);
			int height = CalculateHeight(noises);
			if (!BiomesArray.IsEmpty()) 
				{
					GenerateSurfaceLayer(height,noises,Data, x, y);
				}
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

void ACubeGenerator::GenerateSurfaceLayer(int z, FNoises& noises,FChunkBuildData& Data,int x,int y)
{	
	auto LUTData = GetLUTData(noises.Temperature,noises.Humidity);
	auto BiomeLayers = LUTData.Biome->SurfaceLayers;
	for (auto Layer : BiomeLayers)
	{
		for (int i = z; i > z-Layer.Key; --i)
		{
			Data.SetBlock(x,y,z,Layer.Value);
		}
		z-=Layer.Key;
	}	
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
