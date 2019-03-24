#include "SkinChanger.h"
#include "Controls.h"

CSkinChanger g_SkinChanger;

CSkinChanger::CSkinChanger()
{
	Reset();
}

void CSkinChanger::EntryPoint()
{
	if (!g_SkinsCon.m_pEnabled->Value())
	{
		return;
	}

	m_bForceUpdate = false;
	
	for (size_t off = 0; off < 32; off += 4)
	{
		g_PlayerList.GetWeaponFromHandle(m_CurrentWeapon, g_Local.GV<int>(netvars::m_hMyWeapons + off));

		if (!m_CurrentWeapon.Update())
		{
			continue;
		}

		if (m_CurrentWeapon.IsPistol() && g_SkinsCon.m_pPistolSkin->Value() != 0 && (m_Pistol != m_CurrentWeapon.GV<weaponid_t>(CWeapon::IO::WeaponID) ||
			m_CurrentWeapon.GV<int>(CWeapon::IO::FallbackPaintKit) != g_SkinsCon.m_pPistolSkin->Value()))
		{
			m_CurrentWeapon.SV<int>(CWeapon::IO::ItemIDHigh, -1);
			m_CurrentWeapon.SV<int>(CWeapon::IO::FallbackPaintKit, g_SkinsCon.m_pPistolSkin->Value());
			m_CurrentWeapon.SV<float>(CWeapon::IO::FallbackWear, g_SkinsCon.m_pPistolWear->Value());
			m_CurrentWeapon.SV<char[3]>(CWeapon::IO::CustomName, "z1");
			m_CurrentWeapon.SV<int>(CWeapon::IO::FallbackSeed, g_SkinsCon.m_pPistolSeed->Value());
			m_CurrentWeapon.SV<int>(netvars::m_iAccountID, m_CurrentWeapon.GV<int>(netvars::m_OriginalOwnerXuidLow));

			if (g_SkinsCon.m_pPistolStatrack->Value())
			{
				m_CurrentWeapon.SV<int>(CWeapon::IO::FallbackStatTrak, g_SkinsCon.m_pPistolStatrack->Value());
			}

			m_bForceUpdate = true;
			m_Pistol = m_CurrentWeapon.GV<weaponid_t>(CWeapon::IO::WeaponID);
		}
		else if ((m_CurrentWeapon.IsRifle() || m_CurrentWeapon.IsSMG()) && g_SkinsCon.m_pRifleSkin->Value() != 0 && (m_Rifle != m_CurrentWeapon.GV<weaponid_t>(CWeapon::IO::WeaponID) ||
			m_CurrentWeapon.GV<int>(CWeapon::IO::FallbackPaintKit) != g_SkinsCon.m_pRifleSkin->Value()))
		{
			m_CurrentWeapon.SV<int>(CWeapon::IO::ItemIDHigh, -1);
			m_CurrentWeapon.SV<int>(CWeapon::IO::FallbackPaintKit, g_SkinsCon.m_pRifleSkin->Value());
			m_CurrentWeapon.SV<float>(CWeapon::IO::FallbackWear, g_SkinsCon.m_pRifleWear->Value());
			m_CurrentWeapon.SV<char[3]>(CWeapon::IO::CustomName, "z1");
			m_CurrentWeapon.SV<int>(CWeapon::IO::FallbackSeed, g_SkinsCon.m_pRifleSeed->Value());
			m_CurrentWeapon.SV<int>(netvars::m_iAccountID, m_CurrentWeapon.GV<int>(netvars::m_OriginalOwnerXuidLow));

			if (g_SkinsCon.m_pRifleStatrack->Value())
			{
				m_CurrentWeapon.SV<int>(CWeapon::IO::FallbackStatTrak, g_SkinsCon.m_pRifleStatrack->Value());
			}

			m_bForceUpdate = true;
			m_Rifle = m_CurrentWeapon.GV<weaponid_t>(CWeapon::IO::WeaponID);
		}
		else if (m_CurrentWeapon.IsSniper() && g_SkinsCon.m_pSniperSkin->Value() != 0 && (m_Rifle != m_CurrentWeapon.GV<weaponid_t>(CWeapon::IO::WeaponID) ||
			m_CurrentWeapon.GV<int>(CWeapon::IO::FallbackPaintKit) != g_SkinsCon.m_pSniperSkin->Value()))
		{
			m_CurrentWeapon.SV<int>(CWeapon::IO::ItemIDHigh, -1);
			m_CurrentWeapon.SV<int>(CWeapon::IO::FallbackPaintKit, g_SkinsCon.m_pSniperSkin->Value());
			m_CurrentWeapon.SV<float>(CWeapon::IO::FallbackWear, g_SkinsCon.m_pSniperWear->Value());
			m_CurrentWeapon.SV<char[3]>(CWeapon::IO::CustomName, "z1");
			m_CurrentWeapon.SV<int>(CWeapon::IO::FallbackSeed, g_SkinsCon.m_pSniperSeed->Value());
			m_CurrentWeapon.SV<int>(netvars::m_iAccountID, m_CurrentWeapon.GV<int>(netvars::m_OriginalOwnerXuidLow));

			if (g_SkinsCon.m_pSniperStatrack->Value())
			{
				m_CurrentWeapon.SV<int>(CWeapon::IO::FallbackStatTrak, g_SkinsCon.m_pSniperStatrack->Value());
			}

			m_bForceUpdate = true;
			m_Sniper = m_CurrentWeapon.GV<weaponid_t>(CWeapon::IO::WeaponID);
		}
	}

	if (m_bForceUpdate)
	{
		g_Engine.ForceFullUpdate();
	}
}

void CSkinChanger::Reset()
{
	m_Pistol = 0;
	m_Rifle = 0;
	m_Sniper = 0;
	m_bForceUpdate = false;
}
