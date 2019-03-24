#pragma once
#include "Classes.h"
#include <vector>
#include "Math.h"
#include "Engine.h"
#include "Config.h"

class CAimbot : public CBaseThreadHack
{
private:
	CVector m_Source;
	CVector m_UnPredictedSource;
	CVector2D m_Screen;

	CVector2D m_FinalVisiblePoint, m_FinalSilentPoint;
	CVector2D m_CurrentVisiblePoint;

	CVector2D m_CompensatedPunch;

	bool ZeusBot();

	CTimer m_VisibleTimes[g_PlayerList.MaxPlayerCount];
	CTimer m_LastTargetTime, m_AimTime;
	float m_flStartDelta; bool m_bNewVisibleTarget;
	const CPlayer* m_pTarget;

	bool PassesStartConditions() const;
	bool PassesIntermediateConditions(const CPlayer& Player);
	bool PassesFireConditions();

	bool m_Silent, m_Visible;
	bool m_bBacktrack; int m_iTick;

	bool GetFinalTarget();
	bool GetFinalPoint();
	void CurvePoint();

	bool Move();

	bool m_bInAim, m_bInZeus, m_bMouseOneDown, m_bForceNoInput, m_bPrintInfo;
	char m_Buffer[BUFFER_SIZE];

public:
	void EntryPoint() override;

	bool GetInAim() const;
	bool GetInZeus() const;
	bool GetMouseOneDown() const;

	const CTimer& GetLastTargetTime() const;

	static void CreateInput(bool bMouseOneDown, bool bForceNoInput);

	static bool IsSpraying();
	static const std::vector<size_t>& GetBoneList();
};

extern CAimbot g_Aimbot;