#pragma once
#include "Actor.h"
#include <memory>
#include "rapidjson/include/rapidjson/document.h"
#include <unordered_map>
#include "Player.h"

class Game1;
class TemplateDB {
private:
	static std::string folder_path;
private:
	static void Load_Components(Actor& actor, rapidjson::Document& document);
public:
	static inline std::unordered_map<std::string, std::shared_ptr<rapidjson::Document>> template_files;
	static void LoadTemplateActor(Actor& new_actor, const std::string& template_name);
	static void LoadTemplatePlayer(Player& player_template, const std::string& template_name);
};