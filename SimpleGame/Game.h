#pragma once
#include <vector>
#include "renderer.h"

using std::vector;

struct Enemy
{
	TextureId textureId;

	int xPos;
	int yPos;
};

struct GameState
{
	int width;
	int height;

	vector<vector<int>> map;
	vector<pair<int, int>> changedTiles;

	vector<Enemy> enemies;

	int playerX;
	int playerY;

	void ClearChangedTiles()
	{
		changedTiles.clear();
	}
};

enum class Direction
{
	Up, 
	Down, 
	Left, 
	Right
};

class Game
{

public:
	Game(int width, int height, vector<vector<int>> map);
	void RandomFreeTile(int* x, int* y);
	const vector<vector<int>> GetMap();
	GameState* GetGameState();
	void MovePlayer(Direction direction);
	bool Passable(int xPos, int yPos);
	void Update();

private:

	vector<vector<int>> simulatedMap;
	GameState m_gameState;
};