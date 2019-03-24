#include "TriggerBot.h"
#include "Controls.h"

CTriggerBot g_TriggerBot;

bool CTriggerBot::PassesStartConditions() const
{
	if (!g_TriggerCon.m_pEnabled->Value())
	{
		return false;
	}

	if (g_TriggerCon.m_pKey->Value())
	{
		if (g_TriggerCon.m_pKey->Toggle())
		{
			static bool bToggled = false;
			if (g_Input.KeyPressed(g_TriggerCon.m_pKey->Value()))
			{
				bToggled = !bToggled;
			}

			if (bToggled)
			{
				return false;
			}
		}
		else if (!g_Input.KeyDown(g_TriggerCon.m_pKey->Value()))
		{
			return false;
		}
	}

	if (!(g_Local.GV<int>(CPlayer::IO::Flags) & FL_ONGROUND) &&
		g_TriggerCon.m_pDisableWhileX->Value()[TRIGGER_DISABLE_JUMPING].m_bSelected)
	{
		return false;
	}

	const float flSpeed = g_Local.GV<CVector2D>(CPlayer::IO::Velocity).Magnitude();
	const bool bWalking = g_Local.GV<bool>(CPlayer::IO::IsWalking);

	if (!DecimalEnabled(flSpeed) &&
		g_TriggerCon.m_pDisableWhileX->Value()[TRIGGER_DISABLE_STATIONARY].m_bSelected)
	{
		return false;
	}

	if (bWalking &&
		g_TriggerCon.m_pDisableWhileX->Value()[TRIGGER_DISABLE_WALKING].m_bSelected)
	{
		return false;
	}

	if (!bWalking && flSpeed > g_SharedCon.m_pRunningVelocityDefinition->Value() &&
		g_TriggerCon.m_pDisableWhileX->Value()[TRIGGER_DISABLE_RUNNING].m_bSelected)
	{
		return false;
	}

	if (DecimalEnabled(g_TriggerCon.m_pMaxLocalVelocity->Value()) &&
		flSpeed > g_TriggerCon.m_pMaxLocalVelocity->Value())
	{
		return false;
	}

	if (!g_Local.GetActiveWeapon().IsWeapon())
	{
		return false;
	}

	return true;
}

bool CTriggerBot::PassesIntermediateConditions(const CPlayer & Player)
{
	if (!Player.IsEnemy())
	{
		return false;
	}

	if (!(Player.GV<int>(CPlayer::IO::Flags) & FL_ONGROUND) &&
		g_TriggerCon.m_pDisableWhileX->Value()[TRIGGER_DISABLE_JUMPING].m_bSelected)
	{
		return false;
	}

	const float flSpeed = Player.GV<CVector2D>(CPlayer::IO::Velocity).Magnitude();
	const bool bWalking = Player.GV<bool>(CPlayer::IO::IsWalking);

	if (!DecimalEnabled(flSpeed) &&
		g_TriggerCon.m_pDisableWhileX->Value()[TRIGGER_DISABLE_STATIONARY].m_bSelected)
	{
		return false;
	}

	if (bWalking &&
		g_TriggerCon.m_pDisableWhileX->Value()[TRIGGER_DISABLE_WALKING].m_bSelected)
	{
		return false;
	}

	if (!bWalking && flSpeed > g_SharedCon.m_pRunningVelocityDefinition->Value() &&
		g_TriggerCon.m_pDisableWhileX->Value()[TRIGGER_DISABLE_RUNNING].m_bSelected)
	{
		return false;
	}

	return true;
}

bool CTriggerBot::PassesFinalConditions(int Index) 
{
	if (DecimalEnabled(g_TriggerCon.m_pVisibleTicks->Value()))
	{
		if (m_VisibleTimes[Index].Ticks() < g_TriggerCon.m_pVisibleTicks->Value() * g_Engine.GetTickScale())
		{
			return false;
		}
	}

	{
		static int Last = 0;
		if (Last != Index)
		{
			Last = Index;
			m_ReqShotTime.Reset();
		}
	}

	return true;
}

void CTriggerBot::EntryPoint()
{
	if (!PassesStartConditions())
	{
		return;
	}

	for (size_t i = 0; i < g_PlayerList.MaxPlayerCount; i++)
	{
		const CPlayer& Player = g_PlayerList.GetPlayerFromIndex(i);

		if (!Player.IsEnemy())
		{
			continue;
		}

		if (!Player.IsVisible() &&
			g_TriggerCon.m_pDisableWhileX->Value()[TRIGGER_DISABLE_NOT_VISIBLE].m_bSelected)
		{
			m_VisibleTimes[Player.GetIndex()].Reset();
		}
	}

	const int Index = g_Local.GV<int>(CPlayer::IO::ILE::CrosshairID) - 1;
	if (Index < g_PlayerList.Start || Index >= g_PlayerList.MaxPlayerCount)
	{
		return;
	}

	if (!PassesIntermediateConditions(g_PlayerList.GetPlayerFromIndex(Index)))
	{
		return;
	}

	if (!PassesFinalConditions(Index))
	{
		return;
	}

	if (!DecimalEnabled(g_TriggerCon.m_pDelayShotTicks->Value()) || m_ReqShotTime.Ticks() > g_TriggerCon.m_pDelayShotTicks->Value() * g_Engine.GetTickScale())
	{	
		g_Engine.SetInput(IEngine::IO::ForceAttack, 6);
	}
}
