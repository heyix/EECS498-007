#include "Game1.h"
Game1* Game1::instance = nullptr;
void Game1::awake()
{
	init_renderer();
	load_config_files();
	load_current_scene();
	awake_post_check();
}

void Game1::start()
{
	Engine::instance->renderer->set_clear_color(clear_color_r, clear_color_g, clear_color_b, 255);
	if(intro_bgm_name!="")AudioDB::Play_Audio(0, intro_bgm_name, -1);
}

void Game1::update()
{
	if (game_status == GameStatus_intro) {
		if ((current_intro_image_index == -1 || current_intro_image_index >= intro_images_name.size()) && (current_intro_text_index == -1 || current_intro_text_index >= intro_text.size())) {
			game_status = GameStatus_running;
			if (intro_bgm_name != "") {
				AudioDB::Halt_Audio(0);
			}
			if (gameplay_audio_name != "") {
				AudioDB::Play_Audio(0, gameplay_audio_name, -1);
			}
		}
	}

	if (game_status == GameStatus_running) {
		update_actors();
		if (current_scene->player != nullptr)current_scene->check_dialogue();
	}

	check_game_status();
}

void Game1::render()
{
	if (game_status == GameStatus_intro) {
		if(current_intro_image_index!=-1)Engine::instance->renderer->draw_image(intro_images_name[std::min(current_intro_image_index,int(intro_images_name.size()-1))], nullptr, nullptr);
		if(current_intro_text_index !=-1)Engine::instance->renderer->draw_text(font_name, intro_text[std::min(current_intro_text_index,int(intro_text.size()-1))], font_size, font_color, 25, resolution.y - 50);
	}
	else if (game_status == GameStatus_running) {
		render_actors();
		render_hud();
		render_dialogue_messages();
	}
	else if (game_status == GameStatus_bad_ending_show_image) {
		Engine::instance->renderer->draw_image(game_over_bad_image_name, nullptr, nullptr);
	}
	else if (game_status == GameStatus_good_ending_show_image) {
		Engine::instance->renderer->draw_image(game_over_good_image_name, nullptr, nullptr);
	}
	current_frame_dialogue_queue.clear();
}

void Game1::process_input()
{
	SDL_Event e;
	while (Helper::SDL_PollEvent(&e)) {
		if (e.type == SDL_QUIT) {
			is_running = false;
		}
		else {
			if (game_status == GameStatus_intro && intro_images_name.size() != 0) {
				if (e.type == SDL_KEYDOWN && (e.key.keysym.scancode == SDL_SCANCODE_SPACE || e.key.keysym.scancode == SDL_SCANCODE_RETURN)) {
					if (current_intro_image_index != -1 && current_intro_image_index < intro_images_name.size()) {
						current_intro_image_index++;
					}
					if (current_intro_text_index != -1 && current_intro_text_index < intro_text.size()) {
						current_intro_text_index++;
					}
				}
				if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
					if (current_intro_image_index != -1 && current_intro_image_index < intro_images_name.size()) {
						current_intro_image_index++;
					}
					if (current_intro_text_index != -1 && current_intro_text_index < intro_text.size()) {
						current_intro_text_index++;
					}
				}
			}
			if (game_status == GameStatus_running) {
				if (e.type == SDL_KEYDOWN) {
					switch (e.key.keysym.scancode) {
					case SDL_SCANCODE_UP:
						user_input = 'n';
						break;
					case SDL_SCANCODE_DOWN:
						user_input = 's';
						break;
					case SDL_SCANCODE_LEFT:
						user_input = 'w';
						break;
					case SDL_SCANCODE_RIGHT:
						user_input = 'e';
						break;
					}
						
				}
			}
		}

	}
}
void Game1::draw_actor(Actor& actor)
{
	SDL_RendererFlip x_flip = SDL_FLIP_NONE;
	SDL_RendererFlip y_flip = SDL_FLIP_NONE;
	if (actor.transform_scale.x < 0) {
		x_flip = SDL_FLIP_HORIZONTAL;
	}
	if (actor.transform_scale.y < 0) {
		y_flip = SDL_FLIP_VERTICAL;
	}
	SDL_RendererFlip flip = SDL_RendererFlip(x_flip | y_flip);
	std::shared_ptr<SDL_FRect> rect = std::make_shared<SDL_FRect>(SDL_FRect{
		resolution.x/2+(actor.position.x-camera_position.x)*pixel_per_unit_distance -actor.view_pivot_offset.x*glm::abs(actor.transform_scale.x),
		resolution.y/2+(actor.position.y-camera_position.y)*pixel_per_unit_distance -actor.view_pivot_offset.y*glm::abs(actor.transform_scale.y),
		actor.image_width * glm::abs(actor.transform_scale.x),
		actor.image_height * glm::abs(actor.transform_scale.y)
	});
	std::shared_ptr<SDL_FPoint> center = std::make_shared<SDL_FPoint>(SDL_FPoint{
		actor.view_pivot_offset.x,
		actor.view_pivot_offset.y
	});
	Engine::instance->renderer->draw_image(actor.ID, actor.actor_name, actor.view_image, nullptr, rect, actor.transform_rotation_degrees, center, flip);

}

void Game1::awake_post_check()
{
	if (intro_text.size() != 0 && font_name == "") {
		std::cout << "error: text render failed. No font configured";
		exit(0);
	}
	if (current_scene->player != nullptr && hp_image == "") {
		std::cout << "error: player actor requires an hp_image be defined";
		exit(0);
	}
}

void Game1::render_actors()
{
	if(current_scene->player!=nullptr)camera_position = { current_scene->player->position.x + camera_offset.x,current_scene->player->position.y + camera_offset.y };
	std::sort(current_scene->sorted_actor_by_render_order.begin(), current_scene->sorted_actor_by_render_order.end(), EngineUtils::ActorRenderOrderComparator());
	for (auto i : current_scene->sorted_actor_by_render_order) {
		Actor& actor = *i;
		draw_actor(actor);
	}
}

void Game1::render_hud()
{
	if (current_scene->player != nullptr) {
		//render health
		for (int i = 0; i < player_health; i++) {
			float width = 0;
			float height = 0;
			ImageDB::Get_Image_Resolution(hp_image, width, height);
			std::shared_ptr<SDL_FRect> dest_rect = std::make_shared<SDL_FRect>(
				SDL_FRect{
					5+i*(width+5),
					25,
					width,
					height
				}
			);
			Engine::instance->renderer->draw_image(hp_image, nullptr, dest_rect);
		}

		//render score
		Engine::instance->renderer->draw_text(font_name, "score : " + std::to_string(score), font_size, font_color, 5, 5);

	}
}

void Game1::draw_dialogue_message(const std::string& message)
{
	current_frame_dialogue_queue.push_back(message);
}

void Game1::render_dialogue_messages()
{
	int size = current_frame_dialogue_queue.size();
	for (int i = 0; i < size; i++) {
		std::string& message = current_frame_dialogue_queue.front();
		Engine::instance->renderer->draw_text(font_name, message, font_size, font_color, 25, resolution.y - 50 - 50 * (size - 1 - i));
		current_frame_dialogue_queue.pop_front();
	}
}

bool Game1::is_in_damage_cooldown()
{
	return Helper::GetFrameNumber() < last_took_damage_frame + 180;
}


void Game1::cout_frame_output()
{
	std::cout << frame_output.str();
	frame_output.str("");
}

void Game1::input()
{
	std::cin >> user_input;
	if (user_input == "quit") {
		game_status = GameStatus_quit;
	}
}

void Game1::update_actors()
{
	if (game_status != GameStatus_running)return;
	if (Helper::GetFrameNumber() % 60 != 0) {
		if(current_scene->player!=nullptr)current_scene->player->update_position();
	}
	else {
		for (auto i : current_scene->sorted_actor_by_id) {
			Actor& actor = *i;
			actor.update_position();
		}
	}
}

void Game1::check_game_status()
{
	if (game_status == GameStatus_bad_ending || game_status == GameStatus_bad_ending_show_image) {
		if (game_status == GameStatus_bad_ending) {
			if (game_over_bad_audio_name != "") {
				AudioDB::Halt_Audio(0);
				AudioDB::Play_Audio(0, game_over_bad_audio_name, 0);
			}
		}
		if (game_over_bad_image_name != "") {
			game_status = GameStatus_bad_ending_show_image;
		}
		else {
			is_running = false;
		}
	}
	else if (game_status == GameStatus_good_ending || game_status == GameStatus_good_ending_show_image) {
		if (game_status == GameStatus_good_ending) {
			if (game_over_good_audio_name != "") {
				AudioDB::Halt_Audio(0);
				AudioDB::Play_Audio(0, game_over_good_audio_name, 0);
			}
		}
		if (game_over_good_image_name != "") {
			game_status = GameStatus_good_ending_show_image;
		}
		else {
			is_running = false;
		}
	}
	else if (game_status != GameStatus_running && game_status != GameStatus_intro) {
		is_running = false;
	}
	if (game_status == GameStatus_changing_scene) {
		load_current_scene();
		change_game_status(GameStatus_running);
		is_running = true;
	}
}


void Game1::print_turn_info()
{
	frame_output << "health : " << player_health << ", score : " << score << std::endl;
	frame_output << "Please make a decision..." << std::endl;
	frame_output << "Your options are \"n\", \"e\", \"s\", \"w\", \"quit\"" << std::endl;
	cout_frame_output();
}


void Game1::change_player_health(int change)
{
	if (change > 0) {
		player_health += change;
	}
	else {
		if (is_in_damage_cooldown()) {
			return;
		}
		player_health += change;
		last_took_damage_frame = Helper::GetFrameNumber();
		if (player_health <= 0) {
			game_status = GameStatus_bad_ending;
		}
	}

}

void Game1::change_score(int change)
{
	score += change;
}

void Game1::load_config_files()
{
	pre_check_config_files();
	for (auto& file_name : config_files_name) {
		load_config_file(file_name);
	}
	post_check_config_files();
	after_config_files_loaded();
	clear_config_data();
}

void Game1::pre_check_config_files()
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

void Game1::load_config_file(const std::string& file_path)
{
	if (!std::filesystem::exists(EngineUtils::Get_Resource_File_Path(file_path)))return;
	config_file_map[file_path] = std::make_unique<rapidjson::Document>();
	EngineUtils::Read_Json_File(EngineUtils::Get_Resource_File_Path(file_path), *config_file_map[file_path]);
}

void Game1::after_config_files_loaded()
{
	if (auto it = config_file_map.find("rendering.config"); it != config_file_map.end()) {
		rapidjson::Document& rendering_config = *it->second;
		if (auto it_x = rendering_config.FindMember("x_resolution"); it_x != rendering_config.MemberEnd()) {
			camera_dimension.x = it_x->value.GetInt();
			resolution.x = it_x->value.GetInt();
		}
		if (auto it_y = rendering_config.FindMember("y_resolution"); it_y != rendering_config.MemberEnd()) {
			camera_dimension.y = it_y->value.GetInt();
			resolution.y = it_y->value.GetInt();
		}
		if (auto it = rendering_config.FindMember("clear_color_r"); it != rendering_config.MemberEnd()) {
			clear_color_r = it->value.GetInt();
		}
		if (auto it = rendering_config.FindMember("clear_color_g"); it != rendering_config.MemberEnd()) {
			clear_color_g = it->value.GetInt();
		}
		if (auto it = rendering_config.FindMember("clear_color_b"); it != rendering_config.MemberEnd()) {
			clear_color_b = it->value.GetInt();
		}
		if (auto it = rendering_config.FindMember("cam_offset_x"); it != rendering_config.MemberEnd()) {
			camera_offset.x = it->value.GetFloat();
		}
		if (auto it = rendering_config.FindMember("cam_offset_y"); it != rendering_config.MemberEnd()) {
			camera_offset.y = it->value.GetFloat();
		}
		
	}



	rapidjson::Document& game_config = *config_file_map["game.config"];
	if (auto it = game_config.FindMember("game_over_bad_message"); it != game_config.MemberEnd())game_over_bad_message = it->value.GetString();
	if (auto it = game_config.FindMember("game_over_good_message"); it != game_config.MemberEnd())game_over_good_message = it->value.GetString();
	if (auto it = game_config.FindMember("game_start_message"); it != game_config.MemberEnd())game_start_message = it->value.GetString();
	if (auto it = game_config.FindMember("game_title"); it != game_config.MemberEnd())game_title = it->value.GetString();
	
	if (auto it = game_config.FindMember("intro_image"); it != game_config.MemberEnd()) {
		for (auto& image : it->value.GetArray()) {
			intro_images_name.push_back(image.GetString());
		}
		if (intro_images_name.size() != 0)current_intro_image_index = 0;
	}

	if (auto it = game_config.FindMember("intro_text"); it != game_config.MemberEnd()) {
		for (auto& text : it->value.GetArray()) {
			intro_text.push_back(text.GetString());
		}
		if (intro_text.size() != 0)current_intro_text_index = 0;
	}
	if (auto it = game_config.FindMember("font"); it != game_config.MemberEnd()) {
		font_name = it->value.GetString();
	}

	if (auto it = game_config.FindMember("intro_bgm"); it != game_config.MemberEnd()) {
		intro_bgm_name = it->value.GetString();
	}
	if (auto it = game_config.FindMember("gameplay_audio"); it != game_config.MemberEnd()) {
		gameplay_audio_name = it->value.GetString();
	}
	if (auto it = game_config.FindMember("hp_image"); it != game_config.MemberEnd()) {
		hp_image = it->value.GetString();
	}
	if (auto it = game_config.FindMember("game_over_bad_image"); it != game_config.MemberEnd()) {
		game_over_bad_image_name = it->value.GetString();
	}
	if (auto it = game_config.FindMember("game_over_bad_audio"); it != game_config.MemberEnd()) {
		game_over_bad_audio_name = it->value.GetString();
	}
	if (auto it = game_config.FindMember("game_over_good_image"); it != game_config.MemberEnd()) {
		game_over_good_image_name = it->value.GetString();
	}
	if (auto it = game_config.FindMember("game_over_good_audio"); it != game_config.MemberEnd()) {
		game_over_good_audio_name = it->value.GetString();
	}

	current_scene_name = game_config["initial_scene"].GetString();
}

void Game1::load_current_scene()
{
	std::string relative_scene_path = "scenes/" + current_scene_name + ".scene";
	if (!EngineUtils::Resource_File_Exist(relative_scene_path)) {
		cout_frame_output();
		std::cout << "error: scene " + current_scene_name + " is missing";
		exit(0);
	}
	rapidjson::Document out_document;
	EngineUtils::Read_Json_File(EngineUtils::Get_Resource_File_Path(relative_scene_path), out_document);
	current_scene = std::make_unique<Scene>();
	current_scene->load_actors(out_document);
	current_frame_dialogue_queue = std::deque<std::string>();
}

void Game1::post_check_config_files()
{
	if (!config_file_map["game.config"]->HasMember("initial_scene")) {
		std::cout << "error: initial_scene unspecified";
		exit(0);
	}
}

void Game1::clear_config_data()
{
	config_file_map.clear();
}

void Game1::init_renderer()
{
	Engine::instance->renderer->init_renderer(game_title.c_str(), 150, 150, resolution.x, resolution.y, -1, 0, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
}



bool Game1::move_actor(Actor& actor, int target_y, int target_x)
{
	return current_scene->move_actor(actor, target_y, target_x);
}

void Game1::change_game_status(GameStatus new_satus)
{
	game_status = new_satus;
}

void Game1::change_current_scene(const std::string& new_scene_name)
{
	game_status = GameStatus_changing_scene;
	current_scene_name = new_scene_name;
}
 