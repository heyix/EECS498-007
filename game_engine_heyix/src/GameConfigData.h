#pragma once
#include <string>
#include <vector>
#include "rapidjson/include/rapidjson/document.h"
class GameConfigData {
public:
	//std::string game_start_message;
	//std::string game_over_bad_message;
	//std::string game_over_good_message;
	//std::string game_over_bad_image_name;
	std::string game_title;
	//std::string game_over_bad_audio_name;
	//std::string game_over_good_image_name;
	//std::string game_over_good_audio_name;
	//std::string font_name;
	//std::string intro_bgm_name;
	//std::string gameplay_audio_name;
	//std::string hp_image;
	//std::vector<std::string> intro_images_name;
	//std::vector<std::string> intro_text;
	//std::string score_sfx;
	std::string initial_scene_name;
public:
	void set_game_config_data(rapidjson::Document& game_config);

};