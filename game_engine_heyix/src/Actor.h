#pragma once
#include <string>
#include "glm/glm.hpp"
#include "ImageDB.h"
#include "Helper.h"
#include <optional>
class Actor {
public:
	int ID = 0;
	std::string actor_name = "";
	char view = '?';
	glm::ivec2 position{ 0,0 };
	glm::ivec2 velocity{ 0,0 };
	bool blocking = false;
	std::string nearby_dialogue;
	std::string contact_dialogue;
	std::string view_image;
	glm::vec2 transform_scale{ 1.0f,1.0f };
	float transform_rotation_degrees = 0.0f;
	glm::vec2 view_pivot_offset{0,0};
	float image_width = 0;
	float image_height = 0;
	std::optional<int> render_order;

	bool triggered_score_up = false;



	Actor();
public:
	virtual void update_position();
	virtual void set_view_image(const std::string& image_name);
};