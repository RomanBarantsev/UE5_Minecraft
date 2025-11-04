// Fill out your copyright notice in the Description page of Project Settings.


#include "PerlinNoise2D.h"
#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <random>


float UPerlinNoise2D::Fade(float T)
{
	return T * T * T * (T * (T * 6 - 15) + 10);
}


float UPerlinNoise2D::Lerp(float A, float B, float T)
{
	return A + T * (B - A);
}


float UPerlinNoise2D::Grad(int32 Hash, float X, float Y)
{
    int32 h = Hash & 3; // 4 directions
    float u = h & 1 ? -X : X;
    float v = h & 2 ? -Y : Y;
    return u + v;
}

float UPerlinNoise2D::Perlin2D(float X, float Y, float Scale, int32 Octaves, float Persistence, float Lacunarity, int32 Seed)
{
    uint8 Perm[512];
    if (Seed == 0)
    {
        for (int i = 0; i < 256; ++i)
        {
            Perm[i] = PermBase[i];
            Perm[i + 256] = PermBase[i];
        }
    }
    else
    {
        // Shuffle permutation table based on seed
        TArray<uint8> Temp;
        Temp.Reserve(256);
        for (int i = 0; i < 256; ++i)
            Temp.Add(PermBase[i]);

        uint32 s = Seed;
        for (int i = 255; i > 0; --i)
        {
            s = (s * 1664525u + 1013904223u); // более устойчивый LCG
            int j = s % (i + 1);
            uint8 t = Temp[i];
            Temp[i] = Temp[j];
            Temp[j] = t;
        }

        for (int i = 0; i < 256; ++i)
        {
            Perm[i] = Temp[i];
            Perm[i + 256] = Temp[i];
        }
    }

    float total = 0.0f;
    float frequency = Scale;
    float amplitude = 1.0f;
    float maxValue = 0.0f;

    for (int32 o = 0; o < Octaves; ++o)
    {
        float xf = X * frequency;
        float yf = Y * frequency;

        int32 xi = FMath::FloorToInt(xf) & 255;
        int32 yi = FMath::FloorToInt(yf) & 255;

        float x_frac = xf - FMath::FloorToFloat(xf);
        float y_frac = yf - FMath::FloorToFloat(yf);

        float u = Fade(x_frac);
        float v = Fade(y_frac);

        int32 A = Perm[xi] + yi;
        int32 B = Perm[xi + 1] + yi;

        float x1 = Lerp(
            Grad(Perm[A], x_frac, y_frac),
            Grad(Perm[B], x_frac - 1, y_frac),
            u
        );

        float x2 = Lerp(
            Grad(Perm[A + 1], x_frac, y_frac - 1),
            Grad(Perm[B + 1], x_frac - 1, y_frac - 1),
            u
        );

        float res = Lerp(x1, x2, v);

        total += res * amplitude;
        maxValue += amplitude;

        amplitude *= Persistence;
        frequency *= Lacunarity;
    }

    return total / maxValue;
}