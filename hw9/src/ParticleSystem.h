#pragma once
#include "CppComponent.h"
#include "SDL2/SDL.h"
#include "Helper.h"
#include <optional>
#include <queue>
class ParticleSystem :public CppComponent {
private:
	struct Particle {
		float x, y;
		float angle;
		float radius;
		float initial_scale;
		float current_scale;
		float rotation;
		SDL_Color color;
		int start_frame;
		bool active;
		float speed_x;
		float speed_y;
		float rotation_speed;
	};
public:
	ParticleSystem(GameObject& holder, const std::string& key, const std::string& template_name);
private:
	void burst_one_particle();
	void render_particle(Particle& particle);
	void burst_particles();
	bool check_particle_lifetime(int index);
	void apply_particle_transformation(Particle& particle);
	void apply_particle_velocity(Particle& particle);
	void apply_particle_gravity(Particle& particle);
	void apply_particle_drag(Particle& particle);
	void process_particle_color_and_scale(Particle& particle);
private:
	static void Register_Default_Particle_Texture_With_Name(const std::string& name);
public:
	float Lua_GetX()const { return x; }
	float Lua_GetY()const { return y; }
	int Lua_Get_Frame_Between_Burst() const { return frames_between_bursts; }
	int Lua_Get_Burst_Quantity() const { return burst_quantity; }
	float Lua_Get_Start_Scale_Min() const { return start_scale_min; }
	float Lua_Get_Start_Scale_Max() const { return start_scale_max; }
	float Lua_Get_Rotation_Min() const { return rotation_min; }
	float Lua_Get_Rotation_Max() const { return rotation_max; }
	int Lua_Get_Start_Color_r() const { return start_color.r; }
	int Lua_Get_Start_Color_g() const { return start_color.g; }
	int Lua_Get_Start_Color_b() const { return start_color.b; }
	int Lua_Get_Start_Color_a() const { return start_color.a; }
	float Lua_Get_Emit_Radius_Min() const { return emit_radius_min; }
	float Lua_Get_Emit_Radius_Max() const { return emit_radius_max; }
	float Lua_Get_Emit_Angle_Min() const { return emit_angle_min; }
	float Lua_Get_Emit_Angle_Max() const { return emit_angle_max; }
	std::string Lua_Get_Image() const { return image; }
	int Lua_Get_Sorting_Order() const { return sorting_order; }

	int Lua_Get_Duration_Frames() const { return duration_frames; }
	float Lua_Get_Start_Speed_Min() const { return start_speed_min; }
	float Lua_Get_Start_Speed_Max() const { return start_speed_max; }
	float Lua_Get_Rotation_Speed_Min() const { return rotation_speed_min; }
	float Lua_Get_Rotation_Speed_Max() const { return rotation_speed_max; }
	float Lua_Get_Gravity_Scale_X() const { return gravity_scale_x; }
	float Lua_Get_Gravity_Scale_Y() const { return gravity_scale_y; }
	float Lua_Get_Drag_Factor() const { return drag_factor; }
	float Lua_Get_Angular_Drag_Factor() const { return angular_drag_factor; }
	float Lua_Get_End_Scale() const { return end_scale.value_or(1.0f); }
	int Lua_Get_End_Color_r() const { return end_color_r ? end_color_r.value() : 255; }
	int Lua_Get_End_Color_g() const { return end_color_g ? end_color_g.value() : 255; }
	int Lua_Get_End_Color_b() const { return end_color_b ? end_color_b.value() : 255; }
	int Lua_Get_End_Color_a() const { return end_color_a ? end_color_a.value() : 255; }






	void Lua_SetX(float new_x) { x = new_x; }
	void Lua_SetY(float new_y) { y = new_y; }
	void Lua_Set_Frame_Between_Burst(int frames) { frames_between_bursts = std::max(frames, 1); }
	void Lua_Set_Burst_Quantity(int quantity) { burst_quantity = std::max(quantity, 1); }
	void Lua_Set_Start_Scale_Min(float min_scale);
	void Lua_Set_Start_Scale_Max(float max_scale);
	void Lua_Set_Rotation_Min(float min_rotation);
	void Lua_Set_Rotation_Max(float max_rotation);
	void Lua_Set_Start_Color_r(int r) { start_color.r = r; }
	void Lua_Set_Start_Color_g(int g) { start_color.g = g; }
	void Lua_Set_Start_Color_b(int b) { start_color.b = b; }
	void Lua_Set_Start_Color_a(int a) { start_color.a = a; }
	void Lua_Set_Emit_Radius_Min(float min_radius);
	void Lua_Set_Emit_Radius_Max(float max_radius);
	void Lua_Set_Emit_Angle_Min(float min_angle);
	void Lua_Set_Emit_Angle_Max(float max_angle);
	void Lua_Set_Image(const std::string& image_name) { image = image_name; }
	void Lua_Set_Sorting_Order(int order) { sorting_order = order; }

	void Lua_Set_Duration_Frames(int frames) { duration_frames = std::max(frames,1); }
	void Lua_Set_Start_Speed_Min(float min_speed) { start_speed_min = min_speed; }
	void Lua_Set_Start_Speed_Max(float max_speed) { start_speed_max = max_speed; }
	void Lua_Set_Rotation_Speed_Min(float min_rotation_speed) { rotation_speed_min = min_rotation_speed; }
	void Lua_Set_Rotation_Speed_Max(float max_rotation_speed) { rotation_speed_max = max_rotation_speed; }
	void Lua_Set_Gravity_Scale_X(float gravity_x) { gravity_scale_x = gravity_x; }
	void Lua_Set_Gravity_Scale_Y(float gravity_y) { gravity_scale_y = gravity_y; }
	void Lua_Set_Drag_Factor(float factor) { drag_factor = factor; }
	void Lua_Set_Angular_Drag_Factor(float factor) { angular_drag_factor = factor; }
	void Lua_Set_End_Scale(float scale) { end_scale = scale; }
	void Lua_Set_End_Color_r(float r) { end_color_r = r; }
	void Lua_Set_End_Color_g(float g) { end_color_g = g; }
	void Lua_Set_End_Color_b(float b) { end_color_b = b; }
	void Lua_Set_End_Color_a(float a) { end_color_a = a; }

	void Lua_Stop();
	void Lua_Play();
	void Lua_Burst();

public:
	virtual void On_Start()override;
	virtual void On_Update()override;


	virtual void Add_Int_Property(const std::string& key, int new_property) override;
	virtual void Add_Float_Property(const std::string& key, float new_property) override;
	virtual void Add_String_Property(const std::string& key, const std::string& new_property) override;
private:
	int local_frame_number = 0;
	std::vector<Particle> particles;
	RandomEngine emit_angle_distribution;
	RandomEngine emit_radius_distribution;
	RandomEngine rotation_distribution;
	RandomEngine scale_distribution;
	RandomEngine speed_distribution;
	RandomEngine rotation_speed_distribution;
	std::queue<int> free_list;
	bool burst_enabled = true;
public:
	float x = 0.0f;
	float y = 0.0f;
	int frames_between_bursts = 1;
	int burst_quantity = 1;
	float start_scale_min = 1.0f;
	float start_scale_max = 1.0f;
	float rotation_min = 0.0f;
	float rotation_max = 0.0f;
	SDL_Color start_color = { 255,255,255,255 };
	float emit_radius_min = 0.0f;
	float emit_radius_max = 0.5f;
	float emit_angle_min = 0.0f;
	float emit_angle_max = 360.0f;
	std::string image = default_particle_system_texture_name;
	int sorting_order = 9999;

	int duration_frames = 300;
	float start_speed_min = 0.0f;
	float start_speed_max = 0.0f;
	float rotation_speed_min = 0.0f;
	float rotation_speed_max = 0.0f;
	float gravity_scale_x = 0.0f;
	float gravity_scale_y = 0.0f;
	float drag_factor = 1.0f;
	float angular_drag_factor = 1.0f;
	std::optional<float> end_scale;
	std::optional<float> end_color_r;
	std::optional<float> end_color_g;
	std::optional<float> end_color_b;
	std::optional<float> end_color_a;

private:
	static inline std::string default_particle_system_texture_name = "__Default_Particle_System_Texture";
	static inline bool registered_default_particle_system_texture = false;
};