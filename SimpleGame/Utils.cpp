#include "Utils.h"
#include <cstdio>
#include <corecrt_malloc.h>
#include <cmath>
#include <iostream>
#define DATA_OFFSET_OFFSET 0x000A
#define WIDTH_OFFSET 0x0012
#define HEIGHT_OFFSET 0x0016
#define BITS_PER_PIXEL_OFFSET 0x001C
#define HEADER_SIZE 14
#define INFO_HEADER_SIZE 40
#define NO_COMPRESION 0
#define MAX_NUMBER_OF_COLORS 0
#define ALL_COLORS_REQUIRED 0

int Utils::Clamp(int min, int val, int max)
{
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

bool Utils::GetGridTileValue(vector<vector<int>> grid, int xPos, int yPos, int* value)
{
    if (xPos < 0 || xPos >= grid[0].size() ||
        yPos < 0 || yPos >= grid.size())
    { // out of bounds, return -1
        *value = -1;
        return false;

    }
    *value = grid[yPos][xPos];
    return true;
}

void Utils::ReadImage(const char* fileName, unsigned int** pixels, unsigned int* width, unsigned int* height, unsigned int* bytesPerPixel)
{
    // Get file
    FILE* imageFile = fopen(fileName, "rb");

    // Get Pixel Data Offset
    unsigned int dataOffset;
    fseek(imageFile, DATA_OFFSET_OFFSET, SEEK_SET);
    fread(&dataOffset, 4, 1, imageFile);

    // Get image width and height
    fseek(imageFile, WIDTH_OFFSET, SEEK_SET);
    fread(width, 4, 1, imageFile);
    fseek(imageFile, HEIGHT_OFFSET, SEEK_SET);
    fread(height, 4, 1, imageFile);

    // Get bits per pixel
    short bitsPerPixel;
    fseek(imageFile, BITS_PER_PIXEL_OFFSET, SEEK_SET);
    fread(&bitsPerPixel, 2, 1, imageFile);
    *bytesPerPixel = ((unsigned int)bitsPerPixel) / 8;

    // Read pixel data
    int totalSize = *width * *height * (int)*bytesPerPixel;
    *pixels = (unsigned int*)malloc(totalSize);

    for (int parsedBytes = 0; parsedBytes < totalSize; parsedBytes += (int)bytesPerPixel)
    {
        unsigned int pixelOffset = dataOffset + parsedBytes;
        fseek(imageFile, dataOffset + parsedBytes, SEEK_SET);
        fread(*pixels + parsedBytes, 1, (int)bytesPerPixel, imageFile);
    }

    // Clean up
    fclose(imageFile);
}