#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerlinNoise3D.generated.h"

/**
 * Простая статическая реализация Perlin 3D.
 * Возвращает значения в прибл. диапазоне [-1, 1].
 */
UCLASS()
class MINECRAFT_API UPerlinNoise3D : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Noise")
	static float Perlin3D(float X, float Y, float Z, float Scale = 0.01f, int32 Octaves = 4, float Persistence = 0.5f, float Lacunarity = 2.0f, int32 Seed = 0);

private:
	static float Fade(float T);
	static float Lerp(float A, float B, float T);
	static float Grad(int32 Hash, float X, float Y, float Z);
};
