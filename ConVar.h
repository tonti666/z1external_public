#pragma once
#include "Entity.h"
#include "Structs.h"

struct ConCommandData_t
{
	PAD(vtable, 0x4, public);
	ConCommandData_t* m_pNext;
	bool m_bRegistered;
	const char* m_pszRegistered;
	const char* m_pszHelp;
	int m_nFlags;
	PAD(whocareslol, 0x8, public);
};

struct ConVarData_t
{
public:
	ConCommandData_t m_BaseData;
	ConVarData_t				*m_pParent;
	// Static data
	const char					*m_pszDefaultValue;
	// Value
	// Dynamically allocated
	char						*m_pszString;
	int							m_StringLength;
	// Values
	float						m_fValue; 
	int							m_nValue;
	// Min/Max values
	bool						m_bHasMin;
	float						m_fMinVal;
	bool						m_bHasMax;
	float						m_fMaxVal;

	// Call this function when ConVar changes
	void*			m_fnChangeCallback;
};

class CConVar
{
private:
	DWORD_PTR m_dwBaseAddress;
	CGameManagedResource<ConVarData_t> m_Data;

public:
	CConVar(DWORD_PTR dwBase);
	CConVar(const char* szName);
	~CConVar() = default;

	const ConVarData_t& Raw() const;
	const ConVarData_t& operator()() const;

	float GetFloat() const;
	int GetInt() const;
	const char* GetName() const;

	void SetFloat(float flValue);
	void SetInt(int iValue);

	bool Update();
};