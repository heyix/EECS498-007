#pragma once
#include <string>
#include "glm/glm.hpp"
class Game1;
class Actor
{
public:
	int ID=0;
	std::string actor_name="";
	char view='?';
	glm::ivec2 position{0,0};
	glm::ivec2 velocity{0,0};
	bool blocking=false;
	std::string nearby_dialogue="";
	std::string contact_dialogue="";
	bool triggered_score_up = false;
	Actor(std::string actor_name, char view, glm::ivec2 position, glm::ivec2 initial_velocity,
		bool blocking, std::string nearby_dialogue, std::string contact_dialogue)
		: actor_name(actor_name), view(view), position(position), velocity(initial_velocity), blocking(blocking), nearby_dialogue(nearby_dialogue), contact_dialogue(contact_dialogue) {}

	Actor() {}
public:
	virtual void update_position();
};
 
