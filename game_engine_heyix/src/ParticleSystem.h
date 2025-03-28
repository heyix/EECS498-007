#pragma once
#include "CppComponent.h"
class ParticleSystem :public CppComponent {
public:
	ParticleSystem(GameObject& holder, const std::string& key, const std::string& template_name);

};