#pragma once
#include "Engine.h"
#include <string>
#include "glm/glm.hpp"
#include "MapHelper.h"
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
enum GameStatus
{
	GameStatus_running,GameStatus_good_ending,GameStatus_bad_ending,GameStatus_quit
};

class Game1:public Engine
{
	struct HashIvec2
	{
		size_t operator()(const glm::ivec2 & k)const
		{
			int32_t ux = static_cast<uint32_t>(k.x);
			int32_t uy = static_cast<uint32_t>(k.y);
			int64_t result = static_cast<int64_t>(ux);
			result = result << 32;
			result = result | static_cast<int64_t>(uy);
			return std::hash<int64_t>()(result);
		}

		bool operator()(const glm::ivec2& a, const glm::ivec2& b)const
		{
			return a.x == b.x && a.y == b.y;
		}
	};
	struct ActorPointerComparator {
		bool operator()(const Actor* const& a,const Actor* const& b) const {
			return a->ID < b->ID;
		}
	};
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
	void update_map();
	void update_actor();
	void check_dialogue();
	void trigger_contact_dialogue(Actor& actor);
	void trigger_nearby_dialogue(Actor& actor);
	bool check_substring_exist(std::string& origin_string, std::string& substring);
	void check_game_status();
	void change_player_health(int change);
	bool check_out_of_bound(int index_y, int index_x);
	void cout_frame_output();
	void load_config_files();
	void load_config_file(const std::string& file_path);
	void config_files_pre_check();
	void config_files_post_check();
	void update_config_variables();
	void load_actors();
	void load_scene(const std::string& scene_name);
public:
	bool check_grid_accessible(int index_y, int index_x);
	bool move_actor(Actor& actor, int target_y, int target_x);

public:
	std::string user_input = "";
	int score = 0;
	int player_health = 3;
	Player* player;
	static Game1* instance;
	std::map<int,Actor*> id_to_actor_map;
	std::unordered_map<glm::ivec2, std::set<int>, HashIvec2, HashIvec2> actor_position_map;
	std::vector<Actor*> sorted_actor_by_id;

private:
	char render_layer[HARDCODED_MAP_HEIGHT][HARDCODED_MAP_WIDTH + 1];
	std::vector<std::string> special_dialogue{ "health down","score up","you win","game over" };
	GameStatus game_status = GameStatus_running;
	std::stringstream frame_output;
	std::unordered_map<std::string, std::unique_ptr<rapidjson::Document>> config_file_map;
	std::vector<std::string> config_files{ "game.config","rendering.config"};
	glm::ivec2 camera_dimension{13,9};
	glm::ivec2 camera_position{ 19,15 };
	std::string current_scene_name;
	int current_id = 0;
	std::vector<Actor> _underlying_actor_storage;
	std::vector<Player> _underlying_player_storage;
};