#include "PerlinNoise3D.h"
#include "Math/UnrealMathUtility.h"

float UPerlinNoise3D::Fade(float T)
{
    return T * T * T * (T * (T * 6 - 15) + 10);
}

float UPerlinNoise3D::Lerp(float A, float B, float T)
{
    return A + T * (B - A);
}

float UPerlinNoise3D::Grad(int32 Hash, float X, float Y, float Z)
{
    int32 h = Hash & 15;
    float u = h < 8 ? X : Y;
    float v = h < 4 ? Y : (h == 12 || h == 14 ? X : Z);
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

float UPerlinNoise3D::Perlin3D(float X, float Y, float Z, float Scale, int32 Octaves, float Persistence, float Lacunarity, int32 Seed)
{
    // Classic permutation table (256 values) duplicated to 512
    static const uint8 PermBase[256] = {
        151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,
        140,36,103,30,69,142,8,99,37,240,21,10,23,190, 6,148,
        247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,
        57,177,33,88,237,149,56,87,174,20,125,136,171,168, 68,175,
        74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,
        60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,
        65,25,63,161, 1,216,80,73,209,76,132,187,208,89,18,169,
        200,196,135,130,116,188,159,86,164,100,109,198,173,186, 3,64,
        52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,
        207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,
        119,248,152, 2,44,154,163,70,221,153,101,155,167, 43,172,9,
        129,22,39,253, 19,98,108,110,79,113,224,232,178,185,112,104,
        218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,
        81,51,145,235,249,14,239,107,49,192,214,31,181,199,106,157,
        184,84,204,176,115,121,50,45,127, 4,150,254,138,236,205,93,
        222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
    };

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
        int32 s = Seed;
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

        float u = Fade(x_frac);
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
