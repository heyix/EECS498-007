#include "CppComponent.h"


bool CppComponent::Get_Enabled()const 
{
	return enabled;
}

void CppComponent::Set_Enabled(bool new_enable)
{
	enabled = new_enable;
}
