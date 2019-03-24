#include "Magnet.h"
#include "Controls.h"
#include "Backtrack.h"
#include "PlayerPrediction.h"
#include "Aimbot.h"

#include <optional>

CMagnet g_Magnet;

bool CMagnet::GetFinalTarget()
{
	m_pTarget = nullptr;
	float flBestFOV = 39.f;

	m_Source = g_Local.GetSource();
	m_UnPredictedSource = m_Source;
	g_Prediction.Extrapolate(g_Local, m_Source, g_Math.TicksToTime(g_MagnetCon.m_pLocalExtrapolationTicks->Value() * g_Engine.GetTickScale()));

	m_Screen = g_Engine.GetScreenAngles2D();
	m_bPrintInfo = g_ESPCon.m_pAimInfo->Value() && g_ESPCon.m_pEnabled->Value();

	m_Punch = g_Local.GV<CVector2D>(CPlayer::IO::ILE::AimPunch) * 2.f;
	if (g_Local.GV<int>(CPlayer::IO::ShotsFired) <= 1 ||
		(!g_Local.GetActiveWeapon().IsRifle() && !g_Local.GetActiveWeapon().IsSMG()))
	{
		m_Punch = CVector2D();
	}


	CVector2D Angles;
	for (size_t i = g_PlayerList.Start; i < g_PlayerList.MaxPlayerCount; i++)
	{
		const CPlayer& Player = g_PlayerList.GetPlayerFromIndex(i);

		if (!PassesIntermediateConditions(Player))
		{
			continue;
		}

		CVector Position = Player.GetBonePosition(g_MagnetCon.m_pPriorityBone->Value());
		g_Prediction.Extrapolate(Player, Position, g_Math.TicksToTime(g_MagnetCon.m_pEnemyExtrapolaitonTicks->Value() * g_Engine.GetTickScale()));

		g_Math.VectorAngles(Position - m_Source, Angles);
		Angles -= m_Punch;

		g_Math.ClampAngle(Angles);

		const float flFOV = g_Math.GetFOV(m_Screen, Angles);
		if (flFOV < flBestFOV)
		{
			flBestFOV = flFOV;
			m_pTarget = &Player;
		}
	}

	return m_pTarget != nullptr;
}

bool CMagnet::GetFinalPoint()
{
	m_bBacktrack = false;
	m_iTick = g_GlobalVars.m_iTickCount;
	m_VisiblePoint = CVector2D();
	m_SilentPoint = CVector2D();

	const auto PassesBackTrackConditions = [&]() -> bool
	{
		if (!g_AimCon.m_pBackTrack->Value())
		{
			return false;
		}

		if (m_bVisible && (g_MagnetCon.m_pVisibleAtBacktrack->Value() || g_MagnetCon.m_pVisibleAtForwardtrack->Value()))
		{
			;
		}
		else if (!g_Local.GetActiveWeapon().CanFire())
		{
			return false;
		}

		return true;
	};

	if (PassesBackTrackConditions())
	{
		float flBestFOV = 39.f;
		const Record_t* pBestRecord = nullptr;
		const auto& Records = g_Backtrack.GetRecords(m_pTarget->GetIndex());
		const auto& FutureRecords = g_Backtrack.GetFutureRecords(m_pTarget->GetIndex());
		bool bIsForwardTrack = false;

		CVector2D Screen(m_Screen + g_Local.GV<CVector2D>(CPlayer::IO::ILE::AimPunch) * 2.f);
		g_Math.ClampAngle(Screen);

		for (const auto& i : Records)
		{
			if (!i.IsValid())
			{
				continue;
			}

			CVector2D Angles; g_Math.VectorAngles(i.GetBonePosition(g_MagnetCon.m_pPriorityBone->Value()) - m_UnPredictedSource, Angles);
			g_Math.ClampAngle(Angles);

			const auto FOV = g_Math.GetFOV(Screen, Angles);

			if (FOV <= flBestFOV)
			{
				flBestFOV = FOV;
				pBestRecord = &i;
				m_iTick = g_Math.iTimeToTicks(pBestRecord->m_flSimulationTime) + g_Engine.GetLerpTicks();
			}
		}

		const auto PassesForwardTrackConditions = [&]() -> bool
		{
			if (!g_AimCon.m_pForwardTrack->Value())
			{
				return false;
			}

			switch (g_MagnetCon.m_pForwardTrackActivationConditions->Value())
			{
			case FORWARD_TRACK_ALWAYS:
				return true;

			case FORWARD_TRACK_WHEN_NO_AIM:
				if (m_bVisible)
				{
					if (g_MagnetCon.m_pClampFOV->Value())
					{
						return flBestFOV < g_MagnetCon.m_pFOV->Value();
					}

					return flBestFOV * (1.f - g_MagnetCon.m_pPriorityValue->Value()) < g_MagnetCon.m_pFOV->Value();
				}

			default:
				return false;
			}
		};

		if (PassesForwardTrackConditions())
		{
			for (const auto& i : FutureRecords)
			{
				CVector2D Angles; g_Math.VectorAngles(i.GetBonePosition(g_MagnetCon.m_pPriorityBone->Value()) - m_UnPredictedSource, Angles);
				g_Math.ClampAngle(Angles);

				const auto FOV = g_Math.GetFOV(Screen, Angles);

				if (FOV <= flBestFOV)
				{
					flBestFOV = FOV;
					pBestRecord = &i;
					m_iTick = g_Math.iTimeToTicks(pBestRecord->m_flSimulationTime) + g_Engine.GetLerpTicks();
					bIsForwardTrack = true;
				}
			}
		}

		m_bBacktrack = m_iTick != g_GlobalVars.m_iTickCount;

		if (m_bBacktrack)
		{
			flBestFOV = g_MagnetCon.m_pFOV->Value();
			CVector SilentPosition; CVector2D CurrentSilentPoint, CurrentVisiblePoint;

			for (const auto& b : g_MagnetCon.m_pBoneList->Value())
			{
				CVector Position(pBestRecord->GetBonePosition(b));
				SilentPosition = Position;
				g_Prediction.Extrapolate(*m_pTarget, Position, g_Math.TicksToTime(g_MagnetCon.m_pEnemyExtrapolaitonTicks->Value()));

				g_Math.VectorAngles(Position - m_Source, CurrentVisiblePoint);
				g_Math.VectorAngles(SilentPosition - m_UnPredictedSource, CurrentSilentPoint);

				if (g_Local.GV<int>(CPlayer::IO::ShotsFired) > g_MagnetCon.m_pRecoilCompensateShots->Value())
				{
					CurrentVisiblePoint -= m_Punch;
					CurrentSilentPoint -= m_Punch;
				}

				g_Math.ClampAngle(CurrentVisiblePoint);
				g_Math.ClampAngle(CurrentSilentPoint);

				float flFOV = g_Math.GetFOV(m_Screen, CurrentVisiblePoint);
				if (b == g_MagnetCon.m_pPriorityBone->Value())
				{
					flFOV *= 1.f - g_MagnetCon.m_pPriorityValue->Value();
				}

				if (flFOV < flBestFOV && (!g_MagnetCon.m_pClampFOV->Value() || flFOV <= g_MagnetCon.m_pFOV->Value()))
				{
					flBestFOV = flFOV;

					if (bIsForwardTrack)
					{
						if (m_bVisible && g_MagnetCon.m_pVisibleAtForwardtrack->Value())
						{
							m_VisiblePoint = CurrentVisiblePoint;
						}

						if (m_bSilent && g_MagnetCon.m_pSilentAtForwardtrack->Value())
						{
							m_SilentPoint = CurrentSilentPoint;
						}
					}
					else
					{
						if (m_bVisible && g_MagnetCon.m_pVisibleAtBacktrack->Value())
						{
							m_VisiblePoint = CurrentVisiblePoint;
						}

						if (m_bSilent && g_MagnetCon.m_pSilentAtBacktrack->Value())
						{
							m_SilentPoint = CurrentSilentPoint;
						}
					}
				}
			}

			if (m_bSilent && m_SilentPoint == CVector2D())
			{
				m_bBacktrack = false;
				m_iTick = g_GlobalVars.m_iTickCount;
			}
		}
	}

	if ((m_bVisible && m_VisiblePoint == CVector2D()) || (m_bSilent && m_SilentPoint == CVector2D()))
	{
		float flBestFOV = g_AimCon.m_pFOV->Value();
		CVector SilentPosition; CVector2D CurrentSilentPoint, CurrentVisiblePoint;

		for (const auto& i : g_MagnetCon.m_pBoneList->Value())
		{
			CVector Position = m_pTarget->GetBonePosition(i);
			SilentPosition = Position;
			g_Prediction.Extrapolate(*m_pTarget, Position, g_Math.TicksToTime(g_MagnetCon.m_pEnemyExtrapolaitonTicks->Value() * g_Engine.GetTickScale()));

			g_Math.VectorAngles(Position - m_Source, CurrentVisiblePoint);
			g_Math.VectorAngles(SilentPosition - m_UnPredictedSource, CurrentSilentPoint);

			if (g_Local.GV<int>(CPlayer::IO::ShotsFired) > g_MagnetCon.m_pRecoilCompensateShots->Value())
			{
				CurrentVisiblePoint -= m_Punch;
				CurrentSilentPoint -= m_Punch;
			}

			g_Math.ClampAngle(CurrentVisiblePoint);
			g_Math.ClampAngle(CurrentSilentPoint);

			float flFOV = g_Math.GetFOV(m_Screen, CurrentVisiblePoint);
			if (i == g_MagnetCon.m_pPriorityBone->Value())
			{
				flFOV *= 1.f - g_MagnetCon.m_pPriorityValue->Value();
			}

			if (flFOV < flBestFOV && (!g_MagnetCon.m_pClampFOV->Value() || flFOV <= g_MagnetCon.m_pFOV->Value()))
			{
				flBestFOV = flFOV;

				if (m_bVisible && m_VisiblePoint == CVector2D())
				{
					m_VisiblePoint = CurrentVisiblePoint;
				}

				if (m_bSilent && m_SilentPoint == CVector2D())
				{
					m_SilentPoint = CurrentSilentPoint;
				}
			}
		}
	}

	if (m_VisiblePoint == CVector2D())
	{
		m_bVisible = false;
	}

	if (m_SilentPoint == CVector2D())
	{
		m_bSilent = false;
	}

	if (!g_Local.GetActiveWeapon().CanFire())
	{
		assert(m_bSilent == false);
		m_bBacktrack = false;
	}

	if (!(g_Aimbot.GetMouseOneDown() || g_MagnetCon.m_pSilentAutoFire->Value()))
	{
		assert(m_bSilent == false);
		m_bBacktrack = false;
	}

	if (!g_Aimbot.GetMouseOneDown() && !m_bSilent)
	{
		m_bBacktrack = false;
	}

	return m_bVisible || m_bSilent || m_bBacktrack;
}

bool CMagnet::PassesStartConditions() const
{
	if (!g_MagnetCon.m_pEnabled->Value())
	{
		return false;
	}

	if (g_MagnetCon.m_pKey->Value())
	{
		if (g_MagnetCon.m_pKey->Toggle())
		{
			static bool bToggled = false;
			if (g_Input.KeyPressed(g_MagnetCon.m_pKey->Value()))
			{
				bToggled = !bToggled;
			}

			if (bToggled)
			{
				return false;
			}
		}
		else if (!g_Input.KeyDown(g_MagnetCon.m_pKey->Value()))
		{
			return false;
		}
	}

	if (!(g_Local.GV<int>(CPlayer::IO::Flags) & FL_ONGROUND) &&
		g_MagnetCon.m_pDisableWhileX->Value()[MAGNET_DISABLE_JUMPING].m_bSelected)
	{
		return false;
	}

	const float flSpeed = g_Local.GV<CVector2D>(CPlayer::IO::Velocity).Magnitude();
	const bool bWalking = g_Local.GV<bool>(CPlayer::IO::IsWalking);

	if (!DecimalEnabled(flSpeed) &&
		g_MagnetCon.m_pDisableWhileX->Value()[MAGNET_DISABLE_STATIONARY].m_bSelected)
	{
		return false;
	}

	if (bWalking &&
		g_MagnetCon.m_pDisableWhileX->Value()[MAGNET_DISABLE_WALKING].m_bSelected)
	{
		return false;
	}

	if (!bWalking && flSpeed > g_SharedCon.m_pRunningVelocityDefinition->Value() &&
		g_MagnetCon.m_pDisableWhileX->Value()[MAGNET_DISABLE_RUNNING].m_bSelected)
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

bool CMagnet::PassesIntermediateConditions(const CPlayer & Player) 
{
	if (!Player.IsEnemy())
	{
		return false;
	}

	if (!Player.IsVisible() &&
		g_MagnetCon.m_pDisableWhileX->Value()[MAGNET_DISABLE_NOT_VISIBLE].m_bSelected)
	{
		m_VisibleTimes[Player.GetIndex()].Reset();
		return false;
	}

	if (!(Player.GV<int>(CPlayer::IO::Flags) & FL_ONGROUND) &&
		g_MagnetCon.m_pDisableWhileX->Value()[MAGNET_DISABLE_JUMPING].m_bSelected)
	{
		return false;
	}

	const float flSpeed = Player.GV<CVector2D>(CPlayer::IO::Velocity).Magnitude();
	const bool bWalking = Player.GV<bool>(CPlayer::IO::IsWalking);

	if (!DecimalEnabled(flSpeed) &&
		g_MagnetCon.m_pDisableWhileX->Value()[MAGNET_DISABLE_STATIONARY].m_bSelected)
	{
		return false;
	}

	if (bWalking &&
		g_MagnetCon.m_pDisableWhileX->Value()[MAGNET_DISABLE_WALKING].m_bSelected)
	{
		return false;
	}

	if (!bWalking && flSpeed > g_SharedCon.m_pRunningVelocityDefinition->Value() &&
		g_MagnetCon.m_pDisableWhileX->Value()[MAGNET_DISABLE_RUNNING].m_bSelected)
	{
		return false;
	}

	if (g_Local.GetActiveWeapon().GV<weaponid_t>(CWeapon::IO::WeaponID) == WEAPON_TASER &&
		(g_Local.GV<CVector>(CPlayer::IO::Origin) - Player.GV<CVector>(CPlayer::IO::Origin)).Magnitude() > 183.f)
	{
		return false;
	}

	return true;
}

bool CMagnet::PassesFireConditions() 
{
	m_bVisible = g_MagnetCon.m_pVisible->Value();
	m_bSilent = g_MagnetCon.m_pSilent->Value();

	if (!g_Local.GetActiveWeapon().CanFire())
	{
		m_bSilent = false;
	}

	if (!(g_Aimbot.GetMouseOneDown() || g_MagnetCon.m_pSilentAutoFire->Value()))
	{
		m_bSilent = false;
	}

	if (g_MagnetCon.m_pDisableWhileX->Value()[MAGNET_DISABLE_NOT_VISIBLE].m_bSelected && DecimalEnabled(g_MagnetCon.m_pVisibleVisibleTicks->Value()))
	{
		if (m_VisibleTimes[m_pTarget->GetIndex()].Ticks() < g_MagnetCon.m_pVisibleVisibleTicks->Value() * g_Engine.GetTickScale())
		{
			m_bVisible = false;
		}
	}

	if (g_MagnetCon.m_pDisableWhileX->Value()[MAGNET_DISABLE_NOT_VISIBLE].m_bSelected && DecimalEnabled(g_MagnetCon.m_pSilentVisibleTicks->Value()))
	{
		if (m_VisibleTimes[m_pTarget->GetIndex()].Ticks() < g_MagnetCon.m_pSilentVisibleTicks->Value() * g_Engine.GetTickScale())
		{
			m_bSilent = false;
		}
	}

	if (DecimalEnabled(g_MagnetCon.m_pReTargetTicks->Value()))
	{
		static int Last = m_pTarget->GetIndex();
		static CTimer TargetTime;

		if (m_pTarget->GetIndex() != Last && !g_PlayerList.GetPlayerFromIndex(Last).IsEnemy())
		{
			Last = m_pTarget->GetIndex();
			TargetTime.Reset();
		}

		if (TargetTime.Ticks() < g_MagnetCon.m_pReTargetTicks->Value() * g_Engine.GetTickScale())
		{
			return false;
		}
	}

	if (m_bVisible)
	{
		static int Last = 0;
		if (Last != m_pTarget->GetIndex())
		{
			Last = m_pTarget->GetIndex();
			m_AimTime.Reset();
			m_bNewVisibleTarget = true;
		}
	}

	return m_bSilent || m_bVisible || g_AimCon.m_pBackTrack->Value();
}

void CMagnet::Fire()
{
	if (m_bNewVisibleTarget)
	{
		m_flStartDelta = (m_VisiblePoint - m_Screen).Magnitude();
		m_bNewVisibleTarget = false;
	}

	if (m_bVisible)
	{
		g_Math.ClampAngle(m_VisiblePoint);

		CVector2D SmoothedPoint = m_VisiblePoint;

		if (DecimalEnabled(g_MagnetCon.m_pStartSmooth->Value()) && DecimalEnabled(g_MagnetCon.m_pEndSmooth->Value()))
		{
			const auto GetSmooth = [this]() -> float
			{
				const float flCurDelta = (m_VisiblePoint - m_Screen).Magnitude();
				const float frac = std::clamp(1.f - flCurDelta / m_flStartDelta, 0.f, 1.f);
				const float flSmoothDelta = g_MagnetCon.m_pEndSmooth->Value() - g_MagnetCon.m_pStartSmooth->Value();
				const float Smooth = g_MagnetCon.m_pStartSmooth->Value() + flSmoothDelta * frac;

				if (!DecimalEnabled(g_MagnetCon.m_pSmoothDeviation->Value()))
				{
					return Smooth;
				}

				return g_Math.RandomNumber(Smooth, g_MagnetCon.m_pSmoothDeviation->Value());
			};

			switch (g_MagnetCon.m_pSmoothMode->Value())
			{
			case SMOOTH_LOG:
				g_Math.SmoothAngleLog(m_Screen, SmoothedPoint, GetSmooth());
				break;
			case SMOOTH_EXP:
				g_Math.SmoothAngleExp(m_Screen, SmoothedPoint, GetSmooth());
				break;
			case SMOOTH_LERP:
				g_Math.SmoothAngleLerp(m_Screen, SmoothedPoint, GetSmooth(), m_AimTime.Elapsed());
				break;
			}

			if (DecimalEnabled(g_MagnetCon.m_pCurve->Value()))
			{
				CurvePoint(SmoothedPoint);
			}
		}

		g_Math.ClampAngle(SmoothedPoint);

		if (g_AimCon.m_pMoveMode->Value() == AIM_MODE_SET)
		{
			g_Engine.SetScreenAngles2D(SmoothedPoint);
		}
		else
		{
			const CVector2D Movement
			(
				(m_Screen.y - SmoothedPoint.y) / (0.022f * g_Engine.GetSensitivity()),
				-(m_Screen.x - SmoothedPoint.x) / (0.022f  * g_Engine.GetSensitivity())
			);

			g_Input.SendMouseInput(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, Movement.x, Movement.y);
		}

		const int iCrosshair = g_Local.GV<int>(CPlayer::IO::ILE::CrosshairID) - 1;
		if (g_MagnetCon.m_pVisibleAutoFire->Value() && !g_Aimbot.GetMouseOneDown() && iCrosshair == m_pTarget->GetIndex() && !(m_bSilent || m_bBacktrack))
		{
			g_Engine.SetInput(IEngine::IO::ForceAttack, 6);
		}
	}

	if (m_bSilent)
	{
		g_Math.ClampAngle(m_SilentPoint);

		if (m_bBacktrack)
		{
			g_Engine.SetCMD(CVector(m_SilentPoint.x, m_SilentPoint.y), m_iTick, true);			
			
			if (m_bPrintInfo)
			{
				sprintf(m_Buffer, "backtrack_silent: %s", m_pTarget->GetPlayerInfo().m_szPlayerName);
			}
		}
		else
		{
			g_Engine.SetViewAnglesSilent(CVector(m_SilentPoint.x, m_SilentPoint.y), true);			
			
			if (m_bPrintInfo)
			{
				sprintf(m_Buffer, "silent: %s", m_pTarget->GetPlayerInfo().m_szPlayerName);
			}
		}
	}
	else if (m_bBacktrack)
	{
		g_Engine.SetTickCount(m_iTick, true);		
		
		if (m_bPrintInfo)
		{
			sprintf(m_Buffer, "backtrack: %s", m_pTarget->GetPlayerInfo().m_szPlayerName);
		}
	}

	if (m_bPrintInfo && strcmp(m_Buffer, ""))
	{
		g_Menu.AddNewDebugMessage(m_Buffer, 0.5f);
	}
}

void CMagnet::CurvePoint(CVector2D & Point) const
{
	const auto GetMaxShots = []() -> int
	{
		if (g_MagnetCon.m_pCurveShotsDisable->Value() == NEVER_DISABLE_CURVE)
		{
			return INT_MAX;
		}

		return g_MagnetCon.m_pCurveShotsDisable->Value();
	};

	if (g_Local.GV<int>(CPlayer::IO::ShotsFired) > GetMaxShots())
	{
		return;
	}

	{
		const int i = g_Local.GV<int>(CPlayer::IO::ILE::CrosshairID) - 1;
		if (i >= g_PlayerList.Start && i < g_PlayerList.MaxPlayerCount && g_PlayerList.GetPlayerFromIndex(i).IsEnemy())
		{
			return;
		}
	}

	const auto GetScale = []() -> float
	{
		if (!DecimalEnabled(g_MagnetCon.m_pCurveDeviation->Value()))
		{
			return g_MagnetCon.m_pCurve->Value();
		}

		return g_Math.RandomNumber(g_MagnetCon.m_pCurve->Value(), g_MagnetCon.m_pCurveDeviation->Value());
	};

	float Scale = GetScale();
	if (!DecimalEnabled(Scale))
	{
		return;
	}

	const auto GetCurve = [this]() -> float
	{
		static float counter = 0.f;
		counter += 1.f;

		switch (g_AimCon.m_pCurveMode->Value())
		{
		case CURVE_TRIGONMETRIC:
			return cosf(g_Math.DegreesToRadians(counter));

		case CURVE_AIMTIME_PROPORTIONAL:
			return -std::clamp(0.1f * expf(Squared(m_AimTime.Elapsed())) - sinf(m_AimTime.Elapsed() - g_Math.PI_INV_2) - 1, 0.f, 1.f);

		case CURVE_AIMTIME_INVERSE:
			return std::clamp(0.1f * expf(-Squared(m_AimTime.Elapsed())), 0.f, 1.f);

		default:
			return 0.f;
		}
	};

	auto Adjustment = Scale * GetCurve();

	// The idea here is to take the final point over the screen angles,
	// And as it approaches 1 / 1, negate the effect. 

	const auto FOV = g_Math.GetFOV(m_Screen, m_VisiblePoint);

	if (FOV < g_MagnetCon.m_pFOVReduction->Value())
	{
		Adjustment *= std::clamp(FOV / g_MagnetCon.m_pFOVReduction->Value(), 0.f, 1.f);
	}

	Point.x -= Adjustment;
	g_Math.ClampAngle(Point);
}

bool CMagnet::Move()
{
	memset(m_Buffer, 0, BUFFER_SIZE);

	if (!PassesStartConditions())
	{
		return false;
	}

	if (!GetFinalTarget())
	{
		return false;
	}

	if (!PassesFireConditions())
	{
		return false;
	}

	if (!GetFinalPoint())
	{
		return false;
	}

	Fire();
	m_LastTargetTime.Reset();
	return true;
}

void CMagnet::EntryPoint()
{
	const bool bMove = Move();

	if (!bMove)
	{
		m_AimTime.Reset();
		m_bNewVisibleTarget = false;

		if (m_LastTargetTime.Ticks() < g_MagnetCon.m_pBurstShotsTicks->Value() * g_Engine.GetTickScale() && g_Local.GetActiveWeapon().IsRifle())
		{
			g_Engine.SetInput(IEngine::IO::ForceAttack, 6);
		}
	}
}
