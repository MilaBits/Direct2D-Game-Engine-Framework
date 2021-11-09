#pragma once
#include "Game.h"
#include <ctime>
#include "renderer.h"
#include "Utils.h"


Game::Game(int width, int height, vector<vector<int>> map)
{
	m_gameState.map = map;
	m_gameState.width = width;
	m_gameState.height = height;

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
	if (direction == Direction::Up		&& Passable(m_gameState.playerX, m_gameState.playerY + 1)) m_gameState.playerY += 1;
	if (direction == Direction::Down	&& Passable(m_gameState.playerX, m_gameState.playerY - 1)) m_gameState.playerY -= 1;
	if (direction == Direction::Left	&& Passable(m_gameState.playerX - 1, m_gameState.playerY)) m_gameState.playerX -= 1;
	if (direction == Direction::Right	&& Passable(m_gameState.playerX + 1, m_gameState.playerY)) m_gameState.playerX += 1;
}

void Game::Update()
{
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
			enemy.xPos = newX;
			enemy.yPos = newY;
		}
	}
}
