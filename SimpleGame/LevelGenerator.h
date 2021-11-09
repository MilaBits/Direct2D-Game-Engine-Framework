#pragma once
#include <vector>
using std::vector;

class LevelGenerator
{
public:

	static vector<vector<int>> Generate(int width, int height, int fillRate, int steps);
	static int CountAdjacent(vector<vector<int>> map, int x, int y);
	//static void Regenerate(int fillRate, int steps);

private:

	static vector<std::vector<int>> Simulate(vector<vector<int>>& map, vector<vector<int>>& simulatedMap, int removeLimit, int addLimit);

	/*static vector<vector<int>> map;
	static vector<vector<int>> simulatedMap;*/

};

