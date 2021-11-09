#include <Windows.h>
#include "renderer.h"

#include <chrono>
#include <iostream>

#include "Game.h"
#include "Utils.h"
#include "win32_platform.h"

Renderer::Renderer(GameState* gameState) :
	m_hwnd(NULL),
	m_pDirect2dFactory(NULL),
	m_pRenderTarget(NULL),
	m_pLightSlateGrayBrush(NULL),
	m_pCornflowerBlueBrush(NULL),
	m_pGameState(gameState)
{
	m_renderState.width = m_pGameState->width * m_textureSize;
	m_renderState.height = m_pGameState->height * m_textureSize;

	InitFloorTextures(m_pGameState->width, m_pGameState->height);

	LoadTexture(WallFront,		const_cast<char*>("res\\wallAboveFloor.bmp"));
	LoadTexture(WallTop,		const_cast<char*>("res\\wall.bmp"));
	LoadTexture(WallFrontTop,	const_cast<char*>("res\\wallAboveWallAboveFloor.bmp"));
	LoadTexture(Floor,          const_cast<char*>("res\\floor.bmp"));
	LoadTexture(FloorFront,     const_cast<char*>("res\\floorFront.bmp"));
	LoadTexture(FloorCircle,    const_cast<char*>("res\\floorCircle.bmp"));
	LoadTexture(FloorCrossThin, const_cast<char*>("res\\floorCross1.bmp"));
	LoadTexture(FloorSquares,	const_cast<char*>("res\\floorSquares.bmp"));
	LoadTexture(Enemy1,			const_cast<char*>("res\\enemy1.bmp"));
	LoadTexture(Enemy2,			const_cast<char*>("res\\enemy2.bmp"));
	LoadTexture(Player,			const_cast<char*>("res\\player.bmp"));

	UpdateRenderState();
}

Renderer::~Renderer()
{
	ReleaseResources();
}

HRESULT Renderer::Initialize()
{
	HRESULT hr;

	// Initialize device-indpendent resources, such as the Direct2D factory.
	hr = CreateDeviceIndependentResources();

	if (SUCCEEDED(hr))
	{
		// Register the window class.
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = Win32Platform::WindowCallback;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = HINST_THISCOMPONENT;
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = NULL;
		wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
		wcex.lpszClassName = L"SimpleGame";

		RegisterClassEx(&wcex);

		FLOAT dpiX = (FLOAT)GetDpiForWindow(GetDesktopWindow());
		FLOAT dpiY = dpiX;

		// Create the window.
		m_hwnd = CreateWindow(
			L"SimpleGame",
			L"Simple Game",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			static_cast<UINT>(ceil(640.f * dpiX / 96.f)),
			static_cast<UINT>(ceil(480.f * dpiY / 96.f)),
			NULL,
			NULL,
			HINST_THISCOMPONENT,
			this
		);
		hr = m_hwnd ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			ShowWindow(m_hwnd, SW_SHOWNORMAL);
			UpdateWindow(m_hwnd);
		}
	}

	return hr;
}


void Renderer::UpdateRenderState()
{
	RECT rect;
	GetClientRect(m_hwnd, &rect);
	m_renderState.clientRect = rect;

	m_renderState.width = m_pGameState->width * m_textureSize;
	m_renderState.height = m_pGameState->height * m_textureSize;

	const int buffer_size = m_renderState.width * m_renderState.height * sizeof(unsigned int);

	if (m_renderState.memory) VirtualFree(m_renderState.memory, 0, MEM_RELEASE);
	m_renderState.memory = VirtualAlloc(0, buffer_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	m_renderState.bitmapInfo.bmiHeader.biSize = sizeof(m_renderState.bitmapInfo.bmiHeader);
	m_renderState.bitmapInfo.bmiHeader.biWidth = m_renderState.width;
	m_renderState.bitmapInfo.bmiHeader.biHeight = m_renderState.height;
	m_renderState.bitmapInfo.bmiHeader.biPlanes = 1;
	m_renderState.bitmapInfo.bmiHeader.biBitCount = 32;
	m_renderState.bitmapInfo.bmiHeader.biCompression = BI_RGB;
}

void Renderer::InitFloorTextures(int width, int height)
{
	m_floorTextures = vector<vector<TextureId>>(height, vector<TextureId>(width));
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			m_floorTextures[y][x] = static_cast<TextureId>(RandomFloor());
		}
	}
}

void Renderer::RenderBackground() const
{
	unsigned int* pixel = static_cast<unsigned*>(m_renderState.memory);
	for (int y = 0; y < m_renderState.height; y++)
	{
		for (int x = 0; x < m_renderState.width; x++)
		{
			unsigned int color;
			if (x % 2 == 0 && y % 2 == 0) color = 0x000000;
			else color = 0xffffff;
			*pixel++ = color;
		}
	}
}

void Renderer::ClearScreen(unsigned int color) const
{
	unsigned int* pixel = static_cast<unsigned*>(m_renderState.memory);
	for (int y = 0; y < m_renderState.height; y++)
	{
		for (int x = 0; x < m_renderState.width; x++)
		{
			*pixel++ = color;
		}
	}
}

void Renderer::DrawRect(int xPos, int yPos, int width, int height, unsigned int color) const
{
	for (int y = yPos; y < Utils::Clamp(0, yPos + height, m_renderState.height); y++)
	{
		unsigned int* pixel = static_cast<unsigned*>(m_renderState.memory) + xPos + y * m_renderState.width;
		for (int x = xPos; x < Utils::Clamp(0, xPos + width, m_renderState.width); x++)
		{
			*pixel++ = color;
		}
	}
}

void Renderer::DrawRectTexture(int xPos, int yPos, int width, int height, TextureId textureId) const
{
	vector<unsigned int> texture = m_textures.at(textureId);
	int index = 0;

	int xTile = xPos / width;
	int yTile = yPos / width;

	for (int y = yPos; y < Utils::Clamp(0, yPos + height, m_renderState.height); y++)
	{
		unsigned int* pixel = static_cast<unsigned*>(m_renderState.memory) + xPos + y * m_renderState.width;
		for (int x = xPos; x < Utils::Clamp(0, xPos + width, m_renderState.width); x++)
		{
			int transparency = texture[index] >> (8*3) & 0xff;
			//*pixel++ = transparency == 0xff ? texture[index] : m_textures.at(m_floorTextures[yTile][xTile])[index]; // Reliable when on floor (not future proof)
			//*pixel++ = transparency == 0xff ? texture[index] : *static_cast<unsigned*>(m_renderState.memory) + xPos + (y * m_renderState.width) + x;
			//												   ^ Funky colors? why??
			if (transparency == 0xff) *pixel++ = texture[index];

			index++;
		}
	}
}

int Renderer::RandomFloor()
{
	return 1+ rand() % 4;
}

vector<vector<int>> Renderer::GenerateTextureMap(vector<vector<int>> grid)
{
	vector<vector<int>> map = grid;

	for (int y = 0; y < grid.size(); y++)
	{
		for (int x = 0; x < grid[y].size(); x++)
		{
			int tileBelow;
			Utils::GetGridTileValue(grid, x, y - 1, &tileBelow);
			int tileAbove;
			Utils::GetGridTileValue(grid, x, y + 1, &tileAbove);

			if (tileBelow <= 0 && tileAbove == 0) map[y][x] = FloorFront;
			else if (tileBelow <= 0) map[y][x] = WallFront;
			if (tileBelow == 1) map[y][x] = WallFrontTop;

			if (grid[y][x] == 0) map[y][x] = m_floorTextures[y][x];
		}
	}
	return map;
}

void Renderer::DrawGrid(vector<vector<int>> grid)
{
	vector<vector<int>> textureGrid = GenerateTextureMap(grid);

	for (int y = 0; y < textureGrid.size(); y++)
	{
		for (int x = 0; x < textureGrid[y].size(); x++)
		{
			DrawRectTexture(x * m_textureSize, y * m_textureSize, m_textureSize, m_textureSize, static_cast<TextureId>(textureGrid[y][x]));
		}
	}
}

void Renderer::Render(HDC hdc) const
{
	const int wWidth = m_renderState.clientRect.right - m_renderState.clientRect.left;
	const int wHeight = m_renderState.clientRect.bottom - m_renderState.clientRect.top;

	const float widthRatio = static_cast<float>(wWidth / m_renderState.width);
	const float heightRatio = static_cast<float>(wHeight / m_renderState.height);

	const float bestRatio = widthRatio < heightRatio ? widthRatio : heightRatio;

	const int width = static_cast<int>(bestRatio * m_renderState.width);
	const int height = static_cast<int>(bestRatio * m_renderState.height);

	const long long start = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
	// target
	StretchDIBits(hdc, 
		(wWidth - width) / 2, (wHeight - height) / 2, 
		width, height, 
		0, 0, 
		m_renderState.width, m_renderState.height, 
		m_renderState.memory, &m_renderState.bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
	const long long end = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
	std::cout << "duration: " << (end - start) << "ms" << std::endl;
}

void Renderer::TriggerFullRedraw()
{
	fullRedraw = true;
}

HRESULT Renderer::OnRender()
{
	HRESULT hr = S_OK;
	hr = CreateDeviceResources();

	if (SUCCEEDED(hr))
	{
		m_pRenderTarget->BeginDraw();
		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

		D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

		ID2D1Bitmap* d2dBitmap;
		D2D1_SIZE_U bitmapSize = D2D1::SizeU(m_renderState.width, m_renderState.height);
		D2D1_BITMAP_PROPERTIES props;
		props.dpiX = (FLOAT)GetDpiForWindow(GetDesktopWindow());
		props.dpiY = props.dpiX;
		props.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
		m_pRenderTarget->CreateBitmap(bitmapSize, m_renderState.memory, m_renderState.width*4 , props, &d2dBitmap);

		D2D1_RECT_F windowRect = { 0,0,(float)bitmapSize.width, (float)bitmapSize.height };
		m_pRenderTarget->DrawBitmap(d2dBitmap, windowRect);
		//m_pRenderTarget->DrawRectangle({ 10,60,50,50 }, m_pCornflowerBlueBrush, 1);

		hr = m_pRenderTarget->EndDraw();
		std::cout << "OnRender"<<std::endl;
	}

	if (hr == D2DERR_RECREATE_TARGET)
	{
		hr = S_OK;
		DiscardDeviceResources();
	}

	return hr;
}

void Renderer::ReleaseResources()
{
	SafeRelease(&m_pDirect2dFactory);
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pLightSlateGrayBrush);
	SafeRelease(&m_pCornflowerBlueBrush);
}

void Renderer::RunMessageLoop()
{
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

HRESULT Renderer::CreateDeviceIndependentResources()
{
	HRESULT hr = S_OK;

	// Create a Direct2D factory.
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);

	return hr;
}

HRESULT Renderer::CreateDeviceResources()
{
	HRESULT hr = S_OK;

	if (!m_pRenderTarget)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(
			rc.right - rc.left,
			rc.bottom - rc.top
		);

		// Create a Direct2D render target.
		hr = m_pDirect2dFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
			D2D1::HwndRenderTargetProperties(m_hwnd, size),
			&m_pRenderTarget
		);

		if (SUCCEEDED(hr))
		{
			// Create a gray brush.
			hr = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::LightSlateGray),
				&m_pLightSlateGrayBrush
			);
		}
		if (SUCCEEDED(hr))
		{
			// Create a blue brush.
			hr = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::CornflowerBlue),
				&m_pCornflowerBlueBrush
			);
		}
	}

	return hr;
}

void Renderer::DiscardDeviceResources()
{
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pLightSlateGrayBrush);
	SafeRelease(&m_pCornflowerBlueBrush);
}

void Renderer::OnResize(UINT width, UINT height)
{
	if (m_pRenderTarget)
	{
		UpdateRenderState();
		m_pRenderTarget->Resize(D2D1::SizeU(width, height));
	}
	std::cout << "OnResize" << std::endl;
}

void Renderer::LoadTexture(TextureId texture, const char* name)
{
	unsigned int* pixels;
	unsigned int width;
	unsigned int height;
	unsigned int bytesPerPixel;
	Utils::ReadImage(name, &pixels, &width, &height, &bytesPerPixel);

	int size = width * height;
	vector<unsigned int> textureData;

	textureData.reserve(width * height);
	for (int i = 0; i < width * height; i++)
	{
		textureData.push_back(pixels[i]);
	}
	m_textures.insert({ texture, textureData });
}

void Renderer::ClearFullWindow(HDC hdc, unsigned int color) const
{
	ClearScreen(color);

	const int wWidth = m_renderState.clientRect.right - m_renderState.clientRect.left;
	const int wHeight = m_renderState.clientRect.bottom - m_renderState.clientRect.top;

	StretchDIBits(hdc, 
		0, 0, 
		wWidth, wHeight, 
		0, 0, 
		m_renderState.width, m_renderState.height, 
		m_renderState.memory, &m_renderState.bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

}

void Renderer::Update(const GameState* pGameState)
{
	m_tilesToUpdate = pGameState->changedTiles;

	DrawGrid(pGameState->map);
	fullRedraw = false;

	DrawRectTexture(pGameState->playerX * m_textureSize, pGameState->playerY * m_textureSize, m_textureSize, m_textureSize, Player);

	for (const Enemy &enemy : pGameState->enemies)
	{
		DrawRectTexture(enemy.xPos * m_textureSize, enemy.yPos * m_textureSize, m_textureSize, m_textureSize, enemy.textureId);
	}

	RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW);
}
