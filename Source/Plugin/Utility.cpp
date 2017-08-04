#include "Utility.h"
#include "../API/RainmeterAPI.h"


Utility& Utility::GetUtility()
{
	static Utility instance;
	return instance;
}

Utility::Utility() : m_Rm(nullptr)
{
}

void Utility::SetRm(void* rm)
{
	m_Rm = rm;
}

Utility::~Utility()
{
}
