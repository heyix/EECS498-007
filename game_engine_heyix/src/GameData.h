#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include "GameConfigData.h"
#include "RenderingConfigData.h"
class GameData {
private:
	void load_config_files();
	void pre_check_config_files();
	void load_config_file(const std::string& file_path);
	void after_config_files_loaded();
	void post_check_config_files();
	void clear_config_data();
public:
	std::vector<std::string> config_files_name{ "game.config","rendering.config" };
	std::unordered_map<std::string, std::unique_ptr<rapidjson::Document>> config_file_map;
	GameConfigData game_config_data;
	RenderingConfigData rendering_config_data;
public:
	void Init_Game_Data();
};