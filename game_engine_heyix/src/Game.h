#pragma once
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include "rapidjson/include/rapidjson/document.h"
#include <unordered_map>
#include <algorithm>
#include "Engine.h"
#include <optional>
#include "SDL2_mixer/SDL_mixer.h"
#include "SDL2_image/SDL_image.h"
#include <deque>
#include "GameConfigData.h"
#include "RenderingConfigData.h"
#include <thread>
#include "Scene.h"
#include "GameData.h"
#include "GameObject.h"
class Camera;
class Game {
public:
	void game_loop();
protected:
	virtual void awake();
	virtual void start();
	virtual void update();
	virtual void render();
	virtual void process_input();
protected:
	bool is_running = true;
private:
	void load_current_scene(const std::shared_ptr<Scene>& new_scene);
	void init_renderer();
	void init_camera();
	void init_game_data();
	void after_init_game_data();
	void sync_rigidbody_and_transform();
	//void change_current_scene(const std::string& new_scene_name);
	std::shared_ptr<GameObject> instantiate_gameobject_recursive(const rapidjson::Value& actor_json, std::shared_ptr<GameObject> parent);
public:
	Game() {
		instance = this;
	}
	static void Lua_Quit();
	static void Lua_Sleep(int miliseconds);
	static int Lua_Get_Frame();
	static void Lua_Open_URL(const std::string& url);

public:
	void Remove_GameObject(GameObject* gameobject);
	std::weak_ptr<GameObject> Instantiate_GameObject_From_Template(const std::string& actor_template_name);
	void Add_Instantiated_GameObject(std::shared_ptr<GameObject> new_object);
	glm::ivec2 Get_Camera_Dimension();
	glm::vec2 Get_Camera_Position();
	float Get_Zoom_Factor();
	void Set_Zoom_Factor(float zoom_factor);
	void Set_Camera_Position(float x, float y);
	void Load_Scene(const std::string& scene_name);
	std::string& Get_Current_Scene_Name();
	void Dont_Destroy(luabridge::LuaRef& actor);
	std::weak_ptr<GameObject> Find_GameObject_By_Name(const std::string& name);
	std::vector<std::weak_ptr<GameObject>> Find_All_GameObjects_By_Name(const std::string& name);
	luabridge::LuaRef Find_All_Lua_GameObjects_By_Name(const std::string& name);
private:
	GameData game_data;
	std::string current_scene_name;
	std::shared_ptr<Scene> current_scene;
	std::set<std::shared_ptr<GameObject>,GameObject::GameObjectComparatorByKey> pending_adding_gameobjects;
	std::set<std::shared_ptr<GameObject>,GameObject::GameObjectComparatorByKey> pending_removing_gameobjects;
	std::shared_ptr<Camera> camera;
	bool pending_change_scene = false;
public:
	static inline Game* instance = nullptr;
};
#include "Engine.h"