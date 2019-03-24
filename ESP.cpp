#include "ESP.h"
#include "Engine.h"
#include "Controls.h"
#include "Movement.h"
#include "Backtrack.h"
#include "PlayerPrediction.h"
#include "Overlay.h"

#include <thread>
#include <Mmsystem.h>

CESP g_ESP;

static float s_flFOV = 90.f;

enum class CrosshairMode_t
{
	Regular,
	HitMarker
};

static void DrawCrosshair(int x, int y, int size, int thick, CrosshairMode_t mode, const CColor& c)
{
	switch (mode)
	{
	case CrosshairMode_t::Regular:
		g_Render.Line(x - size, y, x + size, y, thick, c);
		g_Render.Line(x, y - size, x, y + size, thick, c);
		break;
	case CrosshairMode_t::HitMarker:
		g_Render.Line(x - size, y - size, x + size, y + size, thick, c);
		g_Render.Line(x + size, y - size, x - size, y + size, thick, c);
		break;
	}
}

static void DrawAimRing(int x, int y)
{	
	const auto MakeRing = [&](float Diameter, const CColor& c) -> void
	{
		const float flRadius = tanf(Diameter / 2.f) * ((g_Render.Dimensions().right - g_Render.Dimensions().left) / tan(s_flFOV / 2.f));

		constexpr float flJump = g_Math.TAU / 20.f;
		CVector2D Last(flRadius * cosf(0) + x,
			flRadius * sinf(0) + y);

		for (float flCur = 0.f; flCur <= g_Math.TAU + flJump; flCur += flJump)
		{
			const CVector2D Current(flRadius * cosf(flCur) + x,
				flRadius * sinf(flCur) + y);

			g_Render.Line(Last.x, Last.y, Current.x, Current.y, 1, c);
			Last = Current;
		}
	};

	if (g_AimCon.m_pEnabled->Value())
	{
		MakeRing(g_AimCon.m_pFOV->Value(), IMenu::GetOutlineColor());

		if (g_AimCon.m_pSilentBeforeShots->Value() != NEVER_SILENT_SHOTS)
		{
			MakeRing(g_AimCon.m_pSilentFOV->Value(), IMenu::GetBackGroundColor());
		}
	}

	if (g_MagnetCon.m_pEnabled->Value())
	{
		MakeRing(g_MagnetCon.m_pFOV->Value(), IMenu::GetSelectColor());
	}

	if (g_SlowAimCon.m_pEnabled->Value())
	{
		MakeRing(g_SlowAimCon.m_pFOV->Value(), IMenu::GetTextColor());
	}
}

static void DrawHitMarker()
{
	static int LastHitsSound = g_Local.GV<int>(CPlayer::IO::TotalHitsOnServer);
	static int LastHitsVisual = g_Local.GV<int>(CPlayer::IO::TotalHitsOnServer);
	static CTimer StartDraw;

	if (g_SharedCon.m_pHitMarker->Value() &&
		(g_Local.GV<int>(CPlayer::IO::TotalHitsOnServer) != LastHitsVisual || StartDraw.Ticks() < g_SharedCon.m_pDrawTicks->Value() * g_Engine.GetTickScale()))
	{
		DrawCrosshair((g_Render.Dimensions().left + g_Render.Dimensions().right) / 2,
			(g_Render.Dimensions().top + g_Render.Dimensions().bottom) / 2, 6, 1, CrosshairMode_t::HitMarker, CColor::Green);

		if (g_Local.GV<int>(CPlayer::IO::TotalHitsOnServer) != LastHitsVisual)
		{
			LastHitsVisual = g_Local.GV<int>(CPlayer::IO::TotalHitsOnServer);
			StartDraw.Reset();
		}
	}

	if (g_Local.GV<int>(CPlayer::IO::TotalHitsOnServer) != LastHitsSound &&
		g_SharedCon.m_pHitSound->Value())
	{
		LastHitsSound = g_Local.GV<int>(CPlayer::IO::TotalHitsOnServer);
		PlaySoundA("hitmarker.wav", NULL, SND_ASYNC);
	}
}

static void DrawBunnyHopIndicator(char* pBuf)
{
	static bool bPlayed = false;
	if ((g_Local.GV<int>(CPlayer::IO::Flags) & FL_ONGROUND))
	{
		bPlayed = false;
		return;
	}

	const float flRemaining = g_Math.TimeToTicks(g_Movement.GetLandTime() - g_GlobalVars.m_flCurTime);
	if (flRemaining > g_MovementCon.m_pAssistanceTicks->Value() * g_Engine.GetTickScale())
	{
		sprintf(pBuf, "jump_in_%0.2f_ticks", (flRemaining - g_MovementCon.m_pAssistanceTicks->Value()) / g_Engine.GetTickScale());

		g_Render.StringCentred((g_Render.Dimensions().left + g_Render.Dimensions().right) / 2,
			50, CColor::Red, pBuf);
		bPlayed = false;
	}
	else if (flRemaining > 0.f)
	{
		if (!bPlayed && g_SharedCon.m_pJumpHitMarker->Value())
		{
			PlaySoundA("hitmarker.wav", NULL, SND_ASYNC);
			bPlayed = true;
		}

		sprintf(pBuf, "%0.2f_ticks_remaining", flRemaining / g_Engine.GetTickScale());

		g_Render.StringCentred((g_Render.Dimensions().left + g_Render.Dimensions().right) / 2,
			50, CColor::Blue, pBuf);
	}
}

static void DrawRecoilCrosshair()
{
	unsigned int x = (g_Render.Dimensions().right - g_Render.Dimensions().left) / 2,
		y = (g_Render.Dimensions().bottom - g_Render.Dimensions().top) / 2;

	const unsigned int dy = y / s_flFOV, dx = x / s_flFOV;

	const CVector Punch = g_Local.GV<CVector>(CPlayer::IO::ILE::AimPunch) +
		g_Local.GV<CVector>(CPlayer::IO::ILE::ViewPunch);
	x -= dx * Punch.y * 2.f;
	y += dy * Punch.x * 2.f;

	DrawCrosshair(x, y, 4, 1, CrosshairMode_t::Regular, IMenu::GetSelectColor());
}

static void DrawRegularCrosshair()
{
	const auto DrawArc = [](const CVector2D& centre, float radius, float radstart, float radend, const CColor& color) -> void
	{
		constexpr int LineCount = 10;
		const float Jump = (radend - radstart) / LineCount;

		CVector2D Last(centre.x + radius * cosf(radstart), centre.y + radius * sinf(radstart));
		radstart += Jump;

		for (size_t i = 1; i < LineCount; i++)
		{
			const CVector2D Current(centre.x + radius * cosf(radstart), centre.y + radius * sinf(radstart));
			g_Render.Line(Last.x, Last.y, Current.x, Current.y, 1, color);
			Last = Current;
			radstart += Jump;
		}
	};

	const auto DrawCircle = [](const CVector2D& centre, float radius, const CColor& color) -> void
	{
		constexpr int LineCount = 20;
		constexpr float Jump = g_Math.TAU / LineCount;
		float curangle = 0.f;
		CVector2D last(centre.x + radius * cosf(curangle), centre.y + radius * sinf(curangle));
		const CVector2D first = last;
		curangle += Jump;

		while (curangle <= (g_Math.TAU + Jump * 2.f))
		{
			const CVector2D cur(centre.x + radius * cosf(curangle), centre.y + radius * sinf(curangle));
			g_Render.Line(last.x, last.y, cur.x, cur.y, 1, color);
			last = cur;
			curangle += Jump;
		}
	};

	static const CPlayer* pTarget = nullptr;
	const int iCrosshairid = g_Local.GV<int>(CPlayer::IO::ILE::CrosshairID) - 1;
	float flfov = 0.f;
	if (iCrosshairid >= g_PlayerList.Start && iCrosshairid < g_PlayerList.MaxPlayerCount)
	{
		const CPlayer& p = g_PlayerList.GetPlayerFromIndex(iCrosshairid);

		if (p.IsEnemy())
		{
			pTarget = &p;
		}
	}

	if (pTarget)
	{
		if (!pTarget->IsEnemy())
		{
			pTarget = nullptr;
		}
		else
		{
			CVector ang____;
			g_Math.VectorAngles(pTarget->GetBonePosition(BONE_CHEST) - (g_Local.GV<CVector>(CPlayer::IO::Origin) + g_Local.GV<CVector>(CPlayer::IO::ViewOffset)), ang____);
			flfov = g_Math.GetFOV(g_Engine.GetScreenAngles(), ang____);
			if (flfov > 15.f)
			{
				pTarget = nullptr;
			}
		}
	}

	const CVector2D CENRTE((g_Render.Dimensions().left + g_Render.Dimensions().right) / 2.f,
		(g_Render.Dimensions().top + g_Render.Dimensions().bottom) / 2.f);

	const int flRadius = TERANY(g_AimCon.m_pEnabled->Value(), 
		int((g_AimCon.m_pFOV->Value() / 2.f) * ((g_Render.Dimensions().right - g_Render.Dimensions().left) / s_flFOV) + 0.5f), 20);

	DrawCircle(CENRTE, flRadius, CColor::Yellow);

	if (pTarget)
	{
		constexpr int max_bar_count = 8;
		const float bars = (pTarget->GV<int>(CPlayer::IO::Health) / 100.f) * max_bar_count;
		constexpr float gap = g_Math.DegreesToRadians(10.f);
		constexpr float size = (g_Math.TAU - gap * max_bar_count) / (float)max_bar_count;
		const int fullbars = int(bars);
		static float flcur = -g_Math.PI / 2.f;
		const float __start = flcur;
		//const float ______flrot = g_Math.DegreesToRadians((360.f / (g_Math.PHI * (g_Overlay.GetFrameRate() / 30.f))));

		for (size_t i = 0; i < fullbars; i++)
		{
			DrawArc(CENRTE, flRadius * 1.25f, flcur, flcur + size, CColor::Yellow);
			flcur += size + gap;
		}

		if (fullbars != 5)
		{
			DrawArc(CENRTE, flRadius * 1.25f, flcur, flcur + size * (bars - fullbars), CColor::Yellow);
		}

		flcur = __start;

		if (iCrosshairid == pTarget->GetIndex() || flfov < g_AimCon.m_pFOV->Value())
		{
			//looks dumb on low hp
			//flcur += ______flrot;
		}
	}
}

static void DrawBone(const BaseBoneMatrices_t& Matrices, int Bone1, int Bone2, const CColor& Color)
{
	CVector2D Bone1Screen, Bone2Screen;

	if (g_Render.WorldToScreen(Matrices.GetBonePosition(Bone1), Bone1Screen) &&
		g_Render.WorldToScreen(Matrices.GetBonePosition(Bone2), Bone2Screen))
	{
		g_Render.Line(Bone1Screen.x, Bone1Screen.y, Bone2Screen.x, Bone2Screen.y, 1, Color);
	}
};

static void DrawSkeleton(const BaseBoneMatrices_t& Matrices, const CColor& Color)
{
	//stolen from https://www.unknowncheats.me/forum/counterstrike-global-offensive/321397-bone-esp-screen-coordinate.html
	DrawBone(Matrices, 8, 7, Color); //neck
	DrawBone(Matrices, 7, 41, Color); // left shoulder
	DrawBone(Matrices, 7, 11, Color);// right shoulder
	DrawBone(Matrices, 41, 42, Color); // left hand1
	DrawBone(Matrices, 11, 12, Color);// right hand1
	DrawBone(Matrices, 42, 43, Color); // left hand2
	DrawBone(Matrices, 12, 13, Color); // right hand2
	DrawBone(Matrices, 7, 6, Color); // upper body
	DrawBone(Matrices, 6, 5, Color); // middle
	DrawBone(Matrices, 5, 3, Color); // lower body
	DrawBone(Matrices, 3, 77, Color); // left pelvis
	DrawBone(Matrices, 3, 70, Color); // right pelvis
	DrawBone(Matrices, 77, 78, Color); // left leg
	DrawBone(Matrices, 70, 71, Color); // right leg
	DrawBone(Matrices, 78, 74, Color); // left shin
	DrawBone(Matrices, 71, 67, Color); // righit shin
};

void CESP::HandlePlayer()
{
	this->DrawEssentials();

	if (m_CurrentMode == DrawMode_t::Full)
	{
		if (g_ESPCon.m_pDisableWhileX->Value()[ESP_DISABLE_NOT_VISIBLE].m_bSelected && 
			m_VisibleTimes[m_Current->m_Player.GetIndex()].Ticks() >= g_ESPCon.m_pSpottedESPTicks->Value() * g_Engine.GetTickScale())
		{
			return;
		}

		if (g_ESPCon.m_pDisableWhileX->Value()[ESP_DISABLE_NOT_VISIBLE_BY_TEAM].m_bSelected &&
			m_SpottedTimes[m_Current->m_Player.GetIndex()].Ticks() >= g_ESPCon.m_pSpottedESPTicks->Value() * g_Engine.GetTickScale())
		{
			return;
		}

		this->DrawNonEssentials();
	}
}

void CESP::DrawEssentials()
{
	if (g_ESPCon.m_pOptions->Value()[ESP_OPTION_RADAR].m_bSelected)
	{
		if (!m_Current->m_Player.GV<bool>(CPlayer::IO::Spotted))
		{
			m_Current->m_Player.SV<bool>(CPlayer::IO::Spotted, true);
		}
	}

	if (g_ESPCon.m_pOptions->Value()[ESP_OPTION_CHAMS].m_bSelected)
	{
		if (m_Current->m_Player.GV<CColor>(CPlayer::IO::CLRRender) != m_Current->m_Color)
		{
			m_Current->m_Player.SV<CColor>(CPlayer::IO::CLRRender, m_Current->m_Color);
		}
	}

	if (g_ESPCon.m_pOptions->Value()[ESP_OPTION_DAMAGE_INDICATORS].m_bSelected)
	{
		static int LastHP[g_PlayerList.MaxPlayerCount]{};
		const int CurHP = m_Current->m_Player.GV<int>(CPlayer::IO::Health);
		const int i = m_Current->m_Player.GetIndex();

		struct Job_t
		{
			Job_t(int v, CTimer reg) : Val(v), RegTime(reg)
			{}

			int Val;
			CTimer RegTime;
		};

		static std::vector<Job_t> Draws[g_PlayerList.MaxPlayerCount]{};

		if (CurHP != LastHP[i])
		{
			int delta = CurHP - LastHP[i];
			
			if (delta < 0)
			{
				Draws[i].emplace_back(delta, CTimer());
			}
		}

		for (size_t it = 0; it < Draws[i].size(); it++)
		{
			if (Draws[i][it].RegTime.Ticks() * g_Engine.GetTickScale() > 64.f)
			{
				Draws[i].erase(Draws[i].begin() + it);
				continue;
			}

			sprintf(m_Buffer, "%d", Draws[i][it].Val);
			m_Current->RenderInfo(m_Buffer);
		}

		LastHP[i] = CurHP;
	}
}

void CESP::DrawNonEssentials()
{
	const auto& records = g_Backtrack.GetRecords(m_Current->m_Player.GetIndex());
	const auto& futureue = g_Backtrack.GetFutureRecords(m_Current->m_Player.GetIndex());
	
	if (g_AimCon.m_pBackTrack->Value() &&
		g_ESPCon.m_pOptions->Value()[ESP_OPTION_BACKTRACK_SKELETONS].m_bSelected)
	{
		const CVector Source(g_Local.GetSource());
		float flBestFOV = 180.f;
		CVector Angles;
		size_t BestRecord = UINT_MAX;

		for (size_t i = 0; i < records.size(); i++)
		{
			g_Math.VectorAngles(records[i].GetBonePosition(BONE_HEAD) - Source, Angles);
			g_Math.ClampAngle(Angles);

			const float flFOV = g_Math.GetFOV(g_Engine.GetScreenAngles(), Angles);
			if (flFOV < flBestFOV)
			{
				flBestFOV = flFOV;
				BestRecord = i;
			}
		}

		if (g_AimCon.m_pForwardTrack->Value())
		{
			for (size_t i = 0; i < futureue.size(); i++)
			{
				g_Math.VectorAngles(futureue[i].GetBonePosition(BONE_HEAD) - Source, Angles);
				g_Math.ClampAngle(Angles);

				const float flFOV = g_Math.GetFOV(g_Engine.GetScreenAngles(), Angles);
				if (flFOV < flBestFOV)
				{
					flBestFOV = flFOV;
					BestRecord = i;
				}
			}
		}

		for (size_t i = 0; i < records.size(); i++)
		{
			DrawSkeleton(records[i].m_BoneMatrices(), TERANY(i == BestRecord, IMenu::GetSelectColor(), IMenu::GetTextColor()));
		}

		if (g_AimCon.m_pForwardTrack->Value())
		{
			for (size_t i = 0; i < futureue.size(); i++)
			{
				DrawSkeleton(futureue[i].m_BoneMatrices(), TERANY(i == BestRecord, IMenu::GetSelectColor(), IMenu::GetTextColor()));
			}
		}
	}
	else if (g_ESPCon.m_pOptions->Value()[ESP_OPTION_SKELETONS].m_bSelected)
	{
		DrawSkeleton(m_Current->m_Player.GetBoneMatrices(), IMenu::GetTextColor());
	}

	if (g_AimCon.m_pBackTrack->Value() && g_ESPCon.m_pOptions->Value()[ESP_OPTION_TRAILS].m_bSelected)
	{
		if (records.size() > 1)
		{
			CVector2D BestScreenPos;
			const CVector Source(g_Local.GetSource());
			float flBestFOV = 180.f;
			CVector Angles;

			CVector Last = records[0].GetBonePosition(BONE_HEAD);
			CVector2D LastScreen, CurScreen;
			for (size_t i = 1; i < records.size(); i++)
			{
				const CVector Point(records[i].GetBonePosition(BONE_HEAD));
				const bool bVisible = g_Render.WorldToScreen(Point, CurScreen) && g_Render.WorldToScreen(Last, LastScreen);
				if (bVisible)
				{
					g_Render.Line(LastScreen.x, LastScreen.y, CurScreen.x, CurScreen.y, 1,
						TERANY(g_Backtrack.RecordValid(records[i]), m_Current->m_Color, CColor::Blue));
				}

				Last = Point;

				if (bVisible)
				{
					g_Math.VectorAngles(Point - Source, Angles);
					g_Math.ClampAngle(Angles);

					const float flFOV = g_Math.GetFOV(g_Engine.GetScreenAngles(), Angles);
					if (flFOV < flBestFOV)
					{
						flBestFOV = flFOV;
						BestScreenPos = g_Math.Lerp(LastScreen, CurScreen, 0.5f);
					}
				}
			}

			if (g_AimCon.m_pForwardTrack->Value() && futureue.size() > 1)
			{
				Last = futureue[0].GetBonePosition(BONE_HEAD);
				for (size_t i = 1; i < futureue.size(); i++)
				{
					const CVector Point(futureue[i].GetBonePosition(BONE_HEAD));
					const bool bVisible = g_Render.WorldToScreen(Point, CurScreen) && g_Render.WorldToScreen(Last, LastScreen);
					if (bVisible)
					{
						g_Render.Line(LastScreen.x, LastScreen.y, CurScreen.x, CurScreen.y, 1,
							TERANY(g_Backtrack.RecordValid(futureue[i]), m_Current->m_Color, CColor::Blue));
					}

					Last = Point;

					if (bVisible)
					{
						g_Math.VectorAngles(Point - Source, Angles);
						g_Math.ClampAngle(Angles);

						const float flFOV = g_Math.GetFOV(g_Engine.GetScreenAngles(), Angles);
						if (flFOV < flBestFOV)
						{
							flBestFOV = flFOV;
							BestScreenPos = g_Math.Lerp(LastScreen, CurScreen, 0.5f);
						}
					}
				}
			}

			if (!DecimalEqual(flBestFOV, 180.f))
			{
				DrawCrosshair(BestScreenPos.x, BestScreenPos.y, 4, 1, CrosshairMode_t::Regular, IMenu::GetSelectColor());
			}
		}
	}

	if (g_ESPCon.m_pOptions->Value()[ESP_OPTION_PREDICT].m_bSelected)
	{
		CVector last(m_Current->m_Player.GetBonePosition(BONE_HEAD)), extrpaolated = last;
		CVector2D lstsrenn, ursreen;
		const int loop = g_ESPCon.m_pPredictTicks->Value() * (int)(g_Engine.GetTickScale() + 0.5f);
		for (size_t i = 1; i < loop; i++)
		{
			g_Prediction.Extrapolate(m_Current->m_Player, extrpaolated, g_Math.TicksToTime(i));

			if (g_Render.WorldToScreen(last, lstsrenn) && g_Render.WorldToScreen(extrpaolated, ursreen))
			{
				g_Render.Line(lstsrenn.x, lstsrenn.y, ursreen.x, ursreen.y, 1, m_Current->m_Color);
			}

			last = extrpaolated;
			extrpaolated = m_Current->m_Player.GetBonePosition(BONE_HEAD);
		}
	}

	if (g_ESPCon.m_pOptions->Value()[ESP_OPTION_INFO].m_bSelected)
	{
		if (g_ESPCon.m_pInfoOptions->Value()[ESP_INFO_NAME].m_bSelected)
		{
			g_Render.FormatName(m_Current->m_Player.GetPlayerInfo().m_szPlayerName, m_Buffer, 13);
			m_Current->RenderInfo(m_Buffer);
		}

		if (g_ESPCon.m_pInfoOptions->Value()[ESP_INFO_WEAPON].m_bSelected)
		{
			const weaponid_t ID = m_Current->m_Player.GetActiveWeapon().GV<weaponid_t>(CWeapon::IO::WeaponID);
			const char* szWeapon = TERANY(ID >= 0 && ID < szWeaponNames.size(), szWeaponNames[ID], "knife");
			m_Current->RenderInfo(szWeapon);
		}

		if (g_ESPCon.m_pInfoOptions->Value()[ESP_INFO_HEALTH].m_bSelected)
		{
			sprintf(m_Buffer, "hp: %d", m_Current->m_Player.GV<int>(CPlayer::IO::Health));
			m_Current->RenderInfo(m_Buffer);
		}

		if (g_ESPCon.m_pInfoOptions->Value()[ESP_INFO_ARMOR].m_bSelected)
		{
			sprintf(m_Buffer, "ap: %d", m_Current->m_Player.GV<int>(CPlayer::IO::Armor));
			m_Current->RenderInfo(m_Buffer);
		}

		if (g_ESPCon.m_pInfoOptions->Value()[ESP_INFO_DEFUSER].m_bSelected || 
			(m_Current->m_Player.GV<bool>(netvars::m_bIsDefusing) || m_Current->m_Player.GV<bool>(CPlayer::IO::IsGrabbyingHostage)))
		{
			if (m_Current->m_Player.GV<bool>(netvars::m_bHasDefuser))
			{
				m_Current->RenderInfo("has_kit");
			}
		}

		if (g_ESPCon.m_pInfoOptions->Value()[ESP_INFO_DEFUSING].m_bSelected)
		{
			struct DefuseInteraction_t
			{
				CTimer m_Time;
				int m_Index;
			};

			static DefuseInteraction_t CurrentDefuse;

			if (m_Current->m_Player.GV<bool>(netvars::m_bIsDefusing) || m_Current->m_Player.GV<bool>(CPlayer::IO::IsGrabbyingHostage))
			{
				if (m_Current->m_Player.GetIndex() != CurrentDefuse.m_Index)
				{
					CurrentDefuse.m_Time.Reset();
					CurrentDefuse.m_Index = m_Current->m_Player.GetIndex();
				}

				sprintf(m_Buffer, "interacting: %0.3f", CurrentDefuse.m_Time.Elapsed());
				m_Current->RenderInfo(m_Buffer);
			}
		}

		if (g_ESPCon.m_pInfoOptions->Value()[ESP_INFO_RANK].m_bSelected)
		{
			m_Current->RenderInfo(szCompetitiveRanks[std::clamp(m_Current->m_Player.GetCompetitiveRank() + 1, 0, 20)]);
		}
	}

	if (g_ESPCon.m_pOptions->Value()[ESP_OPTION_BOXES].m_bSelected)
	{
		g_Render.BorderedBox(m_Current->m_ScreenTop.x - m_Current->m_iWidth, m_Current->m_ScreenTop.y,
			m_Current->m_iWidth * 2, m_Current->m_iHeight, 1, m_Current->m_Brush);
	}

	if (g_ESPCon.m_pOptions->Value()[ESP_OPTION_HEALTH_BARS].m_bSelected)
	{
		int Count = 0;
		constexpr int Height = 3;

		const auto MakeBar = [&](int Value, HBRUSH Brush) -> void
		{
			const RECT Bounds = { m_Current->m_ScreenTop.x - m_Current->m_iWidth, m_Current->m_ScreenTop.y - Height * (Count + 1) - Height,
				m_Current->m_ScreenTop.x + m_Current->m_iWidth, m_Current->m_ScreenTop.y - Height * Count - Height };

			g_Render.FilledBox({ Bounds.left, Bounds.top + 1, LONG(Bounds.left + 2.f * m_Current->m_iWidth * ((float)Value / 100.f)), Bounds.bottom }, Brush);
			g_Render.BorderedBox(Bounds.left, Bounds.top, Bounds.right - Bounds.left, Bounds.bottom - Bounds.top, 1, IMenu::GetOutlineBrush());
			Count++;
		};

		MakeBar(m_Current->m_Player.GV<int>(CPlayer::IO::Health), CColor::BlueBrush);
		MakeBar(m_Current->m_Player.GV<int>(CPlayer::IO::Armor), m_Current->m_Brush);
	}

	if (g_ESPCon.m_pOptions->Value()[ESP_OPTION_IDEAL_CROSSHAIR].m_bSelected)
	{
		unsigned int x = m_Current->m_ScreenHead.x,
			y = m_Current->m_ScreenHead.y;
		const unsigned int dy = y / s_flFOV, dx = x / s_flFOV;

		const CVector Punch = g_Local.GV<CVector>(CPlayer::IO::ILE::AimPunch) +
			g_Local.GV<CVector>(CPlayer::IO::ILE::ViewPunch);

		x += dx * Punch.y * 2.f;
		y -= dy * Punch.x * 2.f;

		DrawCrosshair(x, y, 4, 1, CrosshairMode_t::Regular, m_Current->m_Color);
	}

	if (g_ESPCon.m_pOptions->Value()[ESP_OPTION_AIM_POINTS].m_bSelected)
	{
		CVector2D out;
		for (auto i : g_AimCon.m_pBoneList->Value())
		{
			if (!g_Render.WorldToScreen(m_Current->m_Player.GetBonePosition(i), out))
			{
				continue;
			}

			DrawCrosshair(out.x, out.y, 3, 1, CrosshairMode_t::Regular, m_Current->m_Color);
		}
	}

	if (g_ESPCon.m_pOptions->Value()[ESP_OPTION_GLOW].m_bSelected)
	{
		GlowObject_t settings;
		FillBufRead(g_Engine.GetGlowPointer() + (m_Current->m_Player.GV<int>(CPlayer::IO::GlowIndex) * 0x38) + 0x4, settings);

		settings.a = g_ESPCon.m_pGlowAlpha->Value();
		settings.r = m_Current->m_Color.rBase();
		settings.g = m_Current->m_Color.gBase();
		settings.b = m_Current->m_Color.bBase();
		settings.BloomAmount = g_ESPCon.m_pGlowBloomAmount->Value();
		settings.Style = g_ESPCon.m_pGlowStyle->Value();
		settings.RenderWhenOccluded = true; 
		settings.RenderWhenUnoccluded = false;
		settings.FullBloom = g_ESPCon.m_pGlowBloom->Value();

		switch (m_Current->m_Player.GetVunerable())
		{
		case CPlayer::VunerableContext_t::Flashed:
		{
			settings.r = CColor::White.r() / 255.f;
			settings.g = CColor::White.g() / 255.f;
			settings.b = CColor::White.b() / 255.f;
			settings.a = m_Current->m_Player.GV<float>(CPlayer::IO::FlashAlpha) / 255.f;
		}
		break;

		case CPlayer::VunerableContext_t::NoAmmo:
		case CPlayer::VunerableContext_t::Reloading:
			settings.r = CColor::Blue.r() / 255.f;
			settings.g = CColor::Blue.g() / 255.f;
			settings.b = CColor::Blue.b() / 255.f;
			settings.a = 1.f;
			break;

		default:
			break;
		}

		Write(g_Engine.GetGlowPointer() + (m_Current->m_Player.GV<int>(CPlayer::IO::GlowIndex) * 0x38) + 0x4, settings);
	}

	if (g_ESPCon.m_pOptions->Value()[ESP_OPTION_JUMP_RINGS].m_bSelected)
	{
		static bool bWasOnGround[g_PlayerList.MaxPlayerCount]{};
		static CVector JumpVelocity[g_PlayerList.MaxPlayerCount]{};
		static CVector JumpCentre[g_PlayerList.MaxPlayerCount]{};
		static float flAngle[g_PlayerList.MaxPlayerCount]{};
		static float flRangle[g_PlayerList.MaxPlayerCount]{};

		const bool OnGround = (m_Current->m_Player.GV<int>(CPlayer::IO::Flags) & FL_ONGROUND);
		const int i = m_Current->m_Player.GetIndex();

		if (!OnGround && bWasOnGround[i])
		{
			JumpVelocity[i] = m_Current->m_Player.GV<CVector>(CPlayer::IO::Velocity);
			JumpCentre[i] = m_Current->m_Player.GV<CVector>(CPlayer::IO::Origin);
			flAngle[i] = atanf(JumpVelocity[i].z / JumpVelocity[i].BaseMagnitude());
			flRangle[i] = (Squared(JumpVelocity[i].Magnitude()) * sinf(2 * flRangle[i])) / 800.f;
		}

		if (!OnGround)
		{
			constexpr float flJump = g_Math.TAU / 20.f;

			CVector Last = CVector(flRangle[i], 0.f) + JumpCentre[i];

			CVector2D OutCur, OutLast;
			for (float it = 0.f; it <= g_Math.TAU; it += flJump)
			{
				const CVector Current = CVector(cosf(it) * flRangle[i], sinf(it) * flRangle[i]) + JumpCentre[i];
				if (g_Render.WorldToScreen(Current, OutCur) && g_Render.WorldToScreen(Last, OutLast))
				{
					g_Render.Line(OutLast.x, OutLast.y, OutCur.x, OutCur.y, 1, m_Current->m_Color);
				}

				Last = Current;
			}
		}

		bWasOnGround[i] = OnGround;
	}

	if (g_ESPCon.m_pOptions->Value()[ESP_OPTION_SNAP_LINES].m_bSelected)
	{
		g_Render.Line((g_Render.Dimensions().left + g_Render.Dimensions().right) / 2, g_Render.Dimensions().bottom - 1,
			m_Current->m_ScreenBottom.x, m_Current->m_ScreenBottom.y + m_Current->GetInfoVerticalOffset(), 1, m_Current->m_Color);
	}
}

void CESP::EntryPoint()
{
	while (!g_Engine.IsValid())
	{
		std::this_thread::yield();
	}

	if (g_SharedCon.m_pHitMarker->Value() || g_SharedCon.m_pHitSound->Value())
	{
		DrawHitMarker();
	}

	if(g_SharedCon.m_pHud->Value())
	{
		int _x = g_Render.Dimensions().left + 1;
		int _y = g_Render.Dimensions().bottom - (FONT_TITLE_HEIGHT + 2);

		const CPlayer& Player = []() -> const CPlayer&
		{
			if (g_Local.GV<int>(CPlayer::IO::Health) <= 0 && g_Local.GV<int>(CPlayer::IO::ObserverMode) != OBS_NONE)
			{
				const int Index = (g_Local.GV<int>(netvars::m_hObserverTarget) & 0xFFF) - 1;
				if (Index >= g_PlayerList.Start && Index < g_PlayerList.MaxPlayerCount)
				{
					return g_PlayerList.GetPlayerFromIndex(Index);
				}
			}

			return g_Local;
		}();

		g_Render.FormatName(Player.GetPlayerInfo().m_szPlayerName, m_Buffer, 13);

		sprintf(m_Buffer, "hp: %d ap: %d, name: %s", 
			Player.GV<int>(CPlayer::IO::Health), Player.GV<int>(CPlayer::IO::Armor), m_Buffer);
		
		const int w = g_Render.GetTextWidthTitle(m_Buffer);
		g_Render.FilledBox(_x, _y + 1, w + 1, FONT_TITLE_HEIGHT, IMenu::GetBackGroundBrush());
		g_Render.BorderedBox(_x - 1, _y, w + 2, FONT_TITLE_HEIGHT + 1, 1, IMenu::GetTextBrush());
		g_Render.TitleNotCentred(_x + 1, _y + 1, IMenu::GetTextColor(), m_Buffer);
	}

	if (!g_ESPCon.m_pEnabled->Value())
	{
		return;
	}

	if (g_MovementCon.m_pBunnyHop->Value() == BHOP_LEGIT)
	{
		DrawBunnyHopIndicator(m_Buffer);
	}

	if (g_ESPCon.m_pRecoil->Value())
	{
		DrawRecoilCrosshair();
	}

	if (g_ESPCon.m_pOptions->Value()[ESP_OPTION_AIM_RINGS].m_bSelected)
	{
		DrawAimRing((g_Render.Dimensions().right - g_Render.Dimensions().left) / 2, 
			(g_Render.Dimensions().bottom - g_Render.Dimensions().top) / 2);
	}

	if (g_ESPCon.m_pViewModel->Value())
	{
		const DWORD_PTR Base = g_PlayerList.GetBaseEntityFromHandle(g_Local.GV<int>(CPlayer::IO::ViewModel));
		CColor col;

		if (DecimalEnabled(g_ESPCon.m_pBrightness->Value()))
		{
			col = CColor
			(
				(int)std::clamp(((float)g_ESPCon.m_pViewModelColour->Value().r() * g_ESPCon.m_pViewModelBrightnessMod->Value()) / g_ESPCon.m_pBrightness->Value(), 0.f, 255.f),
				(int)std::clamp(((float)g_ESPCon.m_pViewModelColour->Value().g() * g_ESPCon.m_pViewModelBrightnessMod->Value()) / g_ESPCon.m_pBrightness->Value(), 0.f, 255.f),
				(int)std::clamp(((float)g_ESPCon.m_pViewModelColour->Value().b() * g_ESPCon.m_pViewModelBrightnessMod->Value()) / g_ESPCon.m_pBrightness->Value(), 0.f, 255.f),
				255
			);
		}
		else
		{
			col = g_ESPCon.m_pViewModelColour->Value();
		}

		Write(Base + netvars::m_clrRender, col);
	}

	if (g_ESPCon.m_pCrosshair->Value())
	{
		DrawRegularCrosshair();
	}

	if (g_ESPCon.m_pKey->Value())
	{
		if (g_ESPCon.m_pKey->Toggle())
		{
			static bool bToggled = false;
			if (g_Input.KeyPressed(g_ESPCon.m_pKey->Value()))
			{
				bToggled = !bToggled;
			}

			if (bToggled)
			{
				m_CurrentMode = DrawMode_t::Full;
			}
			else
			{
				m_CurrentMode = DrawMode_t::Essentials;
			}
		}
		else if (g_Input.KeyDown(g_ESPCon.m_pKey->Value()))
		{
			m_CurrentMode = DrawMode_t::Full;
		}
		else
		{
			m_CurrentMode = DrawMode_t::Essentials;
		}
	}
	else
	{
		m_CurrentMode = DrawMode_t::Full;
	}

	if (g_ESPCon.m_pDisableWhileX->Value()[ESP_DISABLE_LOCAL_ALIVE].m_bSelected && g_Local.GV<int>(CPlayer::IO::Health) > 0)
	{
		m_CurrentMode = DrawMode_t::Essentials;
	}

	if (g_FakeLagCon.m_pEnabled->Value())
	{
		sprintf(m_Buffer, "choked_ticks: %d", g_Engine.GetChokedCommands());
		g_Render.StringNotCentred(FONT_HEIGHT * 1.5, 290, TERANY(g_Engine.GetChokedCommands() > 0, IMenu::GetSelectColor(), IMenu::GetOutlineColor()), m_Buffer);
	}

	for (size_t i = g_PlayerList.Start; i < g_PlayerList.MaxPlayerCount; i++)
	{
		CPlayer& Player = g_PlayerList.GetPlayerFromIndex(i);

		if (!Player.IsValid())
		{
			continue;
		}

		if (Player.IsVisible())
		{
			m_VisibleTimes[i].Reset();
		}

		if (Player.GV<bool>(CPlayer::IO::Spotted))
		{
			m_SpottedTimes[i].Reset();
		}

		ESPObject_t obj(Player);

		if (obj.IsValid())
		{
			m_Current = &obj;
			HandlePlayer();
		}
		else 
		{
			if (!g_ESPCon.m_pTargets->Value()[ESP_TARGET_ALLY].m_bSelected && Player.IsAlly() && Player != g_Local &&
				Player.GV<CColor>(CPlayer::IO::CLRRender) != CColor(255, 255, 255, 255))
			{
				Player.SV<CColor>(CPlayer::IO::CLRRender, CColor(255, 255, 255, 255));
			}
			else if (g_ESPCon.m_pTargets->Value()[ESP_OPTION_RADAR].m_bSelected &&
				!Player.GV<bool>(CPlayer::IO::Spotted) && Player.IsEnemy())
			{
				Player.SV<bool>(CPlayer::IO::Spotted, true);
			}	
		}
	}
}

bool ESPObject_t::Register()
{
	if (!PlayerPassesConditions())
	{
		return false;
	}

	m_WorldHeed = m_Player.GetBonePosition(BONE_HEAD);
	if (!g_Render.WorldToScreen(m_WorldHeed, m_ScreenHead))
	{
		return false;
	}

	m_WorldFeet = m_WorldHeed - m_Player.GV<CVector>(CPlayer::IO::ViewOffset);
	if (!g_Render.WorldToScreen(m_WorldFeet, m_ScreenBottom))
	{
		return false;
	}

	if (!g_Render.WorldToScreen(m_WorldHeed + CVector(0.f, 0.f, 9.f), m_ScreenTop))
	{
		return false;
	}

	m_iHeight = m_ScreenBottom.y - m_ScreenTop.y;
	m_iWidth = m_iHeight / 5;
	m_iHeight -= m_iWidth / 2;
	m_InfoSpot = CVector2D(m_ScreenBottom.x, m_ScreenBottom.y + FONT_HEIGHT + 1);

	if (m_Player.GV<int>(CPlayer::IO::Team) != g_Local.GV<int>(CPlayer::IO::Team))
	{
		if (m_Player.IsVisible())
		{
			m_Color = CColor::Red;
			m_Brush = CColor::RedBrush;
		}
		else
		{
			m_Color = CColor::Orange;
			m_Brush = CColor::OrangeBrush;
		}
	}
	else
	{
		m_Color = CColor::Blue;
		m_Brush = CColor::BlueBrush;
	}

	return true;
}

ESPObject_t::ESPObject_t(CPlayer & Binding) :
	m_Player(Binding)
{
	m_bValid = this->Register();
}

bool ESPObject_t::IsValid() const
{
	return m_bValid;
}

int ESPObject_t::GetInfoVerticalOffset() const
{
	return m_InfoSpot.y - m_ScreenBottom.y;
}

void ESPObject_t::RenderInfo(const char * pInfo)
{
	g_Render.StringCentred(m_InfoSpot.x, m_InfoSpot.y, IMenu::GetTextColor(), pInfo);
	m_InfoSpot.y += FONT_HEIGHT + 1;
}

bool ESPObject_t::PlayerPassesConditions() const
{
	const bool bEnemy = m_Player.GV<int>(CPlayer::IO::Team) != g_Local.GV<int>(CPlayer::IO::Team);
	const bool bAlly = m_Player.GV<int>(CPlayer::IO::Team) == g_Local.GV<int>(CPlayer::IO::Team);

	if (bEnemy &&
		!g_ESPCon.m_pTargets->Value()[ESP_TARGET_ENEMY].m_bSelected)
	{
		return false;
	}

	if (bAlly && m_Player != g_Local &&
		!g_ESPCon.m_pTargets->Value()[ESP_TARGET_ALLY].m_bSelected)
	{
		return false;
	}

	if (m_Player == g_Local &&
		!g_ESPCon.m_pTargets->Value()[ESP_TARGET_LOCAL].m_bSelected)
	{
		return false;
	}

	if (m_Player.GV<bool>(CPlayer::IO::IsWalking) &&
		g_ESPCon.m_pDisableWhileX->Value()[ESP_DISABLE_WALKING].m_bSelected)
	{
		return false;
	}

	if (!DecimalEnabled(m_Player.GV<CVector2D>(CPlayer::IO::Velocity).Magnitude(), 0.1f) &&
		g_ESPCon.m_pDisableWhileX->Value()[ESP_DISABLE_STATIONARY].m_bSelected)
	{
		return false;
	}

	if (m_Player.IsVisible() &&
		g_ESPCon.m_pDisableWhileX->Value()[ESP_DISABLE_VISIBLE].m_bSelected)
	{
		return false;
	}

	if (m_Player == g_Local && !g_Local.GV<int>(netvars::m_iObserverMode))
	{
		return false;
	}

	return true;
}
