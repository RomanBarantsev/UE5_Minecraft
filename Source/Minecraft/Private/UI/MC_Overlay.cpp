// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MC_Overlay.h"

#include <string>

#include "CubeGenerator.h"
#include "UnrealWidgetFwd.h"
#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"
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
	FSlateColor Color(FLinearColor::Black);
	if (NoiseManager)
	{
		NoisesMap = NoiseManager->GetNoisesMap();
		if (!NoisesMap.IsEmpty())
		{
			for (auto Noise : NoisesMap)
			{				
				UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>();	
				Text->SetColorAndOpacity(Color);
				UTextBlock* NoiseName = WidgetTree->ConstructWidget<UTextBlock>();
				NoiseName->SetText(Noise.Value);
				NoiseName->SetColorAndOpacity(Color);
				TextBlocks.FindOrAdd(Noise.Value.ToString(),Text);
				auto HorizBox = WidgetTree->ConstructWidget<UHorizontalBox>();
				HorizBox->AddChild(NoiseName);
				HorizBox->AddChild(Text);
				VerticalBox->AddChild(HorizBox);				
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MC_Overlay.cpp - can't find NoiseManager"));
	}
	
	auto HorizBox = WidgetTree->ConstructWidget<UHorizontalBox>();
	PlayerPos = WidgetTree->ConstructWidget<UTextBlock>();
	PlayerPos->SetColorAndOpacity(Color);
	HorizBox->AddChild(PlayerPos);
	VerticalBox->AddChild(HorizBox);
	
	GetWorld()->GetTimerManager().SetTimer(TimerUpdateNoises,this,&UMC_Overlay::UpdateUI,0.1f,true);
	UpdateUI();
}
