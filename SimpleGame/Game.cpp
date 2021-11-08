#pragma once
#include "Game.h"
#include <ctime>
#include "renderer.h"
#include "Utils.h"


Game::Game(int width, int height)
{
	m_gameState.width = width;
	m_gameState.height = height;
	Generate(55,8);

	int x = 0;
	int y = 0;

	// Add the player
	RandomFreeTile(&x, &y);
	m_gameState.playerX = x;
	m_gameState.playerY = y;

	// Add some enemies
	RandomFreeTile(&x, &y);
	m_gameState.enemies.push_back({ Enemy1, x,y });
	RandomFreeTile(&x, &y);
	m_gameState.enemies.push_back({ Enemy1, x,y });
	RandomFreeTile(&x, &y);
	m_gameState.enemies.push_back({ Enemy2, x,y });
}

void Game::RandomFreeTile(int* x, int* y)
{
	while(!Passable(*x,*y))
	{
		srand(time(nullptr));
		*x = rand() % m_gameState.width;
		*y = rand() % m_gameState.height;
	}
}

const vector<vector<int>> Game::GetMap()
{
	return m_gameState.map;
}

GameState* Game::GetGameState()
{
	return &m_gameState;
}

bool Game::Passable(int xPos, int yPos)
{
	bool passable;
	if (xPos < 0 || xPos >= m_gameState.width ||
		yPos < 0 || yPos >= m_gameState.height ||
		m_gameState.map[yPos][xPos] != 0) return false;
	return true;
}

void Game::MovePlayer(Direction direction)
{
	m_gameState.changedTiles.emplace_back(m_gameState.playerX, m_gameState.playerY);
	if (direction == Direction::Up		&& Passable(m_gameState.playerX, m_gameState.playerY + 1)) m_gameState.playerY += 1;
	if (direction == Direction::Down	&& Passable(m_gameState.playerX, m_gameState.playerY - 1)) m_gameState.playerY -= 1;
	if (direction == Direction::Left	&& Passable(m_gameState.playerX - 1, m_gameState.playerY)) m_gameState.playerX -= 1;
	if (direction == Direction::Right	&& Passable(m_gameState.playerX + 1, m_gameState.playerY)) m_gameState.playerX += 1;
	m_gameState.changedTiles.emplace_back(m_gameState.playerX, m_gameState.playerY);
}

void Game::MarkMapChanged()
{
	m_gameState.changedTiles.clear();
	for (int y = 0; y < m_gameState.height; y++)
	{
		for (int x = 0; x < m_gameState.width; x++)
		{
			//vector<int, int> pair = { x,y };
			//if (std::find(m_pGameState.changedTiles.begin(), m_pGameState.changedTiles.end(), pair) != m_pGameState.changedTiles.end())
			//{
				m_gameState.changedTiles.emplace_back(x, y);
			//}
		}
	}
}

void Game::Generate(int fillRate, int steps)
{
	srand(time(nullptr));

	m_gameState.map.resize(m_gameState.height);
	simulatedMap.resize(m_gameState.height);
	for (int y = 0; y < m_gameState.height; y++)
	{
		m_gameState.map[y].resize(m_gameState.width);
		simulatedMap[y].resize(m_gameState.width);
		for (int x = 0; x < m_gameState.width; x++)
		{
			bool alive = (rand() % 100) < fillRate;
			m_gameState.map[y][x] = alive ? 1 : 0;
		}
	}

	for (int i = 0; i < steps; i++)
	{
		m_gameState.map = Simulate(3,3);
	}

	MarkMapChanged();
}

void Game::Regenerate(int fillRate, int steps)
{
	for (int y = 0; y < m_gameState.height; y++)
		for (int x = 0; x < m_gameState.width; x++)
			simulatedMap[y][x] = 0;
	Generate(fillRate, steps);
}

vector<vector<int>> Game::Simulate(int removeLimit, int addLimit)
{
	for (int y = 0; y < m_gameState.height; y++)
	{
		for (int x = 0; x < m_gameState.width; x++)
		{
			if (m_gameState.map[y][x] == 1)
			{
				if (CountAdjacent(x, y) < removeLimit) simulatedMap[y][x] = 0;
			}
			else if (m_gameState.map[y][x] == 0)
			{
				if (CountAdjacent(x, y) > addLimit) simulatedMap[y][x] = 1;
			}
		}
	}

	m_gameState.map = simulatedMap;
	return simulatedMap;
}

int Game::CountAdjacent(int x, int y) const
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
			else if (neighbourX < 0 || neighbourY < 0 || neighbourX >= m_gameState.map[0].size() || neighbourY >= m_gameState.map.size()) {
				count++; // Outside the border, +1
			}
			else if (m_gameState.map[neighbourY][neighbourX]) {
				count = count + 1; // Normal check of the neighbour
			}
		}
	}

	return count;
}

void Game::Update()
{
	m_gameState.changedTiles.clear();

	// Move Enemies
	for (Enemy& enemy : m_gameState.enemies)
	{
		const int xOffset = ((rand() % 3) - 1);
		const int yOffset = ((rand() % 3) - 1);
		const int newX = enemy.xPos + xOffset;
		const int newY = enemy.yPos + yOffset;

		int targetValue;
		if (Utils::GetGridTileValue(m_gameState.map, newX, newY, &targetValue) && targetValue == 0)
		{
			m_gameState.changedTiles.emplace_back(enemy.xPos, enemy.yPos);
			enemy.xPos = newX;
			enemy.yPos = newY;
			m_gameState.changedTiles.emplace_back(enemy.xPos, enemy.yPos);
		}
	}
}
