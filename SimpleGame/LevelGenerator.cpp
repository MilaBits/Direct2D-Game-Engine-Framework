#include "LevelGenerator.h"
#include <ctime>

vector<vector<int>> LevelGenerator::Generate(int width, int height, int fillRate, int steps)
{
	vector<vector<int>> map;
	vector<vector<int>> simulatedMap;
	srand(time(nullptr));

	// Create and populate initial map
	map.resize(height);
	simulatedMap.resize(height);
	for (int y = 0; y < height; y++)
	{
		map[y].resize(width);
		simulatedMap[y].resize(width);
		for (int x = 0; x < width; x++)
		{
			bool alive = (rand() % 100) < fillRate;
			map[y][x] = alive ? 1 : 0;
		}
	}

	// Execute Cellular Automata
	for (int i = 0; i < steps; i++)
	{
		map = Simulate(map, simulatedMap, 3, 3);
	}

	return map;
}

vector<vector<int>> LevelGenerator::Simulate(vector<vector<int>>& map, vector<vector<int>>& simulatedMap, int removeLimit, int addLimit)
{
	int height = map.size();
	int width = map[0].size();

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (map[y][x] == 1)
			{
				if (CountAdjacent(map, x, y) < removeLimit) simulatedMap[y][x] = 0;
			}
			else if (map[y][x] == 0)
			{
				if (CountAdjacent(map, x, y) > addLimit) simulatedMap[y][x] = 1;
			}
		}
	}
	
	return simulatedMap;
}

int LevelGenerator::CountAdjacent(vector<vector<int>> map, int x, int y)
{
	int count = 0;
	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			int neighbourY = y + i;
			int neighbourX = x + j;

			if (i == 0 && j == 0)
			{
				// Center, skip
			}
			else if (neighbourX < 0 || neighbourY < 0 || neighbourX >= map[0].size() || neighbourY >= map.size()) {
				count++; // Outside the border, +1
			}
			else if (map[neighbourY][neighbourX]) {
				count = count + 1; // Normal check of the neighbour
			}
		}
	}

	return count;
}