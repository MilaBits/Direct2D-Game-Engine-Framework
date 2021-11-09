#include "win32_platform.h"
#include <iostream>
#include <ostream>

#include "LevelGenerator.h"

LRESULT CALLBACK Win32Platform::WindowCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    if (uMsg == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        Renderer* pRenderer = (Renderer*)pcs->lpCreateParams;

		::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pRenderer));

        result = 1;
    }
    else
    {
        Renderer* pRenderer = reinterpret_cast<Renderer*>(static_cast<LONG_PTR>(
			::GetWindowLongPtrW(hwnd, GWLP_USERDATA)));

        bool wasHandled = false;

        if (pRenderer)
        {
            switch (uMsg)
            {
                case WM_SIZE:
                {
                    UINT width = LOWORD(lParam);
                    UINT height = HIWORD(lParam);
                    pRenderer->OnResize(width, height);
                }
                result = 0;
                wasHandled = true;
                break;

                case WM_DISPLAYCHANGE:
                {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                result = 0;
                wasHandled = true;
                break;

                case WM_PAINT:
                {
                    pRenderer->OnRender();
                    ValidateRect(hwnd, NULL);
                }
                result = 0;
                wasHandled = true;
                break;

                case WM_DESTROY:
                {
                    PostQuitMessage(0);
                }
                result = 1;
                wasHandled = true;
                break;
            }
        }

        if (!wasHandled)
        {
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    return result;
}

Win32Platform::Win32Platform(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

    m_pGame = new Game(gameSettings.gameWidth, gameSettings.gameHeight, 
        LevelGenerator::Generate(gameSettings.gameWidth, gameSettings.gameHeight, 55,8));
    m_pInput = new Input(m_window, m_pGame);

	// Use HeapSetInformation to specify that the process should
	// terminate if the heap manager detects an error in any heap used
	// by the process.
	// The return value is ignored, because we want to continue running in the
	// unlikely event that HeapSetInformation fails.
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	if (SUCCEEDED(CoInitialize(NULL)))
	{
		m_pRenderer = new Renderer(m_pGame->GetGameState());
		if (SUCCEEDED(m_pRenderer->Initialize()))
		{
            lastFrameTime = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();

            while (IsRunning())
            {
                Update();
            }
		}
		CoUninitialize();
	}
}

void Win32Platform::Update()
{
    // Input
    m_pInput->Update();

    long long now = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
    if (now - lastFrameTime >= frameLength)
    {
        // Simulate
        m_pGame->Update();

        // Render
        m_pRenderer->Update(m_pGame->GetGameState());

        lastFrameTime = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
    }
}


bool Win32Platform::IsRunning()
{
	return m_running;
}

void Win32Platform::StopRunning()
{
	m_running = false;
}

