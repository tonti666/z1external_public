#include "Engine.h"
#include "Memory.h"
#include "Overlay.h"
#include "Menu.h"
#include "Aimbot.h"
#include "Controls.h"
#include "PlayerPrediction.h"
#include "Backtrack.h"

#include <thread>
#include <chrono>

IRender g_Render;
IInput g_Input;
IPlayerList g_PlayerList;
IEngine g_Engine;

void IRender::Initialize()
{
	GetWindowRect(g_Memory.GetWindow(), &m_Dimensions);

	m_Width = m_Dimensions.right - m_Dimensions.left;
	m_Height = m_Dimensions.bottom - m_Dimensions.top;

	m_Font = CreateFont(FONT_HEIGHT, 0, 0, 0, FW_ULTRABOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_ROMAN, "Verdana");
	m_TitleFont = CreateFont(FONT_TITLE_HEIGHT, 0, 0, 0, FW_ULTRABOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FF_ROMAN, "Verdana");
}

void IRender::OnFrame(HDC DC)
{
	m_HDC = DC;

	ManualRead(g_Memory.Client() + IO::ViewMatrix, &m_ViewMatrix, sizeof(m_ViewMatrix));
}

HDC& IRender::GetDC() 
{
	return m_HDC;
}

const RECT& IRender::Dimensions() const
{
	return m_Dimensions;
}

bool IRender::WorldToScreen(const CVector& In, CVector2D& Out) const
{
	auto W = m_ViewMatrix.m_matrix[3][0] * In.x + m_ViewMatrix.m_matrix[3][1] * In.y + m_ViewMatrix.m_matrix[3][2] * In.z +
		m_ViewMatrix.m_matrix[3][3];

	if (W < 0.01f)
		return false;

	Out.x = m_ViewMatrix.m_matrix[0][0] * In.x + m_ViewMatrix.m_matrix[0][1] * In.y + m_ViewMatrix.m_matrix[0][2] * In.z + m_ViewMatrix.m_matrix[0][3];
	Out.y = m_ViewMatrix.m_matrix[1][0] * In.x + m_ViewMatrix.m_matrix[1][1] * In.y + m_ViewMatrix.m_matrix[1][2] * In.z + m_ViewMatrix.m_matrix[1][3];

	const auto invw = 1.f / W;
	Out.x *= invw;
	Out.y *= invw;

	auto X = (float)(m_Width / 2);
	auto Y = (float)(m_Height / 2);

	X += 0.5f * Out.x * m_Width + 0.5f;
	Y -= 0.5f * Out.y * m_Height + 0.5f;

	Out.x = X + m_Dimensions.left;
	Out.y = Y + m_Dimensions.top;

	return true;
}

void IRender::FilledBox(const RECT & Bounds, HBRUSH Brush) const
{
	FillRect(m_HDC, &Bounds, Brush);
}

void IRender::FilledBox(int x, int y, int w, int h, HBRUSH Brush) const
{
	const RECT Bounds = { x, y, x + w, y + h };
	FillRect(m_HDC, &Bounds, Brush);
}

void IRender::BorderedBox(int x, int y, int w, int h, int thickness, HBRUSH Brush) const
{
	FilledBox(x, y, w, thickness, Brush);
	FilledBox(x, y, thickness, h, Brush);
	FilledBox((x + w), y, thickness, h, Brush);
	FilledBox(x, y + h, w + thickness, thickness, Brush);
}

void IRender::Line(int StartX, int StartY, int EndX, int EndY, int thickness, const CColor& Color) const
{
	HPEN hNPen = CreatePen(PS_SOLID, thickness, Color.GetRawColor());
	HPEN hOPen = (HPEN)SelectObject(m_HDC, hNPen);

	MoveToEx(m_HDC, StartX, StartY, NULL);
	LineTo(m_HDC, EndX, EndY);
	DeleteObject(SelectObject(m_HDC, hOPen));
}

void IRender::StringCentred(int x, int y, const CColor& Color, const char* text) const
{
	SelectObject(m_HDC, m_Font);
	SetTextAlign(m_HDC, TA_CENTER | TA_NOUPDATECP);
	SetBkColor(m_HDC, RGB(0, 0, 0));
	SetBkMode(m_HDC, TRANSPARENT);
	SetTextColor(m_HDC, Color.GetRawColor());
	TextOutA(m_HDC, x, y, text, strlen(text));
}

void IRender::StringNotCentred(int x, int y, const CColor& Color, const char * text) const
{
	SelectObject(m_HDC, m_Font);
	SetTextAlign(m_HDC, TA_NOUPDATECP);
	SetBkColor(m_HDC, RGB(0, 0, 0));
	SetBkMode(m_HDC, TRANSPARENT);
	SetTextColor(m_HDC, Color.GetRawColor());
	TextOutA(m_HDC, x, y, text, strlen(text));
}

void IRender::TitleCentred(int x, int y, const CColor& Color, const char* text) const
{
	SelectObject(m_HDC, m_TitleFont);
	SetTextAlign(m_HDC, TA_CENTER | TA_NOUPDATECP);
	SetBkColor(m_HDC, RGB(0, 0, 0));
	SetBkMode(m_HDC, TRANSPARENT);
	SetTextColor(m_HDC, Color.GetRawColor());
	TextOutA(m_HDC, x, y, text, strlen(text));
}

void IRender::TitleNotCentred(int x, int y, const CColor& Color, const char* text) const
{
	SelectObject(m_HDC, m_TitleFont);
	SetTextAlign(m_HDC, TA_NOUPDATECP);
	SetBkColor(m_HDC, RGB(0, 0, 0));
	SetBkMode(m_HDC, TRANSPARENT);
	SetTextColor(m_HDC, Color.GetRawColor());
	TextOutA(m_HDC, x, y, text, strlen(text));
}

int IRender::GetTextWidth(const char * szText) const
{
	SIZE sz;
	SelectObject(m_HDC, m_Font);
	GetTextExtentPoint32A(m_HDC, szText, strlen(szText), &sz);
	return sz.cx;
}

int IRender::GetTextWidthTitle(const char * szText) const
{
	SIZE sz;
	SelectObject(m_HDC, m_TitleFont);
	GetTextExtentPoint32A(m_HDC, szText, strlen(szText), &sz);
	return sz.cx;
}

void IRender::FormatName(const char * pName, char * Buffer, int MaxLength)
{
	static const char szAppend[] = { "..." };
	memset(Buffer, 0, BUFFER_SIZE);

	int Length = strlen(pName);
	bool Append = false;

	if (Length > MaxLength)
	{
		Length = MaxLength;
		Append = true;
	}

	memcpy(Buffer, pName, Length);

	if (Append)
	{
		memcpy(Buffer + Length, szAppend, sizeof(szAppend));
	}

	for (size_t i = 0; i < Length; i++)
	{
		if (Buffer[i] < 0 || (!isalpha(Buffer[i]) && !isdigit(Buffer[i])))
		{
			Buffer[i] = ' ';
		}
		else
		{
			Buffer[i] = std::tolower(Buffer[i]);
		}
	}
}
void IInput::RegisterHotKey(const key_t Key, const std::function<void()>& F)
{
	m_Keys.emplace_back(Key, F);
}

void IInput::EvaluateAllHotKeys() const
{
	for (const auto& i : m_Keys)
	{
		if (KeyDown(i.first))
		{
			i.second();
		}
	}
}

void IInput::EvaluateHotKeyByKey(key_t Key) const
{
	if (KeyDown(Key))
	{
		for (const auto& i : m_Keys)
		{
			if (i.first == Key)
			{
				i.second();
			}
		}
	}
}

void IInput::EvaluateHotKeyByKey(key_t Key, const std::function<void()>& F) const
{
	if (KeyDown(Key))
	{
		F();
	}
}

void IInput::SendMouseInput(DWORD Flags, DWORD X, DWORD Y, DWORD Data, DWORD ExtraInfo) const
{
	mouse_event(Flags, X, Y, Data, ExtraInfo);
}

bool IInput::KeyPressed(key_t Key) const
{
	return (GetAsyncKeyState(Key) & 1);
}

bool IInput::KeyDown(key_t Key) const
{
	return (GetAsyncKeyState(Key) & 0x8000);
}

void IEngine::Update()
{
	if (!FindWindowA(NULL, "Counter-Strike: Global Offensive"))
	{
		ForceShutDown();
	}

	m_dwPlayerInfoItems = Read(g_Memory.ClientState() + IO::PlayerInfoItems, DWORD_PTR);
	m_dwPlayerInfoItems = Read(m_dwPlayerInfoItems + 0x40, DWORD_PTR);
	m_dwPlayerInfoItems = Read(m_dwPlayerInfoItems + 0x0C, DWORD_PTR);
	
	const DWORD sensitivity = Read(g_Memory.Client() + signatures::dwSensitivity, DWORD) ^ 
		(g_Memory.Client() + signatures::dwSensitivityPtr);
	m_flSensivity = *(float*)&sensitivity;

	FillBufRead(g_Memory.Client() + IEngine::IO::LocalPlayer, g_Local());
	g_Local.Update();

	{
		static int iShotsFired = 0;
		static int iLastClip = 0;
		static weaponid_t LastID = 0;

		const bool bMouse1Down = TERANY(g_AimCon.m_pEnabled->Value(), g_Aimbot.GetMouseOneDown(), g_Backtrack.GetMouseOneDown());

		if (g_Menu.IsVisible() || LastID != g_Local.GetActiveWeapon().GV<weaponid_t>(CWeapon::IO::WeaponID) ||
			(!bMouse1Down && 
			(!g_AimCon.m_pEnabled->Value() || !g_Input.KeyDown(g_AimCon.m_pKey->Value())) && 
			(!g_MagnetCon.m_pEnabled->Value() || !g_Input.KeyDown(g_MagnetCon.m_pKey->Value()))
			))
		{
			LastID = g_Local.GetActiveWeapon().GV<weaponid_t>(CWeapon::IO::WeaponID);
			iShotsFired = 0;
		}
		else
		{
			iShotsFired += iLastClip - g_Local.GetActiveWeapon().GV<int>(CWeapon::IO::Clip1);
		}

		iLastClip = g_Local.GetActiveWeapon().GV<int>(CWeapon::IO::Clip1);
		g_Local.SLV<int>(CPlayer::IO::ShotsFired, iShotsFired);
	}

	FillBufRead(g_Memory.Engine() + IEngine::IO::GlobalVars, g_GlobalVars);
	FillBufRead(g_Memory.ClientState() + IEngine::IO::ScreenAngles, m_ScreenAngles);
	m_ScreenAngles2D = CVector2D(m_ScreenAngles.x, m_ScreenAngles.y);
	FillBufRead(g_Memory.Client() + IEngine::IO::GameInput, g_GameInput);
	FillBufRead(g_Memory.ClientState() + IEngine::IO::ChokedCommands, m_iChokedCommands);
	FillBufRead(g_Memory.ClientState() + IEngine::IO::State, m_iState);
	FillBufRead(g_GameInput.m_nCommands + 
		((Read(g_Memory.ClientState() + signatures::clientstate_last_outgoing_command, int) - 1) % MULTIPLAYER_BACKUP) * sizeof(CUserCMD),
		m_LastCMD);
	FillBufRead(g_Memory.Client() + IEngine::IO::GlowPointer, m_dwGlowPointer);
	FillBufRead(g_Memory.Client() + signatures::dwPlayerResource, m_dwPlayerResourceItems);

	m_bIsFocused = GetForegroundWindow() == g_Memory.GetWindow();
	g_Engine.m_flServerTime = (g_Local.GV<int>(CPlayer::IO::TickBase) + 1) * g_GlobalVars.m_flIntervalPerTick;
	m_iTickRate = int((1.f / g_GlobalVars.m_flIntervalPerTick) + 0.5f);
	m_flTickScale = (float)GetTickRate() / 64.f;

	if (m_CurrentMap == Map_t::MAX || GetState() != SIG_FULL || !g_Local.IsValid())
	{
		char buf[64];
		FillBufRead(g_Memory.ClientState() + IO::MapName, buf);

		if (strstr(buf, "de_cache"))
		{
			m_CurrentMap = Map_t::Cache;
		}
		else if (strstr(buf, "de_train"))
		{
			m_CurrentMap = Map_t::Train;
		}
		else if (strstr(buf, "de_overpass"))
		{
			m_CurrentMap = Map_t::Overpass;
		}
		else if (strstr(buf, "de_nuke"))
		{
			m_CurrentMap = Map_t::Nuke;
		}
		else if (strstr(buf, "de_inferno"))
		{
			m_CurrentMap = Map_t::Inferno;
		}
		else if (strstr(buf, "de_dust2"))
		{
			m_CurrentMap = Map_t::Dust2;
		}
		else if (strstr(buf, "de_mirage"))
		{
			m_CurrentMap = Map_t::Mirage;
		}
		else if (strstr(buf, "de_cbble"))
		{
			m_CurrentMap = Map_t::Cobble;
		}
		else if (strstr(buf, "de_office"))
		{
			m_CurrentMap = Map_t::Office;
		}
		else if (strstr(buf, "dz_"))
		{
			m_CurrentMap = Map_t::DangerZone;
		}
		else
		{
			m_CurrentMap = Map_t::MAX;
		}
	}
}

void IEngine::EntryPoint()
{
	CTimer FrameTime;

	while (true)
	{
		m_bInUpdate = true;
		{
			g_Prediction.SetCurrentState(PredictionState_t::Start);
			g_Prediction.EntryPoint();
			g_Engine.Update();
			g_PlayerList.Update();
			g_Prediction.SetCurrentState(PredictionState_t::End);
			g_Prediction.EntryPoint();
		}
		m_bInUpdate = false;

		g_Backtrack.EntryPoint();

		const float flUpdate = 1.f / (float)g_Overlay.GetFrameRate();
		if (FrameTime.Elapsed() > flUpdate)
		{
			FrameTime.Reset();
			InvalidateRect(g_Overlay.GetOverlayWindow(), &g_Render.Dimensions(), TRUE);
		}

		CTimer::SleepThread(g_GlobalVars.m_flIntervalPerTick / 4.f);
	}
}

Map_t IEngine::GetMap() const
{
	return m_CurrentMap;
}

const CUserCMD & IEngine::GetLastCMD() const
{
	return m_LastCMD;
}

float IEngine::GetServerTime() const
{
	return m_flServerTime;
}

int IEngine::GetChokedCommands() const
{
	return m_iChokedCommands;
}

DWORD_PTR IEngine::GetPlayerInfoItems() const
{
	return m_dwPlayerInfoItems;
}

DWORD_PTR IEngine::GetPlayerResourceItems() const
{
	return m_dwPlayerResourceItems;
}

ESigOnState IEngine::GetState() const
{
	return m_iState;
}

DWORD_PTR IEngine::GetGlowPointer() const
{
	return m_dwGlowPointer;
}

bool IEngine::ForceFullUpdate() const
{
	return Write(g_Memory.Engine() + IEngine::IO::DeltaTick, -1);
}

int IEngine::GetLerpTicks() const
{
	const int ud_rate = GetTickRate();
	constexpr float ratio = 1.f;
	constexpr float lerp = 1.f / 128.f;

	return int(max(lerp, (ratio / ud_rate)) + 0.5f);
}

int IEngine::GetTickRate() const
{
	return m_iTickRate;
}

float IEngine::GetTickScale() const
{
	return m_flTickScale;
}

void IEngine::SetViewAnglesSilent(const CVector & Angles, bool Fire)
{
	const auto DesiredCMDNumber = Read(g_Memory.ClientState() + signatures::clientstate_last_outgoing_command, int) + 2;

	const DWORD_PTR pOldUserCmd = g_GameInput.m_nCommands + ((DesiredCMDNumber - 1) % MULTIPLAYER_BACKUP) * sizeof(CUserCMD);
	const DWORD_PTR pVerifiedOldUserCmd = g_GameInput.m_nVerifiedCommands + ((DesiredCMDNumber - 1) % MULTIPLAYER_BACKUP) * sizeof(CVerifiedUserCMD);

	SetSendPacket(false);

	const DWORD_PTR pUserCmd = g_GameInput.m_nCommands + (DesiredCMDNumber % MULTIPLAYER_BACKUP) * sizeof(CUserCMD);

	while (Read(pUserCmd + 0x4, int) < DesiredCMDNumber)
	{
		;
	}

	CUserCMD CMD;
	FillBufRead(pOldUserCmd, CMD);
	memcpy(&m_LastCMD, &CMD, sizeof(CUserCMD));
	CMD.m_vecViewAngles = Angles;

	if (Fire)
	{
		CMD.m_iButtons |= IN_ATTACK;
	}

	const auto delta = Angles - GetScreenAngles();

	CMD.m_iMouseDx = (GetScreenAngles().y - delta.y) / 0.022f;
	CMD.m_iMouseDy = -(GetScreenAngles().x - delta.x) / 0.022f;

	Write(pOldUserCmd, CMD);
	Write(pVerifiedOldUserCmd, CMD);

	if (g_AimCon.m_pDelayPostAim->Value())
	{
		CTimer::SleepThread(g_GlobalVars.m_flIntervalPerTick);
	}

	SetSendPacket(true);
}

void IEngine::SetTickCount(int Tick, bool Fire) const
{
	const auto DesiredCMDNumber = Read(g_Memory.ClientState() + signatures::clientstate_last_outgoing_command, int) + 2;

	const DWORD_PTR pOldUserCmd = g_GameInput.m_nCommands + ((DesiredCMDNumber - 1) % MULTIPLAYER_BACKUP) * sizeof(CUserCMD);
	const DWORD_PTR pVerifiedOldUserCmd = g_GameInput.m_nVerifiedCommands + ((DesiredCMDNumber - 1) % MULTIPLAYER_BACKUP) * sizeof(CVerifiedUserCMD);

	SetSendPacket(false);

	const DWORD_PTR pUserCmd = g_GameInput.m_nCommands + (DesiredCMDNumber % MULTIPLAYER_BACKUP) * sizeof(CUserCMD);

	while (Read(pUserCmd + 0x4, int) < DesiredCMDNumber)
	{
		;
	}

	CUserCMD CMD;
	FillBufRead(pOldUserCmd, CMD);
	CMD.m_iTickCount = Tick;
	g_GlobalVars.SV<int>(CGlobalVarsBase::IO::m_iTickCount, Tick);

	if (Fire)
	{
		CMD.m_iButtons |= IN_ATTACK;
	}

	Write(pOldUserCmd, CMD);
	Write(pVerifiedOldUserCmd, CMD);

	SetSendPacket(true);
}

void IEngine::SetCMD(const CVector & Angles, int Tick, bool Fire) const
{
	const auto DesiredCMDNumber = Read(g_Memory.ClientState() + signatures::clientstate_last_outgoing_command, int) + 2;

	const DWORD_PTR pOldUserCmd = g_GameInput.m_nCommands + ((DesiredCMDNumber - 1) % MULTIPLAYER_BACKUP) * sizeof(CUserCMD);
	const DWORD_PTR pVerifiedOldUserCmd = g_GameInput.m_nVerifiedCommands + ((DesiredCMDNumber - 1) % MULTIPLAYER_BACKUP) * sizeof(CVerifiedUserCMD);

	SetSendPacket(false);

	const DWORD_PTR pUserCmd = g_GameInput.m_nCommands + (DesiredCMDNumber % MULTIPLAYER_BACKUP) * sizeof(CUserCMD);

	while (Read(pUserCmd + 0x4, int) < DesiredCMDNumber)
	{
		;
	}

	CUserCMD CMD;
	FillBufRead(pOldUserCmd, CMD);
	CMD.m_iTickCount = Tick;
	g_GlobalVars.SV<int>(CGlobalVarsBase::IO::m_iTickCount, Tick);
	CMD.m_vecViewAngles = Angles;

	if (Fire)
	{
		CMD.m_iButtons |= IN_ATTACK;
	}

	Write(pOldUserCmd, CMD);
	Write(pVerifiedOldUserCmd, CMD);

	if (g_AimCon.m_pDelayPostAim->Value())
	{
		CTimer::SleepThread(g_GlobalVars.m_flIntervalPerTick);
	}

	SetSendPacket(true);
}

bool IEngine::IsFocused() const
{
	return m_bIsFocused;
}

void IEngine::ForceShutDown()
{
	TerminateProcess(OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, TRUE, GetCurrentProcessId()), 0);
}

bool IEngine::SetScreenAngles(const CVector & Angles)
{
	const bool res = Write(g_Memory.ClientState() + IEngine::IO::ScreenAngles, Angles);
	if (res)
	{
		m_ScreenAngles2D = CVector2D(Angles.x, Angles.y);
		m_ScreenAngles = Angles;
	}

	return res;
}

const CVector & IEngine::GetScreenAngles() const
{
	return m_ScreenAngles;
}

bool IEngine::SetScreenAngles2D(const CVector2D & Angles)
{
	const bool res = Write(g_Memory.ClientState() + IEngine::IO::ScreenAngles, Angles);
	if (res)
	{
		m_ScreenAngles2D = Angles;
		m_ScreenAngles = CVector(Angles.x, Angles.y, m_ScreenAngles.z);
	}

	return res;
}

const CVector2D & IEngine::GetScreenAngles2D() const
{
	return m_ScreenAngles2D;
}

bool IEngine::GetSendPacket() const
{
	return Read(g_Memory.Engine() + IEngine::IO::SendPackets, bool);
}

bool IEngine::IsValid() const
{
	return !m_bInUpdate;
}

void IEngine::SetSensitivity(float flNew) const
{
	auto xorsens = g_Memory.Client() + signatures::dwSensitivityPtr;
	auto sens = *reinterpret_cast<uint32_t*>(&flNew) ^ xorsens;
	Write(g_Memory.Client() + signatures::dwSensitivity, sens);
}

float IEngine::GetSensitivity() const
{
	return m_flSensivity;
}

bool IEngine::SetSendPacket(bool val) const
{
	return Write(g_Memory.Engine() + IEngine::IO::SendPackets, val);
}

bool IEngine::SetInput(DWORD_PTR Add, int Val)
{
	return Write(g_Memory.Client() + Add, Val);
}

int IEngine::GetInput(DWORD_PTR Add) const
{
	return Read(g_Memory.Client() + Add, int);
}

void IPlayerList::Update()
{
	for (size_t it = Start; it < MaxPlayerCount; it++)
	{
		FillBufRead(g_Memory.Client() + IO::PlayerList + it * IO::JumpSize, m_Players[it]());
		m_Players[it].Update();
	}
}

CPlayer & IPlayerList::GetPlayerFromIndex(size_t Index)
{
	return m_Players[Index];
}

const CPlayer & IPlayerList::GetPlayerFromIndex(size_t Index) const
{
	return m_Players[Index];
}

CPlayer& IPlayerList::GetPlayerFromHandle(int hHandle)
{
	return GetPlayerFromIndex((hHandle & 0xFFF) - 1);
}

const CPlayer& IPlayerList::GetPlayerFromHandle(int hHandle) const
{
	return GetPlayerFromIndex((hHandle & 0xFFF) - 1);
}

void IPlayerList::GetWeaponFromHandle(CWeapon & Out, int hHandle) const
{
	Out.Register(hHandle);
}

DWORD_PTR IPlayerList::GetBaseEntityFromHandle(int hHandle) const
{
	return Read(g_Memory.Client() + IO::PlayerList + ((hHandle & 0xFFF) - 1) * IO::JumpSize, DWORD_PTR);
}
