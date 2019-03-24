#pragma once
#include "Classes.h"
#include "Math.h"
#include "Engine.h"

struct Record_t
{
public:
	Record_t();
	Record_t(const CPlayer* P, float sim);

	bool operator==(const Record_t& rhs) const;
	bool IsValid() const;

	CVector GetBonePosition(int Bone) const;

	CGameManagedResource<BaseBoneMatrices_t> m_BoneMatrices;
	const CPlayer* m_pPlayer;
	float m_flSimulationTime;
};

class CBacktrack : public CBaseHack
{
public:
	void EntryPoint() override;

	const std::vector<Record_t>& GetRecords(int Index) const;
	const std::vector<Record_t>& GetFutureRecords(int Index) const;

	bool GetMouseOneDown() const;

	static bool RecordValid(const Record_t& Record);

private:
	std::vector<Record_t> m_Records[g_PlayerList.MaxPlayerCount];
	std::vector<Record_t> m_FutureRecords[g_PlayerList.MaxPlayerCount];

	const Record_t* m_pBestRecord;

	CVector2D m_Screen;
	CVector m_Source;

	bool m_bMouseOneDown, m_bForceNoInput;
	int m_TargetIndex;

	bool Move();

	bool Update();
	bool PassesStartConditions() const;
	bool GetTarget();
	bool GetBestRecord();
};

extern CBacktrack g_Backtrack;