#include "GameData.h"
#include "EngineUtils.h"
void GameData::Init_Game_Data()
{
	load_config_files();
}
void GameData::load_config_files()
{
	pre_check_config_files();
	for (auto& file_name : config_files_name) {
		load_config_file(file_name);
	}
	post_check_config_files();
	after_config_files_loaded();
	clear_config_data();
}

void GameData::pre_check_config_files()
{
	if (!EngineUtils::Resource_File_Exist("")) {
		std::cout << "error: resources/ missing";
		exit(0);
	}
	if (!EngineUtils::Resource_File_Exist("game.config")) {
		std::cout << "error: resources/game.config missing";
		exit(0);
	}
}

void GameData::load_config_file(const std::string& file_path)
{
	if (!std::filesystem::exists(EngineUtils::Get_Resource_File_Path(file_path)))return;
	config_file_map[file_path] = std::make_unique<rapidjson::Document>();
	EngineUtils::Read_Json_File(EngineUtils::Get_Resource_File_Path(file_path), *config_file_map[file_path]);
}

void GameData::after_config_files_loaded()
{
	if (auto it = config_file_map.find("rendering.config"); it != config_file_map.end()) {
		rapidjson::Document& rendering_config = *it->second;
		rendering_config_data.set_rendering_config_data(rendering_config);
	}

	rapidjson::Document& game_config = *config_file_map["game.config"];
	game_config_data.set_game_config_data(game_config);
	//if (game_config_data.intro_images_name.size() != 0)current_intro_image_index = 0;
	//if (game_config_data.intro_text.size() != 0)current_intro_text_index = 0;


}
void GameData::post_check_config_files()
{
	if (!config_file_map["game.config"]->HasMember("initial_scene")) {
		std::cout << "error: initial_scene unspecified";
		exit(0);
	}
}

void GameData::clear_config_data()
{
	config_file_map.clear();
}
