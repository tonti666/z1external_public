#pragma once
#include "Classes.h"
#include "Memory.h"
#include "Math.h"

#include <Windows.h>

constexpr int MULTIPLAYER_BACKUP = 150;

class CGlobalVarsBase 
{
public:
	float     m_flRealTime;                     // 0x0000
	int       m_iFrameCount;                   // 0x0004
	float     m_flAbsoluteFrameTime;            // 0x0008
	float     m_flAbsoluteFrameStartTime; // 0x000C
	float     m_flCurTime;                      // 0x0010
	float     m_flFrameTime;                    // 0x0014
	int       m_iMaxClients;                   // 0x0018
	int       m_iTickCount;                    // 0x001C
	float     m_flIntervalPerTick;            // 0x0020
	float     m_flInterpolationAmount;         // 0x0024
	int       m_iSimTicksThisFrame;            // 0x0028
	int       m_iNetworkProtocol;             // 0x002C
	void*     m_pSaveData;                    // 0x0030
	bool      m_bClient;                    // 0x0031
	bool      m_bRemoteClient;              // 0x0032

											// 100 (i.e., tickcount is rounded down to this base and then the "delta" from this base is networked
	int       m_nTimestampNetworkingBase;
	// 32 (entindex() % nTimestampRandomizeWindow ) is subtracted from gpGlobals->tickcount to Set the networking basis, prevents
	//  all of the entities from forcing a new PackedEntity on the same tick (i.e., prevents them from getting lockstepped on this)
	int       m_nTimestampRandomizeWindow;

	template <typename T>
	__forceinline bool SV(DWORD_PTR Off, const T& Val)
	{
		return Write((DWORD_PTR)this + Off, Val);
	}

	class IO : public CBaseNonInstantiable
	{
	public:
		static constexpr DWORD_PTR     m_flRealTime = 0x0000;
		static constexpr DWORD_PTR     m_iFrameCount = 0x0004;
		static constexpr DWORD_PTR     m_flAbsoluteFrameTime = 0x0008;
		static constexpr DWORD_PTR     m_flAbsoluteFrameStartTime = 0x000C;
		static constexpr DWORD_PTR     m_flCurTime = 0x0010;
		static constexpr DWORD_PTR     m_flFrameTime = 0x0014;
		static constexpr DWORD_PTR     m_iMaxClients = 0x0018;
		static constexpr DWORD_PTR     m_iTickCount = 0x001C;
		static constexpr DWORD_PTR     m_flIntervalPerTick = 0x0020;
		static constexpr DWORD_PTR     m_flInterpolationAmount = 0x0024;
		static constexpr DWORD_PTR     m_iSimTicksThisFrame = 0x0028;
		static constexpr DWORD_PTR     m_iNetworkProtocol = 0x002C;
		static constexpr DWORD_PTR     m_pSaveData = 0x0030;
		static constexpr DWORD_PTR     m_bClient = 0x0031;
		static constexpr DWORD_PTR     m_bRemoteClient = 0x0032;
	};
};

extern CGlobalVarsBase g_GlobalVars;

#define PAD_LOCAL(ALIAS, SIZE_BYTES, OLD_ACCESS) private: unsigned char ALIAS[SIZE_BYTES]; OLD_ACCESS: 

struct PlayerInfo_t
{
	PAD_LOCAL(z1, 8, public);
	int m_nXuidLow;
	int m_nXuidHigh;
	char m_szPlayerName[128];
	int m_nUserID;
	char m_szSteamID[33];
	unsigned int m_nSteam3ID;
	char m_szFriendsName[128];
	bool m_bIsFakePlayer;
	bool m_bIsHLTV;
	int m_dwCustomFiles[4];
	char m_FilesDownloaded;
};

struct CUserCMD
{
public:
	PAD_LOCAL(z0, 4, public);
	int					m_iCMDNumber;        // 0x04
	int					m_iTickCount;        // 0x08
	CVector				m_vecViewAngles;     // 0x0C
	CVector				m_vecAimDirection;   // 0x18
	float				m_flForwardMove;     // 0x24
	float				m_flSideMove;        // 0x28
	float				m_flUpMove;          // 0x2C
	int					m_iButtons;          // 0x30
	unsigned char		m_iImpulse;          // 0x34
	PAD_LOCAL(z1, 3, public);
	int					m_iWeaponSelect;     // 0x38
	int					m_iWeaponSubtype;    // 0x3C
	int					m_iRandomSeed;       // 0x40
	short				m_iMouseDx;         // 0x44
	short				m_iMouseDy;         // 0x46
	bool				m_bHasBeenPredicted; // 0x48
	PAD_LOCAL(z2, 27, public);
};

class CVerifiedUserCMD
{
public:
	CUserCMD m_CMD;
	PAD_LOCAL(dick, 0x4, public);
};

static_assert(sizeof(CVerifiedUserCMD) == 0x68, "");
static_assert(sizeof(CUserCMD) == 0x64, "");

struct Input_t
{
public:
	PAD_LOCAL(z0, 0xC, public);
	bool				m_bTrackIRAvailable;
	bool				m_bMouseInitialized;
	bool				m_bMouseActive;
	bool				m_bJoyStickAdvancedInit;
	PAD_LOCAL(z1, 44, public);
	unsigned int		m_nKeys;
	PAD_LOCAL(z2, 100, public);
	bool				m_bCameraInterceptingMouse;
	bool				m_bCameraInThirdPerson;
	bool				m_bCameraMovingWithMouse;
	CVector				m_vecCameraOffset;
	bool				m_bCameraDistanceMove;
	int					m_iCameraOldX;
	int					m_iCameraOldY;
	int					m_iCameraX;
	int					m_iCameraY;
	bool				m_bCameraIsOrthographic;
	CVector				m_vecPreviousViewAngles;
	CVector				m_vecPreviousViewAnglesTilt;
	float				m_flLastForwardMove;
	int					m_iClearInputState;
	PAD_LOCAL(z3, 0x8, public);
	unsigned int		m_nCommands;
	unsigned int		m_nVerifiedCommands;
};

extern Input_t g_GameInput;

struct GlowObject_t
{
public:
	float r, g, b, a;
	PAD_LOCAL(z1, 8, public);
	float BloomAmount;
	PAD_LOCAL(z2, 4, public);
	bool RenderWhenOccluded;
	bool RenderWhenUnoccluded;
	bool FullBloom;
	PAD_LOCAL(z3, 5, public);
	int Style;
};

#undef PAD_LOCAL

enum ObserverModes
{
	OBS_NONE = 0,
	OBS_CHASE_LOCKED,
	OBS_CHASE_FREE,
	OBS_ROAMING,
	OBS_IN_EYE,
	OBS_MAP_FREE,
	OBS_MAP_CHASE
};

enum EWeaponIDs
{
	WEAPON_NONE = 0,
	WEAPON_DEAGLE,
	WEAPON_ELITE,
	WEAPON_FIVESEVEN,
	WEAPON_GLOCK,
	WEAPON_AK47 = 7,
	WEAPON_AUG,
	WEAPON_AWP,
	WEAPON_FAMAS,
	WEAPON_G3SG1,
	WEAPON_GALILAR = 13,
	WEAPON_M249,
	WEAPON_M4A1 = 16,
	WEAPON_MAC10,
	WEAPON_P90 = 19,
	WEAPON_MP5SD = 23,
	WEAPON_UMP45,
	WEAPON_XM1014,
	WEAPON_BIZON,
	WEAPON_MAG7,
	WEAPON_NEGEV,
	WEAPON_SAWEDOFF,
	WEAPON_TEC9,
	WEAPON_TASER,
	WEAPON_HKP2000,
	WEAPON_MP7,
	WEAPON_MP9,
	WEAPON_NOVA,
	WEAPON_P250,
	WEAPON_SCAR20 = 38,
	WEAPON_SG553,
	WEAPON_SSG08,
	WEAPON_KNIFEGG,
	WEAPON_KNIFE,
	WEAPON_FLASHBANG,
	WEAPON_HEGRENADE,
	WEAPON_SMOKEGRENADE,
	WEAPON_MOLOTOV,
	WEAPON_DECOY,
	WEAPON_INCGRENADE,
	WEAPON_C4,
	WEAPON_HEALTHSHOT = 57,
	WEAPON_KNIFE_T = 59,
	WEAPON_M4A1_SILENCER,
	WEAPON_USP_SILENCER,
	WEAPON_CZ75A = 63,
	WEAPON_REVOLVER,
	WEAPON_TAGRENADE = 68,
	WEAPON_FISTS,
	WEAPON_BREACHCHARGE,
	WEAPON_TABLET = 72,
	WEAPON_MELEE = 74,
	WEAPON_AXE,
	WEAPON_HAMMER,
	WEAPON_SPANNER = 78,
	WEAPON_KNIFE_GHOST = 80,
	WEAPON_FIREBOMB,
	WEAPON_DIVERSION,
	WEAPON_FRAG_GRENADE,
	WEAPON_KNIFE_BAYONET = 500,
	WEAPON_KNIFE_FLIP = 505,
	WEAPON_KNIFE_GUT,
	WEAPON_KNIFE_KARAMBIT,
	WEAPON_KNIFE_M9_BAYONET,
	WEAPON_KNIFE_TACTICAL,
	WEAPON_KNIFE_FALCHION = 512,
	WEAPON_KNIFE_SURVIVAL_BOWIE = 514,
	WEAPON_KNIFE_BUTTERFLY,
	WEAPON_KNIFE_PUSH,
	WEAPON_KNIFE_URSUS = 519,
	WEAPON_KNIFE_GYPSY_JACKKNIFE,
	WEAPON_KNIFE_STILETTO = 522,
	WEAPON_KNIFE_WIDOWMAKER
};

enum EBoneList
{
	BONE_PELVIS = 0,
	BONE_ORIGIN = 1,
	BONE_TOP_BACK = 2,
	BONE_SPINE_1 = 3,
	BONE_SPINE_2 = 4,
	BONE_SPINE_3 = 5,
	BONE_SPINE_4 = 6,
	BONE_NECK = 7,
	BONE_HEAD = 8,
	BONE_BACK_HEAD = 9,
	BONE_CHEST = 10,
	BONE_MAX = 86
};

enum ESigOnState
{
	SIG_NONE,
	SIG_CHALLENGE,
	SIG_CONNECTED,
	SIG_NEW,
	SIG_PRESPAWN,
	SIG_SPAWN,
	SIG_FULL,
	SIG_CHANGELEVEL
};

enum class Map_t
{
	Cache,
	Train,
	Overpass,
	Nuke,
	Inferno,
	Dust2,
	Mirage,
	Cobble,
	Office,
	DangerZone,
	MAX
};

constexpr int IN_ATTACK = (1 << 0);
constexpr int IN_JUMP = (1 << 1);
constexpr int IN_DUCK = (1 << 2);
constexpr int IN_FORWARD = (1 << 3);
constexpr int IN_BACK = (1 << 4);
constexpr int IN_USE = (1 << 5);
constexpr int IN_CANCEL = (1 << 6);
constexpr int IN_LEFT = (1 << 7);
constexpr int IN_RIGHT = (1 << 8);
constexpr int IN_MOVELEFT = (1 << 9);
constexpr int IN_MOVERIGHT = (1 << 10);
constexpr int IN_ATTACK2 = (1 << 11);
constexpr int IN_RUN = (1 << 12);
constexpr int IN_RELOAD = (1 << 13);
constexpr int IN_ALT1 = (1 << 14);
constexpr int IN_ALT2 = (1 << 15);
constexpr int IN_SCORE = (1 << 16);   // Used by client.dll for when scoreboard is held down
constexpr int IN_SPEED = (1 << 17); // Player is holding the speed key
constexpr int IN_WALK = (1 << 18); // Player holding walk key
constexpr int IN_ZOOM = (1 << 19); // Zoom key for HUD zoom
constexpr int IN_WEAPON1 = (1 << 20); // weapon defines these bits
constexpr int IN_WEAPON2 = (1 << 21); // weapon defines these bits
constexpr int IN_BULLRUSH = (1 << 22);
constexpr int IN_GRENADE1 = (1 << 23); // grenade 1
constexpr int IN_GRENADE2 = (1 << 24); // grenade 2
constexpr int IN_LOOKSPIN = (1 << 25);

constexpr int	FL_ONGROUND = (1 << 0);	// At rest / on the ground
constexpr int FL_DUCKING = (1 << 1);	// Player flag -- Player is fully crouched
constexpr int FL_ANIMDUCKING = (1 << 2);	// Player flag -- Player is in the process of crouching or uncrouching but could be in transition
// examples:                                   Fully ducked:  FL_DUCKING &  FL_ANIMDUCKING
//           Previously fully ducked, unducking in progress:  FL_DUCKING & !FL_ANIMDUCKING
//                                           Fully unducked: !FL_DUCKING & !FL_ANIMDUCKING
//           Previously fully unducked, ducking in progress: !FL_DUCKING &  FL_ANIMDUCKING
constexpr int	FL_WATERJUMP = (1 << 3);	// player jumping out of water
constexpr int FL_ONTRAIN = (1 << 4); // Player is _controlling_ a train, so movement commands should be ignored on client during prediction.
constexpr int FL_INRAIN = (1 << 5);	// Indicates the entity is standing in rain
constexpr int FL_FROZEN = (1 << 6); // Player is frozen for 3rd person camera
constexpr int FL_ATCONTROLS = (1 << 7); // Player can't move, but keeps key inputs for controlling another entity
constexpr int	FL_CLIENT = (1 << 8);	// Is a player
constexpr int FL_FAKECLIENT = (1 << 9);	// Fake client, simulated server side; don't send network messages to them
// NON-PLAYER SPECIFIC = (i.e., not used by GameMovement or the client .dll ); -- Can still be applied to players, though
constexpr int	FL_INWATER = (1 << 10);	// In water

constexpr int	FL_FLY = (1 << 11);	// Changes the SV_Movestep= (); behavior to not need to be on ground
constexpr int	FL_SWIM = (1 << 12);	// Changes the SV_Movestep= (); behavior to not need to be on ground = (but stay in water);
constexpr int	FL_CONVEYOR = (1 << 13);
constexpr int	FL_NPC = (1 << 14);
constexpr int	FL_GODMODE = (1 << 15);
constexpr int	FL_NOTARGET = (1 << 16);
constexpr int	FL_AIMTARGET = (1 << 17);	// set if the crosshair needs to aim onto the entity
constexpr int	FL_PARTIALGROUND = (1 << 18);	// not all corners are valid
constexpr int FL_STATICPROP = (1 << 19);	// Eetsa static prop!		
constexpr int FL_GRAPHED = (1 << 20); // worldgraph has this ent listed as something that blocks a connection
constexpr int FL_GRENADE = (1 << 21);
constexpr int FL_STEPMOVEMENT = (1 << 22);	// Changes the SV_Movestep= (); behavior to not do any processing
constexpr int FL_DONTTOUCH = (1 << 23);	// Doesn't generate touch functions, generates Untouch= (); for anything it was touching when this flag was set
constexpr int FL_BASEVELOCITY = (1 << 24);	// Base velocity has been applied this frame = (used to convert base velocity into momentum);
constexpr int FL_WORLDBRUSH = (1 << 25);	// Not moveable/removeable brush entity = (really part of the world, but represented as an entity for transparency or something);
constexpr int FL_OBJECT = (1 << 26); // Terrible name. This is an object that NPCs should see. Missiles, for example.
constexpr int FL_KILLME = (1 << 27);	// This entity is marked for death -- will be freed by game DLL
constexpr int FL_ONFIRE = (1 << 28);	// You know...
constexpr int FL_DISSOLVING = (1 << 29); // We're dissolving!
constexpr int FL_TRANSRAGDOLL = (1 << 30); // In the process of turning into a client side ragdoll.
constexpr int FL_UNBLOCKABLE_BY_PLAYER = (1 << 31); // pusher that can't be blocked by the player

enum Team
{
	TEAM_SPECTATOR = 1,
	TEAM_TERROIST = 2,
	TEAM_COUNTER_TERROIST = 3
};

static const std::vector<const char*> szWeaponNames =
{
"none",
"deagle",
"elite",
"fiveseven",
"glock",
"unknown" ,
"unknown",
"ak47",
"aug",
"awp",
"famas",
"g3sg1",
"unknown",
"galilar",
"m249",
"unknown",
"m4a1",
"mac10",
"unknown",
"p90",
"unknown",
"unknown",
"unknown",
"mp5sd",
"ump45",
"xm1014",
"bizon",
"mag7",
"negev",
"sawedoff",
"tec9",
"taser",
"hkp2000",
"mp7",
"mp9",
"nova",
"p250",
"unknown",
"scar20",
"sg553",
"ssg08",
"knifegg",
"knife",
"flashbang",
"hegrenade",
"smokegrenade",
"molotov",
"decoy",
"incgrenade",
"c4",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"unknown",
"healthshot",
"unknown",
"knife_t",
"m4a1_silencer",
"usp_silencer",
"unknown",
"cz75a",
"revolver",
"unknown",
"unknown",
"unknown",
"unknown",
"tagrenade",
"fists",
"breachcharge",
"unknown",
"tablet",
"unknown",
"melee",
"axe",
"hammer",
"unknown",
"spanner",
"unknown",
"knife_ghost",
"firebomb",
"diversion",
"frag_grenade"
};

static const char* szTeams[] = { "ud", "spec", "t", "ct" };

static const char* szCompetitiveRanks[21] =
{
		"ud",
		"blnk",
		"s_i",
		"s_ii",
		"s_iii",
		"s_iv",
		"s_e",
		"s_em",
		"gn_i",
		"gn_ii",
		"gn_iii",
		"gn_m",
		"mg_i",
		"mg_ii",
		"mg_e",
		"dmg",
		"le",
		"lem",
		"smfc",
		"ge",
		"ud"
};

/*
	OBS_NONE = 0,
	OBS_CHASE_LOCKED,
	OBS_CHASE_FREE,
	OBS_ROAMING,
	OBS_IN_EYE,
	OBS_MAP_FREE,
	OBS_MAP_CHASE
*/

static const char* szObserverModes[] = 
{ 
	"undefined",
	"none", 
	"chase_locked", 
	"chase_free", 
	"roaming",
	"in_eye",
	"map_free",
	"map_chase",
	"undefined"
};