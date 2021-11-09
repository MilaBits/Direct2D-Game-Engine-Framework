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
			static_cast<UINT>(ceil((m_renderState.width*4) * dpiX / 96.f)),
			static_cast<UINT>(ceil((m_renderState.height*4) * dpiY / 96.f)),
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

void Renderer::DrawRect(int xPos, int yPos, int width, int height, unsigned int color) const
{
	for (int y = yPos; y < Utils::Clamp(0, yPos + height, m_renderState.height); y++)
	{
		unsigned int* pixel = static_cast<unsigned*>(m_renderState.memory) + xPos + y * m_renderState.width;
		for (int x = xPos; x < Utils::Clamp(0, xPos + width, m_renderState.width); x++)
		{
			*pixel = color;
			*pixel++;
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
			if (transparency == 0xff) *pixel = texture[index];

			*pixel++;
			index++;
		}
	}
}

int Renderer::RandomFloor()
{
	return 1+ rand() % 4;
}

vector<vector<int>> Renderer::GenerateTextureMap(vector<vector<int>> grid) const
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

void Renderer::DrawGrid(vector<vector<int>> grid) const
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

HRESULT Renderer::OnRender()
{
	HRESULT hr = S_OK;
	hr = CreateDeviceResources();

	if (SUCCEEDED(hr))
	{
		D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

		m_pRenderTarget->BeginDraw();
		D2D1_POINT_2F center = D2D1::Point2F(rtSize.width / 2.0f, rtSize.height / 2.0f);
		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Scale(1,-1, center));
		m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

		ID2D1Bitmap* d2dBitmap;
		D2D1_SIZE_U bitmapSize = D2D1::SizeU(m_renderState.width, m_renderState.height);
		D2D1_BITMAP_PROPERTIES props;
		props.dpiX = (FLOAT)GetDpiForWindow(GetDesktopWindow());
		props.dpiY = props.dpiX;
		props.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
		m_pRenderTarget->CreateBitmap(bitmapSize, m_renderState.memory, m_renderState.width*4 , props, &d2dBitmap);

		D2D1_RECT_F windowRect = { 0,0,rtSize.width, rtSize.height };
		m_pRenderTarget->DrawBitmap(d2dBitmap, windowRect, 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
		//todo: make it not stretch

		hr = m_pRenderTarget->EndDraw();
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

	textureData.reserve(size);
	for (int i = 0; i < size; i++)
	{
		textureData.push_back(pixels[i]);
	}
	m_textures.insert({ texture, textureData });
}

void Renderer::Update(const GameState* pGameState)
{
	// Draw Map
	DrawGrid(pGameState->map);

	// Draw Player
	DrawRectTexture(pGameState->player.xPos * m_textureSize, pGameState->player.yPos * m_textureSize, m_textureSize, m_textureSize, Player);

	//Draw Enemies
	for (const Entity &enemy : pGameState->enemies)
	{
		DrawRectTexture(enemy.xPos * m_textureSize, enemy.yPos * m_textureSize, m_textureSize, m_textureSize, enemy.textureId);
	}

	// Tell window to redraw
	RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE);
}
