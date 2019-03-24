#include "FakeLag.h"
#include "Engine.h"
#include "Controls.h"

CFakeLag g_FakeLag;

void CFakeLag::EntryPoint()
{
	static bool bReset = false;
	if (!PassesStartConditions())
	{
		if (!bReset || (g_Engine.GetChokedCommands() > g_FakeLagCon.m_pChoke->Value() && g_FakeLagCon.m_pEnabled->Value()))
		{
			bReset = g_Engine.SetSendPacket(true);
		}

		return;
	}

	bReset = false;

	static bool bSendInitialized = false;
	static bool bChokeInitialized = false;

	if (m_State == LagState_t::Send)
	{
		static float flSendTime;
		if (!bSendInitialized)
		{
			bSendInitialized = g_Engine.SetSendPacket(true);
			bChokeInitialized = false;

			m_StateTime.Reset();

			switch (g_FakeLagCon.m_pEnabled->Value())
			{
			case FAKELAG_STANDARD:
				flSendTime = g_FakeLagCon.m_pSend->Value();
				break;
			case FAKELAG_ADAPTIVE:
				flSendTime = std::clamp(265 / g_Local.GV<CVector>(CPlayer::IO::Velocity).BaseMagnitude(), 
					g_FakeLagCon.m_pSend->Value(), g_FakeLagCon.m_pChoke->Value());
				break;
			case FAKELAG_RANDOM:
				flSendTime = std::clamp(g_Math.RandomNumber<float>(g_FakeLagCon.m_pSend->Value(), g_FakeLagCon.m_pSendDeviation->Value()), 
					1.f, 15.f);
				break;
			}
		}

		if (m_StateTime.Ticks() > flSendTime)
		{
			m_State = LagState_t::Choke;
		}
	}
	else
	{
		static float flChokeTime;
		if (!bChokeInitialized)
		{
			bSendInitialized = false;
			bChokeInitialized = g_Engine.SetSendPacket(false);

			m_StateTime.Reset();

			switch (g_FakeLagCon.m_pEnabled->Value())
			{
			case FAKELAG_STANDARD:
				flChokeTime = g_FakeLagCon.m_pChoke->Value();
				break;
			case FAKELAG_ADAPTIVE:
				flChokeTime = std::clamp(log(g_Local.GV<CVector>(CPlayer::IO::Velocity).BaseMagnitude()) / log(1.5f),
					g_FakeLagCon.m_pChoke->Value(), g_FakeLagCon.m_pChoke->Value());
				break;
			case FAKELAG_RANDOM:
				flChokeTime = std::clamp(g_Math.RandomNumber<float>(g_FakeLagCon.m_pChoke->Value(), g_FakeLagCon.m_pChokeDeviation->Value()),
					1.f, 15.f);
				break;
			}
		}

		if (m_StateTime.Ticks() > flChokeTime)
		{
			m_State = LagState_t::Send;
		}
	}
}

bool CFakeLag::PassesStartConditions()
{
	if (g_FakeLagCon.m_pEnabled->Value() == FAKELAG_DISABLED)
	{
		return false;
	}

	if (g_FakeLagCon.m_pKey->Value())
	{
		if (g_FakeLagCon.m_pKey->Toggle())
		{
			static bool bToggled = false;
			if (g_Input.KeyPressed(g_FakeLagCon.m_pKey->Value()))
			{
				bToggled = !bToggled;
			}

			if (!bToggled)
			{
				return false;
			}
		}
		else if (!g_Input.KeyDown(g_FakeLagCon.m_pKey->Value()))
		{
			return false;
		}
	}

	const float flSpeed = g_Local.GV<CVector>(CPlayer::IO::Velocity).BaseMagnitude();
	if (DecimalEnabled(flSpeed, 10.f) &&
		!g_FakeLagCon.m_pWhileX->Value()[FAKELAG_WHILE_RUNNING].m_bSelected)
	{
		return false;
	}

	if (!DecimalEnabled(flSpeed, 10.f) &&
		!g_FakeLagCon.m_pWhileX->Value()[FAKELAG_WHILE_STATIONARY].m_bSelected)
	{
		return false;
	}

	if (!(g_Local.GV<int>(CPlayer::IO::Flags) & FL_ONGROUND) &&
		!g_FakeLagCon.m_pWhileX->Value()[FAKELAG_WHILE_JUMPING].m_bSelected)
	{
		return false;
	}

	constexpr int OneKey = 31;
	constexpr int TwoKey = 32;
	constexpr int ThreeKey = 33;
	constexpr int EKey = 45;

	if ((g_Input.KeyDown(VK_LBUTTON) || g_Input.KeyDown(VK_RBUTTON) ||
		g_Input.KeyDown(OneKey) || g_Input.KeyDown(TwoKey) || g_Input.KeyDown(ThreeKey) || g_Input.KeyDown(EKey)) &&
		!g_FakeLagCon.m_pWhileX->Value()[FAKELAG_WHILE_INTERACTING].m_bSelected)
	{
		return false;
	}

	const auto IsVisible = []() -> bool
	{
		for (size_t i = g_PlayerList.Start; i < g_PlayerList.MaxPlayerCount; i++)
		{
			const CPlayer& Player = g_PlayerList.GetPlayerFromIndex(i);

			if (!Player.IsEnemy())
			{
				continue;
			}

			if (Player.IsVisible())
			{
				return true;
			}
		}

		return false;
	};

	const bool bVisible = IsVisible();
	if (bVisible &&
		!g_FakeLagCon.m_pWhileX->Value()[FAKELAG_WHILE_VISIBLE].m_bSelected)
	{
		return false;
	}

	if (!bVisible &&
		!g_FakeLagCon.m_pWhileX->Value()[FAKELAG_WHILE_INVISIBLE].m_bSelected)
	{
		return false;
	}

	return true;
}
