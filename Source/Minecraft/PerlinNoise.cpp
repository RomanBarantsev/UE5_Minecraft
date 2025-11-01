#include "PerlinNoise.h"

PerlinNoise::vector2 PerlinNoise::randomGradient(int ix, int iy)
{
	// No precomputed gradients mean this works for any number of grid coordinates
	const unsigned w = 8 * sizeof(unsigned);
	const unsigned s = w / 2;
	unsigned a = ix, b = iy;
	a *= 3284157443;

	b ^= (a << s) | (a >> (w - s));
	b *= 1911520717;

	b ^= (a << s) | (a >> (w - s));
	a *= 2048419325;
	float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]

	// Create the vector from the angle
	vector2 v;
	v.x = sin(random);
	v.y = cos(random);

	return v;
}

float PerlinNoise::dotGridGradient(int ix, int iy, float x, float y)
{	
	// Get gradient from integer coordinates
	vector2 gradient = randomGradient(ix, iy);

	// Compute the distance vector
	float dx = x - (float)ix;
	float dy = y - (float)iy;

	// Compute the dot-product
	return (dx * gradient.x + dy * gradient.y);
}

float PerlinNoise::perlin(float x, float y)
{
	// Determine grid cell corner coordinates
	int x0 = (int)x;
	int y0 = (int)y;
	int x1 = x0 + 1;
	int y1 = y0 + 1;

	// Compute Interpolation weights
	float sx = x - (float)x0;
	float sy = y - (float)y0;

	// Compute and interpolate top two corners
	float n0 = dotGridGradient(x0, y0, x, y);
	float n1 = dotGridGradient(x1, y0, x, y);
	float ix0 = interpolate(n0, n1, sx);

	// Compute and interpolate bottom two corners
	n0 = dotGridGradient(x0, y1, x, y);
	n1 = dotGridGradient(x1, y1, x, y);
	float ix1 = interpolate(n0, n1, sx);

	// Final step: interpolate between the two previously interpolated values, now in y
	float value = interpolate(ix0, ix1, sy);

	return value;
}
std::vector<std::vector<int>> PerlinNoise::GetMatrix()
{
	const int windowWidth = sizeX;
	const int windowHeight = sizeY;
	const int GRID_SIZE = 400;
	std::vector<std::vector<int>> grid(windowWidth, std::vector<int>(windowHeight, 0));
	for (int x = 0; x < windowWidth; x++)
	{
		for (int y = 0; y < windowHeight; y++)
		{
			int index = (y * windowWidth + x) * 4;
			float val = 0;
			float freq = 1;
			float amp = 1;
			for (int i = 0; i < 12; i++)
			{
				val += perlin(x * freq / GRID_SIZE, y * freq / GRID_SIZE) * amp;
				freq *= 2;
				amp /= 2;
			}
			// Contrast
			val *= 1.2;

			// Clipping
			if (val > 1.0f)
				val = 1.0f;
			else if (val < -1.0f)
				val = -1.0f;
			// Convert 1 to -1 into 255 to 0
			int color = (int)(((val + 1.0f) * 0.5f) * 255);
			grid[x][y]=color;
		}
	}
	return grid;
}
