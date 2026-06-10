// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MC_Overlay.h"
#include "ChunkGenerator.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Minecraft/MC_Pawn.h"

void UMC_Overlay::UpdateUI()
{	
	auto coord = MC_Pawn->GetPlayerVoxelPos();
	if (CurrentCoord==coord)
		return;
	for (auto Noise : NoisesMap)
	{
		auto res = TextBlocks.Find(Noise.Value.ToString());
		if (res && *res)
		{
			auto NoiseValue = Noise.Key->GetNoise((float)coord.X, (float)coord.Y);
			(*res)->SetText(FText::AsNumber(NoiseValue));
		}
	}
	FText PosText = FText::Format(
	NSLOCTEXT("MyNamespace", "PlayerPosKey", "X: {0} Y: {1} Z: {2}"), 
	FText::AsNumber(CurrentCoord.X), 
	FText::AsNumber(CurrentCoord.Y),
	FText::AsNumber(CurrentCoord.Z));
	PlayerPos->SetText(PosText);
	CurrentCoord=coord;	
	chunkCoord.x = FMath::FloorToInt(coord.X/CHUNK_X);
	chunkCoord.y = FMath::FloorToInt(coord.Y/CHUNK_Y);
	
	FText ChunkText = FText::Format(
	NSLOCTEXT("MyNamespace", "ChunkPosKey", "Chunk X: {0} Y: {1}"), 
	FText::AsNumber(chunkCoord.x), 
	FText::AsNumber(chunkCoord.y));
	ChunkPos->SetText(ChunkText);
}

void UMC_Overlay::NativeConstruct()
{
	APawn* PlayerPawn = GetOwningPlayerPawn();
	MC_Pawn = Cast<AMC_Pawn>(PlayerPawn);
	if (!MC_Pawn)
	{		
		FGenericPlatformMisc::RequestExit(false);
	}
	Super::NativeConstruct();
	if (!VerticalBox)
	{
		VerticalBox = WidgetTree->ConstructWidget<UVerticalBox>();
		WidgetTree->RootWidget = VerticalBox;
	}	
	NoiseManager = GetGameInstance()->GetSubsystem<UNoiseManagerSubSystem>();
	if (NoiseManager)
	{
		NoisesMap = NoiseManager->GetNoisesMap();
		if (!NoisesMap.IsEmpty())
		{
			for (auto Noise : NoisesMap)
			{
				UTextBlock* NoiseValueText = nullptr;
				AddTextRow(Noise.Value, NoiseValueText);
				TextBlocks.FindOrAdd(Noise.Value.ToString(), NoiseValueText);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MC_Overlay.cpp - can't find NoiseManager"));
	}
	
	AddTextRow(FText::FromString(TEXT("Position:")), PlayerPos);
	AddTextRow(FText::FromString(TEXT("Chunk:")), ChunkPos);
	
	GetWorld()->GetTimerManager().SetTimer(TimerUpdateNoises,this,&UMC_Overlay::UpdateUI,0.1f,true);
	UpdateUI();
}

void UMC_Overlay::AddTextRow(const FText& Label, UTextBlock*& OutTextBlock)
{
	if (!VerticalBox)
		return;

	FSlateColor Color(FLinearColor::Black);
	auto HorizBox = WidgetTree->ConstructWidget<UHorizontalBox>();
	
	UTextBlock* LabelText = WidgetTree->ConstructWidget<UTextBlock>();
	LabelText->SetText(Label);
	LabelText->SetColorAndOpacity(Color);
	
	OutTextBlock = WidgetTree->ConstructWidget<UTextBlock>();
	OutTextBlock->SetColorAndOpacity(Color);
	
	HorizBox->AddChild(LabelText);
	HorizBox->AddChild(OutTextBlock);
	VerticalBox->AddChild(HorizBox);
}

