#pragma once
#include <Windows.h>
#include <vector>
#include <map>

#pragma comment(lib, "d2d1")
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

struct GameState;
using std::vector;
using std::map;
using std::pair;

template<class Interface>
inline void SafeRelease(
	Interface** ppInterfaceToRelease
)
{
	if (*ppInterfaceToRelease != NULL)
	{
		(*ppInterfaceToRelease)->Release();

		(*ppInterfaceToRelease) = NULL;
	}
}

#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

struct RenderState
{
	int height;
	int width;
	void* memory;

	RECT clientRect;
	BITMAPINFO bitmapInfo;
};

enum TextureId
{
	FloorFront = 0,
	WallFront = 11,
	WallTop = 12,
	WallFrontTop = 13,
	Floor = 1,
	FloorCircle = 2,
	FloorCrossThin = 3,
	FloorSquares = 4,
	Player = 91,
	Enemy1 = 92,
	Enemy2 = 93,
};

class Renderer
{

public:

	Renderer(GameState* gameState);
	~Renderer();

	// Register the window class and call methods for instantiating drawing resources
	HRESULT Initialize();

	void InitFloorTextures(int width, int height);
	void UpdateRenderState();
	void RenderBackground() const;
	void ClearScreen(unsigned int color) const;
	void ClearFullWindow(HDC hdc, unsigned int color) const;
	void DrawRect(int xPos, int yPos, int width, int height, unsigned int color) const;
	void DrawRectTexture(int xPos, int yPos, int width, int height, TextureId textureId) const;
	static int RandomFloor();
	vector<vector<int>> GenerateTextureMap(vector<vector<int>>);
	void DrawGrid(vector<vector<int>> grid);
	void Update(const GameState* pGameState);
	void Render(HDC hdc) const;
	
	// Draw content.
	HRESULT OnRender();

	// Resize the render target.
	void OnResize(
		UINT width,
		UINT height
	);

	void ReleaseResources();
	void RunMessageLoop();

private:

	// Initialize device-independent resources.
	HRESULT CreateDeviceIndependentResources();

	// Initialize device-dependent resources.
	HRESULT CreateDeviceResources();

	// Release device-dependent resource.
	void DiscardDeviceResources();

	// The windows procedure.
	static LRESULT CALLBACK WndProc(
		HWND hWnd,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	);

	HWND m_hwnd;
	ID2D1Factory* m_pDirect2dFactory;
	ID2D1HwndRenderTarget* m_pRenderTarget;
	ID2D1SolidColorBrush* m_pLightSlateGrayBrush;
	ID2D1SolidColorBrush* m_pCornflowerBlueBrush;

	int m_textureSize = 8;
	RenderState m_renderState = {};
	GameState* m_pGameState;

	void LoadTexture(TextureId texture, const char* name);

	map<int, unsigned int> colors =
	{
		{0, 0xbf8569},
		{1, 0x52372a}
	};

	vector<vector<TextureId>> m_floorTextures;
	vector<pair<int, int>> m_tilesToUpdate;

	map <TextureId, vector<unsigned int>> m_textures;
};