#pragma once
#include "Actor.h"
class Game1;
class Player:public Actor
{
public:
	virtual void update_position()override;
};

