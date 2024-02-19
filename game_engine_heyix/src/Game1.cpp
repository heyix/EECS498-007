#include "Game1.h"
Game1* Game1::instance = nullptr;
void Game1::awake()
{
	load_config_files();
	load_current_scene();
}

void Game1::start() {
	if(game_start_message!="")std::cout << game_start_message << std::endl;
	render();
}

void Game1::render()
{
	Player* player = current_scene->player;
	auto& actor_position_map = current_scene->actor_position_map;
	int left_index = player->position.x - camera_dimension.x / 2;
	int right_index = player->position.x + camera_dimension.x / 2;
	int up_index = player->position.y - camera_dimension.y / 2;
	int down_index =player->position.y + camera_dimension.y / 2;
	for (int i = up_index; i <= down_index; i++) {
		for (int j = left_index; j <= right_index; j++) {
			auto pos_key = EngineUtils::create_composite_key(j, i);
			if (auto it = actor_position_map.find(pos_key); it!= actor_position_map.end()) {
				auto& actor_set = it->second;
				frame_output << (*actor_set.rbegin())->view;
			}
			else frame_output << ' ';
		}
		frame_output << std::endl;
	} 
	current_scene->check_dialogue(frame_output);
}

 
void Game1::update()
{
	//render first frame->check dialogue of rendered frame->start to make update for next frame
	input();
	update_actor();
	check_game_status();
	cout_frame_output();
}
 


void Game1::input()
{
	if (game_status != GameStatus_running)return;
	frame_output << "health : " << player_health << ", score : " << score << std::endl;
	frame_output << "Please make a decision..." << std::endl;
	frame_output << "Your options are \"n\", \"e\", \"s\", \"w\", \"quit\"" << std::endl;
	cout_frame_output();
	std::cin >> user_input;
	if (user_input == "quit") {
		game_status = GameStatus_quit;
	}
}


void Game1::update_actor()
{
	if (game_status != GameStatus_running)return;
	for (auto i: current_scene->sorted_actor_by_id) {
		Actor& actor = *i;
		actor.update_position();
	}
}



void Game1::check_game_status() 
{
	if (game_status != GameStatus_running) {
		if(game_status!=GameStatus_quit)frame_output << "health : " << player_health << ", score : " << score << std::endl;
		is_running = false;
	}	
	if (game_status == GameStatus_bad_ending && game_over_bad_message!="")frame_output << game_over_bad_message;
	if (game_status == GameStatus_good_ending && game_over_good_message!="")frame_output << game_over_good_message;
	if (game_status == GameStatus_quit && game_over_bad_message!="")frame_output << game_over_bad_message;
	if (game_status == GameStatus_changing_scene) {
		load_current_scene();
		change_game_status(GameStatus_running);
		is_running = true;
	}
}



bool Game1::move_actor(Actor& actor, int target_y, int target_x)
{
	return current_scene->move_actor(actor, target_y, target_x);
}

void Game1::cout_frame_output()
{
	std::cout << frame_output.str();
	frame_output.str("");
}

void Game1::load_config_files()
{
	config_files_pre_check();
	for (auto& file : config_files) {
		load_config_file(file);
	}
	config_files_post_check();
	update_config_variables();
	clear_config_data();
}

void Game1::clear_config_data()
{
	config_file_map.clear();
}

void Game1::load_config_file(const std::string& file_path)
{
	if (!std::filesystem::exists(EngineUtils::GetResourceFilePath(file_path)))return;
	std::unique_ptr out_document = std::make_unique<rapidjson::Document>();
	EngineUtils::ReadJsonFile(EngineUtils::GetResourceFilePath(file_path), *out_document);
	config_file_map[file_path] = std::move(out_document);
}

void Game1::config_files_pre_check()
{
	if (!EngineUtils::ResourceFileExist("")) { 
		std::cout << "error: resources/ missing";
		exit(0);
	}
	if (!EngineUtils::ResourceFileExist("game.config")) {
		std::cout << "error: resources/game.config missing";
		exit(0);
	}
}

void Game1::config_files_post_check()
{
	if (!(*config_file_map["game.config"]).HasMember("initial_scene")) {
		std::cout << "error: initial_scene unspecified";
		exit(0);
	}
}

void Game1::update_config_variables()
{
	if (auto it_render = config_file_map.find("rendering.config");it_render!= config_file_map.end()) {
		rapidjson::Document& rendering_config = *it_render->second;
		if (auto it_x = rendering_config.FindMember("x_resolution");it_x!=rendering_config.MemberEnd())camera_dimension.x = it_x->value.GetInt();
		if (auto it_y = rendering_config.FindMember("y_resolution"); it_y != rendering_config.MemberEnd())camera_dimension.y = it_y->value.GetInt();
	}
	rapidjson::Document& game_config = (*config_file_map["game.config"]);
	if (auto it = game_config.FindMember("game_over_bad_message"); it!=game_config.MemberEnd())game_over_bad_message = it->value.GetString();
	if (auto it = game_config.FindMember("game_over_good_message"); it != game_config.MemberEnd())game_over_good_message = it->value.GetString();
	if (auto it = game_config.FindMember("game_start_message"); it != game_config.MemberEnd())game_start_message = it->value.GetString();

	current_scene_name = game_config["initial_scene"].GetString();
}


void Game1::load_current_scene()
{
	std::string relative_scene_path = "scenes/" + current_scene_name + ".scene";
	if (!EngineUtils::ResourceFileExist(relative_scene_path)) {
		cout_frame_output();
		std::cout << "error: scene " + current_scene_name + " is missing";
		exit(0);
	}
	rapidjson::Document out_document;
	EngineUtils::ReadJsonFile(EngineUtils::GetResourceFilePath(relative_scene_path), out_document);
	current_scene = std::make_unique<Scene>();
	current_scene->load_actors(out_document);
}

void Game1::change_game_status(GameStatus new_status) {
	game_status = new_status;
}

void Game1::change_current_scene(const std::string& new_scene_name)
{
	game_status = GameStatus_changing_scene;
	current_scene_name = new_scene_name;
}

void Game1::change_player_health(int change)
{
	player_health += change;
	if (player_health <= 0) {
		change_game_status(GameStatus_bad_ending);
	}
}

void Game1::change_score(int change)
{
	score += change;
}
