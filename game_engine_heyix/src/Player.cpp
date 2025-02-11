#include "Player.h"
#include "Game1.h"
void Player::update_position()
{
	std::string& user_input = Game1::instance->user_input;
	if (user_input == "w")Game1::instance->move_actor(*this, position.y, position.x - 1);
	if (user_input == "e")Game1::instance->move_actor(*this, position.y, position.x + 1);
	if (user_input == "n")Game1::instance->move_actor(*this, position.y - 1, position.x);
	if (user_input == "s")Game1::instance->move_actor(*this, position.y + 1, position.x);
	Game1::instance->user_input = "";
}
