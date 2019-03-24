#pragma once
#include "Classes.h"

class CThirdPerson : public CBaseHack
{
private:
	bool PassesStartConditions();

public:
	void EntryPoint() override;
};

extern CThirdPerson g_ThirdPerson;