#include <windows.h>
#include "win32_platform.h"
#pragma warning(disable:28251)

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	Win32Platform* win32Platform = new Win32Platform(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
}