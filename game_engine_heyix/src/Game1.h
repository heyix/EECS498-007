#pragma once
#include "Game.h"
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include "rapidjson/include/rapidjson/document.h"
#include "EngineUtils.h"
#include <unordered_map>
#include <algorithm>
#include "Scene.h" 
#include "Engine.h"
#include <optional>
#include "SDL2_mixer/SDL_mixer.h"
#include "SDL2_image/SDL_image.h"
#include <deque>
enum GameStatus {
	GameStatus_running, GameStatus_good_ending, GameStatus_bad_ending, GameStatus_quit, GameStatus_changing_scene,GameStatus_intro,GameStatus_bad_ending_show_image,GameStatus_good_ending_show_image
};
class Game1 :public Game {
public:

public:
	Game1() {
		instance = this;
	};
protected:
	void awake() override;
	void start() override;
	void update() override;
	void render() override;
	void process_input() override;

private:
	void cout_frame_output();
	void input();
	void update_actors();
	void check_game_status();
	void print_turn_info();
	void load_config_files();
	void pre_check_config_files();
	void load_config_file(const std::string& file_path);
	void after_config_files_loaded();
	void load_current_scene();
	void post_check_config_files();
	void clear_config_data();
	void init_renderer();
	void draw_actor(Actor& actor);
	void awake_post_check();
	void render_actors();
	void render_hud();
public:
	bool move_actor(Actor& actor, int target_y, int target_x);
	void change_game_status(GameStatus new_satus);
	void change_current_scene(const std::string& new_scene_name);
	void change_player_health(int change);
	void change_score(int change);
	void draw_dialogue_message(const std::string& message);
	void render_dialogue_messages();
	bool is_in_damage_cooldown();
public:
	int score = 0;
	int player_health = 3;
	std::string user_input = "";
	static Game1* instance;

private:
	glm::ivec2 camera_dimension{ 13,9 };
	glm::ivec2 resolution{ 640,360 };
	glm::vec2 camera_offset{ 0,0 };
	glm::vec2 camera_position{0,0};
	std::stringstream frame_output;
	GameStatus game_status = GameStatus_intro;
	std::string game_start_message;
	std::string game_over_bad_message;
	std::string game_over_good_message;
	std::vector<std::string> config_files_name{ "game.config","rendering.config" };
	std::unordered_map<std::string, std::unique_ptr<rapidjson::Document>> config_file_map;
	std::string current_scene_name;
	std::unique_ptr<Scene> current_scene;
	std::vector<std::string> intro_images_name;
	std::vector<std::string> intro_text;
	std::string game_over_bad_image_name;
	std::string game_title;
	std::string game_over_bad_audio_name;
	std::string game_over_good_image_name;
	std::string game_over_good_audio_name;
	int clear_color_r = 255;
	int clear_color_g = 255;
	int clear_color_b = 255;
	int current_intro_image_index = -1;
	int current_intro_text_index = -1;
	std::string font_name;
	std::string intro_bgm_name;
	std::string gameplay_audio_name;
	std::string hp_image;
	int font_size = 16;
	SDL_Color font_color{ 255,255,255,255 };
	Mix_Chunk* intro_bgm = nullptr;
	int pixel_per_unit_distance = 100;
	std::deque<std::string> current_frame_dialogue_queue;
	int last_took_damage_frame = -180;
};
