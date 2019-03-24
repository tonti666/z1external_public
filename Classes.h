#pragma once
#include <Windows.h>

#pragma warning(disable : 4244 4018 4838 4305 )

class CBaseNonInstantiable
{
public:
	CBaseNonInstantiable() = delete;

	CBaseNonInstantiable(const CBaseNonInstantiable&) = delete;
	CBaseNonInstantiable& operator=(const CBaseNonInstantiable&) = delete;
};

class CBaseUtility
{
public:
	CBaseUtility() = default;
	virtual ~CBaseUtility() = default;

	CBaseUtility(const CBaseUtility&) = delete;
	CBaseUtility& operator=(const CBaseUtility&) = delete;
};

class CBaseThread : public CBaseUtility
{
public:
	CBaseThread() = default;
	virtual ~CBaseThread() = default;

	virtual void EntryPoint() = 0;

	static DWORD __stdcall Register(LPVOID Obj)
	{
		(reinterpret_cast<CBaseThread*>(Obj))->EntryPoint();
		return 0;
	}
};

class CBaseHack : public CBaseUtility
{
public:
	CBaseHack() = default;
	virtual ~CBaseHack() = default;

	virtual void EntryPoint() = 0;
};

class CBaseThreadHack : virtual public CBaseThread, public CBaseHack
{
public:
	CBaseThreadHack() = default;
	virtual ~CBaseThreadHack() = default;
};

#define RegisterThread(TARGET) CreateThread(nullptr, 0, CBaseThread::Register, (LPVOID)&TARGET, 0, nullptr)

#define TERANY(COND, A, B) ( ( COND ) ? ( A ) : ( B ) )