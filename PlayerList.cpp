#include "PlayerList.h"
#include "Controls.h"

CPlayerListInfo g_PlayerListInfo;
CSpectatorList g_SpectatorList;

CPlayerListInfo::CPlayerListInfo() :
	CWindow({ 50, 50, 465, 450 }, "player_list", M_C(, g_Engine.GetState() == SIG_FULL))
{
	m_nTab = AddNewTab("players");

	m_nSubNames = GetTab(m_nTab).AddNewSubSection("name");
	m_nSubMain = GetTab(m_nTab).AddNewSubSection("main");
	m_nSubInfo = GetTab(m_nTab).AddNewSubSection("info");
}

void CPlayerListInfo::Control()
{
	if (!IsVisible())
	{
		return;
	}

	CTab& Tab = GetTab(m_nTab);

	int iValidEntities = 0;
	for (size_t i = g_PlayerList.Start; i < g_PlayerList.MaxPlayerCount; i++)
	{
		const CPlayer& Player = g_PlayerList.GetPlayerFromIndex(i);

		if (!Player())
		{
			continue;
		}

		if (Player.GetIndex() < g_PlayerList.Start || Player.GetIndex() >= g_PlayerList.MaxPlayerCount)
		{
			continue;
		}

		iValidEntities++;
		{
			static char _buffer[BUFFER_SIZE];
			g_Render.FormatName(Player.GetPlayerInfo().m_szPlayerName, _buffer, 13);
			sprintf(m_Buffer, "(%s) %d: %s", szTeams[Player.GV<int>(CPlayer::IO::Team)], Player.GetIndex(), _buffer);
		}
		Tab.GetSubSection(m_nSubNames).AddNewTextNote(m_Buffer);

		const weaponid_t Weapon = Player.GetActiveWeapon().GV<weaponid_t>(CWeapon::IO::WeaponID);
		const char* szWeapon = TERANY(Weapon >= 0 && Weapon < szWeaponNames.size(), szWeaponNames[Weapon], "knife");
		sprintf(m_Buffer, "active: %d, weap: %s",
			(int)Player.GV<bool>(CPlayer::IO::Dormant), szWeapon);
		Tab.GetSubSection(m_nSubMain).AddNewTextNote(m_Buffer);

		sprintf(m_Buffer, "%s, %s",
			szCompetitiveRanks[std::clamp(Player.GetCompetitiveRank() + 1, 0, 20)], Player.GetPlayerInfo().m_szSteamID);	
		Tab.GetSubSection(m_nSubInfo).AddNewTextNote(m_Buffer);
	}

	RECT New = GetRenderBounds();
	New.bottom = New.top + (iValidEntities + 2) * (FONT_HEIGHT + OBJECT_GAP + 1);
	SetRenderBounds(New);

	CWindow::Control();
}

void CPlayerListInfo::Print()
{
	CWindow::Print();

	CTab& Tab = GetTab(m_nTab);

	Tab.GetSubSection(m_nSubNames).Clear();
	Tab.GetSubSection(m_nSubMain).Clear();
	Tab.GetSubSection(m_nSubInfo).Clear();
}

bool CSpectatorList::PlayerValid(const CPlayer& Player, int TargetIndex) const
{
	if (!Player())
	{
		return false;
	}

	if (Player.GetIndex() < g_PlayerList.Start || Player.GetIndex() >= g_PlayerList.MaxPlayerCount)
	{
		return false;
	}

	if (Player.GV<int>(CPlayer::IO::Health) > 0)
	{
		return false;
	}

	if (Player.GV<bool>(CPlayer::IO::Dormant) || Player.GV<int>(netvars::m_iObserverMode) == OBS_NONE)
	{
		return false;
	}

	if (TargetIndex < g_PlayerList.Start || TargetIndex >= g_PlayerList.MaxPlayerCount)
	{
		return false;
	}

	const CPlayer& Target = g_PlayerList.GetPlayerFromIndex(TargetIndex);
	if (!Target())
	{
		return false;
	}

	switch (g_ESPCon.m_pSpectatorListMode->Value())
	{
	case SPEC_LIST_MODE_ALL:
		break;

	case SPEC_LIST_MODE_LOCAL:
		if (TargetIndex != g_Local.GetIndex())
		{
			return false;
		}
		break;

	default:
		break;
	}

	return true;
}

CSpectatorList::CSpectatorList() :
	CWindow({LONG(FONT_HEIGHT * 1.5f + 0.5f), 310, LONG(FONT_HEIGHT * 1.5f + 250.5f), 300}, "spectator_list")
{
	m_nTab = AddNewTab("players");
	
	m_nSubNames = GetTab(m_nTab).AddNewSubSection("name");
	m_nSubTargets = GetTab(m_nTab).AddNewSubSection("target");
	m_nSubObserverModes = GetTab(m_nTab).AddNewSubSection("observer_mode");
}

void CSpectatorList::Initialise()
{
	g_Menu.AddNewButton(LONG_WIDTH, OBJECT_HEIGHT, "spectator_list", [this]() -> void { SetVisible(!GetVisible()); }, M_C(, g_Engine.GetState() == SIG_FULL));
}

void CSpectatorList::EntryPoint()
{
	this->Control();
	this->Print();
}

void CSpectatorList::Control()
{
	if (!IsVisible())
	{
		return;
	}

	CTab& Tab = GetTab(m_nTab);

	int iValidEntities = 0;
	for (size_t i = g_PlayerList.Start; i < g_PlayerList.MaxPlayerCount; i++)
	{
		const CPlayer& Player = g_PlayerList.GetPlayerFromIndex(i);
		const int TargetIndex = (Player.GV<int>(CPlayer::IO::ObserverTarget) & 0xFFF) - 1;
		
		if (!PlayerValid(Player, TargetIndex))
		{
			continue;
		}

		const CPlayer& Target = g_PlayerList.GetPlayerFromIndex(TargetIndex);

		static char _buffer[BUFFER_SIZE];

		iValidEntities++;
		{
			g_Render.FormatName(Player.GetPlayerInfo().m_szPlayerName, _buffer, 8);
			sprintf(m_Buffer, "(%s) %d: %s", szTeams[Player.GV<int>(CPlayer::IO::Team)], Player.GetIndex(), _buffer);
		}
		Tab.GetSubSection(m_nSubNames).AddNewTextNote(m_Buffer);

		{
			g_Render.FormatName(Target.GetPlayerInfo().m_szPlayerName, _buffer, 8);
			sprintf(m_Buffer, "(%s) %d: %s", szTeams[Target.GV<int>(CPlayer::IO::Team)], Target.GetIndex(), _buffer);
		}
		Tab.GetSubSection(m_nSubTargets).AddNewTextNote(m_Buffer);

		const int obs = std::clamp(Player.GV<int>(netvars::m_iObserverMode), 0, 6);
		Tab.GetSubSection(m_nSubObserverModes).AddNewTextNote(szObserverModes[obs + 1]);
	}

	RECT New = GetRenderBounds();
	New.bottom = New.top + (iValidEntities + 2) * (FONT_HEIGHT + OBJECT_GAP + 1);
	SetRenderBounds(New);

	if (g_Menu.IsVisible())
	{
		CWindow::Control();
	}
}

void CSpectatorList::Print()
{
	CWindow::Print();
	
	CTab& Tab = GetTab(m_nTab);

	Tab.GetSubSection(m_nSubNames).Clear();
	Tab.GetSubSection(m_nSubTargets).Clear();
	Tab.GetSubSection(m_nSubObserverModes).Clear();
}
