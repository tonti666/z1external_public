#pragma once
#include "Menu.h"

constexpr int NEVER_VISIBLE_SHOTS = 30;
constexpr int ALWAYS_VISIBLE_SHOTS = 0;

constexpr int NEVER_SILENT_SHOTS = 0;
constexpr int ALWAYS_SILENT_SHOTS = 30;

constexpr int NEVER_SPRAYING_SHOTS = 30;
constexpr int ALWAYS_SPRAYING_SHOTS = 0;

constexpr int ALWAYS_RCS_SHOTS = 30;
constexpr int NEVER_DISABLE_CURVE = 30;

constexpr int AIM_MAX_SHOTS_DISABLED = 30;
constexpr int AIM_MIN_SHOTS_DISABLED = 0;

enum ESmoothModes
{
	SMOOTH_LOG,
	SMOOTH_EXP,
	SMOOTH_LERP
};

enum ECurveModes
{
	CURVE_DISABLED,
	CURVE_TRIGONMETRIC,
	CURVE_AIMTIME_PROPORTIONAL,
	CURVE_AIMTIME_INVERSE
};

enum EAimMoveModes
{
	AIM_MODE_SET,
	AIM_MODE_PULL
};

enum EAimDisableWhileX
{
	AIM_DISABLE_NOT_VISIBLE,
	AIM_DISABLE_IN_CROSS_ALWAYS,
	AIM_DISABLE_IN_CROSS_INITIAL,
	AIM_DISABLE_STATIONARY,
	AIM_DISABLE_WALKING,
	AIM_DISABLE_RUNNING,
	AIM_DISABLE_JUMPING
};

enum EForwardTrackWhenX
{
	FORWARD_TRACK_NEVER,
	FORWARD_TRACK_ALWAYS,
	FORWARD_TRACK_WHEN_NO_AIM,
	FORWARD_TRACK_WHEN_NO_SILENT
};

enum EMagnetDisableWhileX
{
	MAGNET_DISABLE_NOT_VISIBLE,
	MAGNET_DISABLE_STATIONARY,
	MAGNET_DISABLE_WALKING,
	MAGNET_DISABLE_RUNNING,
	MAGNET_DISABLE_JUMPING
};

enum ETriggerDisableWhileX
{
	TRIGGER_DISABLE_NOT_VISIBLE,
	TRIGGER_DISABLE_STATIONARY,
	TRIGGER_DISABLE_WALKING,
	TRIGGER_DISABLE_RUNNING,
	TRIGGER_DISABLE_JUMPING
};

enum ESlowAimDisableWhileX
{
	SLOWAIM_DISABLE_STATIONARY,
	SLOWAIM_DISABLE_WALKING,
	SLOWAIM_DISABLE_RUNNING,
	SLOWAIM_DISABLE_JUMPING
};

enum EESPTargets
{
	ESP_TARGET_ALLY,
	ESP_TARGET_ENEMY,
	ESP_TARGET_LOCAL
};

enum EESPDisableWhileX
{
	ESP_DISABLE_WALKING,
	ESP_DISABLE_STATIONARY,
	ESP_DISABLE_NOT_VISIBLE,
	ESP_DISABLE_VISIBLE,
	ESP_DISABLE_NOT_VISIBLE_BY_TEAM,
	ESP_DISABLE_LOCAL_ALIVE
};

enum EESPOptions
{
	ESP_OPTION_SNAP_LINES,
	ESP_OPTION_BOXES,
	ESP_OPTION_HEALTH_BARS,
	ESP_OPTION_IDEAL_CROSSHAIR,
	ESP_OPTION_INFO,
	ESP_OPTION_AIM_POINTS,
	ESP_OPTION_DAMAGE_INDICATORS,
	ESP_OPTION_CHAMS,
	ESP_OPTION_GLOW,
	ESP_OPTION_RADAR,
	ESP_OPTION_JUMP_RINGS,
	ESP_OPTION_AIM_RINGS,
	ESP_OPTION_TRAILS,
	ESP_OPTION_PREDICT,
	ESP_OPTION_SKELETONS,
	ESP_OPTION_BACKTRACK_SKELETONS
};

enum EESPInfoOptions
{
	ESP_INFO_NAME,
	ESP_INFO_WEAPON,
	ESP_INFO_HEALTH,
	ESP_INFO_ARMOR,
	ESP_INFO_DEFUSING,
	ESP_INFO_DEFUSER,
	ESP_INFO_RANK
};

enum ESkeletonDrawOptions
{
	BACKTRACK_DRAW_ALL,
	BACKTRACK_DRAW_BEST
};

enum ESpectatorListMode
{
	SPEC_LIST_MODE_ALL,
	SPEC_LIST_MODE_LOCAL
};

enum EMenuThemes
{
	THEME_DEFAULT_OLD,
	THEME_DEFAULT_NEW,
	THEME_CUSTOM_OLD,
	THEME_CUSTOM_NEW
};

enum EBunnyModes
{
	BHOP_DISABLED,
	BHOP_LEGIT,
	BHOP_RAGE
};

enum ELagModes
{
	FAKELAG_DISABLED,
	FAKELAG_STANDARD,
	FAKELAG_ADAPTIVE,
	FAKELAG_RANDOM
};

enum ELagActivateWhileX
{
	FAKELAG_WHILE_RUNNING,
	FAKELAG_WHILE_STATIONARY,
	FAKELAG_WHILE_INVISIBLE,
	FAKELAG_WHILE_VISIBLE,
	FAKELAG_WHILE_JUMPING,
	FAKELAG_WHILE_INTERACTING
};

class IControls : public CBaseUtility
{
public:
	void RegisterControls();
	CWindow* m_pAimbotWindow;
	CWindow* m_pTriggerWindow;
	CWindow* m_pRenderWindow;
	CWindow* m_pMiscWindow;

	class IAimbot : public CBaseUtility
	{
	public:
		CTab* m_pConditions;
		CSubSection* m_pConditionsPart1;
		CSubSection* m_pConditionsPart2;

		CTab* m_pMechanics;
		CSubSection* m_pMechanicsPart1;
		CSubSection* m_pMechanicsPart2;

		CTab* m_pMechanics2;
		CSubSection* m_pMechanicsPart3;
		CSubSection* m_pMechanicsPart4;

		CCheckbox* m_pEnabled;
		CKeySelect* m_pKey;
		CSlider<percentage_t>* m_pActivationRate;
		CSlider<float>* m_pFOV;
		CUniSelect* m_pMoveMode;
		CCheckbox* m_pClampFOV;
		CCheckbox* m_pTargetConservation;

		CSlider<int>* m_pVisibleAfterShots;
		CSlider<int>* m_pSilentBeforeShots;
		CSlider<int>* m_pSprayingAfterShots;
		CSlider<int>* m_pMinimumShots;
		CSlider<int>* m_pMaximumShots;
		CSlider<int>* m_pBaimHP;
		CSlider<float>* m_pPullingAwayThreshold;
		CSlider<float>* m_pPullingAwayCoolDownTIcks;
		CSlider<float>* m_pSilentFOV;
		CCheckbox* m_pAlwaysSilent;
		CCheckbox* m_pDelayPostAim; 
		CCheckbox* m_pSilentAutoFire;
		CCheckbox* m_pVisibleAutoFire;
		CSlider<int>* m_pRecoilCompensateShots;
		CSlider<float>* m_pDelayShotTicks;

		CCycle* m_pSmoothMode;
		CSlider<percentage_t>* m_pSprayingSmoothModifier;
		CSlider<float>* m_pStartSmooth;
		CSlider<float>* m_pEndSmooth;
		CSlider<percentage_t>* m_pSmoothDeviation;
		CUniSelect* m_pCurveMode;

		CSlider<percentage_t>* m_pCurve;
		CSlider<percentage_t>* m_pCurveDeviation;
		CSlider<float>* m_pFOVReduction;
		CSlider<int>* m_pCurveShotsDisable;

		CMultiSelectToggleList* m_pDisableWhileX;

		CUniSelect* m_pPriorityBone;
		CSlider<percentage_t>* m_pPriorityValue;
		CSlider<percentage_t>* m_pPrioritySprayingValue;
		CMultiSelect* m_pBoneList;
		CMultiSelect* m_pSprayingBoneList;

		CSlider<float>* m_pRCSX;
		CSlider<percentage_t>* m_pRCSXDeviation;
		CSlider<float>* m_pRCSY;
		CSlider<percentage_t>* m_pRCSYDeviation;

		CSlider<float>* m_pLocalExtrapolationTicks;
		CSlider<float>* m_pEnemyExtrapolaitonTicks;

		CSlider<float>* m_pTimeOutTicks;
		CSlider<float>* m_pVisibleTicks;
		CSlider<float>* m_pVisibleReationTiks;
		CSlider<float>* m_pSilentReationTiks;
		CSlider<float>* m_pReTargetTicks;

		CSlider<float>* m_pRandomSpotX;
		CSlider<float>* m_pRandomSpotY;
		CSlider<float>* m_pDistanceScaling;

		CCheckbox* m_pBackTrack;
		CCheckbox* m_pForwardTrack;
		CUniSelect* m_pForwardTrackActivationConditions;

		CSlider<percentage_t>* m_pBackTrackRate;

		CSlider<int>* m_pBackTrackTicks;
		CSlider<int>* m_pForwardTrackTicks;

		CCheckbox* m_pSilentAimAtBackTrack;
		CCheckbox* m_pVisibleAimAtBackTrack;
		CCheckbox* m_pSilentAimAtForwardTrack;
		CCheckbox* m_pVisibleAimAtForwardTrack;
	};
	IAimbot m_Aimbot;

	class IStandaloneRCS : public CBaseUtility
	{
	public:
		CTab* m_pTab;
		CSubSection* m_pConditions;
		CSubSection* m_pMechanics;

		CCheckbox* m_pEnabled;
		CSlider<int>* m_pShotsMin;
		CSlider<int>* m_pShotsMax;
		CCheckbox* m_pWhileAimbotting;
		CSlider<float>* m_pCoolDown;

		CSlider<float>* m_pRCSX;
		CSlider<percentage_t>* m_pRCSXDeviation;
		CSlider<float>* m_pRCSY;
		CSlider<percentage_t>* m_pRCSYDeviation;

		CSlider<float>* m_pRCSSmooth;
		CSlider<percentage_t>* m_pRCSSmoothDeviation;
	};
	IStandaloneRCS m_StandaloneRCS;

	class IMagnet : public CBaseUtility
	{
	public:
		CTab* m_pTab;

		CSubSection* m_pConditionsPart1;
		CSubSection* m_pMechanicsPart1;

		CCheckbox* m_pEnabled;
		CKeySelect* m_pKey;

		CSlider<float>* m_pFOV;
		CCheckbox* m_pClampFOV;

		CCheckbox* m_pSilentAtBacktrack;
		CCheckbox* m_pVisibleAtBacktrack;

		CCheckbox* m_pSilentAtForwardtrack;
		CCheckbox* m_pVisibleAtForwardtrack;
		CUniSelect* m_pForwardTrackActivationConditions;

		CCheckbox* m_pSilent;
		CCheckbox* m_pVisible;
		CCheckbox* m_pSilentAutoFire;
		CCheckbox* m_pVisibleAutoFire;
		CSlider<int>* m_pRecoilCompensateShots;

		CCycle* m_pSmoothMode;
		CSlider<float>* m_pStartSmooth;
		CSlider<float>* m_pEndSmooth;
		CSlider<percentage_t>* m_pSmoothDeviation;
		CUniSelect* m_pCurveMode;
		CSlider<percentage_t>* m_pCurve;
		CSlider<percentage_t>* m_pCurveDeviation;
		CSlider<float>* m_pFOVReduction;
		CSlider<int>* m_pCurveShotsDisable;
		CSlider<float>* m_pBurstShotsTicks;

		CSlider<float>* m_pLocalExtrapolationTicks;
		CSlider<float>* m_pEnemyExtrapolaitonTicks;

		CMultiSelectToggleList* m_pDisableWhileX;
		CCheckbox* m_pRCS;

		CUniSelect* m_pPriorityBone;
		CSlider<percentage_t>* m_pPriorityValue;
		CMultiSelect* m_pBoneList;

		CSlider<float>* m_pSilentVisibleTicks;
		CSlider<float>* m_pVisibleVisibleTicks;

		CSlider<float>* m_pReTargetTicks;
	};
	IMagnet m_Magnet;

	class ITrigger : public CBaseUtility
	{
	public:
		CTab* m_pTab;
		CSubSection* m_pMain;

		CCheckbox* m_pEnabled;
		CKeySelect* m_pKey;
		CSlider<float>* m_pMaxLocalVelocity;
		CSlider<float>* m_pDelayShotTicks;

		CMultiSelectToggleList* m_pDisableWhileX;
		CSlider<float>* m_pVisibleTicks;
	};
	ITrigger m_Trigger;

	class ISlowAim : public CBaseUtility
	{
	public:
		CSubSection* m_pMain;

		CCheckbox* m_pEnabled;
		CKeySelect* m_pKey;

		CSlider<percentage_t>* m_pMinSensitivity;
		CSlider<int>* m_pMinShots;
		CSlider<int>* m_pMaxShots;

		CUniSelect* m_pBone;
		CSlider<float>* m_pFOV;
		CSlider<float>* m_pFactor;
		CSlider<float>* m_pVisibleRequirement;
		CSlider<float>* m_pFOVRequirement;
		CMultiSelectToggleList* m_pDisableWhileX;
	};
	ISlowAim m_SlowAim;

	class IESP : public CBaseUtility
	{
	public:
		CTab* m_pTab;
		CSubSection* m_pMain;
		CSubSection* m_pOptionsSub;

		CCheckbox* m_pEnabled;
		CKeySelect* m_pKey;

		CSlider<float>* m_pBrightness;
		CSlider<float>* m_pMapBrightness;
		CButton* m_pApplyBrightness;
		CSlider<float>* m_pSpottedESPTicks;

		CMultiSelectToggleList* m_pTargets;
		CMultiSelectToggleList* m_pDisableWhileX;

		CMultiSelectToggleList* m_pOptions;
		CMultiSelectToggleList* m_pInfoOptions;
		CCycle* m_pBackTrackStyle;

		CSubSection* m_pMis;
		CCycle* m_pGlowStyle;
		CCheckbox* m_pGlowBloom;
		CSlider<percentage_t>* m_pGlowAlpha;
		CSlider<percentage_t>* m_pGlowBloomAmount;
		CSlider<int>* m_pPredictTicks;
		CColorSelect* m_pViewModelColour;
		CSlider<float>* m_pViewModelBrightnessMod;
		CCheckbox* m_pAimInfo;
		CCheckbox* m_pThirdPerson;
		CCheckbox* m_pViewModel;
		CCheckbox* m_pRecoil;
		CCheckbox* m_pCrosshair;
		CUniSelect* m_pSpectatorListMode;

		CKeySelect* m_pThirdPersonKey;
		CUniSelect* m_pThirdPersonMode;
	};
	IESP m_ESP;

	class ISkins : public CBaseUtility
	{
	public:
		CTab* m_pTab;
		CSubSection* m_pSub;
		CCheckbox* m_pEnabled;
		CNumericalInput<int>* m_pPistolSkin;
		CSlider<float>* m_pPistolWear;
		CNumericalInput<int>* m_pPistolSeed;
		CNumericalInput<int>* m_pPistolStatrack;

		CNumericalInput<int>* m_pRifleSkin;
		CSlider<float>* m_pRifleWear;
		CNumericalInput<int>* m_pRifleSeed;
		CNumericalInput<int>* m_pRifleStatrack;

		CNumericalInput<int>* m_pSniperSkin;
		CSlider<float>* m_pSniperWear;
		CNumericalInput<int>* m_pSniperSeed;
		CNumericalInput<int>* m_pSniperStatrack;
		CButton* m_pForceUpdate;
	};
	ISkins m_Skins;

	class IShared : public CBaseUtility
	{
	public:
		CTab* m_pTab;
		CSubSection* m_pGameSub;
		CSlider<int>* m_pFPS;
		CCheckbox* m_pNoFlash;
		CSlider<float>* m_pMaxFlashRemoveAlpha;
		CCheckbox* m_pZeusBot;
		CSlider<float>* m_pZeusFOV;
		CCheckbox* m_pZeusBotBacktrack;
		CCheckbox* m_pNoVisualRecoil;
		CCheckbox* m_pHitMarker;
		CCheckbox* m_pJumpHitMarker;
		CSlider<float>* m_pDrawTicks;
		CCheckbox* m_pHitSound;
		CSlider<float>* m_pRunningVelocityDefinition;
		CCheckbox* m_pHud;
		CCheckbox* m_pFFA;

		CSubSection* m_pConfigSub;
		CUniSelect* m_pConfig;
		CButton* m_pSave;
		CButton* m_pLoad;

		CColorSelect* m_pOutlineColor;
		CColorSelect* m_pSelectColor;
		CColorSelect* m_pBackGroundColor;
		CColorSelect* m_pTextColor;
		CCycle* m_pTheme;
	};
	IShared m_Shared;

	class IMovement : public CBaseUtility
	{
	public:
		CTab* m_pGameTab;
		CSubSection* m_pMain;

		CUniSelect* m_pBunnyHop;
		CKeySelect* m_pBunnyHopKey;
		CSlider<percentage_t>* m_pPerfectRate;
		CSlider<float>* m_pAssistanceTicks;
		CSlider<float>* m_pCoolDown;
		CSlider<float>* m_pMinVelocity;
		CCheckbox* m_pAutoStrafe;
	};
	IMovement m_Movement;

	class IFakeLag : public CBaseUtility
	{
	public:
		CSubSection* m_pMain;

		CUniSelect* m_pEnabled;
		CKeySelect* m_pKey;

		CSlider<float>* m_pChoke;
		CSlider<float>* m_pSend;

		CSlider<percentage_t>* m_pChokeDeviation;
		CSlider<percentage_t>* m_pSendDeviation;

		CMultiSelectToggleList* m_pWhileX;
	};
	IFakeLag m_FakeLag;
};

extern IControls g_Controls;
extern const IControls::IAimbot& g_AimCon;
extern const IControls::IStandaloneRCS& g_RCSCon;
extern const IControls::IMagnet& g_MagnetCon;
extern const IControls::ISlowAim& g_SlowAimCon;
extern const IControls::ITrigger& g_TriggerCon;
extern const IControls::IESP& g_ESPCon;
extern const IControls::ISkins& g_SkinsCon;
extern const IControls::IFakeLag& g_FakeLagCon;
extern const IControls::IMovement& g_MovementCon;
extern const IControls::IShared& g_SharedCon;