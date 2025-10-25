#include "Camera.h"
#include "Engine.h"

void Camera::Set_Position(float x, float y)
{
	camera_position = { x,y };
}

glm::vec2 Camera::Get_Position()
{
	return camera_position;
}

void Camera::Set_Zoom_Factor(float zoom_factor)
{
	this->zoom_factor = zoom_factor;
}

float Camera::Get_Zoom_Factor()
{
	return zoom_factor;
}

void Camera::Set_Camera_Dimension(int x, int y)
{
	camera_dimension = { x,y };
}

glm::ivec2 Camera::Get_Camera_Dimension()
{
	return camera_dimension;
}

void Camera::Lua_Set_Position(float x, float y)
{
	Engine::instance->running_game->Set_Camera_Position(x, y);
}

float Camera::Lua_Get_Position_X()
{
	return Engine::instance->running_game->Get_Camera_Position().x;
}

float Camera::Lua_Get_Position_Y()
{
	return Engine::instance->running_game->Get_Camera_Position().y;
}

void Camera::Lua_Set_Zoom(float zoom_factor)
{
	Engine::instance->running_game->Set_Zoom_Factor(zoom_factor);
}

float Camera::Lua_Get_Zoom()
{
	return Engine::instance->running_game->Get_Zoom_Factor();
}

Vector2 Camera::Lua_Get_Camera_Dimension()
{
	auto dim = Engine::instance->running_game->Get_Camera_Dimension(); // glm::ivec2
	return Vector2{ static_cast<float>(dim.x), static_cast<float>(dim.y) };
}
