#include "Actor.h"
#include "Game1.h"
void Actor::update_position()
{
	if (velocity.x == 0 && velocity.y == 0)return;
	int target_x = position.x + velocity.x;
	int target_y = position.y + velocity.y;
	if (!Game1::instance->move_actor(*this, target_y, target_x)) {
		velocity.x *= -1;
		velocity.y *= -1;
	}
}
