#include "Misc.h"
#include "ThirdPerson.h"
#include "FakeLag.h"
#include "Movement.h"
#include "Engine.h"
#include "SkinChanger.h"
#include "Menu.h"
#include "Controls.h"

#include <thread>

CLaunchMisc g_MiscHacks;

void CLaunchMisc::EntryPoint()
{
	while (true)
	{
		while (!g_Engine.IsFocused() || !(g_Engine.GetState() == SIG_FULL) || !g_Local.IsValid() || g_Menu.IsVisible())
		{
			static bool bReset = false;

			if (!bReset || (g_Engine.GetChokedCommands() > g_FakeLagCon.m_pChoke->Value() && g_FakeLagCon.m_pEnabled->Value()))
			{
				bReset = g_Engine.SetSendPacket(true);
			}

			CTimer::SleepThread(0.01f);
		} 
		
		while (!g_Engine.IsValid())
		{
			std::this_thread::yield();
		}

		g_Movement.EntryPoint();
		g_FakeLag.EntryPoint();
		g_ThirdPerson.EntryPoint();
		g_SkinChanger.EntryPoint();

		CTimer::SleepThread(g_GlobalVars.m_flIntervalPerTick / 3.f);
	}
}
