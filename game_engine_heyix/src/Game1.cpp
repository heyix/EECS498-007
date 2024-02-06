#include "Game1.h"
Game1* Game1::instance = nullptr;
void Game1::awake()
{
	load_config_files();
	load_current_scene();
}

void Game1::start() {
	if(game_start_message!="")std::cout << game_start_message << std::endl;

}

void Game1::render()
{
	for (int i = 0; i < camera_dimension.y; i++) {
		for (int j = 0; j < camera_dimension.x; j++) {
			frame_output << current_scene->render_layer[i][j];
		}
		frame_output << std::endl;
	} 
	reinitialize_render_layer();
}
void Game1::reinitialize_render_layer()
{
	current_scene->render_layer = std::vector<std::vector<char>>(camera_dimension.y, std::vector<char>(camera_dimension.x, ' '));;
}




void Game1::update_camera_range()
{ 
	Player* player = current_scene->player;
	camera_left_index = player->position.x - camera_dimension.x / 2;
	camera_right_index = player->position.x + camera_dimension.x / 2;
	camera_up_index = player->position.y - camera_dimension.y / 2;
	camera_down_index = player->position.y + camera_dimension.y / 2;
}

void Game1::update()
{
	//render first frame->check dialogue of rendered frame->start to make update for next frame
	//scene initialize->initial state->dialogue->update(new state)->dialogue->....
	current_scene->check_dialogue(frame_output); 
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


void Game1::update_actor()
{
	for (auto i: current_scene->sorted_actor_by_id) {
		Actor& actor = *i;
		actor.update_position();
	}
	render_actor_and_detect_dialogue();
}
void Game1::render_actor_and_detect_dialogue()
{
	update_camera_range();
	for (auto i : current_scene->sorted_actor_by_id) {
		Actor& actor = *i;
		render_actor_to_camera(actor);
		if (std::abs(actor.position.x - current_scene->player->position.x) <= 1 && std::abs(actor.position.y - current_scene->player->position.y) <= 1)current_scene->dialogue_list.emplace_back(&actor);
	}
}
void Game1::render_actor_to_camera(Actor& actor)
{
	if (!check_actor_inside_camera(actor))return;
	current_scene->render_layer[actor.position.y - camera_up_index][actor.position.x - camera_left_index] = actor.view;
}
bool Game1::check_actor_inside_camera(Actor& actor)
{
	return actor.position.x >= camera_left_index && actor.position.x <= camera_right_index && actor.position.y >= camera_up_index && actor.position.y <= camera_down_index;
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
	for (auto file : config_files) {
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
	if (auto element = config_file_map.find("rendering.config"); element != config_file_map.end()) {
		rapidjson::Document& rendering_config = *element->second;
		if (rendering_config.HasMember("x_resolution"))camera_dimension.x = rendering_config["x_resolution"].GetInt();
		if (rendering_config.HasMember("y_resolution"))camera_dimension.y = rendering_config["y_resolution"].GetInt();
	}
	rapidjson::Document& game_config = (*config_file_map["game.config"]);
	if (game_config.HasMember("game_over_bad_message"))game_over_bad_message = game_config["game_over_bad_message"].GetString();
	if (game_config.HasMember("game_over_good_message"))game_over_good_message = game_config["game_over_good_message"].GetString();
	if (game_config.HasMember("game_start_message"))game_start_message = game_config["game_start_message"].GetString();

	current_scene_name = game_config["initial_scene"].GetString();
}


void Game1::load_current_scene()
{
	std::string relative_scene_path = "scenes/"+current_scene_name+".scene";
	if (config_file_map.find(relative_scene_path) == config_file_map.end()) {
		if (!EngineUtils::ResourceFileExist(relative_scene_path)) {
			cout_frame_output();
			std::cout << "error: scene " + current_scene_name + " is missing";
			exit(0);
		}
		load_config_file(relative_scene_path);
	}
	current_scene = std::make_unique<Scene>();
	rapidjson::Document& scene_json = (*config_file_map[relative_scene_path]);
	current_scene->load_actors(scene_json);
	config_file_map.erase(relative_scene_path);
	reinitialize_render_layer();
	render_actor_and_detect_dialogue();
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
