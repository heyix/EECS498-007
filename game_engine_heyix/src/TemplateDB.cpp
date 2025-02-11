#include "TemplateDB.h"
#include "Game1.h"
std::string TemplateDB::folder_path = "actor_templates/";
std::unordered_map<std::string, Actor> TemplateDB::template_files;
Player TemplateDB::player_template;

const Actor& TemplateDB::LoadTemplateActor(const std::string& template_name)
{
	if (template_files.find(template_name) != template_files.end()) {
		return template_files[template_name];
	}
	if (!EngineUtils::Resource_File_Exist(folder_path + template_name + ".template")) {
		std::cout << "error: template " << template_name + " is missing";
		exit(0);
	}
	rapidjson::Document document;
	EngineUtils::Read_Json_File(EngineUtils::Get_Resource_File_Path(folder_path + template_name + ".template"), document);
	template_files[template_name] = Actor();
	Actor& new_actor = template_files[template_name];
	if (document.HasMember("name"))new_actor.actor_name = document["name"].GetString();
	if (document.HasMember("view") && std::string(document["view"].GetString()).size() != 0)new_actor.view = document["view"].GetString()[0];
	if (document.HasMember("transform_position_x"))new_actor.position.x = document["transform_position_x"].GetInt();
	if (document.HasMember("transform_position_y"))new_actor.position.y = document["transform_position_y"].GetInt();
	if (document.HasMember("vel_x"))new_actor.velocity.x = document["vel_x"].GetInt();
	if (document.HasMember("vel_y"))new_actor.velocity.y = document["vel_y"].GetInt();
	if (document.HasMember("blocking"))new_actor.blocking = document["blocking"].GetBool();
	if (document.HasMember("nearby_dialogue"))new_actor.nearby_dialogue = document["nearby_dialogue"].GetString();
	if (document.HasMember("contact_dialogue"))new_actor.contact_dialogue = document["contact_dialogue"].GetString();
	if (document.HasMember("view_image"))new_actor.set_view_image(document["view_image"].GetString());
	if (document.HasMember("transform_scale_x"))new_actor.transform_scale.x = document["transform_scale_x"].GetFloat();
	if (document.HasMember("transform_scale_y"))new_actor.transform_scale.y = document["transform_scale_y"].GetFloat();
	if (document.HasMember("transform_rotation_degrees"))new_actor.transform_rotation_degrees = document["transform_rotation_degrees"].GetFloat();
	if (document.HasMember("view_pivot_offset_x"))new_actor.view_pivot_offset.x = document["view_pivot_offset_x"].GetFloat();
	if (document.HasMember("view_pivot_offset_y"))new_actor.view_pivot_offset.y = document["view_pivot_offset_y"].GetFloat();
	if (document.HasMember("render_order"))new_actor.render_order = document["render_order"].GetInt();
	return new_actor;
}

const Player& TemplateDB::LoadTemplatePlayer(const std::string& template_name)
{
	if (!EngineUtils::Resource_File_Exist(folder_path + template_name + ".template")) {
		std::cout << "error: template " << template_name << " is missing";
		exit(0);
	}
	rapidjson::Document document;
	EngineUtils::Read_Json_File(EngineUtils::Get_Resource_File_Path(folder_path + template_name + ".template"), document);
	player_template = Player();
	if (document.HasMember("name"))player_template.actor_name = document["name"].GetString();
	if (document.HasMember("view") && std::string(document["view"].GetString()).size() != 0)player_template.view = document["view"].GetString()[0];
	if (document.HasMember("transform_position_x"))player_template.position.x = document["transform_position_x"].GetInt();
	if (document.HasMember("transform_position_y"))player_template.position.y = document["transform_position_y"].GetInt();
	if (document.HasMember("vel_x"))player_template.velocity.x = document["vel_x"].GetInt();
	if (document.HasMember("vel_y"))player_template.velocity.y = document["vel_y"].GetInt();
	if (document.HasMember("blocking"))player_template.blocking = document["blocking"].GetBool();
	if (document.HasMember("nearby_dialogue"))player_template.nearby_dialogue = document["nearby_dialogue"].GetString();
	if (document.HasMember("contact_dialogue"))player_template.contact_dialogue = document["contact_dialogue"].GetString();
	if (document.HasMember("view_image"))player_template.set_view_image(document["view_image"].GetString());
	if (document.HasMember("transform_scale_x"))player_template.transform_scale.x = document["transform_scale_x"].GetFloat();
	if (document.HasMember("transform_scale_y"))player_template.transform_scale.y = document["transform_scale_y"].GetFloat();
	if (document.HasMember("transform_rotation_degrees"))player_template.transform_rotation_degrees = document["transform_rotation_degrees"].GetFloat();
	if (document.HasMember("view_pivot_offset_x"))player_template.view_pivot_offset.x = document["view_pivot_offset_x"].GetFloat();
	if (document.HasMember("view_pivot_offset_y"))player_template.view_pivot_offset.y = document["view_pivot_offset_y"].GetFloat();
	if (document.HasMember("render_order"))player_template.render_order = document["render_order"].GetInt();
	return player_template;
}
