#include "GameConfigData.h"

void GameConfigData::set_game_config_data(rapidjson::Document& game_config)
{
	//if (auto it = game_config.FindMember("game_over_bad_message"); it != game_config.MemberEnd())game_over_bad_message = it->value.GetString();
	//if (auto it = game_config.FindMember("game_over_good_message"); it != game_config.MemberEnd())game_over_good_message = it->value.GetString();
	//if (auto it = game_config.FindMember("game_start_message"); it != game_config.MemberEnd())game_start_message = it->value.GetString();
	//if (auto it = game_config.FindMember("game_title"); it != game_config.MemberEnd())game_title = it->value.GetString();

	//if (auto it = game_config.FindMember("intro_image"); it != game_config.MemberEnd()) {
	//	for (auto& image : it->value.GetArray()) {
	//		intro_images_name.push_back(image.GetString());
	//	}
	//}

	//if (auto it = game_config.FindMember("intro_text"); it != game_config.MemberEnd()) {
	//	for (auto& text : it->value.GetArray()) {
	//		intro_text.push_back(text.GetString());
	//	}
	//}
	//if (auto it = game_config.FindMember("font"); it != game_config.MemberEnd()) {
	//	font_name = it->value.GetString();
	//}

	//if (auto it = game_config.FindMember("intro_bgm"); it != game_config.MemberEnd()) {
	//	intro_bgm_name = it->value.GetString();
	//}
	//if (auto it = game_config.FindMember("gameplay_audio"); it != game_config.MemberEnd()) {
	//	gameplay_audio_name = it->value.GetString();
	//}
	//if (auto it = game_config.FindMember("hp_image"); it != game_config.MemberEnd()) {
	//	hp_image = it->value.GetString();
	//}
	//if (auto it = game_config.FindMember("game_over_bad_image"); it != game_config.MemberEnd()) {
	//	game_over_bad_image_name = it->value.GetString();
	//}
	//if (auto it = game_config.FindMember("game_over_bad_audio"); it != game_config.MemberEnd()) {
	//	game_over_bad_audio_name = it->value.GetString();
	//}
	//if (auto it = game_config.FindMember("game_over_good_image"); it != game_config.MemberEnd()) {
	//	game_over_good_image_name = it->value.GetString();
	//}
	//if (auto it = game_config.FindMember("game_over_good_audio"); it != game_config.MemberEnd()) {
	//	game_over_good_audio_name = it->value.GetString();
	//}
	//if (auto it = game_config.FindMember("player_movement_speed"); it != game_config.MemberEnd()) {
	//	player_movement_speed = it->value.GetFloat();
	//}
	//if (auto it = game_config.FindMember("score_sfx"); it != game_config.MemberEnd()) {
	//	score_sfx = it->value.GetString();
	//}
	if (auto it = game_config.FindMember("initial_scene"); it != game_config.MemberEnd()) {
		initial_scene_name = it->value.GetString();
	}
}
