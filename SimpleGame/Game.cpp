#pragma once
#include "Game.h"
#include <ctime>
#include <iostream>
#include <ostream>

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
	m_gameState.player = { Player, x,y };

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

void Game::MoveEntity(Entity& entity, Direction direction)
{
	if (direction == Direction::Up		&& Passable(entity.xPos, entity.yPos + 1)) entity.yPos += 1;
	if (direction == Direction::Down	&& Passable(entity.xPos, entity.yPos - 1)) entity.yPos -= 1;
	if (direction == Direction::Left	&& Passable(entity.xPos - 1, entity.yPos)) entity.xPos -= 1;
	if (direction == Direction::Right	&& Passable(entity.xPos + 1, entity.yPos)) entity.xPos += 1;
	std::cout << entity.xPos << "," << entity.yPos << std::endl;
}

void Game::Update()
{
	// Move Enemies
	for (Entity& enemy : m_gameState.enemies)
	{
		MoveEntity(enemy, (Direction)(rand() % 4));
	}
}
