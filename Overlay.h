#include "Classes.h"
#include <Windows.h>

class COverlay : public CBaseThread
{
private:
	WNDCLASSEXA m_WndClass;
	HWND m_Overlay;

	const char* m_ClassName;
	const char* m_WindowName;

public:
	COverlay() = default;

	bool Create(WNDPROC wndproc, HINSTANCE hInstance, const char* ClassName, const char* WindowName);
	void EntryPoint() override;

	int GetFrameRate() const;
	const HWND& GetOverlayWindow() const;
};

extern COverlay g_Overlay;