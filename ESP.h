#pragma once
#include "Classes.h"
#include "Color.h"
#include "Config.h"
#include "Engine.h"
#include "Controls.h"

struct ESPObject_t
{
private:
	bool m_bValid;

	bool Register();
	bool PlayerPassesConditions() const;

	CVector2D m_InfoSpot;

public:
	ESPObject_t() = delete;
	ESPObject_t(CPlayer& Binding);
	~ESPObject_t() = default;

	bool IsValid() const;
	int GetInfoVerticalOffset() const;

	void RenderInfo(const char* pInfo);

	CPlayer& m_Player;

	int m_iWidth, m_iHeight;
	CVector m_WorldHeed, m_WorldFeet;
	CVector2D m_ScreenHead, m_ScreenTop, m_ScreenBottom;

	CColor m_Color;
	HBRUSH m_Brush;
};

enum class DrawMode_t
{
	Essentials,
	Full
};

class CESP : public CBaseHack
{
private:
	CTimer m_SpottedTimes[g_PlayerList.MaxPlayerCount];
	CTimer m_VisibleTimes[g_PlayerList.MaxPlayerCount];

	DrawMode_t m_CurrentMode;
	ESPObject_t* m_Current;
	void HandlePlayer();

	void DrawEssentials();
	void DrawNonEssentials();

	char m_Buffer[BUFFER_SIZE];

public:
	void EntryPoint() override;	
};

extern CESP g_ESP;