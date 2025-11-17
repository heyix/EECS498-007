#include "Time.h"
#include "Engine.h"
void Time::Begin_New_Frame(float raw_frame_dt_seconds)
{
    unscaled_delta_time = raw_frame_dt_seconds;
    delta_time = raw_frame_dt_seconds * time_scale;
    unscaled_time += unscaled_delta_time;
    time += delta_time;

    accumulator += unscaled_delta_time;

    if (count_fps) {
        fps_frame_count++;
        fps_accum_time += raw_frame_dt_seconds;

        if (fps_accum_time >= fps_update_interval) {
            fps = fps_frame_count / fps_accum_time;
            fps_frame_count = 0;
            fps_accum_time = 0.0f;
        }
    }

    if (count_physics_fps) {
        physics_fps_accum_time += raw_frame_dt_seconds;

        if (physics_fps_accum_time >= physics_fps_update_interval) {
            if (physics_fps_accum_time > 0.0f) {
                physics_fps = physics_step_count / physics_fps_accum_time;
            }
            else {
                physics_fps = 0.0f;
            }

            if (physics_step_count > 0) {
                physics_step_time_ms = static_cast<float>(
                    physics_step_time_accum_ms / physics_step_count
                    );
            }
            else {
                physics_step_time_ms = 0.0f;
            }

            physics_step_count = 0;
            physics_fps_accum_time = 0.0f;
            physics_step_time_accum_ms = 0.0;
        }
    }
}

bool Time::Try_Run_Fixed_Step()
{
	if (accumulator + 1e-9f < fixed_delta_time)
		return false;

	accumulator -= fixed_delta_time;

	fixed_time += fixed_delta_time * time_scale;

	if (count_physics_fps)
		physics_step_count++;


	return true;
}

void Time::Enable_FPS_Count()
{
	count_fps = true;
	fps = 0.0f;
	fps_accum_time = 0.0f;
	fps_frame_count = 0;
}

void Time::Enable_Physics_FPS_Count()
{
    count_physics_fps = true;
    physics_fps = 0.0f;
    physics_fps_accum_time = 0.0f;
    physics_step_count = 0;
    physics_step_time_ms = 0.0f;
    physics_step_time_accum_ms = 0.0;
}
void Time::Accumulate_Physics_Step_Time(double step_ms)
{
    if (!count_physics_fps) return;
    physics_step_time_accum_ms += step_ms;
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
