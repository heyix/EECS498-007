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

	float fixed_delta_time = 0.02f;
	float fixed_time = 0;

	double accumulator = 0.0f;
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

public:
	static float Lua_Delta_Time();
	static float Lua_Unscaled_Delta_Time();
	static float Lua_Current_Time();
	static float Lua_Current_Unscaled_Time();
	static float Lua_Get_Time_Scale();
	static void Lua_Set_Time_Scale(float new_scale);
};