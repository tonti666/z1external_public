#pragma once
#include "Offsets.h"
#include "Memory.h"
#include "Math.h"

typedef short weaponid_t;

template <size_t _Size>
struct BaseReadData_t
{
public:
	BaseReadData_t()
	{
		;
	}

	BYTE m_Data[_Size];
};

struct BaseBoneMatrices_t
{
private:
	Matrix3x4_t m_Matrices[BONE_MAX];

public:
	BaseBoneMatrices_t()
	{
		;
	}

	CVector GetBonePosition(int Bone) const
	{
		return CVector(m_Matrices[Bone].m_matrix[0][3], m_Matrices[Bone].m_matrix[1][3], m_Matrices[Bone].m_matrix[2][3]);
	}

	void SetBonePosition(int Bone, const CVector& New)
	{
		m_Matrices[Bone].m_matrix[0][3] = New.x;
		m_Matrices[Bone].m_matrix[1][3] = New.y;
		m_Matrices[Bone].m_matrix[2][3] = New.z;
	}
};

template <typename T>
class CGameManagedResource
{
private:
	T* m_pResource;

public:
	CGameManagedResource() :
		m_pResource(new T)
	{
		;
	}

	CGameManagedResource(const CGameManagedResource<T>& rhs) :
		m_pResource(new T)
	{
		memcpy(m_pResource, rhs.m_pResource, sizeof(T));
	}

	CGameManagedResource(const T& rhs) :
		m_pResource(new T)
	{
		memcpy(m_pResource, &rhs, sizeof(T));
	}

	CGameManagedResource<T>& operator=(const CGameManagedResource<T>& rhs)
	{
		memcpy(m_pResource, rhs.m_pResource, sizeof(T));
		return *this;
	}

	CGameManagedResource<T>& operator=(const T& rhs)
	{
		memcpy(m_pResource, rhs, sizeof(T));
		return *this;
	}

	~CGameManagedResource()
	{
		delete m_pResource;
	}

	bool Update(DWORD_PTR Address)
	{
		return ManualRead(Address, m_pResource, sizeof(T));
	}

	const T& operator()() const
	{
		return *m_pResource;
	}

	T& operator()()
	{
		return *m_pResource;
	}
};

//to the reader: the offsets wrapper is remants from an old css/csgo hybrid from which this current cheat is based
class CWeapon
{
public:
	class IO : public CBaseNonInstantiable
	{
	public:
		static constexpr DWORD_PTR InReload = netvars::m_bInReload;
		static constexpr DWORD_PTR Origin = netvars::m_vecOrigin;
		static constexpr DWORD_PTR CLRRender = netvars::m_clrRender;
		static constexpr DWORD_PTR Clip1 = netvars::m_iClip1;
		static constexpr DWORD_PTR NextPrimaryAttack = netvars::m_flNextPrimaryAttack;
		static constexpr DWORD_PTR WeaponID = netvars::m_iItemDefinitionIndex;
		static constexpr DWORD_PTR OriginalOwnerXuidLow = netvars::m_OriginalOwnerXuidLow;
		static constexpr DWORD_PTR OriginalOwnerXuidHigh = netvars::m_OriginalOwnerXuidHigh;
		static constexpr DWORD_PTR FallbackPaintKit = netvars::m_nFallbackPaintKit;
		static constexpr DWORD_PTR FallbackSeed = netvars::m_nFallbackSeed;
		static constexpr DWORD_PTR FallbackWear = netvars::m_flFallbackWear;
		static constexpr DWORD_PTR FallbackStatTrak = netvars::m_nFallbackStatTrak;
		static constexpr DWORD_PTR AccountID = netvars::m_iAccountID;
		static constexpr DWORD_PTR EntityQuality = netvars::m_iEntityQuality;
		static constexpr DWORD_PTR ItemIDHigh = netvars::m_iItemIDHigh;
		static constexpr DWORD_PTR ItemIDLow = ItemIDHigh + 0x4;
		static constexpr DWORD_PTR CustomName = netvars::m_szCustomName;

	private:
		friend class CWeapon;
		static constexpr DWORD_PTR Max = Clip1;
	};

private:
	static constexpr size_t ReadSize = IO::Max + sizeof(DWORD);

	CGameManagedResource<BaseReadData_t<ReadSize>> m_ReadData;
	DWORD_PTR m_BaseAddress;

public:
	CWeapon(int Handle = 0);
	CWeapon(const CWeapon& rhs);
	~CWeapon();

	void Register(int Handle);

	CWeapon& operator=(const CWeapon& rhs);
	bool operator!=(const CWeapon& rhs);

	DWORD_PTR& Raw();
	DWORD_PTR Raw() const;

	DWORD_PTR& operator()();
	DWORD_PTR operator()() const;

	bool Update();
	bool CanFire() const;

	bool IsRifle() const;
	bool IsGrenade() const;
	bool IsSMG() const;
	bool IsPistol() const;
	bool IsSniper() const;
	bool IsShotgun() const;

	bool IsWeapon() const;

	template <typename T>
	__forceinline const T GV(DWORD_PTR Off) const
	{
		T buffer;
		memcpy(&buffer, &m_ReadData().m_Data[Off], sizeof(T));
		return buffer;
	}

	template <typename T>
	__forceinline bool SV(DWORD_PTR Off, const T& Val)
	{
		return Write(m_BaseAddress + Off, Val);
	}

	template <typename T>
	__forceinline void SLV(DWORD_PTR Off, const T& Val)
	{
		memcpy(&m_ReadData().m_Data[Off], &Val, sizeof(T));
	}
};

class CPlayer
{
public:
	class IO : public CBaseNonInstantiable
	{
	public:
		static constexpr DWORD_PTR Dormant = 0xED;
		static constexpr DWORD_PTR Health = netvars::m_iHealth;
		static constexpr DWORD_PTR Armor = netvars::m_ArmorValue;
		static constexpr DWORD_PTR Team = netvars::m_iTeamNum;
		static constexpr DWORD_PTR Flags = netvars::m_fFlags;
		static constexpr DWORD_PTR Velocity = netvars::m_vecVelocity;
		static constexpr DWORD_PTR Origin = netvars::m_vecOrigin;
		static constexpr DWORD_PTR ShotsFired = netvars::m_iShotsFired;
		static constexpr DWORD_PTR TickBase = netvars::m_nTickBase;
		static constexpr DWORD_PTR BoneMatrix = netvars::m_dwBoneMatrix;
		static constexpr DWORD_PTR LifeState = netvars::m_lifeState;
		static constexpr DWORD_PTR CLRRender = netvars::m_clrRender;
		static constexpr DWORD_PTR hMyWeapons = netvars::m_hMyWeapons;
		static constexpr DWORD_PTR hActiveWeapon = netvars::m_hActiveWeapon;
		static constexpr DWORD_PTR FlashDuration = netvars::m_flFlashDuration;
		static constexpr DWORD_PTR FlashMaxAlpha = netvars::m_flFlashMaxAlpha;
		static constexpr DWORD_PTR ViewOffset = netvars::m_vecViewOffset;
		static constexpr DWORD_PTR Spotted = netvars::m_bSpotted;
		static constexpr DWORD_PTR SpottedByMask = netvars::m_bSpottedByMask;
		static constexpr DWORD_PTR GlowIndex = netvars::m_iGlowIndex;
		static constexpr DWORD_PTR InReload = netvars::m_bInReload;
		static constexpr DWORD_PTR Scoped = netvars::m_bIsScoped;
		static constexpr DWORD_PTR SimulationTime = 0x268;
		static constexpr DWORD_PTR Index = 0x64;
		static constexpr DWORD_PTR ObserverMode = netvars::m_iObserverMode;
		static constexpr DWORD_PTR ViewPunch = netvars::m_viewPunchAngle;
		static constexpr DWORD_PTR FlashAlpha = FlashMaxAlpha - 0x8; 
		static constexpr DWORD_PTR TotalHitsOnServer = 0xA388;
		static constexpr DWORD_PTR IsInBuyZone = 0x394D;
		static constexpr DWORD_PTR IsGrabbyingHostage = 0x3915;
		static constexpr DWORD_PTR IsWalking = 0x390B;
		static constexpr DWORD_PTR NightVisionOn = 0x0A379;
		static constexpr DWORD_PTR HasNightVision = 0x0A37A;
		static constexpr DWORD_PTR EyeAngles = 0xB328;
		static constexpr DWORD_PTR ViewModel = 0x32F8;
		static constexpr DWORD_PTR HealthShotBoostExpirationTime = 0xA3A8;
		static constexpr DWORD_PTR ObserverTarget = netvars::m_hObserverTarget;
		static constexpr DWORD_PTR Ping = 2856;

		class ILE : public CBaseNonInstantiable
		{
		public:
			static constexpr DWORD_PTR CrosshairID = netvars::m_iCrosshairId;
			static constexpr DWORD_PTR AimPunch = netvars::m_aimPunchAngle;
			static constexpr DWORD_PTR ViewPunch = netvars::m_viewPunchAngle;
		};

	private:
		friend class CPlayer;
		static constexpr DWORD_PTR Max = ILE::CrosshairID;
	};

private:
	static constexpr size_t ReadSize = IO::Max + sizeof(DWORD);

	DWORD_PTR m_BaseAddress;

	CGameManagedResource<BaseReadData_t<ReadSize>> m_ReadData;
	CGameManagedResource<PlayerInfo_t> m_Info;
	CGameManagedResource<BaseBoneMatrices_t> m_BoneMatrices;

	CWeapon m_ActiveWeapon;

	static constexpr int COMP_RANK_MIN = 0;
	static constexpr int COMP_RANK_MAX = 18;
	int m_iCompetitiveRanking;

public:
	CPlayer(DWORD_PTR BaseAddress = 0);
	CPlayer(const CPlayer& rhs);
	~CPlayer();

	CPlayer& operator=(const CPlayer& rhs);

	DWORD_PTR Raw() const;
	DWORD_PTR& Raw();

	DWORD_PTR& operator()();
	DWORD_PTR operator()() const;

	int GetIndex() const;

	bool Update();
	bool IsValid() const;

	enum class VunerableContext_t
	{
		NotVunerable,
		Flashed,
		Reloading,
		NoAmmo,
		SniperNoScope
	};

	VunerableContext_t GetVunerable() const;

	bool IsEnemy() const;
	bool IsAlly() const;

	bool IsVisible() const;

	bool operator==(const CPlayer& rhs) const;
	bool operator!=(const CPlayer& rhs) const;
	CVector GetBonePosition(int Bone) const;

	template <typename T>
	__forceinline const T GV(DWORD_PTR Off) const
	{
		T buffer;
		memcpy(&buffer, &m_ReadData().m_Data[Off], sizeof(T));
		return buffer;
	}

	template <typename T>
	__forceinline bool SV(DWORD_PTR Off, const T& Val)
	{
		return Write(m_BaseAddress + Off, Val);
	}

	template <typename T>
	__forceinline void SLV(DWORD_PTR Off, const T& Val)
	{
		memcpy(&m_ReadData().m_Data[Off], &Val, sizeof(T));
	}

	const BaseBoneMatrices_t& GetBoneMatrices() const;
	BaseBoneMatrices_t& GetBoneMatrices();

	const CWeapon& GetActiveWeapon() const;
	CWeapon& GetActiveWeapon();

	const PlayerInfo_t& GetPlayerInfo() const;
	PlayerInfo_t& GetPlayerInfo();

	const int GetCompetitiveRank() const;
	const int GetPing() const;

	CVector GetSource() const;
};

extern CPlayer g_Local;