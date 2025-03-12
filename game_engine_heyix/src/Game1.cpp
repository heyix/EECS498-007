#include "Game1.h"
#include "EngineUtils.h"
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
	for (auto actor : current_scene->sorted_actor_by_id) {
		actor->On_Start();
	}
	AudioHelper::Mix_AllocateChannels(50);
	Engine::instance->renderer->set_clear_color(rendering_config_data.clear_color_r, rendering_config_data.clear_color_g, rendering_config_data.clear_color_b, 255);
	if(game_config_data.intro_bgm_name!="")AudioDB::Play_Audio(0, game_config_data.intro_bgm_name, -1);
	if (current_scene->player != nullptr) {
		camera_position = { current_scene->player->position.x + rendering_config_data.camera_offset.x,current_scene->player->position.y + rendering_config_data.camera_offset.y };
	}
}

void Game1::update()
{
	handle_input();
	if (game_status == GameStatus_intro) {
		if ((current_intro_image_index == -1 || current_intro_image_index >= game_config_data.intro_images_name.size()) && (current_intro_text_index == -1 || current_intro_text_index >= game_config_data.intro_text.size())) {
			game_status = GameStatus_running;
			if (game_config_data.intro_bgm_name != "") {
				AudioDB::Halt_Audio(0);
			}
			if (game_config_data.gameplay_audio_name != "") {
				AudioDB::Play_Audio(0, game_config_data.gameplay_audio_name, -1);
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
	//if (game_status == GameStatus_intro) {
	//	if(current_intro_image_index!=-1)Engine::instance->renderer->draw_image(game_config_data.intro_images_name[std::min(current_intro_image_index,int(game_config_data.intro_images_name.size()-1))], nullptr, nullptr);
	//	if(current_intro_text_index !=-1)Engine::instance->renderer->draw_text(game_config_data.font_name, game_config_data.intro_text[std::min(current_intro_text_index,int(game_config_data.intro_text.size()-1))], font_size, font_color, 25, rendering_config_data.resolution.y - 50);
	//}
	//else if (game_status == GameStatus_running) {
	//	render_actors();
	//	render_hud();
	//	render_dialogue_messages(); 
	//}
	//else if (game_status == GameStatus_bad_ending_show_image) {
	//	Engine::instance->renderer->draw_image(game_config_data.game_over_bad_image_name, nullptr, nullptr);
	//}
	//else if (game_status == GameStatus_good_ending_show_image) {
	//	Engine::instance->renderer->draw_image(game_config_data.game_over_good_image_name, nullptr, nullptr);
	//}
	//current_frame_dialogue_queue.clear();
	//gizmo_draw_colliders();
	//gizmo_draw_triggers();
}

void Game1::handle_input()
{
	if (game_status == GameStatus_intro && game_config_data.intro_images_name.size() != 0) {
		if (Input::GetKey(SDL_SCANCODE_SPACE) || Input::GetKey(SDL_SCANCODE_RETURN)) {
			if (current_intro_image_index != -1 && current_intro_image_index < game_config_data.intro_images_name.size()) {
				current_intro_image_index++;
			}
			if (current_intro_text_index != -1 && current_intro_text_index < game_config_data.intro_text.size()) {
				current_intro_text_index++;
			}
		}
		if (Input::GetMouseButtonDown(SDL_BUTTON_LEFT)) {
			if (current_intro_image_index != -1 && current_intro_image_index < game_config_data.intro_images_name.size()) {
				current_intro_image_index++;
			}
			if (current_intro_text_index != -1 && current_intro_text_index < game_config_data.intro_text.size()) {
				current_intro_text_index++;
			}
		}
	}
}
void Game1::update_camera()
{
	if (current_scene->player != nullptr) {
		glm::vec2 new_position = { current_scene->player->position.x + rendering_config_data.camera_offset.x,current_scene->player->position.y + rendering_config_data.camera_offset.y };
		camera_position = glm::mix(camera_position, new_position, rendering_config_data.cam_ease_factor);
	}
}

void Game1::gizmo_draw_colliders()
{
	for (std::shared_ptr<Actor> a : current_scene->sorted_actor_by_id)
	{
		Actor& actor = *a;
		SDL_FRect rect;

		// Draws this actor's collider
		if (actor.box_collider.has_value())
		{
			// Use the same screen position as the actor's sprite
			rect.x = rendering_config_data.resolution.x / 2 / rendering_config_data.zoom_factor + (actor.position.x - camera_position.x) * pixel_per_unit_distance - actor.box_collider.value().x * pixel_per_unit_distance/2;
			rect.y = rendering_config_data.resolution.y / 2 / rendering_config_data.zoom_factor + (actor.position.y - camera_position.y) * pixel_per_unit_distance - actor.box_collider.value().y * pixel_per_unit_distance/2;
			rect.w = actor.box_collider.value().x * pixel_per_unit_distance;
			rect.h = actor.box_collider.value().y * pixel_per_unit_distance;
			Engine::instance->renderer->draw_frect(rendering_config_data.zoom_factor, rect);
		}
	}
}

void Game1::gizmo_draw_triggers()
{
	for (std::shared_ptr<Actor> a : current_scene->sorted_actor_by_id)
	{
		Actor& actor = *a;
		SDL_FRect rect;

		// Draws this actor's collider
		if (actor.box_trigger.has_value())
		{
			// Use the same screen position as the actor's sprite
			rect.x = rendering_config_data.resolution.x / 2 / rendering_config_data.zoom_factor + (actor.position.x - camera_position.x) * pixel_per_unit_distance - actor.box_trigger.value().x * pixel_per_unit_distance / 2;
			rect.y = rendering_config_data.resolution.y / 2 / rendering_config_data.zoom_factor + (actor.position.y - camera_position.y) * pixel_per_unit_distance - actor.box_trigger.value().y * pixel_per_unit_distance / 2;
			rect.w = actor.box_trigger.value().x * pixel_per_unit_distance;
			rect.h = actor.box_trigger.value().y * pixel_per_unit_distance;
			Engine::instance->renderer->draw_frect(rendering_config_data.zoom_factor, rect);
		}
	}
}
void Game1::render_actors()
{
	update_camera();
	std::vector<std::pair<std::shared_ptr<Actor>,SDL_FRect>> actors_in_screen;
	for (auto i : current_scene->sorted_actor_by_id) {
		Actor& actor = *i;
		const std::string& image_name = actor.get_current_render_image_name();
		float width, height;
		ImageDB::Get_Image_Resolution(image_name, width, height);
		glm::vec2 render_image_size = { width,height };
		float screen_x = rendering_config_data.resolution.x / 2 / rendering_config_data.zoom_factor +
			(actor.position.x - camera_position.x) * pixel_per_unit_distance -
			actor.view_pivot_offset.x * glm::abs(actor.transform_scale.x);

		float screen_y = rendering_config_data.resolution.y / 2 / rendering_config_data.zoom_factor +
			(actor.position.y - camera_position.y) * pixel_per_unit_distance -
			actor.view_pivot_offset.y * glm::abs(actor.transform_scale.y);

		float screen_w = render_image_size.x * glm::abs(actor.transform_scale.x);
		float screen_h = render_image_size.y * glm::abs(actor.transform_scale.y);
		if (!(screen_x + screen_w < 0 || screen_x > rendering_config_data.resolution.x / rendering_config_data.zoom_factor ||
			screen_y + screen_h < 0 || screen_y > rendering_config_data.resolution.y / rendering_config_data.zoom_factor)) {
			actors_in_screen.push_back({ i,{screen_x,screen_y,screen_w,screen_h} });
		}
	}
	std::sort(actors_in_screen.begin(), actors_in_screen.end(), EngineUtils::ActorRenderOrderComparator());
	for (std::pair<std::shared_ptr<Actor>, SDL_FRect>& p :actors_in_screen) {
		Actor& actor = *p.first;
		draw_actor(actor,p.second);
	}
}
void Game1::draw_actor(Actor& actor, SDL_FRect& rect)
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
	if (actor.movement_bounce_enabled && actor.intend_to_move) {
		rect.x += 0;
		rect.y += -glm::abs(glm::sin(Helper::GetFrameNumber() * 0.15f)) * 10.0f;
	}
	SDL_FPoint center =SDL_FPoint{
		actor.view_pivot_offset.x,
		actor.view_pivot_offset.y
	};
	Engine::instance->renderer->draw_image(actor.ID, actor.name, actor.get_current_render_image_name(), nullptr, &rect, actor.transform_rotation_degrees, &center, flip, rendering_config_data.zoom_factor);
}

void Game1::awake_post_check()
{
	if (game_config_data.intro_text.size() != 0 && game_config_data.font_name == "") {
		std::cout << "error: text render failed. No font configured";
		exit(0);
	}
	if (current_scene->player != nullptr && game_config_data.hp_image == "") {
		std::cout << "error: player actor requires an hp_image be defined";
		exit(0);
	}
}



void Game1::render_hud()
{
	SDL_RenderSetScale(Engine::instance->renderer->sdl_renderer, 1.0f, 1.0f);
	if (current_scene->player != nullptr) {
		//render health
		for (int i = 0; i < player_health; i++) {
			float width = 0;
			float height = 0;
			ImageDB::Get_Image_Resolution(game_config_data.hp_image, width, height);
			SDL_FRect dest_rect = 
				SDL_FRect{
					5+i*(width+5),
					25,
					width,
					height
				};
			Engine::instance->renderer->draw_image(game_config_data.hp_image, nullptr, &dest_rect);
		}

		//render score
		Engine::instance->renderer->draw_text(game_config_data.font_name, "score : " + std::to_string(score), font_size, font_color, 5, 5);

	}
	SDL_RenderSetScale(Engine::instance->renderer->sdl_renderer, rendering_config_data.zoom_factor, rendering_config_data.zoom_factor);

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
		Engine::instance->renderer->draw_text(game_config_data.font_name, message, font_size, font_color, 25, rendering_config_data.resolution.y - 50 - 50 * (size - 1 - i));
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


void Game1::update_actors()
{
	if (game_status != GameStatus_running)return;
	for (auto i : current_scene->sorted_actor_by_id) {
		Actor& actor = *i;
		actor.colliding_actors_this_frame.clear();
	}
	for (auto i : current_scene->sorted_actor_by_id) {
		Actor& actor = *i;
		actor.update();
	}
}

void Game1::check_game_status()
{
	if (game_status == GameStatus_bad_ending || game_status == GameStatus_bad_ending_show_image) {
		if (game_status == GameStatus_bad_ending) {
			if (game_config_data.game_over_bad_audio_name != "") {
				AudioDB::Halt_Audio(0);
				AudioDB::Play_Audio(0, game_config_data.game_over_bad_audio_name, 0);
			}
		}
		if (game_config_data.game_over_bad_image_name != "") {
			game_status = GameStatus_bad_ending_show_image;
		}
		else {
			is_running = false;
		}
	}
	else if (game_status == GameStatus_good_ending || game_status == GameStatus_good_ending_show_image) {
		if (game_status == GameStatus_good_ending) {
			if (game_config_data.game_over_good_audio_name != "") {
				AudioDB::Halt_Audio(0);
				AudioDB::Play_Audio(0, game_config_data.game_over_good_audio_name, 0);
			}
		}
		if (game_config_data.game_over_good_image_name != "") {
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


bool Game1::change_player_health(int change)
{
	if (change > 0) {
		player_health += change;
		return true;
	}
	else {
		if (is_in_damage_cooldown()) {
			return false;
		}
		player_health += change;
		last_took_damage_frame = Helper::GetFrameNumber();
		if (current_scene->player) {
			if (current_scene->player->damage_sfx != "") {
				AudioDB::Play_Audio(Helper::GetFrameNumber() % 48 + 2, current_scene->player->damage_sfx, 0);
			}
		}
		if (player_health <= 0) {
			game_status = GameStatus_bad_ending;
		}
		return true;
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
		rendering_config_data.set_rendering_config_data(rendering_config);
	}

	rapidjson::Document& game_config = *config_file_map["game.config"];
	game_config_data.set_game_config_data(game_config);
	if (game_config_data.intro_images_name.size() != 0)current_intro_image_index = 0;
	if (game_config_data.intro_text.size() != 0)current_intro_text_index = 0;


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
	current_scene->initialize_scene(out_document);
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
	Engine::instance->renderer->init_renderer(game_config_data.game_title.c_str(), 150, 150, rendering_config_data.resolution.x, rendering_config_data.resolution.y, -1, 0, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
}



bool Game1::move_actor(std::shared_ptr<Actor> actor, float target_y, float target_x)
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
 