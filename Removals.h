#pragma once
#include "Classes.h"

class CRemovals : public CBaseThreadHack
{
public:
	void EntryPoint() override;
};

extern CRemovals g_Removals;