#pragma once
#include "Classes.h"

class CLaunchMisc : public CBaseThread
{
public:
	void EntryPoint() override;
};

extern CLaunchMisc g_MiscHacks;