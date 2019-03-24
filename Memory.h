#pragma once
#include "Structs.h"
#include "Offsets.h"

#include <TlHelp32.h>
#include <Windows.h>

#define PAD(ALIAS, SIZE_BYTES, OLD_ACCESS) private: unsigned char ALIAS[SIZE_BYTES]; OLD_ACCESS: 

template <typename H>
class CHandle
{
private:
	H m_hThis;
	bool m_IsOwner;

public:
	CHandle() :
		m_hThis(INVALID_HANDLE_VALUE), m_IsOwner(false)
	{
		;
	}

	CHandle(const CHandle<H>& rhs) :
		m_hThis(rhs.m_hThis), m_IsOwner(false)
	{
		;
	}

	CHandle(const H& rhs) : 
		m_hThis(rhs), m_IsOwner(true) 
	{
		;
	}

	~CHandle()
	{
		if (m_hThis && m_IsOwner)
		{
			CloseHandle(m_hThis);
		}
	}

	CHandle<H>& operator=(const H& rhs)
	{
		m_hThis = rhs;
		m_IsOwner = true;
		return *this;
	}

	CHandle<H>& operator=(const CHandle<H>& rhs)
	{
		m_hThis = rhs.m_hThis;
		m_IsOwner = false;
		return *this;
	}

	H& operator()()
	{
		return m_hThis;
	}

	const H& operator()() const
	{
		return m_hThis;
	}

	bool IsValid() const
	{
		return m_hThis != NULL && m_hThis != (H)INVALID_HANDLE_VALUE && m_hThis != (H)ERROR_ACCESS_DENIED;
	}
};

typedef CHandle<HANDLE> WinHandle;

class IMemory : public CBaseUtility
{
private:
	DWORD m_pid;
	WinHandle m_Handle;
	HWND m_Window;

	MODULEENTRY32 m_Engine, m_Client;
	DWORD_PTR m_ClientState;

	class IO : public CBaseNonInstantiable
	{
	public:
		static constexpr DWORD_PTR ClientState = signatures::dwClientState;
	};

public:
	void HookProcess();

	template <typename T>
	bool WriteMemory(size_t Address, const T& Value)
	{
		return WriteProcessMemory(m_Handle(), (LPVOID)Address, &Value, sizeof(T), nullptr) != FALSE;
	}

	bool WriteMemory(size_t Address, LPCVOID Data, size_t Size)
	{
		return WriteProcessMemory(m_Handle(), (LPVOID)Address, Data, Size, nullptr) != FALSE;
	}

	template <typename T>
	bool ReadMemory(size_t Address, T& Out)
	{
		return ReadProcessMemory(m_Handle(), (LPCVOID)Address, &Out, sizeof(T), nullptr) != FALSE;
	}

	bool ReadMemory(DWORD_PTR Address, LPVOID Out, size_t Size)
	{
		return ReadProcessMemory(m_Handle(), (LPCVOID)Address, Out, Size, nullptr) != FALSE;
	}

	template <typename T>
	T ReadMemory(size_t Address)
	{
		T Buffer = T();
		ReadProcessMemory(m_Handle(), (LPCVOID)Address, &Buffer, sizeof(T), nullptr);
		return Buffer;
	}

	const WinHandle& GetHandle() const;
	const HWND& GetWindow() const;

	DWORD_PTR Engine() const;
	const MODULEENTRY32& EngineRaw() const;

	DWORD_PTR Client() const;
	const MODULEENTRY32& ClientRaw() const;

	DWORD_PTR ClientState() const;

private:
	bool FindProcessID();
	bool FindProcessHandle();
	bool FindModules();
};

extern IMemory g_Memory;

#define Read(ADD, TYPE) g_Memory.ReadMemory<TYPE>(ADD)
#define FillBufRead(ADD, BUF) g_Memory.ReadMemory<decltype(BUF)>(ADD, BUF)
#define ManualRead(ADD, BUF, SZ) g_Memory.ReadMemory(ADD, BUF, SZ)

#define Write(ADD, VAL) g_Memory.WriteMemory<decltype(VAL)>(ADD, VAL)
#define ManualWrite(ADD, VAL, SZ) g_Memory.WriteMemory(ADD, VAL, SZ)