#pragma once
#include "renderer.h"
#include "Game.h"
#include "Input.h"

struct GameSettings
{
	int gameWidth = 64;
	int gameHeight = 32;
};

class Win32Platform
{

public:

	Win32Platform(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);

	bool IsRunning();
	bool IsInitialized();
	void StopRunning();
	static LRESULT CALLBACK WindowCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Update();
	void Resize();

	GameSettings gameSettings;

private:

	HDC m_hdc;
	HWND m_window;
	bool m_running = true;
	Renderer* m_pRenderer;
	Game* m_pGame;
	Input* m_pInput;

	long long lastFrameTime;
	int frameLength = 16;
};