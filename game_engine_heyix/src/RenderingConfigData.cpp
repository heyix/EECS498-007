#include "RenderingConfigData.h"

void RenderingConfigData::set_rendering_config_data(rapidjson::Document& rendering_config)
{
	if (auto it_x = rendering_config.FindMember("x_resolution"); it_x != rendering_config.MemberEnd()) {
		resolution.x = it_x->value.GetInt();
	}
	if (auto it_y = rendering_config.FindMember("y_resolution"); it_y != rendering_config.MemberEnd()) {
		resolution.y = it_y->value.GetInt();
	}
	if (auto it = rendering_config.FindMember("clear_color_r"); it != rendering_config.MemberEnd()) {
		clear_color_r = it->value.GetInt();
	}
	if (auto it = rendering_config.FindMember("clear_color_g"); it != rendering_config.MemberEnd()) {
		clear_color_g = it->value.GetInt();
	}
	if (auto it = rendering_config.FindMember("clear_color_b"); it != rendering_config.MemberEnd()) {
		clear_color_b = it->value.GetInt();
	}
	if (auto it = rendering_config.FindMember("cam_offset_x"); it != rendering_config.MemberEnd()) {
		camera_offset.x = it->value.GetFloat();
	}
	if (auto it = rendering_config.FindMember("cam_offset_y"); it != rendering_config.MemberEnd()) {
		camera_offset.y = it->value.GetFloat();
	}
	if (auto it = rendering_config.FindMember("zoom_factor"); it != rendering_config.MemberEnd()) {
		zoom_factor = it->value.GetFloat();
	}
	if (auto it = rendering_config.FindMember("cam_ease_factor"); it != rendering_config.MemberEnd()) {
		cam_ease_factor = it->value.GetFloat();
	}
	if (auto it = rendering_config.FindMember("x_scale_actor_flipping_on_movement"); it != rendering_config.MemberEnd()) {
		x_scale_actor_flipping_on_movement = it->value.GetBool();
	}
}
