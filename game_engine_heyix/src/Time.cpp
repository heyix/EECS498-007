#include "Time.h"
#include "Engine.h"
void Time::Begin_New_Frame(float raw_frame_dt_seconds)
{
	unscaled_delta_time = raw_frame_dt_seconds;
	delta_time = raw_frame_dt_seconds * time_scale;
	unscaled_time += unscaled_delta_time;
	time += delta_time;

	accumulator += unscaled_delta_time;
}

bool Time::Try_Run_Fixed_Step()
{
	if (accumulator + 1e-9f < fixed_delta_time)
		return false;

	accumulator -= fixed_delta_time;

	fixed_time += fixed_delta_time * time_scale;

	return true;
}

float Time::Lua_Delta_Time()
{
	return Engine::instance->running_game->Delta_Time();
}

float Time::Lua_Unscaled_Delta_Time()
{
	return Engine::instance->running_game->Unscaled_Delta_Time();
}

float Time::Lua_Current_Time()
{
	return Engine::instance->running_game->Current_Time();
}

float Time::Lua_Current_Unscaled_Time()
{
	return Engine::instance->running_game->Current_Unscaled_Time();
}

float Time::Lua_Get_Time_Scale()
{
	return Engine::instance->running_game->Get_Time_Scale();
}

void Time::Lua_Set_Time_Scale(float new_scale)
{
	Engine::instance->running_game->Set_Time_Scale(new_scale);
}
