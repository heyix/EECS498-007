#pragma once
#include <memory>
#include "rapidjson/include/rapidjson/document.h"
#include <unordered_map>
#include <string>

class Game1;
class GameObject;
class TemplateDB {
private:
	static std::string folder_path;
private:
	static void Load_Components(GameObject& actor, rapidjson::Document& document);
	static void check_unique_component_type_violation(const rapidjson::Value& components, const std::string& source_name);
public:
	static inline std::unordered_map<std::string, std::shared_ptr<rapidjson::Document>> template_files;
	//static void Load_Template_Actor(Actor& new_actor, const std::string& template_name);
	//static void Load_Template_Player(Player& player_template, const std::string& template_name);
	static void Load_Template_GameObject(GameObject& new_object, const std::string& template_name);
	static void Initialize_Actor(const rapidjson::Value& actor, std::shared_ptr<GameObject> new_actor);

};