#include "ParticleSystem.h"
#include "LuaDB.h"
#include "glm/glm.hpp"
#include "Renderer.h"
#include "Engine.h"
#include "ImageDB.h"
#include "EngineUtils.h"
ParticleSystem::ParticleSystem(GameObject& holder, const std::string& key, const std::string& template_name)
	:CppComponent(holder, key, template_name, luabridge::LuaRef(LuaDB::lua_state, this)) 
{
	has_on_update = true;
	has_on_start = true;
	if (!registered_default_particle_system_texture) {
		registered_default_particle_system_texture = true;
		Register_Default_Particle_Texture_With_Name(default_particle_system_texture_name);
	}
}

void ParticleSystem::burst_one_particle()
{
	float angle_radian = glm::radians(emit_angle_distribution.Sample());
	float radius = emit_radius_distribution.Sample();
	float cos_angle = glm::cos(angle_radian);
	float sin_angle = glm::sin(angle_radian);
	float scale = scale_distribution.Sample();
	float rotation = rotation_distribution.Sample();
	float speed = speed_distribution.Sample();
	float rotation_speed = rotation_speed_distribution.Sample();
	Particle* particle;
	if (!free_list.empty()) {
		int index = free_list.front();
		free_list.pop();
		particle = &particles[index];
	}
	else {
		particles.emplace_back();
		particle = &particles.back();
	}
	particle->x = x + cos_angle * radius;
	particle->y = y + sin_angle * radius;
	particle->angle = angle_radian;
	particle->radius = radius;
	particle->current_scale = scale;
	particle->initial_scale = scale;
	particle->rotation = rotation;
	particle->color = start_color;
	particle->start_frame = local_frame_number;
	particle->active = true;
	particle->speed_x = cos_angle * speed;
	particle->speed_y = sin_angle * speed;
	particle->rotation_speed = rotation_speed;
}

void ParticleSystem::render_particle(Particle& particle)
{
	Engine::instance->renderer->draw_ex(
		image,
		particle.x, particle.y,
		particle.rotation,//rotation
		particle.current_scale, particle.current_scale,//scale
		0.5f, 0.5f,//pivot
		particle.color.r, particle.color.g, particle.color.b, particle.color.a,//rgba
		sorting_order
	);
}

void ParticleSystem::burst_particles()
{
	for (int i = 0; i < burst_quantity; i++) { 
		burst_one_particle(); 
	}
}

bool ParticleSystem::check_particle_lifetime(int index)  
{
	if (!particles[index].active) {
		return false;
	}
	int frame_passed = local_frame_number - particles[index].start_frame;
	if (frame_passed >= duration_frames) {
		particles[index].active = false;
		free_list.push(index);
		return false;
	}
	return true;
}

void ParticleSystem::apply_particle_transformation(Particle& particle)
{
	apply_particle_gravity(particle);
	apply_particle_drag(particle);
	apply_particle_velocity(particle);
	process_particle_color_and_scale(particle);
}

void ParticleSystem::apply_particle_velocity(Particle& particle)
{
	particle.x += particle.speed_x;
	particle.y += particle.speed_y;
	particle.rotation += particle.rotation_speed;
}

void ParticleSystem::apply_particle_gravity(Particle& particle)
{
	particle.speed_x += gravity_scale_x;
	particle.speed_y += gravity_scale_y;
}

void ParticleSystem::apply_particle_drag(Particle& particle)
{
	particle.speed_x *= drag_factor;
	particle.speed_y *= drag_factor;
	particle.rotation_speed *= angular_drag_factor;
}

void ParticleSystem::process_particle_color_and_scale(Particle& particle)
{
	int frame_passed = local_frame_number - particle.start_frame;
	if (end_scale.has_value()) {
		particle.current_scale = EngineUtils::Linear_Interpolate(particle.initial_scale, end_scale.value(), static_cast<float>(frame_passed) / duration_frames);
	}
	if (end_color_r.has_value()) {
		particle.color.r = static_cast<Uint8>(EngineUtils::Linear_Interpolate(start_color.r, end_color_r.value(), static_cast<float>(frame_passed) / duration_frames));
	}
	if (end_color_g.has_value()) {
		particle.color.g = static_cast<Uint8>(EngineUtils::Linear_Interpolate(start_color.g, end_color_g.value(), static_cast<float>(frame_passed) / duration_frames));
	}
	if (end_color_b.has_value()) {
		particle.color.b = static_cast<Uint8>(EngineUtils::Linear_Interpolate(start_color.b, end_color_b.value(), static_cast<float>(frame_passed) / duration_frames));
	}
	if (end_color_a.has_value()) {
		particle.color.a = static_cast<Uint8>(EngineUtils::Linear_Interpolate(start_color.a, end_color_a.value(), static_cast<float>(frame_passed) / duration_frames));
	}
}

void ParticleSystem::Register_Default_Particle_Texture_With_Name(const std::string& name)
{
	SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, 8, 8, 32, SDL_PIXELFORMAT_RGBA8888);

	Uint32 white_color = SDL_MapRGBA(surface->format, 255, 255, 255, 255);
	SDL_FillRect(surface, NULL, white_color);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(Engine::instance->renderer->sdl_renderer, surface);
	SDL_FreeSurface(surface);
	ImageDB::Register_Instantiated_Texture(default_particle_system_texture_name, texture, 8, 8);
}

void ParticleSystem::Lua_Stop()
{
	burst_enabled = false;
}

void ParticleSystem::Lua_Play()
{
	burst_enabled = true;
}

void ParticleSystem::Lua_Burst()
{
	burst_particles();
}

void ParticleSystem::On_Start()
{
	emit_angle_distribution = RandomEngine(emit_angle_min, emit_angle_max, 298);
	emit_radius_distribution = RandomEngine(emit_radius_min, emit_radius_max, 404);

	/* Test Suite #1 Distributions */
	rotation_distribution = RandomEngine(rotation_min, rotation_max, 440);
	scale_distribution = RandomEngine(start_scale_min, start_scale_max, 494);

	/* Test Suite #2 Distributions */
	speed_distribution = RandomEngine(start_speed_min, start_speed_max, 498);
	rotation_speed_distribution = RandomEngine(rotation_speed_min, rotation_speed_max, 305);
}

void ParticleSystem::On_Update()
{
	if (Get_Enabled()) {
		if (local_frame_number % frames_between_bursts == 0 && burst_enabled) {
			burst_particles();
		}
		for (int i = 0; i < particles.size(); i++) {
			if (check_particle_lifetime(i)) {
				apply_particle_transformation(particles[i]);
				render_particle(particles[i]);
			}
		}
		local_frame_number++;
	}
}

void ParticleSystem::Add_Int_Property(const std::string& key, int new_property)
{
	if (key == "frames_between_bursts") {
		frames_between_bursts = std::max(new_property, 1);
	}
	else if (key == "burst_quantity") {
		burst_quantity = std::max(new_property, 1);
	}
	else if (key == "sorting_order") {
		sorting_order = new_property;
	}
	else if (key == "start_color_r") {
		start_color.r = static_cast<Uint8>(new_property);
	}
	else if (key == "start_color_g") {
		start_color.g = static_cast<Uint8>(new_property);
	}
	else if (key == "start_color_b") {
		start_color.b = static_cast<Uint8>(new_property);
	}
	else if (key == "start_color_a") {
		start_color.a = static_cast<Uint8>(new_property);
	}
	else if (key == "x") {
		x = static_cast<float>(new_property);
	}
	else if (key == "y") {
		y = static_cast<float>(new_property);
	}
	else if (key == "start_scale_min") {
		start_scale_min = static_cast<float>(new_property);
	}
	else if (key == "start_scale_max") {
		start_scale_max = static_cast<float>(new_property);
	}
	else if (key == "rotation_min") {
		rotation_min = static_cast<float>(new_property);
	}
	else if (key == "rotation_max") {
		rotation_max = static_cast<float>(new_property);
	}
	else if (key == "emit_radius_min") {
		emit_radius_min = static_cast<float>(new_property);
	}
	else if (key == "emit_radius_max") {
		emit_radius_max = static_cast<float>(new_property);
	}
	else if (key == "emit_angle_min") {
		emit_angle_min = static_cast<float>(new_property);
	}
	else if (key == "emit_angle_max") {
		emit_angle_max = static_cast<float>(new_property);
	}
	else if (key == "duration_frames") {
		duration_frames = std::max(new_property,1);
	}
	else if (key == "start_speed_min") {
		start_speed_min = static_cast<float>(new_property);
	}
	else if (key == "start_speed_max") {
		start_speed_max = static_cast<float>(new_property);
	}
	else if (key == "rotation_speed_min") {
		rotation_speed_min = static_cast<float>(new_property);
	}
	else if (key == "rotation_speed_max") {
		rotation_speed_max = static_cast<float>(new_property);
	}
	else if (key == "gravity_scale_x") {
		gravity_scale_x = static_cast<float>(new_property);
	}
	else if (key == "gravity_scale_y") {
		gravity_scale_y = static_cast<float>(new_property);
	}
	else if (key == "drag_factor") {
		drag_factor = static_cast<float>(new_property);
	}
	else if (key == "angular_drag_factor") {
		angular_drag_factor = static_cast<float>(new_property);
	}
	else if (key == "end_scale") {
		end_scale = static_cast<float>(new_property);
	}
	else if (key == "end_color_r") {
		end_color_r = static_cast<float>(new_property);
	}
	else if (key == "end_color_g") {
		end_color_g = static_cast<float>(new_property);
	}
	else if (key == "end_color_b") {
		end_color_b = static_cast<float>(new_property);
	}
	else if (key == "end_color_a") {
		end_color_a = static_cast<float>(new_property);
	}
}

void ParticleSystem::Add_Float_Property(const std::string& key, float new_property)
{
	if (key == "x") {
		x = new_property;
	}
	else if (key == "y") {
		y = new_property;
	}
	else if (key == "start_scale_min") {
		start_scale_min = new_property;
	}
	else if (key == "start_scale_max") {
		start_scale_max = new_property;
	}
	else if (key == "rotation_min") {
		rotation_min = new_property;
	}
	else if (key == "rotation_max") {
		rotation_max = new_property;
	}
	else if (key == "emit_radius_min") {
		emit_radius_min = new_property;
	}
	else if (key == "emit_radius_max") {
		emit_radius_max = new_property;
	}
	else if (key == "emit_angle_min") {
		emit_angle_min = new_property;
	}
	else if (key == "emit_angle_max") {
		emit_angle_max = new_property;
	}
	else if (key == "start_speed_min") {
		start_speed_min = new_property;
	}
	else if (key == "start_speed_max") {
		start_speed_max = new_property;
	}
	else if (key == "rotation_speed_min") {
		rotation_speed_min = new_property;
	}
	else if (key == "rotation_speed_max") {
		rotation_speed_max = new_property;
	}
	else if (key == "gravity_scale_x") {
		gravity_scale_x = new_property;
	}
	else if (key == "gravity_scale_y") {
		gravity_scale_y = new_property;
	}
	else if (key == "drag_factor") {
		drag_factor = new_property;
	}
	else if (key == "angular_drag_factor") {
		angular_drag_factor = new_property;
	}
	else if (key == "end_scale") {
		end_scale = new_property;
	}
	else if (key == "end_color_r") {
		end_color_r = new_property;
	}
	else if (key == "end_color_g") {
		end_color_g = new_property;
	}
	else if (key == "end_color_b") {
		end_color_b = new_property;
	}
	else if (key == "end_color_a") {
		end_color_a = new_property;
	}
}


void ParticleSystem::Add_String_Property(const std::string& key, const std::string& new_property)
{
	if (key == "image") {
		if (key == "") {
			image = "__Default_Particle_System_Texture";
		}
		else {
			image = new_property;
		}
	}
}



void ParticleSystem::Lua_Set_Start_Scale_Min(float min_scale)
{
	start_scale_min = min_scale;
	scale_distribution = RandomEngine(start_scale_min, start_scale_max, 494);
}

void ParticleSystem::Lua_Set_Start_Scale_Max(float max_scale)
{
	start_scale_max = max_scale;
	scale_distribution = RandomEngine(start_scale_min, start_scale_max, 494);
}

void ParticleSystem::Lua_Set_Rotation_Min(float min_rotation)
{
	rotation_min = min_rotation;
	rotation_distribution = RandomEngine(rotation_min, rotation_max, 440);
}

void ParticleSystem::Lua_Set_Rotation_Max(float max_rotation)
{
	rotation_max = max_rotation;
	rotation_distribution = RandomEngine(rotation_min, rotation_max, 440);
}

void ParticleSystem::Lua_Set_Emit_Radius_Min(float min_radius)
{
	emit_radius_min = min_radius;
	emit_radius_distribution = RandomEngine(emit_radius_min, emit_radius_max, 404);
}

void ParticleSystem::Lua_Set_Emit_Radius_Max(float max_radius)
{
	emit_radius_max = max_radius;
	emit_radius_distribution = RandomEngine(emit_radius_min, emit_radius_max, 404);
}

void ParticleSystem::Lua_Set_Emit_Angle_Min(float min_angle)
{
	emit_angle_min = min_angle;
	emit_angle_distribution = RandomEngine(emit_angle_min, emit_angle_max, 298);
}

void ParticleSystem::Lua_Set_Emit_Angle_Max(float max_angle)
{
	emit_angle_max = max_angle;
	emit_angle_distribution = RandomEngine(emit_angle_min, emit_angle_max, 298);
}
