// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NoiseManagerSubSystem.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "MC_Overlay.generated.h"

class AMC_Pawn;
class FastNoiseLite;
class UVerticalBox;
/**
 * 
 */
UCLASS()
class MINECRAFT_API UMC_Overlay : public UUserWidget
{
	GENERATED_BODY()
	UPROPERTY(meta=(BindWidget));
	UVerticalBox* VerticalBox;
	FTimerDynamicDelegate Delegate;
	UFUNCTION()
	void UpdateUI();
	virtual void NativeConstruct() override;
	TMap<FastNoiseLite*,FText> NoisesMap;
	FTimerHandle TimerUpdateNoises;
	UPROPERTY()
	TMap<FString,UTextBlock*> TextBlocks;
	UPROPERTY()
	UNoiseManagerSubSystem* NoiseManager;
	UPROPERTY()
	AMC_Pawn* MC_Pawn;
	UTextBlock* PlayerPos;
	FVector CurrentCoord;
};
