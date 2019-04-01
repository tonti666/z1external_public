#pragma once
#include <fstream>
#include <iostream>
#include <vector>
#include <cassert>
#include <typeinfo>
#include <string>
#include <Windows.h>
#include <cctype>

typedef LONG id_t;

constexpr size_t BUFFER_SIZE = MAX_PATH;

enum class ConfigReason
{
	Save,
	Load
};

class CConfig
{
private:
	struct Var_t
	{
		union Value_t
		{
			Value_t() = delete;
			Value_t(float Float);
			Value_t(int Int);

			float m_Float;
			int m_Int;
		};

		Var_t(const char* szName, const Value_t& Value);

		const char* m_szName;
		Value_t m_Value;
	};

public:
	template <typename T>
	struct UserVar_t
	{
		UserVar_t() :
			m_Value(T(-1)), m_ID(T(-1))
		{
			m_szName = "error";
		}

		UserVar_t(const Var_t & Var, id_t ID) :
			m_ID(ID), m_szName(Var.m_szName)
		{
			if constexpr (std::is_floating_point<T>::value)
			{
				m_Value = Var.m_Value.m_Float;
			}
			else
			{
				m_Value = Var.m_Value.m_Int;
			}
		}

		UserVar_t(const T & Value, const char * szName, id_t ID) :
			m_ID(ID), m_Value(Value), m_szName(szName)
		{
			;
		}

		T m_Value;
		const char* m_szName;
		id_t m_ID;

		bool IsValid() const
		{
			return m_ID != -1;
		}
	};

private:
	std::ifstream m_IFile;
	std::ofstream m_OFile;
	const char* m_FileName;

	int m_IFileMode, m_OFileMode;

	char m_Buffer[BUFFER_SIZE];

	void SetPointer(std::streamoff Offset, int Base);
	void ResetHandles();

	std::streampos GetPointer();
	std::streampos GetFileLength();

	void WriteLine(const char* Out);
	void GetLine(char* Out);

	size_t GetLineCount();

	template <typename T>
	UserVar_t<T> FieldConfirmedUserVar(const char * szName, id_t nID)
	{
		size_t Count = 0;
		while (m_Buffer[Count] != ':')
		{
			Count++;
		}

		std::string str(m_Buffer);
		str.erase(0, Count + 2);
		if constexpr (std::is_floating_point<T>::value)
		{
			return UserVar_t<T>(std::stof(str), szName, nID);
		}
		else
		{
			return UserVar_t<T>(std::stoi(str), szName, nID);
		}
	}

public:
	CConfig() = delete;
	CConfig(const char* szFileName, ConfigReason Reason);
	~CConfig() = default;

	template <typename T>
	void AddVar(const char * szField, const T & Value)
	{
		SetPointer(0, std::ios_base::end);

		if constexpr (std::is_floating_point<T>::value)
		{
			sprintf(m_Buffer, "%s: %f\n", szField, Value);
		}
		else
		{
			sprintf(m_Buffer, "%s: %d\n", szField, Value);
		}

		WriteLine(m_Buffer);
	}

	template<typename T>
	UserVar_t<T> GV(const char * szField)
	{
		const auto LineCount = GetLineCount();
		SetPointer(0, std::ios_base::beg);

		for (size_t i = 0; i < LineCount; i++)
		{
			GetLine(m_Buffer);

			if (strstr(m_Buffer, szField))
			{
				return FieldConfirmedUserVar<T>(szField, i);
			}
		}

		ResetHandles();
		return UserVar_t<T>();
	}
};
