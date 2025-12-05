#pragma once
class Time {
public:
	Time() {}
private:
	float delta_time = 0;
	float unscaled_delta_time = 0;
	float time = 0;
	float unscaled_time = 0;
	float time_scale = 1.0f;

	float fixed_delta_time = 1.0f/60;
	float fixed_time = 0;

	double accumulator = 0.0f;

	float fps = 0.0f;
	float fps_update_interval = 0.5f;
	float fps_accum_time = 0.0f;
	int fps_frame_count = 0;
	bool count_fps = false;

	float physics_fps = 0.0f;
	float physics_fps_update_interval = 0.5f;
	float physics_fps_accum_time = 0.0f;
	int physics_step_count = 0;
	bool count_physics_fps = false;

	float physics_step_time_ms = 0.0f;
	double physics_step_time_accum_ms = 0.0; 
public:
	void Begin_New_Frame(float raw_frame_dt_seconds);
	bool Try_Run_Fixed_Step();
	float Fixed_Delta_Time() { return fixed_delta_time; }
	float Delta_Time() { return delta_time; }
	float Unscaled_Delta_Time() { return unscaled_delta_time; }
	float Current_Time() { return time; }
	float Current_Unscaled_Time() { return unscaled_time; }
	float Get_Time_Scale() { return time_scale; }
	void Set_Time_Scale(float new_scale) { time_scale = new_scale; }
	float FPS() { if (count_fps)return fps; else return 0; }
	void Enable_FPS_Count();
	void Disable_FPS_Count() { count_fps = false; }
	void Accumulate_Physics_Step_Time(double step_ms);

	void Enable_Physics_FPS_Count();
	void Disable_Physics_FPS_Count() { count_physics_fps = false; }
	float Physics_FPS() const { return count_physics_fps ? physics_fps : 0.0f; }
	float Physics_Step_Time() const { return count_physics_fps ? physics_step_time_ms : 0.0f; }
	void ClampAccumulator(double max_accum);
public:
	static float Lua_Delta_Time();
	static float Lua_Unscaled_Delta_Time();
	static float Lua_Current_Time();
	static float Lua_Current_Unscaled_Time();
	static float Lua_Get_Time_Scale();
	static void Lua_Set_Time_Scale(float new_scale);
};