#include "Controls.h"
#include <array>
#include "Offsets.h"
#include "SkinChanger.h"
#include "PlayerList.h"

IControls g_Controls;

const IControls::IAimbot& g_AimCon = g_Controls.m_Aimbot;
const IControls::IStandaloneRCS& g_RCSCon = g_Controls.m_StandaloneRCS;
const IControls::IMagnet& g_MagnetCon = g_Controls.m_Magnet;
const IControls::ISlowAim& g_SlowAimCon = g_Controls.m_SlowAim;
const IControls::ITrigger& g_TriggerCon = g_Controls.m_Trigger;
const IControls::IESP& g_ESPCon = g_Controls.m_ESP;
const IControls::ISkins& g_SkinsCon = g_Controls.m_Skins;
const IControls::IFakeLag& g_FakeLagCon = g_Controls.m_FakeLag;
const IControls::IMovement& g_MovementCon = g_Controls.m_Movement;
const IControls::IShared& g_SharedCon = g_Controls.m_Shared;

#define S_DEF_PARAMS SHORT_WIDTH, OBJECT_HEIGHT, OBJECT_GAP
#define L_DEF_PARAMS LONG_WIDTH, OBJECT_HEIGHT, OBJECT_GAP

void IControls::RegisterControls()
{
	constexpr int AimbotWindowWidth = 525;
	constexpr int AimbotWindowHeight = (float)AimbotWindowWidth / g_Math.PHI;

	constexpr int TriggersWindowWidth = 380 * g_Math.PHI;
	constexpr int TriggersWindowHeight = 600 / g_Math.PHI;

	constexpr int RenderableWidth = TriggersWindowWidth / 1.25;
	constexpr int RenderableHeight = TriggersWindowHeight * 1.25;

	constexpr int MiscWidth = AimbotWindowWidth;
	constexpr int MiscHeight = AimbotWindowHeight;

	constexpr int StartX = 1;
	constexpr int StartY = 1;

	m_pAimbotWindow = new CWindow({ StartX, StartY, StartX + AimbotWindowWidth, StartY + AimbotWindowHeight }, "aim");
	m_Aimbot.m_pConditions = new CTab("conditions");
	m_Aimbot.m_pConditionsPart1 = new CSubSection("part_1");
	m_Aimbot.m_pConditionsPart2 = new CSubSection("part_2");
	m_Aimbot.m_pConditions->AddSubSection(m_Aimbot.m_pConditionsPart1);
	m_Aimbot.m_pConditions->AddSubSection(m_Aimbot.m_pConditionsPart2);
	m_Aimbot.m_pMechanics = new CTab("mechanics_part_1");
	m_Aimbot.m_pMechanicsPart1 = new CSubSection("part_1");
	m_Aimbot.m_pMechanicsPart2 = new CSubSection("part_2");
	m_Aimbot.m_pMechanics2 = new CTab("mechanics_part_2");
	m_Aimbot.m_pMechanicsPart3 = new CSubSection("part_3");
	m_Aimbot.m_pMechanicsPart4 = new CSubSection("part_4");
	m_Aimbot.m_pMechanics->AddSubSection(m_Aimbot.m_pMechanicsPart1);
	m_Aimbot.m_pMechanics->AddSubSection(m_Aimbot.m_pMechanicsPart2);
	m_Aimbot.m_pMechanics2->AddSubSection(m_Aimbot.m_pMechanicsPart3);
	m_Aimbot.m_pMechanics2->AddSubSection(m_Aimbot.m_pMechanicsPart4);
	m_pAimbotWindow->AddTab(m_Aimbot.m_pConditions);
	m_pAimbotWindow->AddTab(m_Aimbot.m_pMechanics);
	m_pAimbotWindow->AddTab(m_Aimbot.m_pMechanics2);

	m_Aimbot.m_pEnabled = new CCheckbox(S_DEF_PARAMS, "aim_enabled");
	m_Aimbot.m_pKey = new CKeySelect(L_DEF_PARAMS, "aim_key", M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pActivationRate = new CSlider<percentage_t>(L_DEF_PARAMS, "aim_rate", { 1.f, "always" }, { 0.f, "never" }, M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pFOV = new CSlider<float>(L_DEF_PARAMS, "aim_fov", { 39.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pClampFOV = new CCheckbox(S_DEF_PARAMS, "aim_clamp_fov", M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pTargetConservation = new CCheckbox(S_DEF_PARAMS, "aim_target_conservation", M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pMoveMode = new CUniSelect(L_DEF_PARAMS, "aim_move_mode", { "set", "pull" }, M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pPullingAwayThreshold = new CSlider<float>(L_DEF_PARAMS, "aim_pulling_fov", { 0.5f, "" }, { 0.f, "disabled" }, M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pPullingAwayCoolDownTIcks = new CSlider<float>(L_DEF_PARAMS, "aim_pulling_disable_ticks", { 64.f, "" }, { 0.f, "disabled" }, M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pRecoilCompensateShots = new CSlider<int>(L_DEF_PARAMS, "aim_compensate_after_shots", { 5, "" }, { 0, "" }, M_C(&, m_Aimbot.m_pEnabled->Value()));
	//=======
	m_Aimbot.m_pConditionsPart1->AddElement(m_Aimbot.m_pEnabled);
	m_Aimbot.m_pConditionsPart1->AddElement(m_Aimbot.m_pKey);
	m_Aimbot.m_pConditionsPart1->AddElement(m_Aimbot.m_pActivationRate);
	m_Aimbot.m_pConditionsPart1->AddElement(m_Aimbot.m_pFOV);
	m_Aimbot.m_pConditionsPart1->AddElement(m_Aimbot.m_pClampFOV);
	m_Aimbot.m_pConditionsPart1->AddElement(m_Aimbot.m_pTargetConservation);
	m_Aimbot.m_pConditionsPart1->AddElement(m_Aimbot.m_pMoveMode);
	m_Aimbot.m_pConditionsPart1->AddElement(m_Aimbot.m_pPullingAwayThreshold);
	m_Aimbot.m_pConditionsPart1->AddElement(m_Aimbot.m_pPullingAwayCoolDownTIcks);
	m_Aimbot.m_pConditionsPart1->AddElement(m_Aimbot.m_pRecoilCompensateShots);

	m_Aimbot.m_pDisableWhileX = new CMultiSelectToggleList(L_DEF_PARAMS, "aim_disable_conditions", { "not_visible", "in_cross_always", "in_cross_init", "stationary", "walking", "running", "jumping" }, true, M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pTimeOutTicks = new CSlider<float>(L_DEF_PARAMS, "aim_timeout_ticks", { 128.f, "" }, { 0.f, "disabled" }, M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pReTargetTicks = new CSlider<float>(L_DEF_PARAMS, "aim_retarget_ticks", { 128.f, "" }, { 0.f, "disabled" }, M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pVisibleTicks = new CSlider<float>(L_DEF_PARAMS, "aim_visible_ticks", { 64.f, "" }, { 0.f, "disabled" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pDisableWhileX->Value()[AIM_DISABLE_NOT_VISIBLE].m_bSelected));
	m_Aimbot.m_pMinimumShots = new CSlider<int>(L_DEF_PARAMS, "aim_minimum_shots", { 10, "" }, { AIM_MIN_SHOTS_DISABLED, "disabled" }, M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pMaximumShots = new CSlider<int>(L_DEF_PARAMS, "aim_maximum_shots", { AIM_MAX_SHOTS_DISABLED, "disabled" }, { 0, "" }, M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pBaimHP = new CSlider<int>(L_DEF_PARAMS, "aim_baim_hp", { 100, "" }, { 0, "disabled" }, M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pDelayShotTicks = new CSlider<float>(L_DEF_PARAMS, "aim_delay_shot_ticks", { 16.f, "" }, { 0.f, "disabled" }, M_C(&, m_Aimbot.m_pEnabled->Value()));
	//=======
	m_Aimbot.m_pConditionsPart2->AddElement(m_Aimbot.m_pDisableWhileX);
	m_Aimbot.m_pConditionsPart2->AddElement(m_Aimbot.m_pTimeOutTicks);
	m_Aimbot.m_pConditionsPart2->AddElement(m_Aimbot.m_pVisibleTicks);
	m_Aimbot.m_pConditionsPart2->AddElement(m_Aimbot.m_pDelayShotTicks);
	m_Aimbot.m_pConditionsPart2->AddElement(m_Aimbot.m_pReTargetTicks);
	m_Aimbot.m_pConditionsPart2->AddElement(m_Aimbot.m_pMinimumShots);
	m_Aimbot.m_pConditionsPart2->AddElement(m_Aimbot.m_pMaximumShots);
	m_Aimbot.m_pConditionsPart2->AddElement(m_Aimbot.m_pBaimHP);

	m_Aimbot.m_pVisibleAfterShots = new CSlider<int>(L_DEF_PARAMS, "aim_visible_after_shots", { NEVER_VISIBLE_SHOTS, "never" }, { ALWAYS_VISIBLE_SHOTS, "always" }, M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pStartSmooth = new CSlider<float>(L_DEF_PARAMS, "aim_start_smooth", { 50.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pVisibleAfterShots->Value() != NEVER_VISIBLE_SHOTS));
	m_Aimbot.m_pEndSmooth = new CSlider<float>(L_DEF_PARAMS, "aim_end_smooth", { 50.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pVisibleAfterShots->Value() != NEVER_VISIBLE_SHOTS));
	m_Aimbot.m_pSprayingSmoothModifier = new CSlider<percentage_t>(L_DEF_PARAMS, "aim_spraying_smooth_mod", { 1.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pVisibleAfterShots->Value() != NEVER_VISIBLE_SHOTS));
	m_Aimbot.m_pSmoothDeviation = new CSlider<percentage_t>(L_DEF_PARAMS, "aim_smooth_deviation", { 1.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pVisibleAfterShots->Value() != NEVER_VISIBLE_SHOTS));
	m_Aimbot.m_pCurveMode = new CUniSelect(L_DEF_PARAMS, "aim_curve_mode", { "disabled", "trig", "at_prop", "at_inv" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pVisibleAfterShots->Value() != NEVER_VISIBLE_SHOTS));
	m_Aimbot.m_pCurve = new CSlider<percentage_t>(L_DEF_PARAMS, "aim_curve_factor", { 1.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pVisibleAfterShots->Value() != NEVER_VISIBLE_SHOTS && m_Aimbot.m_pCurveMode->Value() != CURVE_DISABLED));
	m_Aimbot.m_pCurveDeviation = new CSlider<percentage_t>(L_DEF_PARAMS, "aim_curve_deviation", { 1.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pVisibleAfterShots->Value() != NEVER_VISIBLE_SHOTS && m_Aimbot.m_pCurveMode->Value() != CURVE_DISABLED));
	m_Aimbot.m_pFOVReduction = new CSlider<float>(L_DEF_PARAMS, "aim_curve_fov_reduction", { 5.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pVisibleAfterShots->Value() != NEVER_VISIBLE_SHOTS && m_Aimbot.m_pCurveMode->Value() != CURVE_DISABLED));
	m_Aimbot.m_pCurveShotsDisable = new CSlider<int>(L_DEF_PARAMS, "aim_curve_disable_shots", { NEVER_DISABLE_CURVE, "never" }, { 0, "" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pVisibleAfterShots->Value() != NEVER_VISIBLE_SHOTS && m_Aimbot.m_pCurveMode->Value() != CURVE_DISABLED));
	m_Aimbot.m_pSilentBeforeShots = new CSlider<int>(L_DEF_PARAMS, "aim_silent_before_shots", { ALWAYS_SILENT_SHOTS, "always" }, { NEVER_SILENT_SHOTS, "never" }, M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pSprayingAfterShots = new CSlider<int>(L_DEF_PARAMS, "aim_spraying_after_shots", { NEVER_SPRAYING_SHOTS, "never" }, { ALWAYS_SPRAYING_SHOTS, "" }, M_C(&, m_Aimbot.m_pEnabled->Value()));
	//=======
	m_Aimbot.m_pMechanicsPart1->AddElement(m_Aimbot.m_pVisibleAfterShots);
	m_Aimbot.m_pMechanicsPart1->AddElement(m_Aimbot.m_pStartSmooth);
	m_Aimbot.m_pMechanicsPart1->AddElement(m_Aimbot.m_pEndSmooth);
	m_Aimbot.m_pMechanicsPart1->AddElement(m_Aimbot.m_pSprayingSmoothModifier);
	m_Aimbot.m_pMechanicsPart1->AddElement(m_Aimbot.m_pSmoothDeviation);
	m_Aimbot.m_pMechanicsPart1->AddElement(m_Aimbot.m_pCurveMode);
	m_Aimbot.m_pMechanicsPart1->AddElement(m_Aimbot.m_pCurve);
	m_Aimbot.m_pMechanicsPart1->AddElement(m_Aimbot.m_pCurveDeviation);
	m_Aimbot.m_pMechanicsPart1->AddElement(m_Aimbot.m_pFOVReduction);
	m_Aimbot.m_pMechanicsPart1->AddElement(m_Aimbot.m_pCurveShotsDisable);
	m_Aimbot.m_pMechanicsPart1->AddElement(m_Aimbot.m_pSilentBeforeShots);
	m_Aimbot.m_pMechanicsPart1->AddElement(m_Aimbot.m_pSprayingAfterShots);

	static std::vector<const char*> szBoneList = { "pelvis", "origin", "top_back", "spine_1", "spine_2", "spine_3", "spine_4", "neck", "head", "back_head", "chest" };

	m_Aimbot.m_pSilentAutoFire = new CCheckbox(S_DEF_PARAMS, "aim_silent_auto_fire", M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pSilentBeforeShots->Value() != NEVER_SILENT_SHOTS));
	m_Aimbot.m_pVisibleAutoFire = new CCheckbox(S_DEF_PARAMS, "aim_visible_auto_fire", M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pVisibleAfterShots->Value() != NEVER_VISIBLE_SHOTS));
	m_Aimbot.m_pSmoothMode = new CCycle(L_DEF_PARAMS, "aim_smooth_mode", { "log", "exp", "lerp" }, M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pPriorityBone = new CUniSelect(L_DEF_PARAMS, "aim_priority_bone", szBoneList, M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pPriorityValue = new CSlider<percentage_t>(L_DEF_PARAMS, "aim_priority_value", { 1.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pPrioritySprayingValue = new CSlider<percentage_t>(L_DEF_PARAMS, "aim_priority_spraying_value", { 1.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pSprayingAfterShots->Value() != NEVER_SPRAYING_SHOTS));
	m_Aimbot.m_pBoneList = new CMultiSelect(L_DEF_PARAMS, "aim_bone_list", szBoneList, M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pSprayingBoneList = new CMultiSelect(L_DEF_PARAMS, "aim_spraying_bone_list", szBoneList, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pSprayingAfterShots->Value() != NEVER_SPRAYING_SHOTS));
	m_Aimbot.m_pLocalExtrapolationTicks = new CSlider<float>(L_DEF_PARAMS, "aim_local_predict_ticks", { 10.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pEnemyExtrapolaitonTicks = new CSlider<float>(L_DEF_PARAMS, "aim_enemy_predict_ticks", { 10.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pRandomSpotX = new CSlider<float>(L_DEF_PARAMS, "aim_random_x", { 10.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pVisibleAfterShots->Value() != NEVER_VISIBLE_SHOTS));
	m_Aimbot.m_pRandomSpotY = new CSlider<float>(L_DEF_PARAMS, "aim_random_y", { 10.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pVisibleAfterShots->Value() != NEVER_VISIBLE_SHOTS));
	m_Aimbot.m_pDistanceScaling = new CSlider<float>(L_DEF_PARAMS, "aim_random_dst_scaling", { 1000.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pVisibleAfterShots->Value() != NEVER_VISIBLE_SHOTS));
	//=======
	m_Aimbot.m_pMechanicsPart1->AddElement(m_Aimbot.m_pSilentAutoFire);
	m_Aimbot.m_pMechanicsPart1->AddElement(m_Aimbot.m_pVisibleAutoFire);
	m_Aimbot.m_pMechanicsPart2->AddElement(m_Aimbot.m_pSmoothMode);
	m_Aimbot.m_pMechanicsPart2->AddElement(m_Aimbot.m_pPriorityBone);
	m_Aimbot.m_pMechanicsPart2->AddElement(m_Aimbot.m_pPriorityValue);
	m_Aimbot.m_pMechanicsPart2->AddElement(m_Aimbot.m_pPrioritySprayingValue);
	m_Aimbot.m_pMechanicsPart2->AddElement(m_Aimbot.m_pBoneList);
	m_Aimbot.m_pMechanicsPart2->AddElement(m_Aimbot.m_pSprayingBoneList);
	m_Aimbot.m_pMechanicsPart2->AddElement(m_Aimbot.m_pLocalExtrapolationTicks);
	m_Aimbot.m_pMechanicsPart2->AddElement(m_Aimbot.m_pEnemyExtrapolaitonTicks);
	m_Aimbot.m_pMechanicsPart2->AddElement(m_Aimbot.m_pRandomSpotX);
	m_Aimbot.m_pMechanicsPart2->AddElement(m_Aimbot.m_pRandomSpotY);
	m_Aimbot.m_pMechanicsPart2->AddElement(m_Aimbot.m_pDistanceScaling);

	m_Aimbot.m_pVisibleReationTiks = new CSlider<float>(L_DEF_PARAMS, "aim_vis_react_ticks", { 32.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pVisibleAfterShots->Value() != NEVER_VISIBLE_SHOTS));
	m_Aimbot.m_pSilentReationTiks = new CSlider<float>(L_DEF_PARAMS, "aim_silent_react_ticks", { 32.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pSilentBeforeShots->Value() != NEVER_SILENT_SHOTS));
	m_Aimbot.m_pRCSX = new CSlider<float>(L_DEF_PARAMS, "aim_rcs_x", { 2.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pVisibleAfterShots->Value() != NEVER_VISIBLE_SHOTS));
	m_Aimbot.m_pRCSXDeviation = new CSlider<percentage_t>(L_DEF_PARAMS, "aim_rcs_x_deviation", { 1.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pVisibleAfterShots->Value() != NEVER_VISIBLE_SHOTS));
	m_Aimbot.m_pRCSY = new CSlider<float>(L_DEF_PARAMS, "aim_rcs_y", { 2.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pVisibleAfterShots->Value() != NEVER_VISIBLE_SHOTS));
	m_Aimbot.m_pRCSYDeviation = new CSlider<percentage_t>(L_DEF_PARAMS, "aim_rcs_y_deviation", { 1.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pVisibleAfterShots->Value() != NEVER_VISIBLE_SHOTS));
	//======
	m_Aimbot.m_pMechanicsPart3->AddElement(m_Aimbot.m_pVisibleReationTiks);
	m_Aimbot.m_pMechanicsPart3->AddElement(m_Aimbot.m_pSilentReationTiks);
	m_Aimbot.m_pMechanicsPart3->AddElement(m_Aimbot.m_pRCSX);
	m_Aimbot.m_pMechanicsPart3->AddElement(m_Aimbot.m_pRCSXDeviation);
	m_Aimbot.m_pMechanicsPart3->AddElement(m_Aimbot.m_pRCSY);
	m_Aimbot.m_pMechanicsPart3->AddElement(m_Aimbot.m_pRCSYDeviation);

	m_Aimbot.m_pBackTrack = new CCheckbox(S_DEF_PARAMS, "backtrack");
	m_Aimbot.m_pForwardTrack = new CCheckbox(S_DEF_PARAMS, "forwardtrack", M_C(&, m_Aimbot.m_pBackTrack->Value()));
	m_Aimbot.m_pForwardTrackActivationConditions = new CUniSelect(L_DEF_PARAMS, "forwardtrack_active", { "never", "always", "out_of_main", "out_of_silent" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pBackTrack->Value() && m_Aimbot.m_pForwardTrack->Value()));
	m_Aimbot.m_pBackTrackRate = new CSlider<percentage_t>(L_DEF_PARAMS, "backtrack_rate", { 1.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pBackTrack->Value()));
	m_Aimbot.m_pBackTrackTicks = new CSlider<int>(L_DEF_PARAMS, "backtrack_ticks", { 12, "" }, {0, ""}, M_C(&, m_Aimbot.m_pBackTrack->Value()));
	m_Aimbot.m_pForwardTrackTicks = new CSlider<int>(L_DEF_PARAMS, "forwardtrack_ticks", { 12, "" }, { 0, "" }, M_C(&, m_Aimbot.m_pBackTrack->Value() && m_Aimbot.m_pForwardTrack->Value()));
	m_Aimbot.m_pSilentAimAtBackTrack = new CCheckbox(S_DEF_PARAMS, "backtrack_silent_aim", M_C(&, m_Aimbot.m_pBackTrack->Value() && m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pVisibleAimAtBackTrack = new CCheckbox(S_DEF_PARAMS, "backtrack_visible_aim", M_C(&, m_Aimbot.m_pBackTrack->Value() && m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pSilentAimAtForwardTrack = new CCheckbox(S_DEF_PARAMS, "forwardtrack_silent_aim", M_C(&, m_Aimbot.m_pBackTrack->Value() && m_Aimbot.m_pForwardTrack->Value() && m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pVisibleAimAtForwardTrack = new CCheckbox(S_DEF_PARAMS, "forwardtrack_visible_aim", M_C(&, m_Aimbot.m_pBackTrack->Value() && m_Aimbot.m_pForwardTrack->Value() && m_Aimbot.m_pEnabled->Value()));
	m_Aimbot.m_pAlwaysSilent = new CCheckbox(S_DEF_PARAMS, "aim_silent_always", M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pSilentBeforeShots->Value() != NEVER_SILENT_SHOTS));
	m_Aimbot.m_pDelayPostAim = new CCheckbox(S_DEF_PARAMS, "aim_silent_delay_post", M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pSilentBeforeShots->Value() != NEVER_SILENT_SHOTS));
	m_Aimbot.m_pSilentFOV = new CSlider<float>(L_DEF_PARAMS, "aim_silent_fov", { 5.f, "" }, { 0.f, "" }, M_C(&, m_Aimbot.m_pEnabled->Value() && m_Aimbot.m_pSilentBeforeShots->Value() != NEVER_SILENT_SHOTS));
	//=======
	m_Aimbot.m_pMechanicsPart4->AddElement(m_Aimbot.m_pBackTrack);
	m_Aimbot.m_pMechanicsPart4->AddElement(m_Aimbot.m_pForwardTrack);
	m_Aimbot.m_pMechanicsPart4->AddElement(m_Aimbot.m_pForwardTrackActivationConditions);
	m_Aimbot.m_pMechanicsPart4->AddElement(m_Aimbot.m_pBackTrackRate);
	m_Aimbot.m_pMechanicsPart4->AddElement(m_Aimbot.m_pBackTrackTicks);
	m_Aimbot.m_pMechanicsPart4->AddElement(m_Aimbot.m_pForwardTrackTicks);
	m_Aimbot.m_pMechanicsPart4->AddElement(m_Aimbot.m_pSilentAimAtBackTrack);
	m_Aimbot.m_pMechanicsPart4->AddElement(m_Aimbot.m_pVisibleAimAtBackTrack);
	m_Aimbot.m_pMechanicsPart4->AddElement(m_Aimbot.m_pSilentAimAtForwardTrack);
	m_Aimbot.m_pMechanicsPart4->AddElement(m_Aimbot.m_pVisibleAimAtForwardTrack);
	m_Aimbot.m_pMechanicsPart4->AddElement(m_Aimbot.m_pAlwaysSilent);
	m_Aimbot.m_pMechanicsPart4->AddElement(m_Aimbot.m_pDelayPostAim);
	m_Aimbot.m_pMechanicsPart4->AddElement(m_Aimbot.m_pSilentFOV);

	m_StandaloneRCS.m_pTab = new CTab("rcs_standalone");
	m_StandaloneRCS.m_pConditions = new CSubSection("conditions");
	m_StandaloneRCS.m_pMechanics = new CSubSection("mechanics");
	m_StandaloneRCS.m_pTab->AddSubSection(m_StandaloneRCS.m_pConditions);
	m_StandaloneRCS.m_pTab->AddSubSection(m_StandaloneRCS.m_pMechanics);
	m_pAimbotWindow->AddTab(m_StandaloneRCS.m_pTab);

	m_StandaloneRCS.m_pEnabled =  new CCheckbox(S_DEF_PARAMS, "standalone_enabled");
	m_StandaloneRCS.m_pWhileAimbotting = new CCheckbox(S_DEF_PARAMS, "standalone_while_aiming", M_C(&, m_StandaloneRCS.m_pEnabled->Value() && m_Aimbot.m_pEnabled->Value()));
	m_StandaloneRCS.m_pCoolDown = new CSlider<float>(L_DEF_PARAMS, "standalone_aim_wait_ticks", { 128.f, "" }, { 0.f, "" }, M_C(&, m_StandaloneRCS.m_pEnabled->Value() && m_Aimbot.m_pEnabled->Value() && !m_StandaloneRCS.m_pWhileAimbotting->Value()));
	m_StandaloneRCS.m_pShotsMin = new CSlider<int>(L_DEF_PARAMS, "standalone_shots_min", { 10, "" }, { 0, "" }, M_C(&, m_StandaloneRCS.m_pEnabled->Value()));
	m_StandaloneRCS.m_pShotsMax = new CSlider<int>(L_DEF_PARAMS, "standalone_shots_max", { ALWAYS_RCS_SHOTS, "always" }, { 0, "" }, M_C(&, m_StandaloneRCS.m_pEnabled->Value()));
	//=======
	m_StandaloneRCS.m_pConditions->AddElement(m_StandaloneRCS.m_pEnabled);
	m_StandaloneRCS.m_pConditions->AddElement(m_StandaloneRCS.m_pWhileAimbotting);
	m_StandaloneRCS.m_pConditions->AddElement(m_StandaloneRCS.m_pCoolDown);
	m_StandaloneRCS.m_pConditions->AddElement(m_StandaloneRCS.m_pShotsMin);
	m_StandaloneRCS.m_pConditions->AddElement(m_StandaloneRCS.m_pShotsMax);

	m_StandaloneRCS.m_pRCSSmooth = new CSlider<float>(L_DEF_PARAMS, "standalone_smooth", { 50.f, "" }, { 0.f, "" }, M_C(&, m_StandaloneRCS.m_pEnabled->Value()));
	m_StandaloneRCS.m_pRCSSmoothDeviation = new CSlider<percentage_t>(L_DEF_PARAMS, "standalone_smooth_deviation", { 1.f, "" }, { 0.f, "" }, M_C(&, m_StandaloneRCS.m_pEnabled->Value()));
	m_StandaloneRCS.m_pRCSX = new CSlider<float>(L_DEF_PARAMS, "standalone_x", { 2.f, "" }, { 0.f, "" }, M_C(&, m_StandaloneRCS.m_pEnabled->Value()));
	m_StandaloneRCS.m_pRCSXDeviation = new CSlider<percentage_t>(L_DEF_PARAMS, "standalone_x_deviation", { 1.f, "" }, { 0.f, "" }, M_C(&, m_StandaloneRCS.m_pEnabled->Value()));
	m_StandaloneRCS.m_pRCSY = new CSlider<float>(L_DEF_PARAMS, "standalone_y", { 2.f, "" }, { 0.f, "" }, M_C(&, m_StandaloneRCS.m_pEnabled->Value()));
	m_StandaloneRCS.m_pRCSYDeviation = new CSlider<percentage_t>(L_DEF_PARAMS, "standalone_y_deviation", { 1.f, "" }, { 0.f, "" }, M_C(&, m_StandaloneRCS.m_pEnabled->Value()));
	//=======
	m_StandaloneRCS.m_pMechanics->AddElement(m_StandaloneRCS.m_pRCSSmooth);
	m_StandaloneRCS.m_pMechanics->AddElement(m_StandaloneRCS.m_pRCSSmoothDeviation);
	m_StandaloneRCS.m_pMechanics->AddElement(m_StandaloneRCS.m_pRCSX);
	m_StandaloneRCS.m_pMechanics->AddElement(m_StandaloneRCS.m_pRCSXDeviation);
	m_StandaloneRCS.m_pMechanics->AddElement(m_StandaloneRCS.m_pRCSY);
	m_StandaloneRCS.m_pMechanics->AddElement(m_StandaloneRCS.m_pRCSYDeviation);

	g_Menu.AddWindow(m_pAimbotWindow);

	m_pTriggerWindow = new CWindow({ StartX, StartY, StartX + TriggersWindowWidth, StartY + TriggersWindowHeight }, "triggers");
	m_Magnet.m_pTab = new CTab("magnet");
	m_Magnet.m_pConditionsPart1 = new CSubSection("conditions");
	m_Magnet.m_pMechanicsPart1 = new CSubSection("mechanics");
	m_Magnet.m_pTab->AddSubSection(m_Magnet.m_pConditionsPart1);
	m_Magnet.m_pTab->AddSubSection(m_Magnet.m_pMechanicsPart1);
	m_pTriggerWindow->AddTab(m_Magnet.m_pTab);

	m_Magnet.m_pVisible = new CCheckbox(S_DEF_PARAMS, "magnet_visible", M_C(&, m_Magnet.m_pEnabled->Value()));
	m_Magnet.m_pVisibleAtBacktrack = new CCheckbox(S_DEF_PARAMS, "magnet_visible_at_backtrack", M_C(&, m_Magnet.m_pEnabled->Value() && m_Magnet.m_pVisible->Value() && m_Aimbot.m_pBackTrack->Value()));
	m_Magnet.m_pVisibleAtForwardtrack = new CCheckbox(S_DEF_PARAMS, "magnet_visible_at_forwardtrack", M_C(&, m_Magnet.m_pEnabled->Value() && m_Magnet.m_pVisible->Value() && m_Aimbot.m_pBackTrack->Value() && m_Aimbot.m_pForwardTrack->Value()));
	m_Magnet.m_pSilent = new CCheckbox(S_DEF_PARAMS, "magnet_silent", M_C(&, m_Magnet.m_pEnabled->Value()));
	m_Magnet.m_pSilentAtBacktrack = new CCheckbox(S_DEF_PARAMS, "magnet_silent_at_backtrack", M_C(&, m_Magnet.m_pEnabled->Value() && m_Magnet.m_pSilent->Value() && m_Aimbot.m_pBackTrack->Value()));
	m_Magnet.m_pSilentAtForwardtrack = new CCheckbox(S_DEF_PARAMS, "magnet_silent_at_forwardtrack", M_C(&, m_Magnet.m_pEnabled->Value() && m_Magnet.m_pSilent->Value() && m_Aimbot.m_pBackTrack->Value() && m_Aimbot.m_pForwardTrack->Value()));
	m_Magnet.m_pForwardTrackActivationConditions = new CUniSelect(L_DEF_PARAMS, "magnet_forwardtrack_active", { "never", "always", "out_of_fov" }, M_C(&, m_Magnet.m_pEnabled->Value() && m_Aimbot.m_pBackTrack->Value() && m_Aimbot.m_pForwardTrack->Value()));

	m_Magnet.m_pEnabled = new CCheckbox(S_DEF_PARAMS, "magnet_enabled");
	m_Magnet.m_pKey = new CKeySelect(L_DEF_PARAMS, "magnet_key", M_C(&, m_Magnet.m_pEnabled->Value()));
	m_Magnet.m_pFOV = new CSlider<float>(L_DEF_PARAMS, "magnet_fov", { 39.f, "" }, { 0.f, "" }, M_C(&, m_Magnet.m_pEnabled->Value()));
	m_Magnet.m_pClampFOV = new CCheckbox(S_DEF_PARAMS, "magnet_clamp_fov", M_C(&, m_Magnet.m_pEnabled->Value()));
	m_Magnet.m_pDisableWhileX = new CMultiSelectToggleList(L_DEF_PARAMS, "magnet_disable_conditions", { "not_visible", "stationary", "walking", "running", "jumping" }, true, M_C(&, m_Magnet.m_pEnabled->Value()));
	m_Magnet.m_pSilentVisibleTicks = new CSlider<float>(L_DEF_PARAMS, "magnet_silent_visible_ticks", { 64.f, "" }, { 0.f, "disabled" }, M_C(&, m_Magnet.m_pEnabled->Value() && m_Magnet.m_pSilent->Value() && m_Magnet.m_pDisableWhileX->Value()[MAGNET_DISABLE_NOT_VISIBLE].m_bSelected));
	m_Magnet.m_pVisibleVisibleTicks = new CSlider<float>(L_DEF_PARAMS, "magnet_visible_visible_ticks", { 64.f, "" }, { 0.f, "disabled" }, M_C(&, m_Magnet.m_pEnabled->Value() && m_Magnet.m_pVisible->Value() && m_Magnet.m_pDisableWhileX->Value()[MAGNET_DISABLE_NOT_VISIBLE].m_bSelected));
	m_Magnet.m_pReTargetTicks = new CSlider<float>(L_DEF_PARAMS, "magnet_retarget_ticks", { 128.f, "" }, { 0.f, "disabled" }, M_C(&, m_Magnet.m_pEnabled->Value()));
	m_Magnet.m_pBurstShotsTicks = new CSlider<float>(L_DEF_PARAMS, "magnet_burst_ticks", { 32.f, "" }, { 0.f, "" }, M_C(&, m_Magnet.m_pEnabled->Value()));
	m_Magnet.m_pSmoothMode = new CCycle(L_DEF_PARAMS, "magnet_smooth_mode", { "log", "exp", "lerp" }, M_C(&, m_Magnet.m_pEnabled->Value() && m_Magnet.m_pVisible->Value()));
	m_Magnet.m_pCurveMode = new CUniSelect(L_DEF_PARAMS, "magnet_curve_mode", { "disabled", "trig", "at_prop", "at_inv" }, M_C(&, m_Magnet.m_pEnabled->Value() && m_Magnet.m_pVisible->Value()));
	m_Magnet.m_pSilentAutoFire = new CCheckbox(S_DEF_PARAMS, "magnet_silent_auto_fire", M_C(&, m_Magnet.m_pEnabled->Value() && m_Magnet.m_pSilent->Value()));
	m_Magnet.m_pVisibleAutoFire = new CCheckbox(S_DEF_PARAMS, "magnet_visible_auto_fire", M_C(&, m_Magnet.m_pEnabled->Value() && m_Magnet.m_pVisible->Value()));
	//============
	m_Magnet.m_pConditionsPart1->AddElement(m_Magnet.m_pEnabled);
	m_Magnet.m_pConditionsPart1->AddElement(m_Magnet.m_pKey);
	m_Magnet.m_pConditionsPart1->AddElement(m_Magnet.m_pFOV);
	m_Magnet.m_pConditionsPart1->AddElement(m_Magnet.m_pClampFOV);
	m_Magnet.m_pConditionsPart1->AddElement(m_Magnet.m_pDisableWhileX);
	m_Magnet.m_pConditionsPart1->AddElement(m_Magnet.m_pSilentVisibleTicks);
	m_Magnet.m_pConditionsPart1->AddElement(m_Magnet.m_pVisibleVisibleTicks);
	m_Magnet.m_pConditionsPart1->AddElement(m_Magnet.m_pReTargetTicks);
	m_Magnet.m_pConditionsPart1->AddElement(m_Magnet.m_pBurstShotsTicks);
	m_Magnet.m_pConditionsPart1->AddElement(m_Magnet.m_pSmoothMode);
	m_Magnet.m_pConditionsPart1->AddElement(m_Magnet.m_pCurveMode);
	m_Magnet.m_pConditionsPart1->AddElement(m_Magnet.m_pSilentAutoFire);
	m_Magnet.m_pConditionsPart1->AddElement(m_Magnet.m_pVisibleAutoFire);
	m_Magnet.m_pConditionsPart1->AddElement(m_Magnet.m_pForwardTrackActivationConditions);

	m_Magnet.m_pPriorityBone = new CUniSelect(L_DEF_PARAMS, "magnet_priority_bone", szBoneList, M_C(&, m_Magnet.m_pEnabled->Value()));
	m_Magnet.m_pPriorityValue = new CSlider<percentage_t>(L_DEF_PARAMS, "magnet_priority_value", { 1.f, "" }, { 0.f, "" }, M_C(&, m_Magnet.m_pEnabled->Value()));
	m_Magnet.m_pBoneList = new CMultiSelect(L_DEF_PARAMS, "magnet_bone_list", szBoneList, M_C(&, m_Magnet.m_pEnabled->Value()));
	m_Magnet.m_pRCS = new CCheckbox(S_DEF_PARAMS, "magnet_rcs", M_C(&, m_Magnet.m_pEnabled->Value()));
	m_Magnet.m_pRecoilCompensateShots = new CSlider<int>(L_DEF_PARAMS, "magnet_compensate_after_shots", { 5, "" }, { 0, "" }, M_C(&, m_Magnet.m_pEnabled->Value()));
	m_Magnet.m_pLocalExtrapolationTicks = new CSlider<float>(L_DEF_PARAMS, "magnet_local_predict_ticks", { 10.f, "" }, { 0.f, "" }, M_C(&, m_Magnet.m_pEnabled->Value()));
	m_Magnet.m_pEnemyExtrapolaitonTicks = new CSlider<float>(L_DEF_PARAMS, "magnet_enemy_predict_ticks", { 10.f, "" }, { 0.f, "" }, M_C(&, m_Magnet.m_pEnabled->Value()));
	m_Magnet.m_pStartSmooth = new CSlider<float>(L_DEF_PARAMS, "magnet_start_smooth", { 50.f, "" }, { 0.f, "" }, M_C(&, m_Magnet.m_pEnabled->Value() && m_Magnet.m_pVisible->Value()));
	m_Magnet.m_pEndSmooth = new CSlider<float>(L_DEF_PARAMS, "magnet_end_smooth", { 50.f, "" }, { 0.f, "" }, M_C(&, m_Magnet.m_pEnabled->Value() && m_Magnet.m_pVisible->Value()));
	m_Magnet.m_pSmoothDeviation = new CSlider<percentage_t>(L_DEF_PARAMS, "magnet_smooth_deviation", { 1.f, "" }, { 0.f, "" }, M_C(&, m_Magnet.m_pEnabled->Value() && m_Magnet.m_pVisible->Value()));
	m_Magnet.m_pCurve = new CSlider<percentage_t>(L_DEF_PARAMS, "magnet_curve", { 1.f, "" }, { 0.f, "" }, M_C(&, m_Magnet.m_pEnabled->Value() && m_Magnet.m_pVisible->Value() && m_Magnet.m_pCurveMode->Value() != CURVE_DISABLED));
	m_Magnet.m_pCurveDeviation = new CSlider<percentage_t>(L_DEF_PARAMS, "magnet_curve_deviation", { 1.f, "" }, { 0.f, "" }, M_C(&, m_Magnet.m_pEnabled->Value() && m_Magnet.m_pVisible->Value() && m_Magnet.m_pCurveMode->Value() != CURVE_DISABLED));
	m_Magnet.m_pFOVReduction = new CSlider<float>(L_DEF_PARAMS, "magnet_curve_fov_reduction", { 5.f, "" }, { 0.f, "" }, M_C(&, m_Magnet.m_pEnabled->Value() && m_Magnet.m_pVisible->Value() && m_Magnet.m_pCurveMode->Value() != CURVE_DISABLED));
	m_Magnet.m_pCurveShotsDisable = new CSlider<int>(L_DEF_PARAMS, "magnet_curve_disable_shots", { NEVER_DISABLE_CURVE, "never" }, { 0, "" }, M_C(&, m_Magnet.m_pEnabled->Value() && m_Magnet.m_pVisible->Value() && m_Magnet.m_pCurveMode->Value() != CURVE_DISABLED));
	//============
	m_Magnet.m_pMechanicsPart1->AddElement(m_Magnet.m_pVisible);
	m_Magnet.m_pMechanicsPart1->AddElement(m_Magnet.m_pVisibleAtBacktrack);
	m_Magnet.m_pMechanicsPart1->AddElement(m_Magnet.m_pVisibleAtForwardtrack);
	m_Magnet.m_pMechanicsPart1->AddElement(m_Magnet.m_pSilent);
	m_Magnet.m_pMechanicsPart1->AddElement(m_Magnet.m_pSilentAtBacktrack);
	m_Magnet.m_pMechanicsPart1->AddElement(m_Magnet.m_pSilentAtForwardtrack);
	m_Magnet.m_pMechanicsPart1->AddElement(m_Magnet.m_pPriorityBone);
	m_Magnet.m_pMechanicsPart1->AddElement(m_Magnet.m_pPriorityValue);
	m_Magnet.m_pMechanicsPart1->AddElement(m_Magnet.m_pBoneList);
	m_Magnet.m_pMechanicsPart1->AddElement(m_Magnet.m_pRCS);
	m_Magnet.m_pMechanicsPart1->AddElement(m_Magnet.m_pRecoilCompensateShots);
	m_Magnet.m_pMechanicsPart1->AddElement(m_Magnet.m_pLocalExtrapolationTicks);
	m_Magnet.m_pMechanicsPart1->AddElement(m_Magnet.m_pEnemyExtrapolaitonTicks);
	m_Magnet.m_pMechanicsPart1->AddElement(m_Magnet.m_pStartSmooth);
	m_Magnet.m_pMechanicsPart1->AddElement(m_Magnet.m_pEndSmooth);
	m_Magnet.m_pMechanicsPart1->AddElement(m_Magnet.m_pSmoothDeviation);
	m_Magnet.m_pMechanicsPart1->AddElement(m_Magnet.m_pCurve);
	m_Magnet.m_pMechanicsPart1->AddElement(m_Magnet.m_pCurveDeviation);
	m_Magnet.m_pMechanicsPart1->AddElement(m_Magnet.m_pFOVReduction);
	m_Magnet.m_pMechanicsPart1->AddElement(m_Magnet.m_pCurveShotsDisable);


	m_Trigger.m_pTab = new CTab("trigger");
	m_Trigger.m_pMain = new CSubSection("main");
	m_Trigger.m_pTab->AddSubSection(m_Trigger.m_pMain);
	m_pTriggerWindow->AddTab(m_Trigger.m_pTab);

	m_Trigger.m_pEnabled = new CCheckbox(S_DEF_PARAMS, "trigger_enabled");
	m_Trigger.m_pKey = new CKeySelect(L_DEF_PARAMS, "trigger_key", M_C(&, m_Trigger.m_pEnabled->Value()));
	m_Trigger.m_pDisableWhileX = new CMultiSelectToggleList(L_DEF_PARAMS, "trigger_disable_conditions", { "not_visible", "stationary", "walking", "running", "jumping" }, true, M_C(&, m_Trigger.m_pEnabled->Value()));
	m_Trigger.m_pVisibleTicks = new CSlider<float>(L_DEF_PARAMS, "trigger_visible_ticks", { 64.f, "" }, { 0.f, "disabled" }, M_C(&, m_Trigger.m_pEnabled->Value() && m_Trigger.m_pDisableWhileX->Value()[TRIGGER_DISABLE_NOT_VISIBLE].m_bSelected));
	m_Trigger.m_pMaxLocalVelocity = new CSlider<float>(L_DEF_PARAMS, "aim_max_local_velocity", { 255.f, "" }, { 0.f, "disabled" });
	m_Trigger.m_pMaxLocalVelocity->SetValue(0.f);
	m_Trigger.m_pDelayShotTicks = new CSlider<float>(L_DEF_PARAMS, "trigger_delay_shot_ticks", { 16.f, "" }, { 0.f, "disabled" }, M_C(&, m_Trigger.m_pEnabled->Value()));
	//============
	m_Trigger.m_pMain->AddElement(m_Trigger.m_pEnabled);
	m_Trigger.m_pMain->AddElement(m_Trigger.m_pKey);
	m_Trigger.m_pMain->AddElement(m_Trigger.m_pDisableWhileX);
	m_Trigger.m_pMain->AddElement(m_Trigger.m_pVisibleTicks);
	m_Trigger.m_pMain->AddElement(m_Trigger.m_pDelayShotTicks);
	m_Trigger.m_pMain->AddElement(m_Trigger.m_pMaxLocalVelocity);

	m_SlowAim.m_pMain = new CSubSection("aim_assist");
	m_Trigger.m_pTab->AddSubSection(m_SlowAim.m_pMain);
	m_SlowAim.m_pEnabled = new CCheckbox(S_DEF_PARAMS, "assist_enabled");
	m_SlowAim.m_pKey = new CKeySelect(L_DEF_PARAMS, "assist_key", M_C(&, m_SlowAim.m_pEnabled->Value()));
	m_SlowAim.m_pFOV = new CSlider<float>(L_DEF_PARAMS, "assist_fov", { 39.f, "" }, { 0.f, "" }, M_C(&, m_SlowAim.m_pEnabled->Value()));
	m_SlowAim.m_pMinShots = new CSlider<int>(L_DEF_PARAMS, "assist_min_shots", { 10, "" }, { 0, "" }, M_C(&, m_SlowAim.m_pEnabled->Value()));
	m_SlowAim.m_pMaxShots = new CSlider<int>(L_DEF_PARAMS, "assist_max_shots", { ALWAYS_RCS_SHOTS, "always" }, { 0, "" }, M_C(&, m_SlowAim.m_pEnabled->Value()));
	m_SlowAim.m_pBone = new CUniSelect(L_DEF_PARAMS, "assist_bone", szBoneList, M_C(&, m_SlowAim.m_pEnabled->Value()));
	m_SlowAim.m_pDisableWhileX = new CMultiSelectToggleList(L_DEF_PARAMS, "assist_disable_conditions", { "stationary", "walking", "running", "jumping", }, false, M_C(&, m_SlowAim.m_pEnabled->Value()));
	m_SlowAim.m_pVisibleRequirement = new CSlider<float>(L_DEF_PARAMS, "assist_visible_ticks", { 64.f, "" }, { 0.f, "" }, M_C(&, m_SlowAim.m_pEnabled->Value()));
	m_SlowAim.m_pFOVRequirement = new CSlider<float>(L_DEF_PARAMS, "assist_fov_ticks", { 64.f, "" }, { 0.f, "" }, M_C(&, m_SlowAim.m_pEnabled->Value()));
	m_SlowAim.m_pFactor = new CSlider<float>(L_DEF_PARAMS, "assist_factor", { 5.f, "" }, { 0.f, "" }, M_C(&, m_SlowAim.m_pEnabled->Value()));
	m_SlowAim.m_pMinSensitivity = new CSlider<percentage_t>(L_DEF_PARAMS, "assist_min_sensitivity", { 1.f, "" }, { 0.f, "" }, M_C(&, m_SlowAim.m_pEnabled->Value()));
	//============
	m_SlowAim.m_pMain->AddElement(m_SlowAim.m_pEnabled);
	m_SlowAim.m_pMain->AddElement(m_SlowAim.m_pKey);
	m_SlowAim.m_pMain->AddElement(m_SlowAim.m_pFOV);
	m_SlowAim.m_pMain->AddElement(m_SlowAim.m_pMinShots);
	m_SlowAim.m_pMain->AddElement(m_SlowAim.m_pMaxShots);
	m_SlowAim.m_pMain->AddElement(m_SlowAim.m_pBone);
	m_SlowAim.m_pMain->AddElement(m_SlowAim.m_pDisableWhileX);
	m_SlowAim.m_pMain->AddElement(m_SlowAim.m_pVisibleRequirement);
	m_SlowAim.m_pMain->AddElement(m_SlowAim.m_pFOVRequirement);
	m_SlowAim.m_pMain->AddElement(m_SlowAim.m_pFactor);
	m_SlowAim.m_pMain->AddElement(m_SlowAim.m_pMinSensitivity);

	g_Menu.AddWindow(m_pTriggerWindow);

	m_pRenderWindow = new CWindow({ StartX, StartY, StartX + RenderableWidth, StartY + RenderableHeight }, "render");
	m_ESP.m_pTab = new CTab("walls");
	m_Skins.m_pTab = new CTab("misc");
	m_ESP.m_pMain = new CSubSection("misc");
	m_ESP.m_pOptionsSub = new CSubSection("main");
	m_ESP.m_pMis = new CSubSection("main_misc");
	m_Skins.m_pSub = new CSubSection("skins");
	m_ESP.m_pTab->AddSubSection(m_ESP.m_pMain);
	m_ESP.m_pTab->AddSubSection(m_ESP.m_pOptionsSub);
	m_Skins.m_pTab->AddSubSection(m_ESP.m_pMis);
	m_Skins.m_pTab->AddSubSection(m_Skins.m_pSub);
	m_pRenderWindow->AddTab(m_ESP.m_pTab);
	m_pRenderWindow->AddTab(m_Skins.m_pTab);

	m_ESP.m_pEnabled = new CCheckbox(S_DEF_PARAMS, "esp_enabled");
	m_ESP.m_pKey =	new CKeySelect(L_DEF_PARAMS, "esp_key", M_C(&, m_ESP.m_pEnabled->Value()));
	m_ESP.m_pTargets = new CMultiSelectToggleList(L_DEF_PARAMS, "esp_targets", { "allies", "enemies", "local" }, true, M_C(&, m_ESP.m_pEnabled->Value()));
	m_ESP.m_pDisableWhileX = new CMultiSelectToggleList(L_DEF_PARAMS, "esp_disable", { "walking", "stationary", "not_visible_local", "visible_local", "not_visible_team", "local_alive" }, true, M_C(&, m_ESP.m_pEnabled->Value()));
	m_ESP.m_pSpottedESPTicks = new CSlider<float>(L_DEF_PARAMS, "esp_time_after_visible", { 960.f, "" }, { 0.f, "" }, M_C(&, m_ESP.m_pEnabled->Value() && 
		(m_ESP.m_pDisableWhileX->Value()[ESP_DISABLE_NOT_VISIBLE].m_bSelected || m_ESP.m_pDisableWhileX->Value()[ESP_DISABLE_NOT_VISIBLE_BY_TEAM].m_bSelected)));
	m_ESP.m_pBrightness = new CSlider<float>(L_DEF_PARAMS, "esp_brightness", { (float)LONG_WIDTH, "" }, { 0.f, "" }, M_C(&, m_ESP.m_pEnabled->Value()));
	m_ESP.m_pMapBrightness = new CSlider<float>(L_DEF_PARAMS, "esp_map_brightness", { 1.f, "" }, { 0.f, "" }, M_C(&, m_ESP.m_pEnabled->Value()));
	m_ESP.m_pApplyBrightness = new CButton(LONG_WIDTH, OBJECT_HEIGHT, "apply", 
		[&]() -> void 
		{ 	
			{
				DWORD _xor = g_Memory.Engine() + signatures::model_ambient_min - 0x2c;
				float towrite = g_Controls.m_ESP.m_pBrightness->Value();
				auto val = *(DWORD*)(&towrite) ^ _xor;
				Write(g_Memory.Engine() + signatures::model_ambient_min, val);
			}

			/*{
				DWORD _xor = g_Memory.Client() + 0x521DB78 - 0x2c;
				float towrite = g_Controls.m_ESP.m_pMapBrightness->Value();
				auto val = *(DWORD*)(&towrite) ^ _xor;
				Write(g_Memory.Client() + 0x521DB78, val);
			}*/
		}, 
		M_C(&, m_ESP.m_pEnabled->Value()));
	//===============
	m_ESP.m_pMain->AddElement(m_ESP.m_pEnabled);
	m_ESP.m_pMain->AddElement(m_ESP.m_pKey);
	m_ESP.m_pMain->AddElement(m_ESP.m_pTargets);
	m_ESP.m_pMain->AddElement(m_ESP.m_pDisableWhileX);
	m_ESP.m_pMain->AddElement(m_ESP.m_pSpottedESPTicks);
	m_ESP.m_pMain->AddElement(m_ESP.m_pBrightness);
	m_ESP.m_pMain->AddElement(m_ESP.m_pMapBrightness);
	m_ESP.m_pMain->AddElement(m_ESP.m_pApplyBrightness);

	m_ESP.m_pOptions = new CMultiSelectToggleList(L_DEF_PARAMS, "esp_options",
		{ "snap_lines", "boxes", "health_bars", "ideal_cross", "info", "aim_points", "indicators", "chams", "glow", "radar", "jump_rings", "aim_ring", "bt_trails", "prediction", "skels", "bt_skels" }, true, M_C(&, m_ESP.m_pEnabled->Value()));
	m_ESP.m_pOptionsSub->AddElement(m_ESP.m_pOptions);
	
	m_ESP.m_pInfoOptions = new CMultiSelectToggleList(L_DEF_PARAMS, "esp_info_options",
		{ "name", "weapon", "health", "armor", "defusing", "defuser", "rank" }, true, M_C(&, m_ESP.m_pEnabled->Value() && m_ESP.m_pOptions->Value()[ESP_OPTION_INFO].m_bSelected));
	m_ESP.m_pOptionsSub->AddElement(m_ESP.m_pInfoOptions);

	m_ESP.m_pBackTrackStyle = new CCycle(L_DEF_PARAMS, "backtrack_style", { "all", "best" }, M_C(&, m_ESP.m_pEnabled->Value() && (m_ESP.m_pOptions->Value()[ESP_OPTION_SKELETONS].m_bSelected || m_ESP.m_pOptions->Value()[ESP_OPTION_TRAILS].m_bSelected) && m_Aimbot.m_pBackTrack->Value()));
	m_ESP.m_pGlowStyle = new CCycle(L_DEF_PARAMS, "glow_style", { "regular", "pulse", "outline", "pulse_outline" }, M_C(&, m_ESP.m_pEnabled->Value() && m_ESP.m_pOptions->Value()[ESP_OPTION_GLOW].m_bSelected));
	m_ESP.m_pGlowBloom = new CCheckbox(S_DEF_PARAMS, "glow_bloom", M_C(&, m_ESP.m_pEnabled->Value() && m_ESP.m_pOptions->Value()[ESP_OPTION_GLOW].m_bSelected));
	m_ESP.m_pGlowAlpha = new CSlider<percentage_t>(L_DEF_PARAMS, "glow_alpha", { 1.f, "" }, { 0.f, "" }, M_C(&, m_ESP.m_pEnabled->Value() && m_ESP.m_pOptions->Value()[ESP_OPTION_GLOW].m_bSelected));
	m_ESP.m_pGlowBloomAmount = new CSlider<percentage_t>(L_DEF_PARAMS, "glow_bloom_amount", { 1.f, "" }, { 0.f, "" }, M_C(&, m_ESP.m_pEnabled->Value() && m_ESP.m_pOptions->Value()[ESP_OPTION_GLOW].m_bSelected));
	m_ESP.m_pMis->AddElement(m_ESP.m_pBackTrackStyle);
	m_ESP.m_pMis->AddElement(m_ESP.m_pGlowStyle);
	m_ESP.m_pMis->AddElement(m_ESP.m_pGlowBloom);
	m_ESP.m_pMis->AddElement(m_ESP.m_pGlowAlpha);
	m_ESP.m_pMis->AddElement(m_ESP.m_pGlowBloomAmount);

	m_ESP.m_pPredictTicks = new CSlider<int>(L_DEF_PARAMS, "prediction_ticks", { 64, "" }, { 0, "" }, M_C(&, m_ESP.m_pEnabled->Value() && m_ESP.m_pOptions->Value()[ESP_OPTION_PREDICT].m_bSelected));
	m_ESP.m_pMis->AddElement(m_ESP.m_pPredictTicks);

	m_ESP.m_pThirdPerson = new CCheckbox(S_DEF_PARAMS, "third_person_enabled", M_C(&, m_ESP.m_pEnabled->Value()));
	m_ESP.m_pThirdPersonKey = new CKeySelect(L_DEF_PARAMS, "third_person_key", M_C(&, m_ESP.m_pEnabled->Value() && m_ESP.m_pThirdPerson->Value()));
	m_ESP.m_pThirdPersonMode = new CUniSelect(L_DEF_PARAMS, "third_person_mode", { "none", "chase_locked", "chase_free", "roaming", "in_eye", "map_free", "map_chase" }, M_C(&, m_ESP.m_pEnabled->Value() && m_ESP.m_pThirdPerson->Value()));
	m_ESP.m_pMis->AddElement(m_ESP.m_pThirdPerson);
	m_ESP.m_pMis->AddElement(m_ESP.m_pThirdPersonKey);
	m_ESP.m_pMis->AddElement(m_ESP.m_pThirdPersonMode);

	m_ESP.m_pViewModel = new CCheckbox(S_DEF_PARAMS, "viewmodel_enabled", M_C(&, m_ESP.m_pEnabled->Value()));
	m_ESP.m_pMis->AddElement(m_ESP.m_pViewModel);
	m_ESP.m_pViewModelColour = new CColorSelect(L_DEF_PARAMS, "viewmodel_colour", M_C(&, m_ESP.m_pEnabled->Value() && m_ESP.m_pViewModel->Value()));
	m_ESP.m_pViewModelColour->SetValue(CColor(255, 255, 255, 255));
	m_ESP.m_pMis->AddElement(m_ESP.m_pViewModelColour);
	m_ESP.m_pViewModelBrightnessMod = new CSlider<float>(L_DEF_PARAMS, "viewmodel_bright_mod", { (float)LONG_WIDTH, "" }, { 0.f, "" }, M_C(&, m_ESP.m_pEnabled->Value() && m_ESP.m_pViewModel->Value()));
	m_ESP.m_pViewModelBrightnessMod->SetValue(1.f);
	m_ESP.m_pMis->AddElement(m_ESP.m_pViewModelBrightnessMod);
	m_ESP.m_pAimInfo = new CCheckbox(S_DEF_PARAMS, "aim_info_enabled", M_C(&, m_ESP.m_pEnabled->Value() && (m_Aimbot.m_pEnabled->Value() || m_Magnet.m_pEnabled->Value()) && m_Aimbot.m_pBackTrack->Value()));
	m_ESP.m_pMis->AddElement(m_ESP.m_pAimInfo);
	m_ESP.m_pRecoil = new CCheckbox(S_DEF_PARAMS, "rc_cross_enabled", M_C(&, m_ESP.m_pEnabled->Value()));
	m_ESP.m_pMis->AddElement(m_ESP.m_pRecoil);
	m_ESP.m_pCrosshair = new CCheckbox(S_DEF_PARAMS, "reg_cross_enabled", M_C(&, m_ESP.m_pEnabled->Value()));
	m_ESP.m_pMis->AddElement(m_ESP.m_pCrosshair);
	m_ESP.m_pSpectatorListMode = new CUniSelect(L_DEF_PARAMS, "spec_list_mode", { "all", "local" }, M_C(&, g_SpectatorList.IsVisible()));
	m_ESP.m_pMis->AddElement(m_ESP.m_pSpectatorListMode);

	m_Skins.m_pEnabled = new CCheckbox(S_DEF_PARAMS, "skins_enabled");
	m_Skins.m_pPistolSkin = new CNumericalInput<int>(L_DEF_PARAMS, "skins_pistol_paint", { INT_MAX, "" }, { 0, "" }, M_C(&, m_Skins.m_pEnabled->Value()));
	m_Skins.m_pPistolWear = new CSlider<float>(L_DEF_PARAMS, "skins_pistol_wear", { 1.f, "" }, { 0.f, "" }, M_C(&, m_Skins.m_pEnabled->Value()));
	m_Skins.m_pPistolSeed = new CNumericalInput<int>(L_DEF_PARAMS, "skins_pistol_seed", { INT_MAX, "" }, { 0, "" }, M_C(&, m_Skins.m_pEnabled->Value()));
	m_Skins.m_pPistolStatrack = new CNumericalInput<int>(L_DEF_PARAMS, "skins_pistol_stat_track", { INT_MAX, "" }, { 0, "" }, M_C(&, m_Skins.m_pEnabled->Value()));
	m_Skins.m_pSub->AddElement(m_Skins.m_pEnabled);
	m_Skins.m_pSub->AddElement(m_Skins.m_pPistolSkin);
	m_Skins.m_pSub->AddElement(m_Skins.m_pPistolWear);
	m_Skins.m_pSub->AddElement(m_Skins.m_pPistolSeed);
	m_Skins.m_pSub->AddElement(m_Skins.m_pPistolStatrack);

	m_Skins.m_pRifleSkin = new CNumericalInput<int>(L_DEF_PARAMS, "skins_rifle_paint", { INT_MAX, "" }, { 0, "" }, M_C(&, m_Skins.m_pEnabled->Value()));
	m_Skins.m_pRifleWear = new CSlider<float>(L_DEF_PARAMS, "skins_rifle_wear", { 1.f, "" }, { 0.f, "" }, M_C(&, m_Skins.m_pEnabled->Value()));
	m_Skins.m_pRifleSeed = new CNumericalInput<int>(L_DEF_PARAMS, "skins_rifle_seed", { INT_MAX, "" }, { 0, "" }, M_C(&, m_Skins.m_pEnabled->Value()));
	m_Skins.m_pRifleStatrack = new CNumericalInput<int>(L_DEF_PARAMS, "skins_rifle_stat_track", { INT_MAX, "" }, { 0, "" }, M_C(&, m_Skins.m_pEnabled->Value()));
	m_Skins.m_pSub->AddElement(m_Skins.m_pRifleSkin);
	m_Skins.m_pSub->AddElement(m_Skins.m_pRifleWear);
	m_Skins.m_pSub->AddElement(m_Skins.m_pRifleSeed);
	m_Skins.m_pSub->AddElement(m_Skins.m_pRifleStatrack);

	m_Skins.m_pSniperSkin = new CNumericalInput<int>(L_DEF_PARAMS, "skins_sniper_paint", { INT_MAX, "" }, { 0, "" }, M_C(&, m_Skins.m_pEnabled->Value()));
	m_Skins.m_pSniperWear = new CSlider<float>(L_DEF_PARAMS, "skins_sniper_wear", { 1.f, "" }, { 0.f, "" }, M_C(&, m_Skins.m_pEnabled->Value()));
	m_Skins.m_pSniperSeed = new CNumericalInput<int>(L_DEF_PARAMS, "skins_sniper_seed", { INT_MAX, "" }, { 0, "" }, M_C(&, m_Skins.m_pEnabled->Value()));
	m_Skins.m_pSniperStatrack = new CNumericalInput<int>(L_DEF_PARAMS, "skins_sniper_stat_track", { INT_MAX, "" }, { 0, "" }, M_C(&, m_Skins.m_pEnabled->Value()));
	m_Skins.m_pForceUpdate = new CButton(LONG_WIDTH, OBJECT_HEIGHT, "force_update", []() -> void
		{
			g_SkinChanger.Reset();
		},
		M_C(&, m_Skins.m_pEnabled->Value()));
	m_Skins.m_pSub->AddElement(m_Skins.m_pSniperSkin);
	m_Skins.m_pSub->AddElement(m_Skins.m_pSniperWear);
	m_Skins.m_pSub->AddElement(m_Skins.m_pSniperSeed);
	m_Skins.m_pSub->AddElement(m_Skins.m_pSniperStatrack);
	m_Skins.m_pSub->AddElement(m_Skins.m_pForceUpdate);

	g_Menu.AddWindow(m_pRenderWindow);

	m_pMiscWindow = new CWindow({StartX, StartY, StartX + MiscWidth, StartY + MiscHeight}, "misc");
	m_Shared.m_pTab = new CTab("shared");
	m_Shared.m_pGameSub = new CSubSection("game");
	m_Shared.m_pConfigSub = new CSubSection("general");
	m_Shared.m_pTab->AddSubSection(m_Shared.m_pGameSub);
	m_Shared.m_pTab->AddSubSection(m_Shared.m_pConfigSub);
	m_pMiscWindow->AddTab(m_Shared.m_pTab);

	m_Movement.m_pGameTab = new CTab("game");
	m_Movement.m_pMain = new CSubSection("movement");
	m_FakeLag.m_pMain = new CSubSection("fakelag");
	m_Movement.m_pGameTab->AddSubSection(m_Movement.m_pMain);
	m_Movement.m_pGameTab->AddSubSection(m_FakeLag.m_pMain);
	m_pMiscWindow->AddTab(m_Movement.m_pGameTab);
	
	m_Shared.m_pNoFlash = new CCheckbox(S_DEF_PARAMS, "game_no_flash");
	m_Shared.m_pMaxFlashRemoveAlpha = new CSlider<float>(L_DEF_PARAMS, "game_max_clear_alpha", { 255.f, "all" }, { 0.f, "" }, M_C(&, m_Shared.m_pNoFlash->Value()));
	m_Shared.m_pZeusBot = new CCheckbox(S_DEF_PARAMS, "game_zeus_bot");
	m_Shared.m_pZeusFOV = new CSlider<float>(L_DEF_PARAMS, "game_zeus_fov", { 39.f, "" }, { 0.f, "" }, M_C(&, m_Shared.m_pZeusBot->Value()));
	m_Shared.m_pZeusBotBacktrack = new CCheckbox(S_DEF_PARAMS, "game_zeus_bot_backtrack", M_C(&, m_Shared.m_pZeusBot->Value() && m_Aimbot.m_pBackTrack->Value()));
	m_Shared.m_pNoVisualRecoil = new CCheckbox(S_DEF_PARAMS, "game_no_visual_recoil");
	m_Shared.m_pHitMarker = new CCheckbox(S_DEF_PARAMS, "game_hit_marker_visual");
	m_Shared.m_pDrawTicks = new CSlider<float>(L_DEF_PARAMS, "game_hit_marker_visual_ticks", { 64.f, "" }, {0.f, ""}, M_C(&, m_Shared.m_pHitMarker->Value()));
	m_Shared.m_pHitSound = new CCheckbox(S_DEF_PARAMS, "game_hit_marker_audio");
	m_Shared.m_pRunningVelocityDefinition = new CSlider<float>(L_DEF_PARAMS, "game_running_value", { 255.f, "" }, { 0.f, "" });
	m_Shared.m_pHud = new CCheckbox(S_DEF_PARAMS, "game_hud");
	m_Shared.m_pFFA = new CCheckbox(S_DEF_PARAMS, "game_ffa");
	//======
	m_Shared.m_pGameSub->AddElement(m_Shared.m_pNoFlash);
	m_Shared.m_pGameSub->AddElement(m_Shared.m_pMaxFlashRemoveAlpha);
	m_Shared.m_pGameSub->AddElement(m_Shared.m_pZeusBot);
	m_Shared.m_pGameSub->AddElement(m_Shared.m_pZeusFOV);
	m_Shared.m_pGameSub->AddElement(m_Shared.m_pZeusBotBacktrack);
	m_Shared.m_pGameSub->AddElement(m_Shared.m_pNoVisualRecoil);
	m_Shared.m_pGameSub->AddElement(m_Shared.m_pHitMarker);
	m_Shared.m_pGameSub->AddElement(m_Shared.m_pDrawTicks);
	m_Shared.m_pGameSub->AddElement(m_Shared.m_pHitSound);
	m_Shared.m_pGameSub->AddElement(m_Shared.m_pRunningVelocityDefinition);
	//m_Shared.m_pGameSub->AddElement(m_Shared.m_pHud);
	m_Shared.m_pGameSub->AddElement(m_Shared.m_pFFA);

	m_Shared.m_pFPS = new CSlider<int>(L_DEF_PARAMS, "config_overlay_framerate", { 144, "" }, { 1, "" });
	m_Shared.m_pFPS->SetValue(30);

	m_Shared.m_pTheme = new CCycle(L_DEF_PARAMS, "config_theme", { "default_old", "default_new", "custom_old", "custom_new" }, CCondition([&]() -> bool
		{
			if (!g_Menu.Mouse1Pressed() || !g_Menu.CursorInBounds(m_Shared.m_pTheme->GetRenderBounds()))
			{
				return true;
			}

			switch (m_Shared.m_pTheme->Value())
			{
			case THEME_DEFAULT_OLD:
				m_Shared.m_pOutlineColor->SetValue(CColor(47, 49, 54));
				m_Shared.m_pSelectColor->SetValue(CColor(54, 57, 64));
				m_Shared.m_pBackGroundColor->SetValue(CColor(32, 34, 37));
				m_Shared.m_pTextColor->SetValue(CColor(255, 255, 255));
				break;

			case THEME_DEFAULT_NEW:
				m_Shared.m_pOutlineColor->SetValue(CColor(255, CColor(GetSysColor(COLOR_3DDKSHADOW)).g(), CColor(GetSysColor(COLOR_3DDKSHADOW)).b()));
				m_Shared.m_pSelectColor->SetValue(CColor(GetSysColor(COLOR_GRAYTEXT)));
				m_Shared.m_pBackGroundColor->SetValue(CColor(30, 10, 10));
				m_Shared.m_pTextColor->SetValue(CColor(255, 255, 255));
				break;

			case THEME_CUSTOM_OLD:
				m_Shared.m_pOutlineColor->SetValue(CColor(255, 255, 105));
				m_Shared.m_pSelectColor->SetValue(CColor(255, 110, 125));
				m_Shared.m_pBackGroundColor->SetValue(CColor(45, 0, 10));
				m_Shared.m_pTextColor->SetValue(CColor(255, 255, 200));
				break;

			case THEME_CUSTOM_NEW:
				m_Shared.m_pOutlineColor->SetValue(CColor(255, 31, 31));
				m_Shared.m_pSelectColor->SetValue(CColor(255, 89, 0));
				m_Shared.m_pBackGroundColor->SetValue(CColor(12, 10, 10));
				m_Shared.m_pTextColor->SetValue(CColor(255, 255, 255));
				break;
				
			default:
				break;
			}

			return true;
		}));
	m_Shared.m_pTheme->SetValue(THEME_DEFAULT_NEW);
	m_Shared.m_pOutlineColor = new CColorSelect(L_DEF_PARAMS, "config_outline_color");
	auto col_outline = CColor(GetSysColor(COLOR_3DDKSHADOW));
	m_Shared.m_pOutlineColor->SetValue(CColor(255, col_outline.g(), col_outline.b()));
	m_Shared.m_pSelectColor = new CColorSelect(L_DEF_PARAMS, "config_select_color");
	m_Shared.m_pSelectColor->SetValue(CColor(GetSysColor(COLOR_GRAYTEXT)));
	m_Shared.m_pBackGroundColor = new CColorSelect(L_DEF_PARAMS, "config_background_color");
	m_Shared.m_pBackGroundColor->SetValue(CColor(30, 10, 10));
	m_Shared.m_pTextColor = new CColorSelect(L_DEF_PARAMS, "config_text_color");
	m_Shared.m_pTextColor->SetValue(CColor::White);

	std::vector<const char*> givetemplatelambdaslol;
	for (const auto& i : IMenu::GetConfigNames())
	{
		givetemplatelambdaslol.emplace_back(i);
	}

	m_Shared.m_pConfig = new CUniSelect(L_DEF_PARAMS, "config", givetemplatelambdaslol);
	m_Shared.m_pSave = new CButton(LONG_WIDTH, OBJECT_HEIGHT, "config_save", 
		[&]() -> void 
		{ 
			PlaySoundA("hitmarker.wav", NULL, SND_ASYNC); 
			g_Menu.Save((ConfigFile_t)(m_Shared.m_pConfig->Value())); 
		}
	);
	m_Shared.m_pLoad = new CButton(LONG_WIDTH, OBJECT_HEIGHT, "config_load", [&]() -> void 
		{ 
			PlaySoundA("hitmarker.wav", NULL, SND_ASYNC); 
			g_Menu.Load((ConfigFile_t)(m_Shared.m_pConfig->Value())); 
			m_ESP.m_pApplyBrightness->CallBack()();
			m_Skins.m_pForceUpdate->CallBack()();
		}
	);
	//=====
	m_Shared.m_pConfigSub->AddElement(m_Shared.m_pFPS);
	m_Shared.m_pConfigSub->AddElement(m_Shared.m_pTheme);
	m_Shared.m_pConfigSub->AddElement(m_Shared.m_pOutlineColor);
	m_Shared.m_pConfigSub->AddElement(m_Shared.m_pSelectColor);
	m_Shared.m_pConfigSub->AddElement(m_Shared.m_pBackGroundColor);
	m_Shared.m_pConfigSub->AddElement(m_Shared.m_pTextColor);
	m_Shared.m_pConfigSub->AddElement(m_Shared.m_pConfig);
	m_Shared.m_pConfigSub->AddElement(m_Shared.m_pSave);
	m_Shared.m_pConfigSub->AddElement(m_Shared.m_pLoad);

	m_Movement.m_pBunnyHop = new CUniSelect(L_DEF_PARAMS, "bunny_hop", { "disabled", "legit", "rage" });
	m_Movement.m_pBunnyHopKey = new CKeySelect(L_DEF_PARAMS, "bunny_hop_key", M_C(&, m_Movement.m_pBunnyHop->Value() != BHOP_DISABLED));
	m_Movement.m_pPerfectRate = new CSlider<percentage_t>(L_DEF_PARAMS, "bunny_hop_rate", { 1.f, "always" }, { 0.f, "never" }, M_C(&, m_Movement.m_pBunnyHop->Value() != BHOP_DISABLED));
	m_Movement.m_pAssistanceTicks = new CSlider<percentage_t>(L_DEF_PARAMS, "bunny_hop_assist_ticks", { 64.f, "" }, { 0.f, "" }, M_C(&, m_Movement.m_pBunnyHop->Value() == BHOP_LEGIT));
	m_Movement.m_pCoolDown = new CSlider<percentage_t>(L_DEF_PARAMS, "bunny_hop_cooldown_ticks", { 64.f, "" }, { 0.f, "" }, M_C(&, m_Movement.m_pBunnyHop->Value() != BHOP_DISABLED));
	m_Movement.m_pMinVelocity = new CSlider<float>(L_DEF_PARAMS, "bunny_hop_min_velocity", { 255.f, "" }, { 0.f, "" }, M_C(&, m_Movement.m_pBunnyHop->Value() != BHOP_DISABLED));
	m_Movement.m_pAutoStrafe = new CCheckbox(S_DEF_PARAMS, "bunny_hop_auto_strafe", M_C(&, m_Movement.m_pBunnyHop->Value() != BHOP_DISABLED));
	//=====
	m_Movement.m_pMain->AddElement(m_Movement.m_pBunnyHop);
	m_Movement.m_pMain->AddElement(m_Movement.m_pBunnyHopKey);
	m_Movement.m_pMain->AddElement(m_Movement.m_pPerfectRate);
	m_Movement.m_pMain->AddElement(m_Movement.m_pAssistanceTicks);
	m_Movement.m_pMain->AddElement(m_Movement.m_pCoolDown);
	m_Movement.m_pMain->AddElement(m_Movement.m_pMinVelocity);
	m_Movement.m_pMain->AddElement(m_Movement.m_pAutoStrafe);
	//==================================
	m_Shared.m_pJumpHitMarker = new CCheckbox(S_DEF_PARAMS, "game_hit_marker_jump", M_C(&, m_Movement.m_pBunnyHop->Value() == BHOP_LEGIT && m_ESP.m_pEnabled->Value()));
	m_Shared.m_pGameSub->AddElement(m_Shared.m_pJumpHitMarker);

	m_FakeLag.m_pEnabled = new CUniSelect(L_DEF_PARAMS, "fake_lag_mode", { "disabled", "standard", "adaptive", "random" });
	m_FakeLag.m_pKey = new CKeySelect(L_DEF_PARAMS, "fake_lag_key", M_C(&, m_FakeLag.m_pEnabled->Value() != FAKELAG_DISABLED));
	m_FakeLag.m_pChoke = new CSlider<float>(L_DEF_PARAMS, "fake_lag_choke", { 15.f, "" }, { 0.f, "" }, M_C(&, m_FakeLag.m_pEnabled->Value() != FAKELAG_DISABLED));
	m_FakeLag.m_pChokeDeviation = new CSlider<percentage_t>(L_DEF_PARAMS, "fake_lag_choke_deviation", { 1.f, "" }, { 0.f, "" }, M_C(&, m_FakeLag.m_pEnabled->Value() == FAKELAG_RANDOM));
	m_FakeLag.m_pSend = new CSlider<float>(L_DEF_PARAMS, "fake_lag_send", { 15.f, "" }, { 1.f, "" }, M_C(&, m_FakeLag.m_pEnabled->Value() != FAKELAG_DISABLED));
	m_FakeLag.m_pSendDeviation = new CSlider<percentage_t>(L_DEF_PARAMS, "fake_lag_send_deviation", { 1.f, "" }, { 0.f, "" }, M_C(&, m_FakeLag.m_pEnabled->Value() == FAKELAG_RANDOM));
	m_FakeLag.m_pWhileX = new CMultiSelectToggleList(L_DEF_PARAMS, "fake_lag_activation conditions", { "running", "stationary", "invisble", "visible", "jumping", "interacting" }, true, M_C(&, m_FakeLag.m_pEnabled->Value() != FAKELAG_DISABLED));
	//=====
	m_FakeLag.m_pMain->AddElement(m_FakeLag.m_pEnabled);
	m_FakeLag.m_pMain->AddElement(m_FakeLag.m_pKey);
	m_FakeLag.m_pMain->AddElement(m_FakeLag.m_pChoke);
	m_FakeLag.m_pMain->AddElement(m_FakeLag.m_pChokeDeviation);
	m_FakeLag.m_pMain->AddElement(m_FakeLag.m_pSend);
	m_FakeLag.m_pMain->AddElement(m_FakeLag.m_pSendDeviation);
	m_FakeLag.m_pMain->AddElement(m_FakeLag.m_pWhileX);

	g_Menu.AddWindow(m_pMiscWindow);
	g_Menu.AddWindow(&g_PlayerListInfo, false);
	g_SpectatorList.Initialise();
}