#pragma once
#include "Classes.h"
#include "Engine.h"

enum class PredictionState_t
{
	Start,
	End
};

class CPlayerPrediction : public CBaseHack
{
private:
	CVector m_LastPosition[g_PlayerList.MaxPlayerCount];
	CVector m_LastVelcoity[g_PlayerList.MaxPlayerCount];
	int m_LastFlags[g_PlayerList.MaxPlayerCount];
	
	CVector m_CurrentPosition[g_PlayerList.MaxPlayerCount];
	CVector m_CurrentVelcoity[g_PlayerList.MaxPlayerCount];
	int m_CurrentFlags[g_PlayerList.MaxPlayerCount];

	CVector m_JumpVelocity[g_PlayerList.MaxPlayerCount];
	CVector m_JumpPosition[g_PlayerList.MaxPlayerCount];

	PredictionState_t m_State;

public:
	CPlayerPrediction() = default;

	void EntryPoint() override;

	void Extrapolate(const CPlayer& Player, CVector& Point, float Time) const;
	void Extrapolate(const CPlayer& Player, BaseBoneMatrices_t& Points, float Time) const;

	void SetCurrentState(PredictionState_t State);
	PredictionState_t GetCurrentState() const;
};

extern CPlayerPrediction g_Prediction;