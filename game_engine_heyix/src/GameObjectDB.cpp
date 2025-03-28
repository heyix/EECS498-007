#include "GameObjectDB.h"
#include "GameObject.h"
#include "LuaDB.h"
#include "TemplateDB.h"
#include "Game.h"



int GameObjectDB::Require_A_ID_For_New_Actor()
{
	return current_id++;
}



GameObject* GameObjectDB::Lua_Find(const std::string& name)
{
	return Engine::instance->running_game->Find_GameObject_By_Name(name).get();
}

luabridge::LuaRef GameObjectDB::Lua_Find_All(const std::string& name)
{
	return Engine::instance->running_game->Find_All_GameObjects_By_Name(name);
}

GameObject* GameObjectDB::Lua_Instantiate(const std::string& actor_template_name)
{
	return Game::instance->Instantiate_GameObject_From_Template(actor_template_name).get();
}

void GameObjectDB::Lua_Destroy(luabridge::LuaRef actor)
{
	GameObject* object = LuaDB::Cast_Lua_Ref<GameObject*>(actor);
	Game::instance->Remove_GameObject(object);
}