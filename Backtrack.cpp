#include "Backtrack.h"
#include "Controls.h"
#include "Aimbot.h"
#include "Overlay.h"
#include "PlayerPrediction.h"

#include <thread>

CBacktrack g_Backtrack;

void CBacktrack::EntryPoint()
{
	if (!g_Engine.IsFocused() || !(g_Engine.GetState() == SIG_FULL) || !g_Local.IsValid() || g_Menu.IsVisible())
	{
		for (auto& i : m_Records)
		{
			i.clear();
		}

		return;
	}

	Move();

	if (!g_AimCon.m_pEnabled->Value())
	{
		g_Aimbot.CreateInput(m_bMouseOneDown, m_bForceNoInput);
	}
}

const std::vector<Record_t>& CBacktrack::GetRecords(int Index) const
{
	return m_Records[Index];
}

const std::vector<Record_t>& CBacktrack::GetFutureRecords(int Index) const
{
	return m_FutureRecords[Index];
}

bool CBacktrack::GetMouseOneDown() const
{
	return m_bMouseOneDown;
}

bool CBacktrack::RecordValid(const Record_t & Record)
{
	return (g_GlobalVars.m_iTickCount - (g_Math.iTimeToTicks(Record.m_flSimulationTime) + g_Engine.GetLerpTicks())) < 
		g_AimCon.m_pBackTrackTicks->Value() * g_Engine.GetTickScale();
}

bool CBacktrack::PassesStartConditions() const
{
	if (!m_bMouseOneDown)
	{
		return false;
	}

	if (g_AimCon.m_pEnabled->Value())
	{
		return false;
	}

	if (!g_Local.GetActiveWeapon().CanFire())
	{
		return false;
	}

	if (g_Local.GV<int>(CPlayer::IO::ILE::CrosshairID) - 1 >= g_PlayerList.Start &&
		g_Local.GV<int>(CPlayer::IO::ILE::CrosshairID) - 1 < g_PlayerList.MaxPlayerCount)
	{
		return false;
	}

	if (!(g_Local.GetActiveWeapon().IsWeapon() || (g_Local.GV<weaponid_t>(CWeapon::IO::WeaponID) == WEAPON_TASER && !g_SharedCon.m_pZeusBot->Value())))
	{
		return false;
	}

	return true;
}

bool CBacktrack::Update()
{
	static int LastTickCount = g_GlobalVars.m_iTickCount;
	m_Source = g_Local.GetSource();

	if (LastTickCount == g_GlobalVars.m_iTickCount)
	{
		return false;
	}

	LastTickCount = g_GlobalVars.m_iTickCount;

	for (size_t i = g_PlayerList.Start; i < g_PlayerList.MaxPlayerCount; i++)
	{
		const CPlayer& Current = g_PlayerList.GetPlayerFromIndex(i);

		if (!Current.IsEnemy())
		{
			m_Records[i].clear();
			continue;
		}

		if (!m_Records[i].size())
		{
			m_Records[i].emplace_back(&Current, Current.GV<float>(CPlayer::IO::SimulationTime));
			continue;
		}

		if (!RecordValid(m_Records[i].front()))
		{
			m_Records[i].erase(m_Records[i].begin());
		}

		if (!DecimalEqual(Current.GV<float>(CPlayer::IO::SimulationTime), m_Records[i].back().m_flSimulationTime))
		{
			m_Records[i].emplace_back(&Current, Current.GV<float>(CPlayer::IO::SimulationTime));
		}

		if (g_AimCon.m_pForwardTrack->Value())
		{
			m_FutureRecords[i].clear();
			//the limit is lat_in + lat_out + (yours - his)
			int iTicks = int(g_AimCon.m_pForwardTrackTicks->Value() * g_Engine.GetTickScale() + 0.5f);
			Record_t Record(&Current, Current.GV<float>(CPlayer::IO::SimulationTime));

			float& flCurSim = Record.m_flSimulationTime;

			while (iTicks > 0)
			{
				flCurSim += g_Math.TicksToTime(1);
				g_Prediction.Extrapolate(Current, Record.m_BoneMatrices(), flCurSim - Current.GV<float>(CPlayer::IO::SimulationTime));
				m_FutureRecords[i].emplace_back(Record);
				memcpy(&Record.m_BoneMatrices(), &Current.GetBoneMatrices(), sizeof(BaseBoneMatrices_t));
				iTicks--;
			}
		}
	}

	return true;
}

bool CBacktrack::GetTarget()
{
	m_TargetIndex = -1;

	float BestFOV = 39.f;
	m_Screen = g_Engine.GetScreenAngles2D() + g_Local.GV<CVector2D>(CPlayer::IO::ILE::AimPunch) * 2.f;
	g_Math.ClampAngle(m_Screen);

	for (size_t i = g_PlayerList.Start; i < g_PlayerList.MaxPlayerCount; i++)
	{
		const CPlayer& Current = g_PlayerList.GetPlayerFromIndex(i);

		if (!Current.IsEnemy())
		{
			continue;
		}

		if (!m_Records[i].size())
		{
			continue;
		}

		CVector2D Angles; g_Math.VectorAngles(m_Records[i].front().GetBonePosition(g_AimCon.m_pPriorityBone->Value()) - m_Source, Angles);
		g_Math.ClampAngle(Angles);

		const auto FOV = g_Math.GetFOV(m_Screen, Angles);

		if (FOV < BestFOV)
		{
			BestFOV = FOV;
			m_TargetIndex = i;
		}
	}

	return m_TargetIndex != -1;
}

bool CBacktrack::GetBestRecord()
{
	m_pBestRecord = nullptr;
	float BestFOV = 39.f;

	for (size_t i = 0; i < m_Records[m_TargetIndex].size(); i++)
	{
		if (!RecordValid(m_Records[m_TargetIndex][i]))
		{
			continue;
		}

		CVector2D Angles; g_Math.VectorAngles(m_Records[m_TargetIndex][i].GetBonePosition(g_AimCon.m_pPriorityBone->Value()) - m_Source, Angles);
		g_Math.ClampAngle(Angles);

		const auto FOV = g_Math.GetFOV(m_Screen, Angles);

		if (FOV < BestFOV)
		{
			BestFOV = FOV;
			m_pBestRecord = &m_Records[m_TargetIndex][i];
		}
	}

	if (g_AimCon.m_pForwardTrack->Value())
	{
		for (size_t i = 0; i < m_FutureRecords[m_TargetIndex].size(); i++)
		{
			CVector2D Angles; g_Math.VectorAngles(m_FutureRecords[m_TargetIndex][i].GetBonePosition(g_AimCon.m_pPriorityBone->Value()) - m_Source, Angles);
			g_Math.ClampAngle(Angles);

			const auto FOV = g_Math.GetFOV(m_Screen, Angles);

			if (FOV < BestFOV)
			{
				BestFOV = FOV;
				m_pBestRecord = &m_FutureRecords[m_TargetIndex][i];
			}
		}
	}

	return m_pBestRecord != nullptr && 
		(g_Math.iTimeToTicks(m_pBestRecord->m_flSimulationTime) + g_Engine.GetLerpTicks()) != g_GlobalVars.m_iTickCount;
}

bool CBacktrack::Move()
{
	m_bMouseOneDown = g_Input.KeyDown(VK_LBUTTON);
	m_bForceNoInput = false;

	if (!g_AimCon.m_pBackTrack->Value())
	{
		return false;
	}

	Update();

	if (!PassesStartConditions())
	{
		return false;
	}

	if (!GetTarget())
	{
		return false;
	}

	if (!GetBestRecord())
	{
		return false;
	}

	if (!g_Math.PassesActivationRate(g_AimCon.m_pBackTrackRate->Value()))
	{
		return false;
	}

	m_bForceNoInput = true;
	g_Engine.SetTickCount(g_Math.iTimeToTicks(m_pBestRecord->m_flSimulationTime) + g_Engine.GetLerpTicks(), true);

	if (g_ESPCon.m_pAimInfo->Value() && g_ESPCon.m_pEnabled->Value())
	{
		static char Buffer[BUFFER_SIZE];
		sprintf(Buffer, "backtrack: %s", g_PlayerList.GetPlayerFromIndex(m_TargetIndex).GetPlayerInfo().m_szPlayerName);
		g_Menu.AddNewDebugMessage(Buffer, 0.500f);
	}

	return true;
}

Record_t::Record_t() :
	m_flSimulationTime(g_Math.TicksToTime(g_GlobalVars.m_iTickCount)), m_pPlayer(nullptr), m_BoneMatrices()
{
	;
}

Record_t::Record_t(const CPlayer * P, float sim)
	: m_pPlayer(P), m_flSimulationTime(sim), m_BoneMatrices(P->GetBoneMatrices())
{
	;
}

bool Record_t::operator==(const Record_t & rhs) const
{
	return *m_pPlayer == *(rhs.m_pPlayer) && DecimalEqual(m_flSimulationTime, rhs.m_flSimulationTime);
}

CVector Record_t::GetBonePosition(int Bone) const
{
	return m_BoneMatrices().GetBonePosition(Bone);
}

bool Record_t::IsValid() const
{
	return CBacktrack::RecordValid(*this);
}
