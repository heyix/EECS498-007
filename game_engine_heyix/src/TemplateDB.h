#pragma once
#include "Actor.h"
#include <memory>
#include "rapidjson/include/rapidjson/document.h"
#include <unordered_map>
#include "Player.h"
class Game1;
class TemplateDB
{
private:
	static std::string folderPath;
public:
	static std::unordered_map<std::string, Actor> template_files;
	static Player player_template;
	static const Actor& LoadTemplateActor(const std::string& template_name);
	static const Player& LoadTemplatePlayer(const std::string& template_name);
};

