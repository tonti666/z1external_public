#pragma once
#include "Classes.h"
#include "Math.h"

class CRecoilControl : public CBaseHack
{
private:
	CVector2D m_LastPunch;

	bool PassesStartConditions() const;

public:
	void EntryPoint();
	void LogPunch();
};

extern CRecoilControl g_RecoilControl;