// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "MC_Overlay.generated.h"

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
	void SetCoordinates(int x, int y,int z);
	virtual void NativeConstruct() override;
	TMap<FastNoiseLite*,FText> NoisesMap;
	FTimerHandle TimerUpdateNoises;
	TArray<UTextBlock*> TextBlocks;
};
