#include "Input.h"

#include <iostream>

#include "win32_platform.h"
using Clock = std::chrono::steady_clock;

Input::Input(HWND window, Game *game)
{
	m_window = window;
	m_pGame = game;

	start = now = Clock::now();
}

void Input::Update()
{
	MSG message;
	while (PeekMessage(&message, m_window, 0, 0, PM_REMOVE))
	{
		switch (message.message)
		{
			case WM_KEYUP:
			case WM_KEYDOWN:
			{
				now = Clock::now();
				duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);

				if (duration.count() > 50)
				{
					start = Clock::now();
					Entity& player = m_pGame->GetGameState()->player;
					if (GetAsyncKeyState(VK_UP)    || GetAsyncKeyState(0x57)) m_pGame->MoveEntity(player, Direction::Up);
					if (GetAsyncKeyState(VK_DOWN)  || GetAsyncKeyState(0x53)) m_pGame->MoveEntity(player, Direction::Down);
					if (GetAsyncKeyState(VK_LEFT)  || GetAsyncKeyState(0x41)) m_pGame->MoveEntity(player, Direction::Left);
					if (GetAsyncKeyState(VK_RIGHT) || GetAsyncKeyState(0x44)) m_pGame->MoveEntity(player, Direction::Right);
				}
				//if (GetAsyncKeyState(VK_SPACE)) m_pGame->Regenerate(55, 8);

				break;
			}
			default:
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
				break;
			}
		}
	}
}
