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
	void validate_registered_actor (std::shared_ptr<GameObject>& new_object);
	void remove_gameobject(GameObject* gameobject);
	std::weak_ptr<GameObject> get_object_by_id(int id);
	void before_scene_unload();
	void register_gameobject(std::shared_ptr<GameObject> gameobject);
	void record_gameobject_by_name(std::shared_ptr<GameObject> gameobject);
	void unrecord_gameobject_by_name(GameObject* gameobject);
	std::shared_ptr<GameObject>& get_gameobject_shared_ptr_by_pointer(GameObject* gameobject);
	std::weak_ptr<GameObject> find_gameobject_by_name(const std::string& name);
	luabridge::LuaRef find_all_gameObjects_by_name(const std::string& name);
private:
	std::shared_ptr<GameObject> instantiate_actor(const rapidjson::Value& actor);
	void instantiate_actor_recursive(const rapidjson::Value& actor_json, std::shared_ptr<GameObject> parent);
	std::map<int, std::shared_ptr<GameObject>> _objects_storage;
public:
	//Player* player = nullptr;
	std::map<int, std::shared_ptr<GameObject>> validated_gameobjects;
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