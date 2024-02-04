#include "Game1.h"
Game1* Game1::instance = nullptr;

void Game1::awake()
{
	load_config_files();
	rapidjson::Document& game_config = (*config_file_map["game.config"]);
	current_scene_name = game_config["initial_scene"].GetString();
	load_scene(current_scene_name);
}

void Game1::start() {
	rapidjson::Document& game_config = (*config_file_map["game.config"]);
	if((game_config.HasMember("game_start_message")))std::cout << game_config["game_start_message"].GetString() << std::endl;
	for (auto i : id_to_actor_map) {
		Actor& actor = *i.second;
		actor_position_map[glm::highp_ivec2{ actor.position.x,actor.position.y }].insert(actor.ID);
	}
}

void Game1::render()
{
	update_map();
	int left_index = player->position.x - camera_dimension.x/2;
	int right_index = player->position.x + camera_dimension.x / 2;
	int up_index = player->position.y - camera_dimension.y / 2;
	int down_index =player->position.y + camera_dimension.y / 2;
	for (int i = up_index; i <= down_index; i++) {
		for (int j = left_index; j <= right_index; j++) {
			if (check_out_of_bound(i, j))frame_output << ' ';
			else frame_output << render_layer[i][j];
		}
		frame_output << std::endl;
	}
}


void Game1::update()
{
	//render first frame->check dialogue of rendered frame->start to make update for next frame
	check_dialogue();

	if (game_status == GameStatus_running) {
		input();
		update_actor();
	}
	check_game_status();
	cout_frame_output();
}



void Game1::input()
{
	frame_output << "health : " << player_health << ", score : " << score << std::endl;
	frame_output << "Please make a decision..." << std::endl;
	frame_output << "Your options are \"n\", \"e\", \"s\", \"w\", \"quit\"" << std::endl;
	cout_frame_output();
	std::cin >> user_input;
	if (user_input == "quit") {
		game_status = GameStatus_quit;
	}
}

void Game1::update_map()
{
	for (int i = 0; i < HARDCODED_MAP_HEIGHT; i++) {
		for (int j = 0; j < HARDCODED_MAP_WIDTH + 1; j++) {
			render_layer[i][j] = hardcoded_map[i][j];
		}
	}
	for (auto i: id_to_actor_map) {
		Actor& actor = *i.second;
		render_layer[actor.position.y][actor.position.x] = actor.view;
	}
}

void Game1::update_actor()
{
	for (auto i: id_to_actor_map) {
		Actor& actor = *i.second;
		actor.update_position();
	}
}

void Game1::check_dialogue()
{
	for (auto& actor_ptr: id_to_actor_map) {
		Actor& actor = *actor_ptr.second;
		int diff_x = actor.position.x - player->position.x;
		int diff_y = actor.position.y - player->position.y;
		if (diff_x == 0 && diff_y == 0 && actor.contact_dialogue != "")	trigger_contact_dialogue(actor);
		else if (std::abs(diff_x) <= 1 && std::abs(diff_y) <= 1 && actor.nearby_dialogue!="") {
			trigger_nearby_dialogue(actor);
		}
	}
}

bool Game1::check_grid_accessible(int index_y, int index_x)
{
	if (check_out_of_bound(index_y, index_x)) return false;
	if (render_layer[index_y][index_x] == 'b')return false;
	if (actor_position_map.find(glm::ivec2{index_x,index_y})!=actor_position_map.end()) {
		for (auto i : actor_position_map[glm::ivec2{index_x,index_y}]) {
			Actor& actor = *id_to_actor_map[i];
			if (actor.blocking == true)return false;
		}
	}
	return true;
}


void Game1::trigger_contact_dialogue(Actor& actor)
{
	frame_output << actor.contact_dialogue << std::endl;
	if (check_substring_exist(actor.contact_dialogue,special_dialogue[0])) {
		change_player_health(-1);
	}
	else if (check_substring_exist(actor.contact_dialogue, special_dialogue[1]) && !actor.triggered_score_up) {
		score += 1;
		actor.triggered_score_up = true;
	}
	else if (check_substring_exist(actor.contact_dialogue, special_dialogue[2])) {
		game_status = GameStatus_good_ending;
	}
	else if (check_substring_exist(actor.contact_dialogue, special_dialogue[3])) {
		game_status = GameStatus_bad_ending;
	}
}

void Game1::trigger_nearby_dialogue(Actor& actor)
{
	frame_output << actor.nearby_dialogue << std::endl;
	if (check_substring_exist(actor.nearby_dialogue, special_dialogue[0])) {
		change_player_health(-1);
	}
	else if (check_substring_exist(actor.nearby_dialogue, special_dialogue[1]) && !actor.triggered_score_up) {
		score += 1;
		actor.triggered_score_up = true;
	}
	else if (check_substring_exist(actor.nearby_dialogue, special_dialogue[2])) {
		game_status = GameStatus_good_ending;
	}
	else if (check_substring_exist(actor.nearby_dialogue, special_dialogue[3])) {
		game_status = GameStatus_bad_ending;
	}
}

bool Game1::check_substring_exist(std::string& origin_string, std::string& substring)
{
	return origin_string.find(substring) != std::string::npos;
}

void Game1::check_game_status() 
{
	if (game_status != GameStatus_running) {
		if(game_status!=GameStatus_quit)frame_output << "health : " << player_health << ", score : " << score << std::endl;
		is_running = false;
	}	
	rapidjson::Document& game_config = (*config_file_map["game.config"]);
	if (game_status == GameStatus_bad_ending && game_config.HasMember("game_over_bad_message"))frame_output << game_config["game_over_bad_message"].GetString();
	if (game_status == GameStatus_good_ending && game_config.HasMember("game_over_good_message"))frame_output << game_config["game_over_good_message"].GetString();
	if (game_status == GameStatus_quit && game_config.HasMember("game_over_bad_message"))frame_output << game_config["game_over_bad_message"].GetString();
}

void Game1::change_player_health(int change)
{
	player_health += change;
	if (player_health <= 0)game_status = GameStatus_bad_ending;
}

bool Game1::check_out_of_bound(int index_y, int index_x)
{
	return index_y < 0 || index_y >= HARDCODED_MAP_HEIGHT || index_x < 0 || index_x >= HARDCODED_MAP_WIDTH;
}

bool Game1::move_actor(Actor& actor, int target_y, int target_x)
{
	if (check_grid_accessible(target_y, target_x)) {
		actor_position_map[glm::ivec2{ actor.position.x,actor.position.y }].erase(actor.ID);
		actor_position_map[glm::ivec2{ target_x,target_y }].insert(actor.ID);
		actor.position.x = target_x;
		actor.position.y = target_y;
		return true;
	}
	return false;

}

void Game1::cout_frame_output()
{
	std::cout << frame_output.str();
	frame_output.str("");
}

void Game1::load_config_files()
{
	config_files_pre_check();
	for (auto file : config_files) {
		load_config_file(file);
	}
	config_files_post_check();
	update_config_variables();
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
	if (config_file_map.find("rendering.config") != config_file_map.end()) {
		rapidjson::Document& rendering_config = *config_file_map["rendering.config"];
		if (rendering_config.HasMember("x_resolution"))camera_dimension.x = rendering_config["x_resolution"].GetInt();
		if (rendering_config.HasMember("y_resolution"))camera_dimension.y = rendering_config["y_resolution"].GetInt();
	}
	
}

void Game1::load_actors()
{
	rapidjson::Document& scene_json = (*config_file_map["scenes/" + current_scene_name + ".scene"]);
	const rapidjson::Value& actors = scene_json["actors"];
	_underlying_actor_storage.reserve(actors.Size());
	_underlying_player_storage.reserve(1);
	for (int i = 0; i < actors.Size();i++) {
		auto& actor = actors[i];
		Actor* new_actor;
		if (actor.HasMember("name") && actor["name"].GetString() == std::string("player")) {
			_underlying_player_storage.push_back(Player());
			player = &(_underlying_player_storage[_underlying_player_storage.size()-1]);
			new_actor = player;
		}
		else {
			_underlying_actor_storage.push_back(Actor());
			new_actor = &(_underlying_actor_storage[_underlying_actor_storage.size() - 1]);
		} 


		if (actor.HasMember("name"))new_actor->actor_name = actor["name"].GetString();
		if (actor.HasMember("view"))new_actor->view = actor["view"].GetString()[0];
		if (actor.HasMember("x"))new_actor->position.x = actor["x"].GetInt();  
		if (actor.HasMember("y"))new_actor->position.y = actor["y"].GetInt();
		if (actor.HasMember("vel_x"))new_actor->velocity.x = actor["vel_x"].GetInt();
		if (actor.HasMember("vel_y"))new_actor->velocity.y = actor["vel_y"].GetInt();
		if (actor.HasMember("blocking"))new_actor->blocking = actor["blocking"].GetBool();
		if (actor.HasMember("nearby_dialogue"))new_actor->nearby_dialogue = actor["nearby_dialogue"].GetString();
		if (actor.HasMember("contact_dialogue"))new_actor->contact_dialogue = actor["contact_dialogue"].GetString();
		new_actor->ID = current_id++;
		id_to_actor_map[new_actor->ID]=new_actor;
		sorted_actor_by_id.push_back(new_actor);

	}
	for (auto actor : id_to_actor_map) {
		actor_position_map[actor.second->position].insert(actor.second->ID);
	}
}

void Game1::load_scene(const std::string& scene_name)
{
	std::string relative_scene_path = "scenes/"+current_scene_name+".scene";
	if (config_file_map.find(relative_scene_path) == config_file_map.end()) {
		if (!EngineUtils::ResourceFileExist(relative_scene_path)) {
			std::cout << "error: scene " + scene_name + " is missing";
			exit(0);
		}
		load_config_file(relative_scene_path);
	}
	load_actors();
}

