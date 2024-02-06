#pragma once
#include "Engine.h"
#include <string>
#include "glm/glm.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <set>
#include <sstream>
#include <memory>
#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/filereadstream.h"
#include <cstdlib>
#include <filesystem>
#include <unordered_map>
#include "EngineUtils.h"
#include "Actor.h"
#include <map>
#include "Player.h"
#include <algorithm>
#include "TemplateDB.h"
#include "Scene.h"
enum GameStatus
{
	GameStatus_running,GameStatus_good_ending,GameStatus_bad_ending,GameStatus_quit,GameStatus_changing_scene
};

class Game1:public Engine
{
public:
	
public:
	Game1() {
		instance = this;
	}
protected:
	void awake() override;
	void start() override;
	void update() override;
	void render() override;
private:
	void input();
	void update_actor();
	void check_game_status();
	void cout_frame_output();
	void load_config_files();
	void clear_config_data();
	void load_config_file(const std::string& file_path);
	void config_files_pre_check();
	void config_files_post_check();
	void update_config_variables();
	void load_current_scene();
public:
	bool move_actor(Actor& actor, int target_y, int target_x);
	void change_game_status(GameStatus new_status);
	void change_current_scene(const std::string& new_scene_name);
	void change_player_health(int change);
	void change_score(int change);
public:
	std::string user_input;
	static Game1* instance;
	int score = 0;
	int player_health = 3;

private:
	GameStatus game_status = GameStatus_running;
	std::stringstream frame_output;
	std::unordered_map<std::string, std::unique_ptr<rapidjson::Document>> config_file_map;
	std::vector<std::string> config_files{ "game.config","rendering.config"};
	glm::ivec2 camera_dimension{13,9};
	glm::ivec2 camera_position{ 19,15 };
	std::string current_scene_name;
	std::unique_ptr<Scene> current_scene;
	std::string game_start_message;
	std::string game_over_bad_message;
	std::string game_over_good_message;
};