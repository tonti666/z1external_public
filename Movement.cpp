#include "Movement.h"
#include "Controls.h"

CMovement g_Movement;

bool CMovement::PassesStartConditions()
{
	if (g_MovementCon.m_pBunnyHop->Value() == BHOP_DISABLED)
	{
		return false;
	}

	if (g_MovementCon.m_pBunnyHopKey->Value())
	{
		if (g_MovementCon.m_pBunnyHopKey->Toggle())
		{
			static bool bToggled = false;
			if (g_Input.KeyPressed(g_MovementCon.m_pBunnyHopKey->Value()))
			{
				bToggled = !bToggled;
			}

			if (!bToggled)
			{
				return false;
			}
		}
		else if (!g_Input.KeyDown(g_MovementCon.m_pBunnyHopKey->Value()))
		{
			return false;
		}
	}

	const float flSpeed = g_Local.GV<CVector>(CPlayer::IO::Velocity).BaseMagnitude();
	if (!DecimalEnabled(flSpeed, g_MovementCon.m_pMinVelocity->Value()))
	{
		return false;
	}

	return true;
}

void CMovement::Jump()
{
	if (DecimalEnabled(g_MovementCon.m_pCoolDown->Value()) &&
		m_LastInput.Ticks() < g_MovementCon.m_pCoolDown->Value() * g_Engine.GetTickScale())
	{
		return;
	}

	switch (g_MovementCon.m_pBunnyHop->Value())
	{
	case BHOP_RAGE:
		if (!m_bOnGround)
		{
			return;
		}

		if (!g_Math.PassesActivationRate(g_AimCon.m_pActivationRate->Value()))
		{
			return;
		}

		g_Engine.SetInput(IEngine::IO::ForceJump, 6);
		m_LastInput.Reset();
		break;

	case BHOP_LEGIT:
		if (g_Engine.GetLastCMD().m_iButtons & IN_JUMP)
		{
			m_LastJumped.Reset();
		}

		if (!m_bOnGround)
		{
			return;
		}

		if (m_LastJumped.Ticks() > g_MovementCon.m_pAssistanceTicks->Value() * g_Engine.GetTickScale())
		{
			return;
		}

		if (!g_Math.PassesActivationRate(g_AimCon.m_pActivationRate->Value()))
		{
			return;
		}

		g_Engine.SetInput(IEngine::IO::ForceJump, 6);
		m_LastInput.Reset();
		break;
	}
}

void CMovement::Strafe()
{
	if (!g_MovementCon.m_pAutoStrafe->Value())
	{
		return;
	}

	static bool bReset = false;
	const auto Reset = []() -> void
	{
		if (!bReset)
		{
			g_Engine.SetInput(IEngine::IO::ForceLeft, 0);
			g_Engine.SetInput(IEngine::IO::ForceRight, 0);
			bReset = true;
		}
	};

	if (m_bOnGround)
	{
		Reset();
		return;
	}

	static float flLastYaw = g_Engine.GetScreenAngles2D().y;
	float flCurYaw = g_Engine.GetScreenAngles2D().y;
	const float flDelta = flCurYaw - flLastYaw;

	if (!DecimalEnabled(flDelta))
	{
		Reset();
		return;
	}

	if (flDelta > 0.f)
	{
		g_Engine.SetInput(IEngine::IO::ForceLeft, 0);
		g_Engine.SetInput(IEngine::IO::ForceRight, 5);
	}
	else
	{
		g_Engine.SetInput(IEngine::IO::ForceLeft, 5);
		g_Engine.SetInput(IEngine::IO::ForceRight, 0);
	}

	flLastYaw = flCurYaw;
	bReset = false;
}

void CMovement::EntryPoint()
{
	m_bOnGround = (g_Local.GV<int>(CPlayer::IO::Flags) & FL_ONGROUND);

	if (!m_bOnGround && m_bWasOnGround)
	{
		m_vecJumpVelocity = g_Local.GV<CVector>(CPlayer::IO::Velocity);
		g_Math.arctan(m_vecJumpVelocity.z, m_vecJumpVelocity.BaseMagnitude(), m_flAngle);		
		m_flJumpTime = g_GlobalVars.m_flCurTime;
		m_flLandTime = m_flJumpTime + (2.f * m_vecJumpVelocity.Magnitude() * sinf(m_flAngle)) / 800.f;
	}

	m_bWasOnGround = m_bOnGround;

	if (!PassesStartConditions())
	{
		return;
	}

	Jump();
	Strafe();
}

float CMovement::GetLandTime() const
{
	return fabsf(m_flLandTime);
}