#pragma once
#include <string>
#include "glm/glm.hpp"
#include "ImageDB.h"
#include "Helper.h"
#include <optional>
#include <unordered_set>
#include <unordered_map>
#include "GameObject.h"
class Actor: public GameObject{
public:
	glm::vec2 position{ 0,0 };
	glm::vec2 velocity{ 0,0 };
	std::string nearby_dialogue;
	std::string contact_dialogue;
	glm::vec2 transform_scale{ 1.0f,1.0f };
	float transform_rotation_degrees = 0.0f;
	glm::vec2 view_pivot_offset{0,0};
	std::optional<int> render_order;
	bool movement_bounce_enabled = false;
	std::optional<glm::vec2> box_collider;
	std::optional<glm::vec2> box_trigger;
	std::string nearby_dialogue_sfx;

	bool triggered_score_up = false;
	bool triggered_nearby_dialogue = false;
	bool intend_to_move = false;
	std::unordered_set<Actor*> colliding_actors_this_frame;
	float last_intented_velocity_y = 0.0f;
	int frameAttacked = -100;
	std::string view_image_attack;
	std::string view_image;
	std::string view_image_back;
	Actor();
public:
	void set_box_collider(float box_collider_width, float box_collider_height);
	void set_box_trigger(float box_trigger_width, float box_trigger_height);
public:
	virtual void update_position();
	virtual void update();
	void set_view_image_attack(const std::string& image_name);
	void set_view_image(const std::string& image_name);
	void set_view_image_back(const std::string& image_name);

	virtual const std::string& get_current_render_image_name();
}; 