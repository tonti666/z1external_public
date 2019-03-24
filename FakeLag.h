#pragma once
#include "Classes.h"
#include "Math.h"

enum class LagState_t
{
	Send,
	Choke
};

class CFakeLag : public CBaseHack
{
private:
	CTimer m_StateTime;
	LagState_t m_State;

	bool PassesStartConditions();

public:
	void EntryPoint() override;
};

extern CFakeLag g_FakeLag;