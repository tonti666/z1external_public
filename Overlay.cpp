#include "Overlay.h"
#include "Memory.h"
#include "Controls.h"

COverlay g_Overlay;

bool COverlay::Create(WNDPROC wndproc, HINSTANCE hInstance, const char* ClassName, const char* WindowName)
{
	m_ClassName = ClassName;
	m_WindowName = WindowName;

	m_WndClass =
	{
		sizeof(WNDCLASSEX),
		0,
		wndproc,
		0,
		0,
		hInstance,
		LoadIconA(NULL, IDI_APPLICATION),
		LoadCursorA(NULL, IDC_ARROW),
		NULL,
		NULL,
		m_ClassName,
		LoadIconA(NULL, IDI_APPLICATION)
	};

	if (!RegisterClassExA(&m_WndClass))
	{
		return false;
	}

	RECT ClientBounds;
	GetClientRect(g_Memory.GetWindow(), &ClientBounds);

	m_Overlay = CreateWindowExA
	(
		WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
		m_ClassName,
		m_WindowName,
		WS_POPUP,
		ClientBounds.left,
		ClientBounds.top,
		ClientBounds.right,
		ClientBounds.bottom,
		g_Memory.GetWindow(),
		NULL,
		NULL,
		NULL
	);

	if (!m_Overlay)
	{
		return false;
	}

	if (!SetLayeredWindowAttributes(m_Overlay, RGB(NULL, NULL, NULL), 255, ULW_COLORKEY | LWA_ALPHA))
	{
		return false;
	}

	ShowWindow(m_Overlay, SW_SHOWDEFAULT);

	return true;
}

void COverlay::EntryPoint()
{
	MSG msg;

	while (GetMessage(&msg, g_Overlay.GetOverlayWindow(), 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

int COverlay::GetFrameRate() const
{
	return g_SharedCon.m_pFPS->Value();
}

const HWND& COverlay::GetOverlayWindow() const
{
	return m_Overlay;
}