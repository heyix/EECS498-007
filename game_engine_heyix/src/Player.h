#pragma once
#include "Actor.h"
class Game1;
class Player:public Actor
{
public:
	virtual void update_position()override;
	Player(std::string actor_name, char view, glm::ivec2 position, glm::ivec2 initial_velocity,
		bool blocking, std::string nearby_dialogue, std::string contact_dialogue):Actor(actor_name, view, position, initial_velocity, blocking, nearby_dialogue, contact_dialogue) {}
	Player():Actor(){}
};

