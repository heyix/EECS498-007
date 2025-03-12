#pragma once
#include "Actor.h"

class Player :public Actor {
public:
	Player();
	float movement_speed = 0.02f;
	std::string damage_sfx;
	std::string step_sfx;
	std::string view_image_damage;

	int frameDamaged = -100;
public:
	void set_view_image_damage(const std::string& image_name);
public:
	virtual void update_position() override;
	virtual const std::string& get_current_render_image_name() override;

};