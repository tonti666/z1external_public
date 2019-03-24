#include "ConVar.h"

CConVar::CConVar(DWORD_PTR dwBase) : 
	m_dwBaseAddress(dwBase)
{
	this->Update();
}

CConVar::CConVar(const char * szName)
{
	//todo findvar
}

const ConVarData_t & CConVar::Raw() const
{
	return m_Data();
}

const ConVarData_t & CConVar::operator()() const
{
	return this->Raw();
}

float CConVar::GetFloat() const
{
	return m_Data().m_fValue;
}

int CConVar::GetInt() const
{
	return m_Data().m_nValue;
}

const char * CConVar::GetName() const
{
	return m_Data().m_pszString;
}

void CConVar::SetFloat(float flValue)
{
	if (DecimalEqual(flValue, m_Data().m_fValue))
	{
		return;
	}

	const DWORD dwRaw = m_dwBaseAddress ^ *(DWORD*)&flValue;
	Write(m_dwBaseAddress + 0x2C, dwRaw);
	m_Data().m_fValue = flValue;
}

void CConVar::SetInt(int iValue)
{
	if (iValue == m_Data().m_nValue)
	{
		return;
	}

	const DWORD dwRaw = m_dwBaseAddress ^ *(DWORD*)&iValue;
	Write(m_dwBaseAddress + 0x30, dwRaw);
	m_Data().m_nValue = iValue;
}

bool CConVar::Update()
{
	if (!m_dwBaseAddress)
	{
		return false;
	}

	return m_Data.Update(m_dwBaseAddress);
}
