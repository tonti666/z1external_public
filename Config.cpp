#include "Config.h"

CConfig::Var_t::Value_t::Value_t(float Float) :
	m_Float(Float)
{
	;
}

CConfig::Var_t::Value_t::Value_t(int Int) :
	m_Int(Int)
{
	;
}

CConfig::Var_t::Var_t(const char * szName, const Value_t & Value) :
	m_Value(Value), m_szName(szName)
{
	;
}

void CConfig::SetPointer(std::streamoff Offset, int Base)
{
	m_IFile.seekg(Offset, Base);
	m_OFile.seekp(Offset, Base);
}

void CConfig::ResetHandles()
{
	//clear eof bit
	m_IFile.close();
	m_IFile.open(m_FileName, m_IFileMode);

	m_OFile.close();
	m_OFile.open(m_FileName, m_OFileMode);
}

std::streampos CConfig::GetPointer()
{
	assert(m_IFile.tellg() == m_OFile.tellp());
	return m_IFile.tellg();
}

std::streampos CConfig::GetFileLength()
{
	SetPointer(0, std::ios_base::end);
	const std::streampos nEnd = GetPointer();
	SetPointer(0, std::ios_base::beg);
	return nEnd - GetPointer();
}

void CConfig::WriteLine(const char * Out)
{
	m_OFile << Out;
	m_OFile.flush();
	m_IFile.seekg(m_OFile.tellp());
}

void CConfig::GetLine(char * Out)
{
	m_IFile.getline(Out, BUFFER_SIZE);
	m_OFile.seekp(m_IFile.tellg());
}

size_t CConfig::GetLineCount()
{
	SetPointer(0, std::ios_base::beg);
	size_t count = 0;
	while (!m_IFile.eof())
	{
		GetLine(m_Buffer);	
		count++;		
	}

	ResetHandles();
	return count;
}

CConfig::CConfig(const char * szFileName, ConfigReason Reason) :
	m_IFile(szFileName, Reason == ConfigReason::Save ? std::ios_base::in : std::ios_base::app),
	m_OFile(szFileName, Reason == ConfigReason::Save ? std::ios_base::out : std::ios_base::app),
	m_IFileMode(Reason == ConfigReason::Save ? std::ios_base::in : std::ios_base::app),
	m_OFileMode(Reason == ConfigReason::Save ? std::ios_base::out : std::ios_base::app),
	m_FileName(szFileName)
{
	memset(m_Buffer, 0, BUFFER_SIZE);
	ResetHandles();
}