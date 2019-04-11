#include "Aimbot.h"
#include "Engine.h"
#include "Controls.h"
#include "Math.h"
#include "RecoilControl.h"
#include "SlowAim.h"
#include "Magnet.h"
#include "TriggerBot.h"
#include "Backtrack.h"
#include "PlayerPrediction.h"

#include <thread>

CAimbot g_Aimbot;

bool CAimbot::PassesStartConditions() const
{
	if (!g_AimCon.m_pEnabled->Value())
	{
		return false;
	}

	if (g_AimCon.m_pKey->Value())
	{
		if (g_AimCon.m_pKey->Toggle())
		{
			static bool bToggled = false;
			if (g_Input.KeyPressed(g_AimCon.m_pKey->Value()))
			{
				bToggled = !bToggled;
			}

			if (bToggled)
			{
				return false;
			}
		}
		else 
		{
			if (g_AimCon.m_pKey->Value() == VK_LBUTTON)
			{
				if (!m_bMouseOneDown)
				{
					return false;
				}
			}
			else
			{
				if (!g_Input.KeyDown(g_AimCon.m_pKey->Value()))
				{
					return false;
				}
			}
		}
	}

	if (g_Local.GV<int>(CPlayer::IO::ShotsFired) < g_AimCon.m_pMinimumShots->Value() &&
		g_AimCon.m_pMinimumShots->Value() != AIM_MIN_SHOTS_DISABLED)
	{
		return false;
	}

	if (g_Local.GV<int>(CPlayer::IO::ShotsFired) > g_AimCon.m_pMaximumShots->Value() &&
		g_AimCon.m_pMinimumShots->Value() != AIM_MAX_SHOTS_DISABLED)
	{
		return false;
	}

	{
		const int i = g_Local.GV<int>(CPlayer::IO::ILE::CrosshairID) - 1;
		if (i >= 0 && i < g_PlayerList.MaxPlayerCount && g_PlayerList.GetPlayerFromIndex(i).IsEnemy())
		{
			if (g_AimCon.m_pDisableWhileX->Value()[AIM_DISABLE_IN_CROSS_ALWAYS].m_bSelected)
			{
				return false;
			}

			if (g_AimCon.m_pDisableWhileX->Value()[AIM_DISABLE_IN_CROSS_INITIAL].m_bSelected &&
				g_Local.GV<int>(CPlayer::IO::ShotsFired) == 0)
			{
				return false;
			}
		}
	}

	if (!(g_Local.GV<int>(CPlayer::IO::Flags) & FL_ONGROUND) &&
		g_AimCon.m_pDisableWhileX->Value()[AIM_DISABLE_JUMPING].m_bSelected)
	{
		return false;
	}

	const float flSpeed = g_Local.GV<CVector2D>(CPlayer::IO::Velocity).Magnitude();
	const bool bWalking = g_Local.GV<bool>(CPlayer::IO::IsWalking);

	if (!DecimalEnabled(flSpeed) &&
		g_AimCon.m_pDisableWhileX->Value()[AIM_DISABLE_STATIONARY].m_bSelected)
	{
		return false;
	}

	if (bWalking &&
		g_AimCon.m_pDisableWhileX->Value()[AIM_DISABLE_WALKING].m_bSelected)
	{
		return false;
	}

	if (!bWalking && flSpeed > g_SharedCon.m_pRunningVelocityDefinition->Value() &&
		g_AimCon.m_pDisableWhileX->Value()[AIM_DISABLE_RUNNING].m_bSelected)
	{
		return false;
	}

	if (!g_Local.GetActiveWeapon().IsWeapon())
	{
		return false;
	}

	if (g_Local.GetActiveWeapon().GV<int>(CWeapon::IO::Clip1) <= 0)
	{
		return false;
	}

	if (g_Local.GV<bool>(CPlayer::IO::InReload))
	{
		return false;
	}

	return true;
}

bool CAimbot::PassesIntermediateConditions(const CPlayer & Player)
{
	if (!Player.IsEnemy())
	{
		return false;
	}

	if (!Player.IsVisible() &&
		g_AimCon.m_pDisableWhileX->Value()[AIM_DISABLE_NOT_VISIBLE].m_bSelected)
	{
		m_VisibleTimes[Player.GetIndex()].Reset();
		return false;
	}

	if (DecimalEnabled(g_AimCon.m_pVisibleTicks->Value()) && 
		g_AimCon.m_pDisableWhileX->Value()[AIM_DISABLE_NOT_VISIBLE].m_bSelected)
	{
		if (m_VisibleTimes[Player.GetIndex()].Ticks() < g_AimCon.m_pVisibleTicks->Value() * g_Engine.GetTickScale())
		{
			return false;
		}
	}

	if (!(Player.GV<int>(CPlayer::IO::Flags) & FL_ONGROUND) &&
		g_AimCon.m_pDisableWhileX->Value()[AIM_DISABLE_JUMPING].m_bSelected)
	{
		return false;
	}

	const float flSpeed = Player.GV<CVector2D>(CPlayer::IO::Velocity).Magnitude();
	const bool bWalking = Player.GV<bool>(CPlayer::IO::IsWalking);

	if (!DecimalEnabled(flSpeed) &&
		g_AimCon.m_pDisableWhileX->Value()[AIM_DISABLE_STATIONARY].m_bSelected)
	{
		return false;
	}

	if (bWalking &&
		g_AimCon.m_pDisableWhileX->Value()[AIM_DISABLE_WALKING].m_bSelected)
	{
		return false;
	}

	if (!bWalking && flSpeed > g_SharedCon.m_pRunningVelocityDefinition->Value() &&
		g_AimCon.m_pDisableWhileX->Value()[AIM_DISABLE_RUNNING].m_bSelected)
	{
		return false;
	}

	return true;
}

bool CAimbot::PassesFireConditions()
{
	const auto GetVisibleShots = []() -> int
	{
		if (g_AimCon.m_pVisibleAfterShots->Value() == ALWAYS_VISIBLE_SHOTS)
		{
			return -1;
		}

		return g_AimCon.m_pVisibleAfterShots->Value();
	};

	const auto GetSilentShots = []() -> int
	{
		if (g_AimCon.m_pSilentBeforeShots->Value() == ALWAYS_SILENT_SHOTS)
		{
			return INT_MAX;
		}

		return g_AimCon.m_pSilentBeforeShots->Value();
	};

	m_Visible = g_Local.GV<int>(CPlayer::IO::ShotsFired) > GetVisibleShots();
	m_Silent = g_Local.GV<int>(CPlayer::IO::ShotsFired) < GetSilentShots();

	if (!g_Local.GetActiveWeapon().CanFire())
	{
		m_Silent = false;
	}

	if (!(m_bMouseOneDown || g_AimCon.m_pSilentAutoFire->Value()))
	{
		m_Silent = false;
	}

	if (g_Local.GV<int>(CPlayer::IO::ILE::CrosshairID) - 1 >= g_PlayerList.Start &&
		g_Local.GV<int>(CPlayer::IO::ILE::CrosshairID) - 1 < g_PlayerList.MaxPlayerCount &&
		!g_AimCon.m_pAlwaysSilent->Value())
	{
		m_Silent = false;
	}

	if (!DecimalEqual(g_AimCon.m_pActivationRate->Value(), 1.f))
	{
		static int Last = 0;
		static bool bBad = false;

		if (Last != m_pTarget->GetIndex())
		{
			Last = m_pTarget->GetIndex();
			bBad = rand() % 100 + 1 > int(g_AimCon.m_pActivationRate->Value() * 100.f + 0.5f);
		}

		if (bBad)
		{
			return false;
		}
	}

	if (DecimalEnabled(g_AimCon.m_pReTargetTicks->Value()))
	{
		static int Last = m_pTarget->GetIndex();
		static CTimer TargetTime;

		if (m_pTarget->GetIndex() != Last && !g_PlayerList.GetPlayerFromIndex(Last).IsEnemy())
		{
			Last = m_pTarget->GetIndex();
			TargetTime.Reset();
		}

		if (TargetTime.Ticks() < g_AimCon.m_pReTargetTicks->Value() * g_Engine.GetTickScale())
		{
			return false;
		}
	}

	if (DecimalEnabled(g_AimCon.m_pPullingAwayThreshold->Value()))
	{
		static float flLastFOV = 0.f;
		static int Last = 0;
		static CTimer Time;

		const float flFOV = g_Math.GetFOV(m_Screen, m_FinalVisiblePoint);

		if (Last == m_pTarget->GetIndex())
		{
			if (flFOV - flLastFOV > g_AimCon.m_pPullingAwayThreshold->Value())
			{
				Time.Reset();
			}

			if (Time.Ticks() < g_AimCon.m_pPullingAwayCoolDownTIcks->Value() * g_Engine.GetTickScale())
			{
				flLastFOV = flFOV;
				return false;
			}
		}

		flLastFOV = flFOV;
		Last = m_pTarget->GetIndex();
	}

	if (DecimalEnabled(g_AimCon.m_pVisibleReationTiks->Value()) || DecimalEnabled(g_AimCon.m_pSilentReationTiks->Value()))
	{
		static int Last = m_pTarget->GetIndex();
		static CTimer TargetTime;

		if (m_pTarget->GetIndex() != Last)
		{
			Last = m_pTarget->GetIndex();
			TargetTime.Reset();
		}

		if (DecimalEnabled(g_AimCon.m_pVisibleReationTiks->Value()) &&
			TargetTime.Ticks() < g_AimCon.m_pVisibleReationTiks->Value() * g_Engine.GetTickScale())
		{
			m_Visible = false;
		}

		if (DecimalEnabled(g_AimCon.m_pSilentReationTiks->Value()) &&
			TargetTime.Ticks() < g_AimCon.m_pSilentReationTiks->Value() * g_Engine.GetTickScale())
		{
			m_Silent = false;
		}
	}

	{
		static int Last = m_pTarget->GetIndex();

		if (m_pTarget->GetIndex() != Last || !m_Visible)
		{
			Last = m_pTarget->GetIndex();
			m_AimTime.Reset();
			m_bNewVisibleTarget = true;
		}
	}

	if (DecimalEnabled(g_AimCon.m_pTimeOutTicks->Value()))
	{
		if (m_AimTime.Ticks() > g_AimCon.m_pTimeOutTicks->Value() * g_Engine.GetTickScale())
		{
			return false;
		}
	}

	return m_Visible || m_Silent || g_AimCon.m_pBackTrack->Value();
}

bool CAimbot::GetFinalTarget()
{
	m_pTarget = nullptr;
	float flBestFOV = 39.f;

	if (g_Local.GV<int>(CPlayer::IO::ShotsFired) > g_AimCon.m_pRecoilCompensateShots->Value() && 
		(g_Local.GetActiveWeapon().IsRifle() || g_Local.GetActiveWeapon().IsSMG()))
	{
		const auto GetXRecoilControl = []() -> float
		{
			if (!DecimalEnabled(g_AimCon.m_pRCSXDeviation->Value()))
			{
				return g_AimCon.m_pRCSX->Value();
			}

			return g_Math.RandomNumber(g_AimCon.m_pRCSX->Value(), g_AimCon.m_pRCSXDeviation->Value());
		};

		const auto GetYRecoilControl = []() -> float
		{
			if (!DecimalEnabled(g_AimCon.m_pRCSYDeviation->Value()))
			{
				return g_AimCon.m_pRCSY->Value();
			}

			return g_Math.RandomNumber(g_AimCon.m_pRCSY->Value(), g_AimCon.m_pRCSYDeviation->Value());
		};

		m_CompensatedPunch = g_Local.GV<CVector2D>(CPlayer::IO::ILE::AimPunch);
		m_CompensatedPunch.x *= GetXRecoilControl();
		m_CompensatedPunch.y *= GetYRecoilControl();
	}
	else
	{
		m_CompensatedPunch = CVector2D();
	}

	for (size_t i = g_PlayerList.Start; i < g_PlayerList.MaxPlayerCount; i++)
	{
		const CPlayer& Player = g_PlayerList.GetPlayerFromIndex(i);

		if (!PassesIntermediateConditions(Player))
		{
			continue;
		}

		const int Bone = TERANY(Player.GV<int>(CPlayer::IO::Health) <= g_AimCon.m_pBaimHP->Value(),
			BONE_CHEST, g_AimCon.m_pPriorityBone->Value());

		CVector Position = Player.GetBonePosition(Bone);
		g_Prediction.Extrapolate(Player, Position, g_Math.TicksToTime(g_AimCon.m_pEnemyExtrapolaitonTicks->Value() * g_Engine.GetTickScale()));

		g_Math.VectorAngles(Position - m_Source, m_CurrentVisiblePoint);
		m_CurrentVisiblePoint -= m_CompensatedPunch;
		
		g_Math.ClampAngle(m_CurrentVisiblePoint);

		if (DecimalEqual(g_AimCon.m_pFOV->Value(), 39.f) && Player.IsVisible())
		{
			m_pTarget = &Player;
			m_FinalVisiblePoint = m_CurrentVisiblePoint;
			break;
		}

		const float flFOV = g_Math.GetFOV(m_Screen, m_CurrentVisiblePoint);
		if (flFOV < flBestFOV)
		{
			flBestFOV = flFOV;
			m_pTarget = &Player;
			m_FinalVisiblePoint = m_CurrentVisiblePoint;
		}
	}

	m_CurrentVisiblePoint = m_FinalVisiblePoint;
	m_FinalSilentPoint = m_FinalVisiblePoint;
	return m_pTarget != nullptr;
}

bool CAimbot::GetFinalPoint()
{
	m_bBacktrack = false;
	m_iTick = g_GlobalVars.m_iTickCount;
	m_FinalVisiblePoint = CVector2D();
	m_CurrentVisiblePoint = CVector2D();
	m_FinalSilentPoint = CVector2D();

	CVector2D SilentPunch;
	if (g_Local.GV<int>(CPlayer::IO::ShotsFired) > 1 && 
		(g_Local.GetActiveWeapon().IsRifle() || g_Local.GetActiveWeapon().IsSMG()))
	{
		SilentPunch = g_Local.GV<CVector2D>(CPlayer::IO::ILE::AimPunch) * 2.f;
	}

	const float flSilentFOV = TERANY(DecimalEqual(g_AimCon.m_pFOV->Value(), 39.f), 39.f, g_AimCon.m_pSilentFOV->Value());
	const float flPriorityValue = TERANY(IsSpraying(), g_AimCon.m_pPrioritySprayingValue->Value(), g_AimCon.m_pPriorityValue->Value());
	const auto& BoneList = GetBoneList();

	const auto PassesBackTrackConditions = [&]() -> bool
	{
		if (!g_AimCon.m_pBackTrack->Value())
		{
			return false;
		}

		if (m_Visible && (g_AimCon.m_pVisibleAimAtBackTrack->Value() || g_AimCon.m_pVisibleAimAtForwardTrack->Value()))
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

		CVector2D Screen(m_Screen + SilentPunch);
		g_Math.ClampAngle(Screen);

		for (const auto& i : Records)
		{
			if (!i.IsValid())
			{
				continue;
			}

			g_Math.VectorAngles(i.GetBonePosition(g_AimCon.m_pPriorityBone->Value()) - m_UnPredictedSource, m_CurrentVisiblePoint);
			g_Math.ClampAngle(m_CurrentVisiblePoint);

			const auto FOV = g_Math.GetFOV(Screen, m_CurrentVisiblePoint);

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

			switch (g_AimCon.m_pForwardTrackActivationConditions->Value())
			{
			case FORWARD_TRACK_ALWAYS:
				return true;

			case FORWARD_TRACK_WHEN_NO_AIM:
				if (m_Visible)
				{
					if (g_AimCon.m_pClampFOV->Value())
					{
						return flBestFOV < g_AimCon.m_pFOV->Value();
					}

					return flBestFOV * (1.f - flPriorityValue) < g_AimCon.m_pFOV->Value();
				}

			case FORWARD_TRACK_WHEN_NO_SILENT:
				if (m_Silent && g_AimCon.m_pSilentAimAtForwardTrack->Value())
				{
					if (g_AimCon.m_pClampFOV->Value())
					{
						return flBestFOV < g_AimCon.m_pSilentFOV->Value();
					}

					return flBestFOV * (1.f - flPriorityValue) < g_AimCon.m_pSilentFOV->Value();
				}

			default:
				return false;
			}
		};

		if (PassesForwardTrackConditions())
		{
			for (const auto& i : FutureRecords)
			{
				g_Math.VectorAngles(i.GetBonePosition(g_AimCon.m_pPriorityBone->Value()) - m_UnPredictedSource, m_CurrentVisiblePoint);
				g_Math.ClampAngle(m_CurrentVisiblePoint);

				const auto FOV = g_Math.GetFOV(Screen, m_CurrentVisiblePoint);

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
			flBestFOV = g_AimCon.m_pFOV->Value();
			CVector SilentPosition; CVector2D CurrentSilentPoint; 

			for (const auto& b : BoneList)
			{
				if (m_pTarget->GV<int>(CPlayer::IO::Health) <= g_AimCon.m_pBaimHP->Value())
				{
					if (b == BONE_HEAD)
					{
						continue;
					}
				}

				CVector Position(pBestRecord->GetBonePosition(b));
				SilentPosition = Position;
				g_Prediction.Extrapolate(*m_pTarget, Position, g_Math.TicksToTime(g_AimCon.m_pEnemyExtrapolaitonTicks->Value() * g_Engine.GetTickScale()));
				
				g_Math.VectorAngles(Position - m_Source, m_CurrentVisiblePoint);
				m_CurrentVisiblePoint -= m_CompensatedPunch;

				g_Math.VectorAngles(SilentPosition - m_UnPredictedSource, CurrentSilentPoint);
				CurrentSilentPoint -= SilentPunch;

				g_Math.ClampAngle(m_CurrentVisiblePoint);
				g_Math.ClampAngle(CurrentSilentPoint);

				float flFOV = g_Math.GetFOV(m_Screen, m_CurrentVisiblePoint);
				if (b == g_AimCon.m_pPriorityBone->Value())
				{
					flFOV *= 1.f - flPriorityValue;
				}
				
				if (flFOV < flBestFOV && (!g_AimCon.m_pClampFOV->Value() || flFOV <= g_AimCon.m_pFOV->Value()))
				{
					flBestFOV = flFOV;

					if (bIsForwardTrack)
					{
						if (m_Visible && g_AimCon.m_pVisibleAimAtForwardTrack->Value())
						{
							m_FinalVisiblePoint = m_CurrentVisiblePoint;
						}

						if (m_Silent && g_AimCon.m_pSilentAimAtForwardTrack->Value() && flBestFOV < flSilentFOV)
						{
							m_FinalSilentPoint = CurrentSilentPoint;
						}
					}
					else
					{
						if (m_Visible && g_AimCon.m_pVisibleAimAtBackTrack->Value())
						{
							m_FinalVisiblePoint = m_CurrentVisiblePoint;
						}

						if (m_Silent && g_AimCon.m_pSilentAimAtBackTrack->Value() && flBestFOV < flSilentFOV)
						{
							m_FinalSilentPoint = CurrentSilentPoint;
						}
					}
				}
			}

			if (m_Silent && m_FinalSilentPoint == CVector2D())
			{
				m_bBacktrack = false;
				m_iTick = g_GlobalVars.m_iTickCount;
			}
		}
	}
	
	if ((m_Visible && m_FinalVisiblePoint == CVector2D()) || (m_Silent && m_FinalSilentPoint == CVector2D()))
	{
		float flBestFOV = g_AimCon.m_pFOV->Value();
		CVector SilentPosition; CVector2D CurrentSilentPoint;
		
		for (const auto& i : BoneList)
		{
			if (m_pTarget->GV<int>(CPlayer::IO::Health) <= g_AimCon.m_pBaimHP->Value())
			{
				if (i == BONE_HEAD)
				{
					continue;
				}
			}

			CVector Position = m_pTarget->GetBonePosition(i);
			SilentPosition = Position;
			g_Prediction.Extrapolate(*m_pTarget, Position, g_Math.TicksToTime(g_AimCon.m_pEnemyExtrapolaitonTicks->Value() * g_Engine.GetTickScale()));

			g_Math.VectorAngles(Position - m_Source, m_CurrentVisiblePoint);
			m_CurrentVisiblePoint -= m_CompensatedPunch;

			g_Math.VectorAngles(SilentPosition - m_UnPredictedSource, CurrentSilentPoint);
			CurrentSilentPoint -= SilentPunch;

			g_Math.ClampAngle(m_CurrentVisiblePoint);
			g_Math.ClampAngle(CurrentSilentPoint);

			float flFOV = g_Math.GetFOV(m_Screen, m_CurrentVisiblePoint);
			if (i == g_AimCon.m_pPriorityBone->Value())
			{
				flFOV *= 1.f - flPriorityValue;
			}

			if (flFOV < flBestFOV && (!g_AimCon.m_pClampFOV->Value() || flFOV <= g_AimCon.m_pFOV->Value()))
			{
				flBestFOV = flFOV;

				if (m_Visible && m_FinalVisiblePoint == CVector2D())
				{
					m_FinalVisiblePoint = m_CurrentVisiblePoint;
				}

				if (m_Silent && m_FinalSilentPoint == CVector2D() && flBestFOV < g_AimCon.m_pSilentFOV->Value())
				{
					m_FinalSilentPoint = CurrentSilentPoint;
				}
			}
		}
	}

	m_CurrentVisiblePoint = m_FinalVisiblePoint;

	if (DecimalEnabled(g_AimCon.m_pRandomSpotX->Value()) || DecimalEnabled(g_AimCon.m_pRandomSpotY->Value()))
	{
		static int Last = m_pTarget->GetIndex();
		static CVector2D flRandom;
		
		if (m_pTarget->GetIndex() != Last)
		{
			Last = m_pTarget->GetIndex();

			float fldist = (g_Local.GV<CVector>(CPlayer::IO::Origin) -
				m_pTarget->GV<CVector>(CPlayer::IO::Origin)).Magnitude() /
				g_AimCon.m_pDistanceScaling->Value();

			if (fldist < 1.f)
			{
				fldist = 1.f;
			}

			flRandom.x = g_Math.RandomNumber(g_AimCon.m_pRandomSpotX->Value() / 2.f, 1) / fldist;
			flRandom.y = g_Math.RandomNumber(g_AimCon.m_pRandomSpotY->Value() / 2.f, 1) / fldist;
		}

		m_CurrentVisiblePoint += flRandom;
	}

	g_Math.ClampAngle(m_CurrentVisiblePoint);
	g_Math.ClampAngle(m_FinalVisiblePoint);
	g_Math.ClampAngle(m_FinalSilentPoint);

	if (m_CurrentVisiblePoint == CVector2D())
	{
		m_Visible = false;
	}

	if (m_FinalSilentPoint == CVector2D())
	{
		m_Silent = false;
	}

	if (!g_Local.GetActiveWeapon().CanFire())
	{
		m_bBacktrack = false;
	}

	if (!(m_bMouseOneDown || g_AimCon.m_pSilentAutoFire->Value()))
	{
		m_bBacktrack = false;
	}

	m_bForceNoInput = m_Silent || m_bBacktrack;

	return m_Silent || m_Visible || m_bBacktrack;
}

void CAimbot::CurvePoint() 
{
	const auto GetMaxShots = []() -> int
	{
		if (g_AimCon.m_pCurveShotsDisable->Value() == NEVER_DISABLE_CURVE)
		{
			return INT_MAX;
		}

		return g_AimCon.m_pCurveShotsDisable->Value();
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
		if (!DecimalEnabled(g_AimCon.m_pCurveDeviation->Value()))
		{
			return g_AimCon.m_pCurve->Value();
		}

		return g_Math.RandomNumber(g_AimCon.m_pCurve->Value(), g_AimCon.m_pCurveDeviation->Value());
	};

	float Scale = GetScale();
	if (!DecimalEnabled(Scale))
	{
		return;
	}

	const auto GetCurve = [this]() -> float
	{
		switch (g_AimCon.m_pCurveMode->Value())
		{
		case CURVE_TRIGONMETRIC:
		{
			static float counter = 0.f;
			counter += 1.f;
			return cosf(g_Math.DegreesToRadians(counter));
		}

		case CURVE_AIMTIME_PROPORTIONAL:
		case CURVE_AIMTIME_INVERSE:
		{
			const auto f = [&]() -> float
			{
				static float A = 0.f;
				static float B = 0.f;
				static CPlayer* pLast = nullptr;

				static float a = 0.f, b = 0.f, c = 0.f;

				if (m_pTarget != pLast)
				{
					const float flMax = (m_FinalVisiblePoint.y - m_Screen.y);
					const float flScale = std::clamp((m_FinalVisiblePoint.x - m_Screen.x), 0.f, 1.f);

					A = g_Math.RandomNumber<float>((m_FinalVisiblePoint.x - m_Screen.x) * g_AimCon.m_pCurve->Value(), g_AimCon.m_pCurveDeviation->Value());
					B = g_Math.RandomNumber<float>((m_FinalVisiblePoint.x - m_Screen.x) * g_AimCon.m_pCurve->Value(), g_AimCon.m_pCurveDeviation->Value());

					A *= flScale;
					B *= flScale;

					if (g_AimCon.m_pCurveMode->Value() == CURVE_AIMTIME_PROPORTIONAL)
					{
						if (A > B)
						{
							std::swap(A, B);
						}
					}
					else
					{
						if (B > A)
						{
							std::swap(A, B);
						}
					}

					c = (m_FinalVisiblePoint.x + (73.f / 2.f) * m_Screen.x + (3.f / 2.f) * B - 30.f * A) / (-11.f * flMax);
					b = (-9 * (B - 8.f * A + 7.f * m_FinalVisiblePoint.x + 2.f * c * flMax)) / (4 * flMax * flMax);
					a = (27.f * (A - m_Screen.x - (1.f / 3.f) * c * flMax - (1.f / 9.f) * b * flMax * flMax)) / (flMax * flMax * flMax);
				}

				const float x = (m_FinalVisiblePoint.y - m_Screen.y);
				return a * x*x*x + b * x*x + c * x - m_Screen.x;
			};

			return f();
		}

		default:
			return 0.f;
		}
	};

	auto Adjustment = Scale * GetCurve();

	// The idea here is to take the final point over the screen angles,
	// And as it approaches 1 / 1, negate the effect. 

	const float FOV = g_Math.GetFOV(m_Screen, m_FinalVisiblePoint);

	if (FOV < g_AimCon.m_pFOVReduction->Value())
	{
		Adjustment *= std::clamp(FOV / g_AimCon.m_pFOVReduction->Value(), 0.f, 1.f);
	}

	m_CurrentVisiblePoint.x -= Adjustment;
	g_Math.ClampAngle(m_CurrentVisiblePoint);
}

bool CAimbot::Move()
{
	memset(m_Buffer, 0, BUFFER_SIZE);

	m_Source = g_Local.GetSource();
	m_UnPredictedSource = m_Source;
	g_Prediction.Extrapolate(g_Local, m_Source, g_Math.TicksToTime(g_AimCon.m_pLocalExtrapolationTicks->Value() * g_Engine.GetTickScale()));

	m_Screen = g_Engine.GetScreenAngles2D();
	m_bPrintInfo = g_ESPCon.m_pAimInfo->Value() && g_ESPCon.m_pEnabled->Value();

	m_bMouseOneDown = g_Input.KeyDown(VK_LBUTTON);
	m_bForceNoInput = false;

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

	if (m_bNewVisibleTarget)
	{
		m_flStartDelta = (m_CurrentVisiblePoint - m_Screen).Magnitude();
		m_bNewVisibleTarget = false;
	}

	if (m_Visible)
	{
		m_LastTargetTime.Reset();

		if (DecimalEnabled(g_AimCon.m_pStartSmooth->Value()) || DecimalEnabled(g_AimCon.m_pEndSmooth->Value()))
		{
			const auto GetSmooth = [this]() -> float
			{
				const float flCurDelta = (m_CurrentVisiblePoint - m_Screen).Magnitude();
				const float flSmoothDelta = g_AimCon.m_pEndSmooth->Value() - g_AimCon.m_pStartSmooth->Value();

				const float frac = std::clamp(1.f - flCurDelta / m_flStartDelta, 0.f, 1.f);
				const float smooth_mod = TERANY(IsSpraying(), g_AimCon.m_pSprayingSmoothModifier->Value(), 1.f);

				const float Smooth = (g_AimCon.m_pStartSmooth->Value() + flSmoothDelta * frac) * smooth_mod;

				if (!DecimalEnabled(g_AimCon.m_pSmoothDeviation->Value()))
				{
					return Smooth;
				}

				return g_Math.RandomNumber(Smooth, g_AimCon.m_pSmoothDeviation->Value());
			};

			switch (g_AimCon.m_pSmoothMode->Value())
			{
			case SMOOTH_LOG:
				g_Math.SmoothAngleLog(m_Screen, m_CurrentVisiblePoint, GetSmooth());
				break;
			case SMOOTH_EXP:
				g_Math.SmoothAngleExp(m_Screen, m_CurrentVisiblePoint, GetSmooth());
				break;
			case SMOOTH_LERP:
				g_Math.SmoothAngleLerp(m_Screen, m_CurrentVisiblePoint, GetSmooth(), m_AimTime.Elapsed());
				break;
			}

			if (DecimalEnabled(g_AimCon.m_pCurve->Value()))
			{
				CurvePoint();
			}
		}

		g_Math.ClampAngle(m_CurrentVisiblePoint);

		if (g_AimCon.m_pMoveMode->Value() == AIM_MODE_SET)
		{
			g_Engine.SetScreenAngles2D(m_CurrentVisiblePoint);
		}
		else
		{
			const CVector2D Movement
			(
				(m_Screen.y - m_CurrentVisiblePoint.y) / (0.022f * g_Engine.GetSensitivity()),
				-(m_Screen.x - m_CurrentVisiblePoint.x) / (0.022f  * g_Engine.GetSensitivity())
			);

			g_Input.SendMouseInput(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, Movement.x, Movement.y);
		}

		const int iCrosshair = g_Local.GV<int>(CPlayer::IO::ILE::CrosshairID) - 1;
		if (DecimalEnabled(g_AimCon.m_pDelayShotTicks->Value()))
		{
			if (m_AimTime.Ticks() < g_AimCon.m_pDelayShotTicks->Value() * g_Engine.GetTickScale() &&
				iCrosshair != m_pTarget->GetIndex() && g_Local.GV<int>(CPlayer::IO::ShotsFired) <= 1)
			{
				m_bForceNoInput = true;
				if (!m_Silent)
				{
					m_bBacktrack = false;
				}
			}
		}

		if (g_AimCon.m_pVisibleAutoFire->Value() && !m_bMouseOneDown && iCrosshair == m_pTarget->GetIndex() && !m_bForceNoInput)
		{
			g_Engine.SetInput(IEngine::IO::ForceAttack, 6);
		}
	}

	if (m_Silent)
	{
		g_Math.ClampAngle(m_FinalSilentPoint);

		if (m_bBacktrack)
		{
			if (m_bPrintInfo)
			{
				sprintf(m_Buffer, "backtrack_silent: %s", m_pTarget->GetPlayerInfo().m_szPlayerName);
			}

			g_Engine.SetCMD(CVector(m_FinalSilentPoint.x, m_FinalSilentPoint.y), m_iTick, true);
		}
		else
		{
			if (m_bPrintInfo)
			{
				sprintf(m_Buffer, "silent: %s", m_pTarget->GetPlayerInfo().m_szPlayerName);
			}

			g_Engine.SetViewAnglesSilent(CVector(m_FinalSilentPoint.x, m_FinalSilentPoint.y), true);
		}
	}
	else if (m_bBacktrack)
	{
		if (m_bPrintInfo)
		{
			sprintf(m_Buffer, "backtrack: %s", m_pTarget->GetPlayerInfo().m_szPlayerName);
		}

		g_Engine.SetTickCount(m_iTick, true);
	}

	if (m_bPrintInfo && strcmp(m_Buffer, ""))
	{
		g_Menu.AddNewDebugMessage(m_Buffer, 0.5f);
	}

	return true;
}

void CAimbot::EntryPoint()
{
	while (true)
	{
		while (!g_Engine.IsFocused() || !(g_Engine.GetState() == SIG_FULL) || !g_Local.IsValid() || g_Menu.IsVisible())
		{
			CTimer::SleepThread(0.01f);
		}

		while (!g_Engine.IsValid())
		{
			std::this_thread::yield();
		}

		g_SlowAim.EntryPoint();
		m_bInAim = Move();

		if (!m_bInAim)
		{
			m_AimTime.Reset();
			m_bNewVisibleTarget = false;
		}

		g_Magnet.EntryPoint();

		if (!m_bInAim || g_RCSCon.m_pWhileAimbotting->Value())
		{
			g_RecoilControl.EntryPoint();
		}

		g_TriggerBot.EntryPoint();
		g_RecoilControl.LogPunch();

		m_bInZeus = ZeusBot();

		if (g_AimCon.m_pEnabled->Value())
		{
			CreateInput(m_bMouseOneDown, m_bForceNoInput);
		}

		CTimer::SleepThread(0.01f);
	}

}

bool CAimbot::GetInAim() const
{
	return m_bInAim;
}

bool CAimbot::GetInZeus() const
{
	return m_bInZeus;
}

bool CAimbot::GetMouseOneDown() const
{
	return m_bMouseOneDown;
}

void CAimbot::CreateInput(bool bMouseOneDown, bool bForceNoInput)
{
	static bool bReset = false;
	if (g_Engine.IsFocused() && !g_Menu.IsVisible() && bMouseOneDown && !bForceNoInput)
	{
		g_Engine.SetInput(IEngine::IO::ForceAttack, 5);
		bReset = false;
	}
	else if (!bReset)
	{
		g_Engine.SetInput(IEngine::IO::ForceAttack, 4);
		bReset = true;
	}	
}

bool CAimbot::IsSpraying()
{
	return g_AimCon.m_pSprayingAfterShots->Value() != NEVER_SPRAYING_SHOTS &&
		g_Local.GV<int>(CPlayer::IO::ShotsFired) > g_AimCon.m_pSprayingAfterShots->Value();
}

const std::vector<size_t>& CAimbot::GetBoneList()
{
	if (IsSpraying())
	{
		return g_AimCon.m_pSprayingBoneList->Value();
	}
	else
	{
		return g_AimCon.m_pBoneList->Value();
	}
}

const CTimer & CAimbot::GetLastTargetTime() const
{
	return m_LastTargetTime;
}

bool CAimbot::ZeusBot() 
{
	if (!g_SharedCon.m_pZeusBot->Value() || !g_Input.KeyDown(VK_LBUTTON) ||
		!(g_Local.GetActiveWeapon().GV<weaponid_t>(CWeapon::IO::WeaponID) == WEAPON_TASER))
	{
		return false;
	}

	float flBestFOV = 39.f;
	CVector2D Point, CurrentPoint;
	int index = -1;
	const CVector Origin(g_Local.GV<CVector>(CPlayer::IO::Origin));

	for (size_t i = g_PlayerList.Start; i < g_PlayerList.MaxPlayerCount; i++)
	{
		const CPlayer& Player = g_PlayerList.GetPlayerFromIndex(i);

		if (!Player.IsEnemy())
		{
			continue;
		}

		if ((Origin - Player.GV<CVector>(CPlayer::IO::Origin)).Magnitude() >= 183.f)
		{
			continue;
		}

		const CVector Position = Player.GetBonePosition(BONE_CHEST);

		g_Math.VectorAngles(Position - m_Source, CurrentPoint);
		g_Math.ClampAngle(CurrentPoint);

		const float flFOV = g_Math.GetFOV(m_Screen, CurrentPoint);
		if (flFOV < flBestFOV)
		{
			flBestFOV = flFOV;
			Point = CurrentPoint;
			index = i;
		}
	}

	if (index == -1)
	{
		return false;
	}

	if (g_AimCon.m_pBackTrack->Value() && g_SharedCon.m_pZeusBotBacktrack->Value())
	{
		int tick = -1;

		for (const auto& i : g_Backtrack.GetRecords(index))
		{
			if (!i.IsValid())
			{
				continue;
			}

			if ((Origin - i.GetBonePosition(BONE_ORIGIN)).Magnitude() >= 183.f)
			{
				continue;
			}

			g_Math.VectorAngles(i.GetBonePosition(BONE_CHEST) - m_Source, CurrentPoint);
			g_Math.ClampAngle(CurrentPoint);

			const float flFOV = g_Math.GetFOV(m_Screen, CurrentPoint);
			if (flFOV < flBestFOV)
			{
				flBestFOV = flFOV;
				Point = CurrentPoint;
				tick = g_Math.iTimeToTicks(i.m_flSimulationTime) + g_Engine.GetLerpTicks();
			}
		}

		if (tick == g_GlobalVars.m_iTickCount)
		{
			tick = -1;
		}

		m_bForceNoInput = true;

		if (flBestFOV < g_SharedCon.m_pZeusFOV->Value())
		{
			if (tick != -1)
			{
				g_Engine.SetCMD(CVector(Point.x, Point.y), tick, true);
			}
			else
			{
				g_Engine.SetViewAnglesSilent(CVector(Point.x, Point.y), true);
			}
		}
		else if (tick != -1)
		{
			g_Engine.SetTickCount(tick, true);
		}
	}
	else if (flBestFOV < g_SharedCon.m_pZeusFOV->Value())
	{
		m_bForceNoInput = true;
		g_Engine.SetViewAnglesSilent(CVector(Point.x, Point.y), true);
	}
	

	return true;
}
