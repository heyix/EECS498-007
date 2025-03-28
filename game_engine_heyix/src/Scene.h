#pragma once
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include "rapidjson/include/rapidjson/document.h"
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "TemplateDB.h"
#include "map"
#include "lua/lua.hpp"
#include "LuaBridge/LuaBridge.h"
class Game;
class Scene {
public:
	void load_actors(rapidjson::Document& scene_json);
	void initialize_actor(const rapidjson::Value& actor, std::shared_ptr<GameObject> new_actor);
	/*bool check_collider_collision(Actor& actor,float target_y, float target_x);
	bool move_actor(Actor& actor, float target_y, float target_x);
	void trigger_contact_dialogue(Actor& actor);
	void trigger_nearby_dialogue(Actor& actor);
	bool check_substring_exist(const std::string& origin_string, const std::string& substring);
	void check_special_dialogue(std::string& origin_string, Actor& actor);
	void check_dialogue();*/
	void initialize_scene(rapidjson::Document& scene_json);
	//glm::ivec2 get_box_collider_region_position(const glm::vec2& vec);
	//glm::ivec2 get_box_trigger_region_position(const glm::vec2& vec);
	void add_actor(std::shared_ptr<GameObject>& new_object);
	void remove_gameobject(std::shared_ptr<GameObject> gameobject);
	std::shared_ptr<GameObject> get_object_by_id(int id);
	void before_scene_unload();
	void record_gameobject(std::shared_ptr<GameObject> gameobject);
	void unrecord_gameobject(std::shared_ptr<GameObject> gameobject);
	std::shared_ptr<GameObject>& get_gameobject_shared_ptr_by_pointer(GameObject* gameobject);
	std::shared_ptr<GameObject> find_gameobject_by_name(const std::string& name);
	luabridge::LuaRef find_all_gameObjects_by_name(const std::string& name);
private:
	std::shared_ptr<GameObject> instantiate_actor(const rapidjson::Value& actor);
public:
	//Player* player = nullptr;
	std::map<int, std::shared_ptr<GameObject>> sorted_actor_by_id;
	std::unordered_map<std::string, std::map<int, std::shared_ptr<GameObject>>> gameobjects_by_name;

	//std::unordered_map<uint64_t, std::vector<Actor*>> box_collider_map;
	//std::unordered_map<uint64_t, std::vector<Actor*>> box_trigger_map;
	//const std::vector<glm::ivec2> directions = { glm::ivec2{0,0}, glm::ivec2{0,1}, glm::ivec2{0,-1}, glm::ivec2{-1,0}, glm::ivec2{1,0}, glm::ivec2{-1,-1}, glm::ivec2{1,-1}, glm::ivec2{-1,1}, glm::ivec2{1,1} };

private:
public:
	static void Lua_Load(const std::string& scene_name);
	static std::string Lua_Get_Current();
	static void Lua_Dont_Destroy(luabridge::LuaRef actor);
}; 