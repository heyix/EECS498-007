#include "Game.h"
#include "EngineUtils.h"
#include "Input.h"
#include "GameObjectDB.h"
#include "Camera.h"
#include "LuaDB.h"
#include "AudioHelper.h"
#include "PhysicsDB.h"
#include "EventBus.h"
void Game::game_loop()
{
	Input::Init();
	awake();
	start();
	while (is_running) {
		process_input();
		Engine::instance->renderer->clear_renderer();
		update();
		EventBus::Process_Subscription();
		PhysicsDB::Physics_Step();
		render(); 
		Input::LateUpdate();
	}
}
void Game::update()
{
	if (pending_change_scene == true) {
		pending_change_scene = false;
		std::shared_ptr<Scene> new_scene = std::make_shared<Scene>();
		for (auto& actor:current_scene->sorted_actor_by_id) {
			if (actor.second->Get_Dont_Destroy_On_Load()) {
				new_scene->add_actor(actor.second);
			}
		}
		current_scene->before_scene_unload();
		load_current_scene(new_scene);
		for (auto& actor : current_scene->sorted_actor_by_id) {
			actor.second->On_Start();
		}

	}
	std::set<std::shared_ptr<GameObject>, GameObject::GameObjectComparatorByKey> temp_pending_adding_gameobjects = pending_adding_gameobjects;
	pending_adding_gameobjects.clear();
	for (std::shared_ptr<GameObject> object : temp_pending_adding_gameobjects) {
		current_scene->add_actor(object);
		object->On_Start();
	}
	for (auto& p : current_scene->sorted_actor_by_id) {
		p.second->Process_Added_Components();
	}
	for (auto& p : current_scene->sorted_actor_by_id) {
		p.second->On_Update();
	}
	for (auto& p : current_scene->sorted_actor_by_id) {
		p.second->On_LateUpdate();
	}
	for (auto& p : current_scene->sorted_actor_by_id) {
		p.second->Process_Removed_Components();
	}
	for (std::shared_ptr<GameObject> object : pending_removing_gameobjects) {
		object->On_Destroy();
		current_scene->remove_gameobject(object);
	}
	/*handle_input();
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

	check_game_status(); */
}
void Game::render()
{
	Engine::instance->renderer->render_frame();
}

void Game::process_input()
{
	SDL_Event e;
	while (Helper::SDL_PollEvent(&e)) {
		if (e.type == SDL_QUIT) {
			is_running = false;
		}
		else {
			Input::ProcessEvent(e);
		}
	}
}


void Game::Lua_Quit()
{
	exit(0);
}

void Game::Lua_Sleep(int miliseconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(miliseconds));
}


int Game::Lua_Get_Frame()
{
	return Helper::GetFrameNumber();
}


void Game::Lua_Open_URL(const std::string& url)
{
#ifdef _WIN32
	std::string command = "start " + url;
#elif __APPLE__
	std::string command = "open " + url;
#else
	std::string command = "xdg-open " + url;
#endif
	std::system(command.c_str());
}
void Game::Remove_GameObject(GameObject* gameobject)
{
	std::shared_ptr<GameObject> object = current_scene->get_gameobject_shared_ptr_by_pointer(gameobject);
	pending_removing_gameobjects.insert(object);
	object->Deactive_All_Components();
	current_scene->unrecord_gameobject(object);
	auto it = pending_adding_gameobjects.find(object);
	if (it != pending_adding_gameobjects.end()) {
		pending_adding_gameobjects.erase(it);
	} 
}
std::shared_ptr<GameObject> Game::Instantiate_GameObject_From_Template(const std::string& actor_template_name)
{
	std::shared_ptr<GameObject> new_object = std::make_shared<GameObject>();
	TemplateDB::Load_Template_GameObject(*new_object, actor_template_name);
	Add_Instantiated_GameObject(new_object);
	return new_object;
}
void Game::Add_Instantiated_GameObject(std::shared_ptr<GameObject> new_object)
{
	new_object->ID = GameObjectDB::Require_A_ID_For_New_Actor();
	current_scene->record_gameobject(new_object);
	pending_adding_gameobjects.insert(new_object);
}
glm::ivec2 Game::Get_Camera_Dimension()
{
	return camera->Get_Camera_Dimension();
}
glm::vec2 Game::Get_Camera_Position()
{
	return camera->Get_Position();
}
float Game::Get_Zoom_Factor()
{
	return camera->Get_Zoom_Factor();
}
void Game::Set_Zoom_Factor(float zoom_factor)
{
	camera->Set_Zoom_Factor(zoom_factor);
}
void Game::Set_Camera_Position(float x, float y)
{
	camera->Set_Position(x, y);
}
void Game::Load_Scene(const std::string& scene_name)
{
	pending_change_scene = true;
	current_scene_name = scene_name;
}
std::string& Game::Get_Current_Scene_Name()
{
	return current_scene_name;
}
void Game::Dont_Destroy(luabridge::LuaRef& actor)
{
	GameObject* old_object = LuaDB::Cast_Lua_Ref<GameObject*>(actor);
	old_object->Set_Dont_Destroy_On_Load(true);
}
std::shared_ptr<GameObject> Game::Find_GameObject_By_Name(const std::string& name)
{
	return current_scene->find_gameobject_by_name(name);
}
luabridge::LuaRef Game::Find_All_GameObjects_By_Name(const std::string& name)
{
	return current_scene->find_all_gameObjects_by_name(name);
}
void Game::awake()
{
	init_game_data();
	after_init_game_data();
	load_current_scene(std::make_shared<Scene>());
}

void Game::start()
{
	//for (auto& p : current_scene->sorted_actor_by_id) {
	//	p.second->On_Start();
	//}
	AudioHelper::Mix_AllocateChannels(50);
	Engine::instance->renderer->set_clear_color(game_data.rendering_config_data.clear_color_r, game_data.rendering_config_data.clear_color_g, game_data.rendering_config_data.clear_color_b, 255);
	//if(game_config_data.intro_bgm_name!="")AudioDB::Play_Audio(0, game_config_data.intro_bgm_name, -1);
	//if (current_scene->player != nullptr) {
	//	camera_position = { current_scene->player->position.x + rendering_config_data.camera_offset.x,current_scene->player->position.y + rendering_config_data.camera_offset.y };
	//}
}



void Game::init_camera()
{
	camera = std::make_shared<Camera>();
}

void Game::init_game_data()
{
	game_data.Init_Game_Data();
}

void Game::after_init_game_data()
{
	init_renderer();
	init_camera();
	camera->Set_Camera_Dimension(game_data.rendering_config_data.resolution.x, game_data.rendering_config_data.resolution.y);
	current_scene_name = game_data.game_config_data.initial_scene_name;
}


void Game::load_current_scene(const std::shared_ptr<Scene>& new_scene)
{
	std::string relative_scene_path = "scenes/" + current_scene_name + ".scene";
	if (!EngineUtils::Resource_File_Exist(relative_scene_path)) {
		//cout_frame_output();
		std::cout << "error: scene " + current_scene_name + " is missing";
		exit(0);
	}
	rapidjson::Document out_document;
	EngineUtils::Read_Json_File(EngineUtils::Get_Resource_File_Path(relative_scene_path), out_document);
	current_scene = new_scene;
	current_scene->initialize_scene(out_document);
	//current_frame_dialogue_queue = std::deque<std::string>();
}


void Game::init_renderer()
{
	Engine::instance->renderer->init_renderer(game_data.game_config_data.game_title.c_str(), 150, 150, game_data.rendering_config_data.resolution.x, game_data.rendering_config_data.resolution.y, -1, 0, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
}