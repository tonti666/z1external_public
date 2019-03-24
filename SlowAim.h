#pragma once
#include "Classes.h"
#include "Math.h"
#include "Engine.h"

class CSlowAim : public CBaseHack
{
private:
	int m_iTargetIndex;
	float m_flBestFOV;
	CTimer m_VisibleTimes[g_PlayerList.MaxPlayerCount];
	CTimer m_FOVTimer;

	bool GetTarget();

	bool PassesStartConditions() const;
	bool PassesIntermediateConditions(const CPlayer& Player);
	bool PassesFinalConditions() const;

public:
	void EntryPoint() override;
};

extern CSlowAim g_SlowAim;