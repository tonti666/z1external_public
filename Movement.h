#pragma once
#include "Classes.h"
#include "Math.h"

class CMovement : public CBaseHack
{
private:
	bool PassesStartConditions();
	void Jump();
	void Strafe();

	bool m_bOnGround, m_bWasOnGround;
	CTimer m_LastInput, m_LastJumped;

	CVector m_vecJumpVelocity;
	float m_flAngle, m_flJumpTime, m_flLandTime;

public:
	void EntryPoint() override;

	float GetLandTime() const;
};

extern CMovement g_Movement;