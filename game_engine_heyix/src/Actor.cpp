#include "Actor.h"
#include "Game1.h"

Actor::Actor() {
}

void Actor::set_box_collider(float box_collider_width, float box_collider_height)
{
	this->box_collider = { box_collider_width * glm::abs(this->transform_scale.x),box_collider_height * glm::abs(this->transform_scale.y) };
}

void Actor::set_box_trigger(float box_trigger_width, float box_trigger_height)
{
	this->box_trigger = { box_trigger_width * glm::abs(this->transform_scale.x),box_trigger_height * glm::abs(this->transform_scale.y) };
}




const std::string& Actor::get_current_render_image_name()
{
	if (view_image_attack != "" && Helper::GetFrameNumber() < frameAttacked + 30) {
		return view_image_attack;
	}
	if (view_image_back != "") {
		if (last_intented_velocity_y > 0) {
			return view_image;
		}
		if (last_intented_velocity_y < 0) {
			return view_image_back;
		}
	}
	return view_image;
}

void Actor::update_position()
{
	float target_x = position.x + velocity.x;
	float target_y = position.y + velocity.y;
	if (velocity.x > 0 && Game1::instance->rendering_config_data.x_scale_actor_flipping_on_movement) {
		this->transform_scale.x = glm::abs(this->transform_scale.x);
	}
	if (velocity.x < 0 && Game1::instance->rendering_config_data.x_scale_actor_flipping_on_movement) {
		this->transform_scale.x = -glm::abs(this->transform_scale.x);
	}


	if (!Game1::instance->move_actor(*this, target_y, target_x)) {
		velocity.x *= -1;
		velocity.y *= -1;
	}
	else {
		last_intented_velocity_y = velocity.y;
		this->intend_to_move = true;
	}
	if (velocity.x == 0 && velocity.y == 0) {
		this->intend_to_move = false;
	}
}

void Actor::update()
{
	update_position();
}

void Actor::set_view_image(const std::string& image_name)
{
	float width, height;
	ImageDB::Get_Image_Resolution(image_name, width, height);
	view_image = image_name;
	view_pivot_offset = { width * 0.5f,height * 0.5f };
}

void Actor::set_view_image_back(const std::string& image_name)
{
	view_image_back = image_name;
}

void Actor::set_view_image_attack( const std::string& image_name)
{
	view_image_attack = image_name;
}
