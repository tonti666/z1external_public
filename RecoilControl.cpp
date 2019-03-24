#include "RecoilControl.h"
#include "Engine.h"
#include "Controls.h"
#include "Math.h"
#include "Aimbot.h"

CRecoilControl g_RecoilControl;

bool CRecoilControl::PassesStartConditions() const
{
	const auto GetMaxShots = []() -> int
	{
		if (g_RCSCon.m_pShotsMax->Value() == ALWAYS_RCS_SHOTS)
		{
			return INT_MAX;
		}

		return g_RCSCon.m_pShotsMax->Value();
	};

	if (!g_RCSCon.m_pEnabled->Value() ||
		g_Local.GetActiveWeapon().GV<int>(CWeapon::IO::Clip1) <= 0)
	{
		return false;
	}

	const int shots = g_Local.GV<int>(CPlayer::IO::ShotsFired);
	if (shots < g_RCSCon.m_pShotsMin->Value() ||
		shots > GetMaxShots())
	{
		return false;
	}

	if (DecimalEnabled(g_RCSCon.m_pCoolDown->Value()))
	{
		if (g_RCSCon.m_pCoolDown->Value() > g_Aimbot.GetLastTargetTime().Ticks() * g_Engine.GetTickScale())
		{
			return false;
		}
	}

	if (!g_Local.GetActiveWeapon().IsRifle() && !g_Local.GetActiveWeapon().IsSMG())
	{
		return false;
	}

	return true;
}

void CRecoilControl::EntryPoint()
{
	if (!PassesStartConditions())
	{
		return;
	}

	const auto GetXRecoilControl = []() -> float
	{
		if (!DecimalEnabled(g_RCSCon.m_pRCSXDeviation->Value()))
		{
			return g_RCSCon.m_pRCSX->Value();
		}

		return g_Math.RandomNumber(g_RCSCon.m_pRCSX->Value(), g_RCSCon.m_pRCSXDeviation->Value());
	};

	const auto GetYRecoilControl = []() -> float
	{
		if (!DecimalEnabled(g_RCSCon.m_pRCSYDeviation->Value()))
		{
			return g_RCSCon.m_pRCSY->Value();
		}

		return g_Math.RandomNumber(g_RCSCon.m_pRCSY->Value(), g_RCSCon.m_pRCSYDeviation->Value());
	};

	const auto GetSmooth = []() -> float
	{
		if (!DecimalEnabled(g_RCSCon.m_pRCSSmoothDeviation->Value()))
		{
			return g_RCSCon.m_pRCSSmooth->Value();
		}

		return g_Math.RandomNumber(g_RCSCon.m_pRCSSmooth->Value(), g_RCSCon.m_pRCSSmoothDeviation->Value());
	};

	const CVector2D Punch = g_Local.GV<CVector2D>(CPlayer::IO::ILE::AimPunch);

	CVector2D Angles
	(
		(Punch.x - m_LastPunch.x) * GetYRecoilControl(),
		(Punch.y - m_LastPunch.y) * GetXRecoilControl()
	);

	g_Math.SmoothAngleRelative(g_Engine.GetScreenAngles2D(), Angles, GetSmooth());

	const CVector2D Movement
	(
		(Angles.y) / (0.022f * g_Engine.GetSensitivity()),
		-(Angles.x) / (0.022f  * g_Engine.GetSensitivity())
	);

	g_Input.SendMouseInput(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, Movement.x, Movement.y);
}

void CRecoilControl::LogPunch()
{
	static weaponid_t LastID = g_Local.GetActiveWeapon().GV<weaponid_t>(CWeapon::IO::WeaponID);
	if (g_Local.GetActiveWeapon().GV<weaponid_t>(CWeapon::IO::WeaponID) != LastID)
	{
		m_LastPunch = CVector2D();
		g_Local.SLV<int>(CPlayer::IO::ShotsFired, 0);
		LastID = g_Local.GetActiveWeapon().GV<weaponid_t>(CWeapon::IO::WeaponID);
	}
	else
	{
		m_LastPunch = g_Local.GV<CVector2D>(CPlayer::IO::ILE::AimPunch);
	}
}
