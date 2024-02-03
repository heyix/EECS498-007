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

enum GameStatus
{
	GameStatus_running,GameStatus_good_ending,GameStatus_bad_ending,GameStatus_quit
};

class Game1:public Engine
{
public:
	Game1() {
		actor_layer = std::make_unique<std::unique_ptr<std::set<int>[]>[]>(HARDCODED_MAP_HEIGHT);
		for (int i = 0; i < HARDCODED_MAP_HEIGHT; i++) {
			actor_layer[i] = std::make_unique<std::set<int>[]>(HARDCODED_MAP_WIDTH + 1);
		}
	}
protected:
	void start() override;
	void update() override;
	void render() override;
private:
	void input();
	void update_map();
	void update_actor();
	void check_dialogue();
	bool check_grid_accessible(int index_y, int index_x);
	void trigger_contact_dialogue(Actor& actor);
	void trigger_nearby_dialogue(Actor& actor);
	bool check_substring_exist(std::string& origin_string, std::string& substring);
	void check_game_status();
	void change_player_health(int change);
	bool check_out_of_bound(int index_y, int index_x);
	void move_actor(int actor_index,int target_y, int target_x);
	void cout_frame_output();
public:
	std::string user_input = "";
	glm::ivec2 camera_position{ 19,15 };
	int score = 0;
	int player_health = 3;
	Actor player{
		"player",			// actor name
		'p',				// view
		glm::ivec2(19, 15),	// starting velocity
		glm::ivec2(0, 0),	// starting velocity
		false,				// blocking?
		"",					// nearby dialogue
		""					// made-contact dialogue
	};
private:
	char render_layer[HARDCODED_MAP_HEIGHT][HARDCODED_MAP_WIDTH + 1];
	std::unique_ptr<std::unique_ptr<std::set<int>[]>[]> actor_layer;
	std::vector<std::string> special_dialogue{ "health down","score up","you win","game over" };
	GameStatus game_status = GameStatus_running;
	std::stringstream frame_output;
};