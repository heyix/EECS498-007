#pragma once
#include "glm/glm.hpp"
#include "Vector2.h"
class Camera {
private:
	glm::vec2 camera_position{ 0,0 };
	float zoom_factor = 1.0f;
	glm::ivec2 camera_dimension = { 640,360 };
public:
	void Set_Position(float x, float y);
	glm::vec2 Get_Position();
	void Set_Zoom_Factor(float zoom_factor);
	float Get_Zoom_Factor();
	void Set_Camera_Dimension(int x, int y);
	glm::ivec2 Get_Camera_Dimension();
public:
	static void Lua_Set_Position(float x, float y);
	static float Lua_Get_Position_X();
	static float Lua_Get_Position_Y();
	static void Lua_Set_Zoom(float zoom_factor);
	static float Lua_Get_Zoom();
	static Vector2 Lua_Get_Camera_Dimension();
};