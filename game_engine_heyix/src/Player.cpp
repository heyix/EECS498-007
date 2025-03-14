#include "Player.h"
#include "Game1.h"
Player::Player():Actor()
{
}
void Player::set_view_image_damage(const std::string& image_name)
{
	view_image_damage = image_name;
}
void Player::update_position()
{
	glm::vec2 move_position = { 0,0 };
	if (Input::GetKey(SDL_SCANCODE_UP) || Input::GetKey(SDL_SCANCODE_W)) {
		move_position += glm::vec2{0, -1};
		last_intented_velocity_y = -1 * movement_speed;
	}
	if (Input::GetKey(SDL_SCANCODE_DOWN) || Input::GetKey(SDL_SCANCODE_S)) {
		move_position += glm::vec2{ 0,1 };
		last_intented_velocity_y = 1 * movement_speed;
	}
	if (Input::GetKey(SDL_SCANCODE_LEFT) || Input::GetKey(SDL_SCANCODE_A)) {
		move_position += glm::vec2{ -1,0 };
		if(Game1::instance->rendering_config_data.x_scale_actor_flipping_on_movement)this->transform_scale.x = -glm::abs(this->transform_scale.x);
	}
	if (Input::GetKey(SDL_SCANCODE_RIGHT) || Input::GetKey(SDL_SCANCODE_D)) {
		move_position += glm::vec2{ 1,0 };
		if(Game1::instance->rendering_config_data.x_scale_actor_flipping_on_movement)this->transform_scale.x = glm::abs(this->transform_scale.x);
	}

	if (!(move_position.x == 0 && move_position.y == 0)) {
		move_position = glm::normalize(move_position);
		this->intend_to_move = true;
		if (Helper::GetFrameNumber() % 20 == 0) {
			if (step_sfx != "") {
				AudioDB::Play_Audio(Helper::GetFrameNumber() % 48 + 2, step_sfx, 0);
			}
		}
	}
	else {
		this->intend_to_move = false;
	}
	move_position *= movement_speed;
	if (Game1::instance->move_actor(*this, position.y + move_position.y, position.x + move_position.x)) {
		 
	}
}

const std::string& Player::get_current_render_image_name()
{
	if (view_image_damage != "" && Helper::GetFrameNumber() < frameDamaged + 30) {
		return view_image_damage;
	}
	if (view_image_back != "") {
		if (last_intented_velocity_y < 0) {
			return view_image_back;
		}
		if (last_intented_velocity_y > 0) {
			return view_image;
		}
	}
	return view_image;
}
 