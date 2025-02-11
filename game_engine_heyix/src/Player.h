#pragma once
#include "Actor.h"

class Player :public Actor {
public:
	virtual void update_position() override;
};