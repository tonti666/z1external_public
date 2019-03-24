#pragma once
#include "Classes.h"
#include "Math.h"
#include "Engine.h"

class CTriggerBot : public CBaseHack
{
private:
	CTimer m_VisibleTimes[g_PlayerList.MaxPlayerCount];
	CTimer m_ReqShotTime;

	bool PassesStartConditions() const;
	bool PassesIntermediateConditions(const CPlayer& Player);
	bool PassesFinalConditions(int Index);

public:
	void EntryPoint() override;
};

extern CTriggerBot g_TriggerBot;