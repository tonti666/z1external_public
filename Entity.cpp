#include "Entity.h"
#include "Engine.h"
#include "Controls.h"
#include "PlayerPrediction.h"

CPlayer g_Local;

CWeapon::CWeapon(int Handle) : 
	m_ReadData(), m_BaseAddress(0)
{
	if (Handle)
	{
		Register(Handle);
		this->Update();
	}
}

CWeapon::CWeapon(const CWeapon & rhs) :
	m_BaseAddress(rhs.m_BaseAddress), m_ReadData(rhs.m_ReadData)
{
	;
}

CWeapon::~CWeapon()
{
	m_ReadData.~CGameManagedResource();
}

void CWeapon::Register(int Handle) 
{
	Handle &= 0xFFF;
	m_BaseAddress = Read(g_Memory.Client() + IPlayerList::IO::PlayerList + (Handle - 1) * 0x10, int);
}

CWeapon& CWeapon::operator=(const CWeapon& rhs)
{
	m_BaseAddress = rhs.m_BaseAddress;
	m_ReadData = rhs.m_ReadData;
	return *this;
}

bool CWeapon::operator!=(const CWeapon& rhs)
{
	return GV<weaponid_t>(CWeapon::IO::WeaponID) != rhs.GV<weaponid_t>(CWeapon::IO::WeaponID);
}

DWORD_PTR& CWeapon::Raw()
{
	return m_BaseAddress;
}

DWORD_PTR CWeapon::Raw() const
{
	return m_BaseAddress;
}

DWORD_PTR& CWeapon::operator()()
{
	return this->Raw();
}

DWORD_PTR CWeapon::operator()() const
{
	return this->Raw();
}

bool CWeapon::Update()
{
	if (!m_BaseAddress)
	{
		return false;
	}

	m_ReadData.Update(m_BaseAddress);

	return true;
}

bool CWeapon::CanFire() const
{
	return this->GV<float>(IO::NextPrimaryAttack) < g_Engine.GetServerTime() && 
		this->GV<int>(CWeapon::IO::Clip1) > 0;
}

template <typename T>
static bool IsX(const std::vector<T>& Container, const T& Value)
{
	for (const auto& i : Container)
	{
		if (Value == i)
		{
			return true;
		}
	}

	return false;
}

bool CWeapon::IsRifle() const
{
	static const std::vector<weaponid_t> Rifles
	{
		WEAPON_AK47, WEAPON_AUG, WEAPON_FAMAS, WEAPON_GALILAR, WEAPON_M249, WEAPON_M4A1, WEAPON_SG553, WEAPON_M4A1_SILENCER
	};

	return IsX(Rifles, this->GV<weaponid_t>(IO::WeaponID));
}

bool CWeapon::IsGrenade() const
{
	static const std::vector<weaponid_t> Grenades
	{
		WEAPON_FLASHBANG, WEAPON_SMOKEGRENADE, WEAPON_MOLOTOV, WEAPON_INCGRENADE
	};

	return IsX(Grenades, this->GV<weaponid_t>(IO::WeaponID));
}

bool CWeapon::IsSMG() const
{
	static const std::vector<weaponid_t> SMGs
	{
		WEAPON_MAC10, WEAPON_P90, WEAPON_MP5SD, WEAPON_UMP45, WEAPON_BIZON, WEAPON_MP7, WEAPON_MP9, WEAPON_CZ75A
	};

	return IsX(SMGs, this->GV<weaponid_t>(IO::WeaponID));
}

bool CWeapon::IsPistol() const
{
	static const std::vector<weaponid_t> Pistols
	{
		WEAPON_DEAGLE, WEAPON_ELITE, WEAPON_FIVESEVEN, WEAPON_GLOCK, WEAPON_TEC9, WEAPON_HKP2000, WEAPON_P250, WEAPON_USP_SILENCER
	};

	return IsX(Pistols, this->GV<weaponid_t>(IO::WeaponID));
}

bool CWeapon::IsSniper() const
{
	static const std::vector<weaponid_t> Snipers
	{
		WEAPON_AWP, WEAPON_G3SG1, WEAPON_SCAR20, WEAPON_SSG08
	};

	return IsX(Snipers, this->GV<weaponid_t>(IO::WeaponID));
}

bool CWeapon::IsShotgun() const
{
	static const std::vector<weaponid_t> Shotguns
	{
		WEAPON_NOVA, WEAPON_SAWEDOFF, WEAPON_MAG7, WEAPON_XM1014
	};

	return IsX(Shotguns, this->GV<weaponid_t>(IO::WeaponID));
}

bool CWeapon::IsWeapon() const
{
	return IsRifle() || IsPistol() || IsSniper() || IsSMG() || IsShotgun();
}

CPlayer::CPlayer(DWORD_PTR BaseAddress) : 
	m_BaseAddress(BaseAddress), m_ReadData(), m_ActiveWeapon(), m_Info(), m_BoneMatrices(), m_iCompetitiveRanking(-1)
{
	this->Update();
}

CPlayer::CPlayer(const CPlayer& rhs) : 
	m_BaseAddress(rhs.m_BaseAddress), m_ReadData(rhs.m_ReadData), m_ActiveWeapon(rhs.m_ActiveWeapon),
	m_Info(rhs.m_Info), m_BoneMatrices(rhs.m_BoneMatrices), m_iCompetitiveRanking(rhs.m_iCompetitiveRanking)
{
	;
}

CPlayer::~CPlayer()
{
	m_ReadData.~CGameManagedResource();
	m_Info.~CGameManagedResource();
	m_BoneMatrices.~CGameManagedResource();
	m_ActiveWeapon.~CWeapon();
}

CPlayer& CPlayer::operator=(const CPlayer& rhs)
{
	m_BaseAddress = rhs.m_BaseAddress;
	m_ActiveWeapon = rhs.m_ActiveWeapon;

	m_ReadData = rhs.m_ReadData;
	m_Info = rhs.m_Info;
	m_BoneMatrices = rhs.m_BoneMatrices;
	m_iCompetitiveRanking = rhs.m_iCompetitiveRanking;

	return *this;
}

DWORD_PTR CPlayer::Raw() const
{
	return m_BaseAddress;
}

DWORD_PTR & CPlayer::Raw()
{
	return m_BaseAddress;
}

DWORD_PTR& CPlayer::operator()()
{
	return m_BaseAddress;
}

DWORD_PTR CPlayer::operator()() const
{
	return m_BaseAddress;
}

int CPlayer::GetIndex() const
{
	return this->GV<int>(CPlayer::IO::Index) - 1;
}

bool CPlayer::Update()
{
	if (!m_BaseAddress)
	{
		return false;
	}

	m_ReadData.Update(m_BaseAddress);

	if (this->IsValid())
	{
		m_ActiveWeapon.Register(this->GV<int>(IO::hActiveWeapon));
		m_ActiveWeapon.Update();
	}

	m_Info.Update(Read(g_Engine.GetPlayerInfoItems() + 0x28 + GetIndex() * 0x34, DWORD_PTR));
	m_BoneMatrices.Update(GV<DWORD_PTR>(CPlayer::IO::BoneMatrix));

	if (m_iCompetitiveRanking < COMP_RANK_MIN || m_iCompetitiveRanking > COMP_RANK_MAX)
	{
		m_iCompetitiveRanking = Read(g_Engine.GetPlayerResourceItems() + netvars::m_iCompetitiveRanking + GetIndex() * 0x4, int);
	}

	return true;
}

bool CPlayer::IsValid() const
{
	return
		m_BaseAddress != 0 &&
		GetIndex() != -1 &&
		GV<int>(IO::Health) > 0 &&
		GV<CVector>(IO::Origin) != CVector(0, 0, 0) &&
		(g_Engine.GetMap() == Map_t::DangerZone || !GV<bool>(IO::Dormant)) &&
		(g_Engine.GetMap() == Map_t::DangerZone || GV<int>(IO::Team) != TEAM_SPECTATOR);
}

CPlayer::VunerableContext_t CPlayer::GetVunerable() const
{
	if (this->GV<float>(CPlayer::IO::FlashDuration) > 0.f)
	{
		return VunerableContext_t::Flashed;
	}

	if (this->GV<bool>(CPlayer::IO::InReload))
	{
		return VunerableContext_t::Reloading;
	}

	if (this->GetActiveWeapon().GV<int>(CWeapon::IO::Clip1) <= 0)
	{
		return VunerableContext_t::NoAmmo;
	}

	if (this->GetActiveWeapon().IsSniper() && !this->GV<bool>(CPlayer::IO::Scoped))
	{
		return VunerableContext_t::SniperNoScope;
	}

	return VunerableContext_t::NotVunerable;
}

bool CPlayer::IsEnemy() const
{
	if (!IsValid())
	{
		return false;
	}

	if (g_SharedCon.m_pFFA->Value())
	{
		return true;
	}
	else if (g_Engine.GetMap() == Map_t::DangerZone)
	{
		return TERANY(g_Local.GV<int>(IO::Team) == -1, true, g_Local.GV<int>(IO::Team != this->GV<int>(IO::Team)));
	}
	else
	{
		return GV<int>(IO::Team) != g_Local.GV<int>(IO::Team);
	}
}

bool CPlayer::IsAlly() const
{
	if (g_SharedCon.m_pFFA->Value())
	{
		return false;
	}
	else
	{
		return IsValid() && !IsEnemy();
	}
}

bool CPlayer::IsVisible() const
{
	return (this->GV<int>(CPlayer::IO::SpottedByMask) & (1 << g_Local.GetIndex())) && 
		g_Local.GV<float>(CPlayer::IO::FlashAlpha) < 150.f;
}

bool CPlayer::operator==(const CPlayer& rhs) const
{
	return GetIndex() == rhs.GetIndex();
}

bool CPlayer::operator!=(const CPlayer& rhs) const
{
	return !((*this) == rhs);
}

CVector CPlayer::GetBonePosition(int Bone) const
{
	return m_BoneMatrices().GetBonePosition(Bone);
}

const BaseBoneMatrices_t & CPlayer::GetBoneMatrices() const
{
	return m_BoneMatrices();
}

BaseBoneMatrices_t & CPlayer::GetBoneMatrices()
{
	return m_BoneMatrices();
}

const CWeapon& CPlayer::GetActiveWeapon() const
{
	return m_ActiveWeapon;
}

CWeapon& CPlayer::GetActiveWeapon()
{
	return m_ActiveWeapon;
}

const PlayerInfo_t & CPlayer::GetPlayerInfo() const
{
	return m_Info();
}

PlayerInfo_t & CPlayer::GetPlayerInfo()
{
	return m_Info();
}

const int CPlayer::GetCompetitiveRank() const
{
	return m_iCompetitiveRanking;
}

const int CPlayer::GetPing() const
{
	return Read(g_Engine.GetPlayerResourceItems() + IO::Ping + GetIndex() * 0x4, int);
}

CVector CPlayer::GetSource() const
{
	CVector ret(GV<CVector>(IO::Origin) + GV<CVector>(IO::ViewOffset));
	g_Prediction.Extrapolate(*this, ret, g_Math.TicksToTime(1));
	return ret;
}
