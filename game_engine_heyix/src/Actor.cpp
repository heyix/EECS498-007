#include "Actor.h"
#include "Game1.h"

Actor::Actor()
{
}

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

void Actor::set_view_image(const std::string& image_name)
{
	view_image = image_name;
	float width, height;
	ImageDB::Get_Image_Resolution(image_name, width, height);
	view_pivot_offset = { width * 0.5f,height * 0.5f };
	image_width = width;
	image_height = height;
}
