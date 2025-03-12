#pragma once
#include "Game.h"
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include "rapidjson/include/rapidjson/document.h"
#include <unordered_map>
#include <vector>
#include "Player.h"
#include <algorithm>
#include "Actor.h"
#include "TemplateDB.h"

class Game1;
class Scene {
public:
	void load_actors(rapidjson::Document& scene_json);
	void initialize_actor(const rapidjson::Value& actor, std::shared_ptr<Actor> new_actor);
	bool check_collider_collision(std::shared_ptr<Actor> actor,float target_y, float target_x);
	bool move_actor(std::shared_ptr<Actor> actor, float target_y, float target_x);
	void trigger_contact_dialogue(std::shared_ptr<Actor> actor);
	void trigger_nearby_dialogue(std::shared_ptr<Actor> actor);
	bool check_substring_exist(const std::string& origin_string, const std::string& substring);
	void check_special_dialogue(std::string& origin_string, std::shared_ptr<Actor> actor);
	void check_dialogue();
	void initialize_scene(rapidjson::Document& scene_json);
	glm::ivec2 get_box_collider_region_position(const glm::vec2& vec);
	glm::ivec2 get_box_trigger_region_position(const glm::vec2& vec);
private:
	std::shared_ptr<Actor> instantiate_actor(const rapidjson::Value& actor, int& actor_index);
public:
	std::shared_ptr<Player> player = nullptr;
	std::vector<std::shared_ptr<Actor>> sorted_actor_by_id;
	std::unordered_map<uint64_t, std::vector<std::shared_ptr<Actor>>> box_collider_map;
	std::unordered_map<uint64_t, std::vector<std::shared_ptr<Actor>>> box_trigger_map;
	const std::vector<glm::ivec2> directions = { glm::ivec2{0,0}, glm::ivec2{0,1}, glm::ivec2{0,-1}, glm::ivec2{-1,0}, glm::ivec2{1,0}, glm::ivec2{-1,-1}, glm::ivec2{1,-1}, glm::ivec2{-1,1}, glm::ivec2{1,1} };

private:
	std::vector<std::shared_ptr<Actor>> _underlying_actor_storage;
	std::vector<std::shared_ptr<Player>> _underlying_player_storage;
	int current_id = 0;
	float box_collider_region_size_x = 0.001f;
	float box_collider_region_size_y = 0.001f;
	float box_trigger_region_size_x = 0.001f;
	float box_trigger_region_size_y = 0.001f;
}; 