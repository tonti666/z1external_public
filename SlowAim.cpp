#include "SlowAim.h"
#include "Controls.h"
#include "PlayerPrediction.h"

CSlowAim g_SlowAim;

bool CSlowAim::GetTarget()
{
	m_iTargetIndex = -1;
	m_flBestFOV = g_SlowAimCon.m_pFOV->Value();

	CVector Source(g_Local.GetSource());
	g_Prediction.Extrapolate(g_Local, Source, g_Math.TicksToTime(g_AimCon.m_pLocalExtrapolationTicks->Value() * g_Engine.GetTickScale()));

	const CVector2D Screen(g_Engine.GetScreenAngles().x, g_Engine.GetScreenAngles().y);

	for (size_t i = g_PlayerList.Start; i < g_PlayerList.MaxPlayerCount; i++)
	{
		const CPlayer& Player = g_PlayerList.GetPlayerFromIndex(i);

		if (!PassesIntermediateConditions(Player))
		{
			continue;
		}

		CVector Position = Player.GetBonePosition(g_SlowAimCon.m_pBone->Value());
		g_Prediction.Extrapolate(Player, Position, g_Math.TicksToTime(g_AimCon.m_pEnemyExtrapolaitonTicks->Value() * g_Engine.GetTickScale()));

		CVector2D Angles;
		g_Math.VectorAngles(Position - Source, Angles);

		if (g_Local.GV<int>(CPlayer::IO::ShotsFired) > 1)
		{
			Angles -= g_Local.GV<CVector2D>(CPlayer::IO::ILE::AimPunch) * 2.f;
		}

		g_Math.ClampAngle(Angles);

		const float flFOV = g_Math.GetFOV(Screen, Angles);
		if (flFOV < m_flBestFOV)
		{
			m_flBestFOV = flFOV;
			m_iTargetIndex = i;
		}
	}

	return m_iTargetIndex != -1;
}

bool CSlowAim::PassesStartConditions() const
{
	if (!g_SlowAimCon.m_pEnabled->Value())
	{
		return false;
	}

	if (g_SlowAimCon.m_pKey->Value())
	{
		if (g_SlowAimCon.m_pKey->Value())
		{
			static bool bToggled = false;
			if (g_Input.KeyPressed(g_SlowAimCon.m_pKey->Value()))
			{
				bToggled = !bToggled;
			}

			if (!bToggled)
			{
				return false;
			}
		}
		else if (!g_Input.KeyDown(g_SlowAimCon.m_pKey->Value()))
		{
			return false;
		}
	}

	if (!(g_Local.GV<int>(CPlayer::IO::Flags) & FL_ONGROUND) &&
		g_SlowAimCon.m_pDisableWhileX->Value()[SLOWAIM_DISABLE_JUMPING].m_bSelected)
	{
		return false;
	}

	const float flSpeed = g_Local.GV<CVector2D>(CPlayer::IO::Velocity).Magnitude();
	const bool bWalking = g_Local.GV<bool>(CPlayer::IO::IsWalking);

	if (!DecimalEnabled(flSpeed) &&
		g_SlowAimCon.m_pDisableWhileX->Value()[SLOWAIM_DISABLE_STATIONARY].m_bSelected)
	{
		return false;
	}

	if (bWalking &&
		g_SlowAimCon.m_pDisableWhileX->Value()[SLOWAIM_DISABLE_WALKING].m_bSelected)
	{
		return false;
	}

	if (!bWalking && flSpeed > g_SharedCon.m_pRunningVelocityDefinition->Value() &&
		g_SlowAimCon.m_pDisableWhileX->Value()[SLOWAIM_DISABLE_RUNNING].m_bSelected)
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

bool CSlowAim::PassesIntermediateConditions(const CPlayer & Player)
{
	if (!Player.IsEnemy())
	{
		return false;
	}

	if (!Player.IsVisible())
	{
		m_VisibleTimes[Player.GetIndex()].Reset();
		return false;
	}

	if (!(Player.GV<int>(CPlayer::IO::Flags) & FL_ONGROUND) &&
		g_SlowAimCon.m_pDisableWhileX->Value()[SLOWAIM_DISABLE_JUMPING].m_bSelected)
	{
		return false;
	}

	const float flSpeed = Player.GV<CVector2D>(CPlayer::IO::Velocity).Magnitude();
	const bool bWalking = Player.GV<bool>(CPlayer::IO::IsWalking);

	if (!DecimalEnabled(flSpeed) &&
		g_SlowAimCon.m_pDisableWhileX->Value()[SLOWAIM_DISABLE_STATIONARY].m_bSelected)
	{
		return false;
	}

	if (bWalking &&
		g_SlowAimCon.m_pDisableWhileX->Value()[SLOWAIM_DISABLE_WALKING].m_bSelected)
	{
		return false;
	}

	if (!bWalking && flSpeed > g_SharedCon.m_pRunningVelocityDefinition->Value() &&
		g_SlowAimCon.m_pDisableWhileX->Value()[SLOWAIM_DISABLE_RUNNING].m_bSelected)
	{
		return false;
	}

	return true;
}

bool CSlowAim::PassesFinalConditions() const
{
	if (DecimalEnabled(g_SlowAimCon.m_pFOVRequirement->Value()))
	{
		if (m_FOVTimer.Ticks() < g_SlowAimCon.m_pFOVRequirement->Value() * g_Engine.GetTickScale())
		{
			return false;
		}
	}

	if (DecimalEnabled(g_SlowAimCon.m_pVisibleRequirement->Value()))
	{
		if (m_VisibleTimes[m_iTargetIndex].Ticks() < g_SlowAimCon.m_pVisibleRequirement->Value() * g_Engine.GetTickScale())
		{
			return false;
		}
	}

	const auto GetMaxShots = []() -> int
	{
		if (g_SlowAimCon.m_pMaxShots->Value() == ALWAYS_RCS_SHOTS)
		{
			return INT_MAX;
		}

		return g_SlowAimCon.m_pMaxShots->Value();
	};

	const int Shots = g_Local.GV<int>(CPlayer::IO::ShotsFired);
	if (Shots < g_SlowAimCon.m_pMinShots->Value() ||
		Shots > GetMaxShots())
	{
		return false;
	}

	return true;
}

void CSlowAim::EntryPoint()
{
	static const float flStartSensitivity = g_Engine.GetSensitivity();

	const auto Reset = []() -> void
	{
		if (!DecimalEqual(flStartSensitivity, g_Engine.GetSensitivity()))
		{
			g_Engine.SetSensitivity(flStartSensitivity);
		}
	};

	const auto FullReset = [&]() -> void
	{
		m_FOVTimer.Reset();
		Reset();
	};

	if (!PassesStartConditions())
	{
		FullReset();
		return;
	}

	if (!GetTarget())
	{
		FullReset();
		return;
	}

	if (!PassesFinalConditions())
	{
		Reset();
		return;
	}

	const float NewSensitivity = std::clamp((m_flBestFOV / g_SlowAimCon.m_pFOV->Value()) * flStartSensitivity * g_SlowAimCon.m_pFactor->Value(),
		flStartSensitivity, g_SlowAimCon.m_pMinSensitivity->Value() * flStartSensitivity);

	if (!DecimalEqual(NewSensitivity, g_Engine.GetSensitivity()))
	{
		g_Engine.SetSensitivity(NewSensitivity);
	}
}
