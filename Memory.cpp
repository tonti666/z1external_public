#include "Memory.h"

#include "Math.h"

IMemory g_Memory;

void IMemory::HookProcess()
{
	const auto yield = []
	{
		constexpr float time = 1.5f;
		constexpr float quit = time * 10;

		static int count = 0;

		CTimer::SleepThread(time);
		count += time;

		if (count >= quit)
		{
			MessageBox(NULL, "Couldn't find proccess", "z1", MB_OK);
			ExitProcess(1);
		}
	};

	while (!FindProcessID())
	{
		yield();
	}

	while (!FindProcessHandle())
	{
		yield();
	}

	while (!FindModules())
	{
		yield();
	}

	PlaySoundA("hitmarker.wav", NULL, SND_ASYNC);
}

const WinHandle & IMemory::GetHandle() const
{
	return m_Handle;
}

const HWND & IMemory::GetWindow() const
{
	return m_Window;
}

DWORD_PTR IMemory::Engine() const
{
	return (DWORD_PTR)m_Engine.modBaseAddr;
}

const MODULEENTRY32 & IMemory::EngineRaw() const
{
	return m_Engine;
}

DWORD_PTR IMemory::Client() const
{
	return (DWORD_PTR)m_Client.modBaseAddr;
}

const MODULEENTRY32 & IMemory::ClientRaw() const
{
	return m_Client;
}

DWORD_PTR IMemory::ClientState() const
{
	return m_ClientState;
}

bool IMemory::FindProcessID()
{
	m_Window = FindWindowA(0, "Counter-Strike: Global Offensive");

	if (!m_Window)
	{
		return false;
	}

	return GetWindowThreadProcessId(m_Window, &m_pid) != 0;
}

bool IMemory::FindProcessHandle()
{
	m_Handle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, 0, m_pid);
	return m_Handle.IsValid();
}

bool IMemory::FindModules()
{
	WinHandle Snap(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_pid));
	MODULEENTRY32 Mod; Mod.dwSize = sizeof(MODULEENTRY32);

	if (Module32First(Snap(), &Mod))
	{
		do
		{
			if (strcmp("client_panorama.dll", Mod.szModule) == 0)
			{
				m_Client = Mod;
			}

			if (strcmp("engine.dll", Mod.szModule) == 0)
			{
				m_Engine = Mod;
				m_ClientState = Read(Engine() + IO::ClientState, DWORD_PTR);
			}


		} while (Module32Next(Snap(), &Mod));
	}

	return Engine() && Client() && m_ClientState;
}
