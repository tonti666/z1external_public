#pragma once
#include "Classes.h"
#include "Engine.h"

class CSkinChanger : public CBaseHack
{
private:
	weaponid_t m_Pistol, m_Rifle, m_Sniper;
	CWeapon m_CurrentWeapon;
	bool m_bForceUpdate;

public:
	CSkinChanger();

	void EntryPoint() override;
	void Reset();
};

extern CSkinChanger g_SkinChanger;