#pragma once
#include <glm/glm.hpp>
#include "rapidjson/include/rapidjson/document.h"
class RenderingConfigData {
public:
	int clear_color_r = 255;
	int clear_color_g = 255;
	int clear_color_b = 255;
	glm::ivec2 resolution{ 640,360 };
	glm::vec2 camera_offset{ 0,0 };
	float zoom_factor = 1.0f;
	float cam_ease_factor = 1.0f;
	bool x_scale_actor_flipping_on_movement = false;
public:
	void set_rendering_config_data(rapidjson::Document& rendering_config);
};