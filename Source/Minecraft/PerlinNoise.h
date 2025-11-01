#pragma once
#include <vector>

class PerlinNoise
{
private:
	int sizeX;
	int sizeY;
	typedef struct {
		float x, y;
	} vector2;

	vector2 randomGradient(int ix, int iy);
	// Computes the dot product of the distance and gradient vectors.
	float dotGridGradient(int ix, int iy, float x, float y);

	float interpolate(float a0, float a1, float w)
	{
		return (a1 - a0) * (3.0 - w * 2.0) * w * w + a0;
	}
	float perlin(float x, float y);

public:
	PerlinNoise(int x,int y) : sizeX(x), sizeY(y){};
	std::vector<std::vector<int>> GetMatrix();
};
