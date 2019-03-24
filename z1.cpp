#include "Structs.h"
#include "Engine.h"
#include "Overlay.h"
#include "Memory.h"
#include "Menu.h"
#include "Controls.h"
#include "Misc.h"
#include "Removals.h"
#include "ESP.h"
#include "Aimbot.h"
#include "Backtrack.h"
#include "PlayerList.h"

#include <time.h>

CGlobalVarsBase g_GlobalVars;
Input_t g_GameInput;
/*
#ifdef UNICODE
#define WinMain WINAPI wWinMain
#else
#define WinMain WINAPI WinMain
#endif
*/
#define WinMain WINAPI WinMain

#ifdef UNICODE
static_assert(UNICODE, "ansi only");
#endif

LRESULT WINAPI WndProc(HWND hwWindow, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		auto hdc = BeginPaint(hwWindow, &ps);
		auto memhdc = CreateCompatibleDC(hdc);
		auto membitmap = CreateCompatibleBitmap(hdc, g_Render.Dimensions().right, g_Render.Dimensions().bottom);
		SelectObject(memhdc, membitmap);

		g_Render.OnFrame(memhdc);

		if (g_Engine.IsFocused())
		{
			g_Menu.Control();
			g_Menu.Print();

			if (g_Engine.GetState() == SIG_FULL)
			{
				g_ESP.EntryPoint();
				g_SpectatorList.EntryPoint();
			}
		}

		BitBlt(hdc, 0, 0, g_Render.Dimensions().right, g_Render.Dimensions().bottom, memhdc, 0, 0, SRCCOPY);

		DeleteObject(membitmap);
		DeleteDC(hdc);
		DeleteDC(memhdc);

		EndPaint(hwWindow, &ps);

		ValidateRect(hwWindow, &g_Render.Dimensions());
		return 0;
	}
	case WM_ERASEBKGND:
		return 1;
	case WM_CLOSE:
		DestroyWindow(hwWindow);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwWindow, uMsg, wParam, lParam);
	}

	return 0;
}

int WinMain(HINSTANCE, HINSTANCE, char*, int)
{
	srand(time(0));
	g_Memory.HookProcess();

	if (!g_Overlay.Create(WndProc, 0, "z1", "z1"))
	{
		return 1;
	}

	g_Render.Initialize();
	g_Controls.RegisterControls();

	RegisterThread(g_Engine);
	RegisterThread(g_MiscHacks);
	RegisterThread(g_Removals);
	RegisterThread(g_Aimbot);

	g_Overlay.EntryPoint();

	return 0;
}