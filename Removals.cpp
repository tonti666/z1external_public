#include "Removals.h"
#include "Controls.h"
#include <thread>

CRemovals g_Removals;

void CRemovals::EntryPoint()
{
	while (true)
	{
		while (!g_Engine.IsFocused() || !(g_Engine.GetState() == SIG_FULL) || !g_Local.IsValid() || 
			(!g_SharedCon.m_pNoVisualRecoil->Value() && !g_SharedCon.m_pNoFlash->Value()) || g_Menu.IsVisible())
		{
			CTimer::SleepThread(0.01f);
		}

		while (!g_Engine.IsValid())
		{
			std::this_thread::yield();
		}

		if (g_SharedCon.m_pNoVisualRecoil->Value())
		{
			if (DecimalEnabled(g_Local.GV<CVector2D>(CPlayer::IO::ViewPunch).Magnitude()))
			{
				g_Local.SV<CVector2D>(CPlayer::IO::ViewPunch, CVector2D());
			}
		}

		static float flRestore = 255.f;

		if (g_SharedCon.m_pNoFlash->Value())
		{
			if (g_Local.GV<float>(CPlayer::IO::FlashDuration) * (255.f / 5.f) < g_SharedCon.m_pMaxFlashRemoveAlpha->Value())
			{
				flRestore = g_Local.GV<float>(CPlayer::IO::FlashMaxAlpha);
				g_Local.SV<float>(CPlayer::IO::FlashMaxAlpha, 0.f);
				g_Local.SV<float>(CPlayer::IO::FlashDuration, 0.f);
			}
			
			if (!DecimalEnabled(g_Local.GV<float>(CPlayer::IO::FlashDuration)) && 
				!DecimalEqual(g_Local.GV<float>(CPlayer::IO::FlashMaxAlpha), flRestore))
			{
				g_Local.SV<float>(CPlayer::IO::FlashMaxAlpha, flRestore);
			}
		}

		const auto Shots = g_Local.GV<int>(CPlayer::IO::ShotsFired);
		CTimer::SleepThread(TERANY(g_SharedCon.m_pNoVisualRecoil->Value() && Shots > 0, 0, 0.005f));
	}
}
