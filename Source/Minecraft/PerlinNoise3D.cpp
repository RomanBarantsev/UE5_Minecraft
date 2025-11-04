#include "PerlinNoise3D.h"
#include "Math/UnrealMathUtility.h"


float UPerlinNoise3D::Grad(int32 Hash, float X, float Y, float Z)
{
    int32 h = Hash & 15;
    float u = h < 8 ? X : Y;
    float v = h < 4 ? Y : (h == 12 || h == 14 ? X : Z);
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

float UPerlinNoise3D::Perlin3D(float X, float Y, float Z, float Scale, int32 Octaves, float Persistence, float Lacunarity, int32 Seed)
{
    uint8 Perm[512];
    if (Seed == 0)
    {
        for (int i = 0; i < 256; ++i) Perm[i] = PermBase[i], Perm[i + 256] = PermBase[i];
    }
    else
    {
        // Simple pseudo shuffle using seed (not cryptographically strong, but OK for procedural noise)
        TArray<uint8> temp;
        temp.Reserve(256);
        for (int i = 0; i < 256; ++i) temp.Add(PermBase[i]);
        // Fisher-Yates shuffle with seed
        uint32 s = Seed;
        for (int i = 255; i > 0; --i)
        {
            s = (s * 9301 + 49297) % 233280;
            int j = s % (i + 1);
            uint8 t = temp[i];
            temp[i] = temp[j];
            temp[j] = t;
        }
        for (int i = 0; i < 256; ++i) Perm[i] = temp[i], Perm[i + 256] = temp[i];
    }

    float total = 0.0f;
    float frequency = Scale;
    float amplitude = 1.0f;
    float maxValue = 0.0f;

    for (int32 o = 0; o < Octaves; ++o)
    {
        float xf = X * frequency;
        float yf = Y * frequency;
        float zf = Z * frequency;

        int32 xi = ((int32)FMath::FloorToInt(xf)) & 255;
        int32 yi = ((int32)FMath::FloorToInt(yf)) & 255;
        int32 zi = ((int32)FMath::FloorToInt(zf)) & 255;

        float x_frac = xf - FMath::FloorToFloat(xf);
        float y_frac = yf - FMath::FloorToFloat(yf);
        float z_frac = zf - FMath::FloorToFloat(zf);

        float u = UPerlinNoise2D::Fade(x_frac);
        float v = Fade(y_frac);
        float w = Fade(z_frac);

        int32 A = Perm[xi] + yi;
        int32 AA = Perm[A] + zi;
        int32 AB = Perm[A + 1] + zi;
        int32 B = Perm[xi + 1] + yi;
        int32 BA = Perm[B] + zi;
        int32 BB = Perm[B + 1] + zi;

        float x1 = Lerp(
            Lerp(Grad(Perm[AA], x_frac, y_frac, z_frac), Grad(Perm[BA], x_frac - 1, y_frac, z_frac), u),
            Lerp(Grad(Perm[AB], x_frac, y_frac - 1, z_frac), Grad(Perm[BB], x_frac - 1, y_frac - 1, z_frac), u),
            v);

        float x2 = Lerp(
            Lerp(Grad(Perm[AA + 1], x_frac, y_frac, z_frac - 1), Grad(Perm[BA + 1], x_frac - 1, y_frac, z_frac - 1), u),
            Lerp(Grad(Perm[AB + 1], x_frac, y_frac - 1, z_frac - 1), Grad(Perm[BB + 1], x_frac - 1, y_frac - 1, z_frac - 1), u),
            v);

        float res = Lerp(x1, x2, w);

        total += res * amplitude;
        maxValue += amplitude;

        amplitude *= Persistence;
        frequency *= Lacunarity;
    }

    // Normalize
    return total / maxValue;
}
