#pragma once
#include <vector>
using std::vector;

class Utils
{
public:

	static int Clamp(int min, int val, int max);
	static bool GetGridTileValue(vector<vector<int>> grid, int xPos, int yPos, int* value);
	static void ReadImage(const char* fileName, unsigned int** pixels, unsigned int* width, unsigned int* height, unsigned int* bytesPerPixel);
};