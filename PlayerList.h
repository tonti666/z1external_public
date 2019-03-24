#pragma once
#include "Classes.h"
#include "Engine.h"
#include "Menu.h"

class CPlayerListInfo : public CWindow
{
private:
	char m_Buffer[BUFFER_SIZE];

	id_t m_nTab;

	id_t m_nSubNames;
	id_t m_nSubMain;
	id_t m_nSubInfo;

public:
	CPlayerListInfo();
	~CPlayerListInfo() = default;

	void Control() override;
	void Print() override;
};

extern CPlayerListInfo g_PlayerListInfo;

class CSpectatorList : public CWindow, public CBaseHack
{
private:
	char m_Buffer[BUFFER_SIZE];

	id_t m_nTab;

	id_t m_nSubNames;
	id_t m_nSubTargets;
	id_t m_nSubObserverModes;

	bool PlayerValid(const CPlayer& Player, int TargetIndex) const;

public:
	CSpectatorList();
	~CSpectatorList() = default;
	void Initialise();

	void EntryPoint() override;

	void Control() override;
	void Print() override;
};

extern CSpectatorList g_SpectatorList;