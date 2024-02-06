#include "TemplateDB.h"
#include "Game1.h"
std::string TemplateDB::folderPath = "actor_templates/";
std::unordered_map<std::string, Actor> TemplateDB::template_files;
Player TemplateDB::player_template;
const Actor& TemplateDB::LoadTemplateActor(const std::string& template_name)
{
	if (!EngineUtils::ResourceFileExist(folderPath + template_name + ".template")) {
		std::cout << "error: template " << template_name << " is missing";
		exit(0);
	}
	if (auto element = template_files.find(template_name);element != template_files.end()) {
		return element->second;
	}
	rapidjson::Document document;
	EngineUtils::ReadJsonFile(EngineUtils::GetResourceFilePath(folderPath + template_name + ".template"), document);
	template_files[template_name] = Actor();
	Actor& new_actor=template_files[template_name];
	if (document.HasMember("name"))new_actor.actor_name = document["name"].GetString();
	if (document.HasMember("view") && std::string(document["view"].GetString()).size() != 0)new_actor.view = document["view"].GetString()[0];
	if (document.HasMember("x"))new_actor.position.x = document["x"].GetInt();
	if (document.HasMember("y"))new_actor.position.y = document["y"].GetInt();
	if (document.HasMember("vel_x"))new_actor.velocity.x = document["vel_x"].GetInt();
	if (document.HasMember("vel_y"))new_actor.velocity.y = document["vel_y"].GetInt();
	if (document.HasMember("blocking"))new_actor.blocking = document["blocking"].GetBool();
	if (document.HasMember("nearby_dialogue"))new_actor.nearby_dialogue = document["nearby_dialogue"].GetString();
	if (document.HasMember("contact_dialogue"))new_actor.contact_dialogue = document["contact_dialogue"].GetString();
	return new_actor;
}

const Player& TemplateDB::LoadTemplatePlayer(const std::string& template_name)
{
	if (!EngineUtils::ResourceFileExist(folderPath + template_name + ".template")) {
		std::cout << "error: template " << template_name << " is missing";
		exit(0);
	}
	rapidjson::Document document;
	EngineUtils::ReadJsonFile(EngineUtils::GetResourceFilePath(folderPath + template_name + ".template"), document);
	player_template = Player();
	if (document.HasMember("name"))player_template.actor_name = document["name"].GetString();
	if (document.HasMember("view") && std::string(document["view"].GetString()).size() != 0)player_template.view = document["view"].GetString()[0];
	if (document.HasMember("x"))player_template.position.x = document["x"].GetInt();
	if (document.HasMember("y"))player_template.position.y = document["y"].GetInt();
	if (document.HasMember("vel_x"))player_template.velocity.x = document["vel_x"].GetInt();
	if (document.HasMember("vel_y"))player_template.velocity.y = document["vel_y"].GetInt();
	if (document.HasMember("blocking"))player_template.blocking = document["blocking"].GetBool();
	if (document.HasMember("nearby_dialogue"))player_template.nearby_dialogue = document["nearby_dialogue"].GetString();
	if (document.HasMember("contact_dialogue"))player_template.contact_dialogue = document["contact_dialogue"].GetString();
	return player_template;
}
 