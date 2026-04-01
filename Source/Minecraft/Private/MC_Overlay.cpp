// Fill out your copyright notice in the Description page of Project Settings.


#include "MC_Overlay.h"

#include "CubeGenerator.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"

void UMC_Overlay::UpdateUI()
{
	
}

void UMC_Overlay::SetCoordinates(int x, int y, int z)
{
	
}

void UMC_Overlay::NativeConstruct()
{
	Super::NativeConstruct();
	AActor* Actor = UGameplayStatics::GetActorOfClass(GetWorld(),TSubclassOf<class ACubeGenerator>());
	ACubeGenerator* CubeGenerator = Cast<ACubeGenerator>(Actor);
	if (CubeGenerator)
	{
		NoisesMap = CubeGenerator->GetFastNoises();
		if (!NoisesMap.IsEmpty())
		{
			for (auto Noise : NoisesMap)
			{
				UTextBlock* Text = CreateDefaultSubobject<UTextBlock>("Text");
				UTextBlock* TextName = CreateDefaultSubobject<UTextBlock>("Text");
				TextBlocks.Add(Text);
				UHorizontalBox* HorizontalBox = CreateDefaultSubobject<UHorizontalBox>("HorizontalBox");
				HorizontalBox->AddChild(TextName);
				HorizontalBox->AddChild(Text);
				VerticalBox->AddChild(HorizontalBox);			
			}
		}
	}
	GetWorld()->GetTimerManager().SetTimer(TimerUpdateNoises,this,&UMC_Overlay::UpdateUI,1.0f,true);	
}
