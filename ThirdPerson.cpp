#include "ThirdPerson.h"
#include "Controls.h"
#include "Engine.h"

CThirdPerson g_ThirdPerson;

bool CThirdPerson::PassesStartConditions()
{
	if (!g_ESPCon.m_pEnabled->Value())
	{
		return false;
	}

	if (!g_ESPCon.m_pThirdPerson->Value())
	{
		return false;
	}

	if (g_ESPCon.m_pThirdPersonKey->Value())
	{
		static bool bToggled = false;
		if (g_Input.KeyPressed(g_ESPCon.m_pThirdPersonKey->Value()))
		{
			bToggled = !bToggled;
		}

		if (!bToggled)
		{
			return false;
		}
	}
	else if (!g_Input.KeyDown(g_ESPCon.m_pThirdPersonKey->Value()))
	{
		return false;
	}

	return true;
}

void CThirdPerson::EntryPoint()
{
	if (!PassesStartConditions())
	{
		if (g_Local.GV<size_t>(CPlayer::IO::ObserverMode) != 0)
		{
			g_Local.SV<size_t>(CPlayer::IO::ObserverMode, 0);
		}

		return;
	}

	if (g_Local.GV<size_t>(CPlayer::IO::ObserverMode) != g_ESPCon.m_pThirdPersonMode->Value())
	{
		g_Local.SV<size_t>(CPlayer::IO::ObserverMode, g_ESPCon.m_pThirdPersonMode->Value());
	}
}
