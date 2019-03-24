#pragma once
#include "Classes.h"
#include "Engine.h"
#include "Config.h"

class CMagnet : public CBaseHack
{
private:
	const CPlayer* m_pTarget;
	CVector m_Source;
	CVector m_UnPredictedSource;
	CVector2D m_Screen, m_Punch;

	CVector2D m_VisiblePoint;
	CVector2D m_SilentPoint;
	bool m_bBacktrack; int m_iTick;
	float m_flStartDelta; bool m_bNewVisibleTarget;

	bool m_bVisible, m_bSilent;

	CTimer m_VisibleTimes[g_PlayerList.MaxPlayerCount];
	CTimer m_LastTargetTime;
	CTimer m_AimTime;

	bool GetFinalTarget();
	bool GetFinalPoint();

	bool PassesStartConditions() const;
	bool PassesIntermediateConditions(const CPlayer& Player);
	bool PassesFireConditions();

	void Fire();

	void CurvePoint(CVector2D& Point) const;

	bool Move();

	bool m_bPrintInfo;
	char m_Buffer[BUFFER_SIZE];

public:
	void EntryPoint() override;
};

extern CMagnet g_Magnet;