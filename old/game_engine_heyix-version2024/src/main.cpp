#include "Game1.h"
#include <iostream>
#include "EngineUtils.h"
int main() {
	Game1 game;
	game.game_loop();
	//EngineUtils::ModifyStringInJsonFile(EngineUtils::GetResourceFilePath("game.config"), "initial_scene", "level_1");

	//rapidjson::Document out_document;
	//EngineUtils::ReadJsonFile(EngineUtils::GetResourceFilePath("game.config"),out_document);
	//std::cout << out_document.IsObject();
	//std::cout << out_document["game_start_message"].GetString();
}