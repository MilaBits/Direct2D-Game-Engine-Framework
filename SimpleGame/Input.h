#pragma once
#include <Windows.h>
#include "Game.h"
#include <chrono>

class Input
{

public:

	Input(HWND Window, Game* game);
	//delegate void ButtonPressedEventHandler(Input sender, int^ keyCode);
	//event ButtonPressedEventHandler^ KeyPressed;
	void Update();

private:

	HWND m_window;
	Game* m_pGame;

	std::chrono::time_point<std::chrono::steady_clock> start;
	std::chrono::time_point<std::chrono::steady_clock> now;
	std::chrono::milliseconds duration;
};