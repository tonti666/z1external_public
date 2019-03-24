#include "PlayerPrediction.h"
#include <algorithm>

CPlayerPrediction g_Prediction;

//there is a bug here
//if the player doesn't get updated, then the lastvleocity = currentvelcoity
//therefore no centripetal or linear acceleration 
//todo fix it lol

void CPlayerPrediction::EntryPoint()
{
	if (m_State == PredictionState_t::Start)
	{
		for (size_t i = g_PlayerList.Start; i < g_PlayerList.MaxPlayerCount; i++)
		{
			const CPlayer& Player = g_PlayerList.GetPlayerFromIndex(i);

			if (!Player.IsValid())
			{
				continue;
			}

			m_CurrentVelcoity[i] = Player.GV<CVector>(CPlayer::IO::Velocity);
			m_CurrentPosition[i] = Player.GV<CVector>(CPlayer::IO::Origin);
			m_CurrentFlags[i] = Player.GV<int>(CPlayer::IO::Flags);

			if (!(m_CurrentFlags[i] & FL_ONGROUND) && (m_LastFlags[i] & FL_ONGROUND))
			{
				m_JumpVelocity[i] = m_CurrentVelcoity[i];
				m_JumpPosition[i] = m_CurrentPosition[i];
			}
		}
	}
	else
	{
		static int last = 0;
		if (last != g_GlobalVars.m_iTickCount)
		{
			memcpy(m_LastVelcoity, m_CurrentVelcoity, sizeof(CVector) * g_PlayerList.MaxPlayerCount);
			memcpy(m_LastPosition, m_CurrentPosition, sizeof(CVector) * g_PlayerList.MaxPlayerCount);
			memcpy(m_LastFlags, m_CurrentFlags, sizeof(int) * g_PlayerList.MaxPlayerCount);

			last = g_GlobalVars.m_iTickCount;
		}
	}
}

void CPlayerPrediction::SetCurrentState(PredictionState_t State)
{
	m_State = State;
}

PredictionState_t CPlayerPrediction::GetCurrentState() const
{
	return m_State;
}

void CPlayerPrediction::Extrapolate(const CPlayer & Player, CVector& Point, float Time) const
{
	const int i = Player.GetIndex();

	if (i < g_PlayerList.Start || i >= g_PlayerList.MaxPlayerCount || !DecimalEnabled(Time))
	{
		return;
	}

	const auto arg = [](const CVector& _in) -> float
	{
		float ang = atanf(_in.y / _in.x);

		if (_in.x < 0.f && _in.y < 0.f)
		{
			ang -= g_Math.PI;
		}
		else if (_in.x < 0.f && _in.y > 0.f)
		{
			ang += g_Math.PI;
		}

		return ang;
	};

	CVector CurrentVelocity(m_CurrentVelcoity[i]);
	CVector LastVelocity(m_LastVelcoity[i]);

	if (!DecimalEnabled(CurrentVelocity.Magnitude()) && !DecimalEnabled(LastVelocity.Magnitude()))
	{
		return;
	}

	while (Time > 0.f)
	{
		const CVector OldVelocity(CurrentVelocity);

		const float flCurrentArg = arg(CurrentVelocity);
		const float flLastArg = arg(LastVelocity);

		const float w = (flCurrentArg - flLastArg) / g_Math.TicksToTime(1);
		const float flNextArg = flCurrentArg + w * g_Math.TicksToTime(1);

		const float flLastMagnitude = LastVelocity.BaseMagnitude();
		const CVector2D Alligned(cosf(flCurrentArg) * flLastMagnitude, sinf(flCurrentArg) * flLastMagnitude);
		const CVector2D LinearAcceleration = (CVector2D(CurrentVelocity.x, CurrentVelocity.y) - Alligned) / g_Math.TicksToTime(1);

		const float flCurrentMagnitude = CurrentVelocity.BaseMagnitude();
		CurrentVelocity.x = cosf(flNextArg) * flCurrentMagnitude;
		CurrentVelocity.y = sinf(flNextArg) * flCurrentMagnitude;

		CurrentVelocity += CVector(LinearAcceleration.x, LinearAcceleration.y) * g_Math.TicksToTime(1);

		if (!(m_CurrentFlags[i] & FL_ONGROUND))
		{
			CurrentVelocity.z -= 800.f * g_Math.TicksToTime(1);
		}

		Point += CurrentVelocity * g_Math.TicksToTime(1);

		if (!(m_CurrentFlags[i] & FL_ONGROUND))
		{
			if (Point.z < m_JumpPosition[i].z)
			{
				Point.z = m_JumpPosition[i].z;
			}
		}

		LastVelocity = OldVelocity;
		Time -= g_Math.TicksToTime(1);
	}

	if (DecimalEnabled(Time))
	{
		Point += CVector(CurrentVelocity.x, CurrentVelocity.y) * Time;
	}

	/*
	i prefer the above because it's unclear what force compels the player to move in a circle
	const CVector2D Radius(CurrentVelocity.x / w, CurrentVelocity.y / w);
	for (size_t t = 0; t < count; t++)
	{
		const CVector2D CentripitalAcc(Squared(CurrentVelocity.x) / Radius.x, Squared(CurrentVelocity.y) / Radius.y);
		CurrentVelocity += CentripitalAcc * g_Math.TicksToTime(1);
		Point += CVector(CurrentVelocity.x, CurrentVelocity.y, 0.f) * g_Math.TicksToTime(1);
	}
	*/
}

void CPlayerPrediction::Extrapolate(const CPlayer & Player, BaseBoneMatrices_t & Points, float Time) const
{
	CVector Point;
	for (size_t i = 0; i < BONE_MAX; i++)
	{
		Point = Points.GetBonePosition(i);
		Extrapolate(Player, Point, Time);
		Points.SetBonePosition(i, Point);
	}
}