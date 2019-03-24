#pragma once
#include "Classes.h"
#include "Color.h"
#include "Math.h"
#include "Offsets.h"
#include "Structs.h"
#include "Entity.h"
#include <Windows.h>

typedef int key_t;

class IRender : public CBaseUtility
{
public:
	class IO : public CBaseNonInstantiable
	{
	public:
		static constexpr DWORD_PTR ViewMatrix = signatures::dwViewMatrix;
	};

private:
	HDC m_HDC;
	HFONT m_Font;
	HFONT m_TitleFont;

	RECT m_Dimensions; size_t m_Width, m_Height;
	Matrix4_t m_ViewMatrix;

public:
	void Initialize();
	void OnFrame(HDC DC);

	HDC& GetDC();

	const RECT& Dimensions() const;
	bool WorldToScreen(const CVector& In, CVector2D& Out) const;

	void FilledBox(const RECT& Bounds, HBRUSH Brush) const;
	void FilledBox(int x, int y, int w, int h, HBRUSH Brush) const;
	void BorderedBox(int x, int y, int w, int h, int thickness, HBRUSH Brush) const;
	void Line(int StartX, int StartY, int EndX, int EndY, int thickness, const CColor& Color) const;

	void StringCentred(int x, int y, const CColor& Color, const char* text) const;
	void StringNotCentred(int x, int y, const CColor& Color, const char* text) const;
	void TitleCentred(int x, int y, const CColor& Color, const char* text) const;
	void TitleNotCentred(int x, int y, const CColor& Color, const char* text) const;

	int GetTextWidth(const char* szText) const;
	int GetTextWidthTitle(const char* szText) const;

	void FormatName(const char* pName, char* Buffer, int MaxLength);
};

extern IRender g_Render;

class IInput : public CBaseUtility
{
private:
	std::vector<std::pair<key_t, std::function<void()>>> m_Keys;

public:
	void SendMouseInput(DWORD Flags = 0, DWORD X = 0, DWORD Y = 0, DWORD Data = 0, DWORD ExtraInfo = 0) const;
	bool KeyPressed(key_t Key) const;
	bool KeyDown(key_t Key) const;

	void RegisterHotKey(key_t Key, const std::function<void()>& F);

	void EvaluateAllHotKeys() const;
	void EvaluateHotKeyByKey(key_t Key) const;
	void EvaluateHotKeyByKey(key_t Key, const std::function<void()>& F) const;
};

extern IInput g_Input;

class IPlayerList : public CBaseUtility
{
public:
	static constexpr size_t Start = 0;
	static constexpr size_t MaxPlayerCount = 65;

	class IO : public CBaseNonInstantiable
	{
	public:
		static constexpr DWORD_PTR PlayerList = signatures::dwEntityList;
		static constexpr DWORD_PTR JumpSize = 0x10;
	};

private:
	friend class IEngine;
	void Update();
	CPlayer m_Players[MaxPlayerCount];

public:
	CPlayer& GetPlayerFromIndex(size_t Index);
	const CPlayer& GetPlayerFromIndex(size_t Index) const;

	CPlayer& GetPlayerFromHandle(int hHandle);
	const CPlayer& GetPlayerFromHandle(int hHandle) const;

	void GetWeaponFromHandle(CWeapon& Out, int hHandle) const;
	DWORD_PTR GetBaseEntityFromHandle(int hHandle) const;
};

extern IPlayerList g_PlayerList;

class IEngine : public CBaseThread
{
public:
	class IO : public CBaseNonInstantiable
	{
	public:
		static constexpr DWORD_PTR LocalPlayer = signatures::dwLocalPlayer;
		static constexpr DWORD_PTR ForceJump = signatures::dwForceJump;
		static constexpr DWORD_PTR ForceAttack = signatures::dwForceAttack;
		static constexpr DWORD_PTR ForceLeft = signatures::dwForceLeft;
		static constexpr DWORD_PTR ForceRight = signatures::dwForceRight;
		static constexpr DWORD_PTR ForceForward = signatures::dwForceForward;
		static constexpr DWORD_PTR ForceBackward = signatures::dwForceBackward;
		static constexpr DWORD_PTR ScreenAngles = signatures::dwClientState_ViewAngles;
		static constexpr DWORD_PTR GlowPointer = signatures::dwGlowObjectManager;
		static constexpr DWORD_PTR SendPackets = signatures::dwbSendPackets;
		static constexpr DWORD_PTR GameInput = signatures::dwInput;
		static constexpr DWORD_PTR State = signatures::dwClientState_State;
		static constexpr DWORD_PTR ChokedCommands = signatures::clientstate_choked_commands;
		static constexpr DWORD_PTR DeltaTick = signatures::clientstate_delta_ticks;
		static constexpr DWORD_PTR MapName = signatures::dwClientState_Map;
		static constexpr DWORD_PTR PlayerInfoItems = signatures::dwClientState_PlayerInfo;
		static constexpr DWORD_PTR GlobalVars = signatures::dwGlobalVars;
	};

private:
	float m_flServerTime, m_flSensivity, m_flTickScale;
	bool m_bIsFocused, m_bInUpdate;
	CVector m_ScreenAngles;
	CVector2D m_ScreenAngles2D;

	DWORD_PTR m_dwPlayerInfoItems, m_dwPlayerResourceItems;
	DWORD_PTR m_dwGlowPointer;

	int m_iChokedCommands, m_iTickRate;
	ESigOnState m_iState;

	CUserCMD m_LastCMD;
	Map_t m_CurrentMap;

	void Update();
public:
	void EntryPoint() override;

	Map_t GetMap() const;
	const CUserCMD& GetLastCMD() const;
	float GetServerTime() const;
	int GetChokedCommands() const;
	DWORD_PTR GetPlayerInfoItems() const;
	DWORD_PTR GetPlayerResourceItems() const;
	ESigOnState GetState() const;
	DWORD_PTR GetGlowPointer() const;

	bool ForceFullUpdate() const;
	int GetLerpTicks() const;
	int GetTickRate() const;
	float GetTickScale() const;

	void SetViewAnglesSilent(const CVector& Angles, bool Fire);
	void SetTickCount(int Tick, bool Fire) const;
	void SetCMD(const CVector& Angles, int Tick, bool Fire) const;

	bool IsFocused() const;
	void ForceShutDown();

	bool SetScreenAngles(const CVector& Angles);
	const CVector& GetScreenAngles() const;

	bool SetScreenAngles2D(const CVector2D& Angles);
	const CVector2D& GetScreenAngles2D() const;

	bool SetInput(DWORD_PTR Add, int Val);
	int GetInput(DWORD_PTR Add) const;

	bool SetSendPacket(bool val) const;
	bool GetSendPacket() const;

	bool IsValid() const;

	void SetSensitivity(float flNew) const;
	float GetSensitivity() const;
};

extern IEngine g_Engine;