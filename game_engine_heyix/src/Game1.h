#pragma once
#include "Engine.h"
#include "MapHelper.h"
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <string>

class Game1 :public Engine {
public:
	enum GameStatus {
		GameStatus_running,GameStatus_good_ending,GameStatus_bad_ending,GameStatus_quit
	};
public:
	Game1();
protected:
	void awake() override;
	void start() override;
	void update() override;
	void render() override;

private:
	void prepare_rendering();
	void cout_frame_output();
	void input();
	void update_actor();
	void check_game_status();
	bool check_grid_accessible(int y, int x);
	void print_turn_info();
	void check_dialogue();
	void trigger_contact_dialogue(Actor& actor);
	void trigger_nearby_dialogue(Actor& actor);
	void move_actor(int actor_index, int target_y, int target_x);
	bool check_substring_exist(std::string& origin_string, std::string& substring);
	void check_special_dialogue(std::string& origin_string,Actor& actor);
	void change_player_health(int change);
	bool check_out_of_bound(int y, int x);
public:
	int score = 0;
	int player_health = 3;
	glm::ivec2 camera_position{ 19,15 };
	std::string user_input = "";
	Actor player{
		"player",			// actor name
		'p',				// view
		glm::ivec2(19, 15),	// starting position
		glm::ivec2(0, 0),	// starting velocity
		false,				// blocking?
		"",					// nearby dialogue
		""					// made-contact dialogue

	};

private:
	char render_layer[HARDCODED_MAP_HEIGHT][HARDCODED_MAP_WIDTH + 1];
	std::vector<std::vector<std::set<int>>> actor_layer;
	std::stringstream frame_output;
	GameStatus game_status = GameStatus_running;
	std::vector<std::string> special_dialogue{ "health down","score up","you win","game over" };
};