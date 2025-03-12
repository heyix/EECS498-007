#include "TemplateDB.h"
#include "Game1.h"
#include "EngineUtils.h"

std::string TemplateDB::folder_path = "actor_templates/";

void TemplateDB::Load_Components(Actor& actor, rapidjson::Document& document)
{
	if (document.HasMember("components") && document["components"].IsObject()) {
		for (auto it = document["components"].MemberBegin(); it != document["components"].MemberEnd(); ++it) {
			std::string component_key = it->name.GetString();
			std::string component_type = it->value["type"].GetString();
			std::shared_ptr<Component> new_component = actor.Add_Component_Without_Calling_On_Start(component_key, component_type);
			if (it->value.IsObject()) {
				for (auto compIt = it->value.MemberBegin(); compIt != it->value.MemberEnd(); ++compIt) {
					std::string field_key = compIt->name.GetString();
					if (field_key == "type")continue;
					if (compIt->value.IsString()) {
						new_component->Inject_Value_Pair(field_key, compIt->value.GetString());
					}
					else if (compIt->value.IsInt()) { 
						new_component->Inject_Value_Pair(field_key, compIt->value.GetInt());
					}
					else if (compIt->value.IsFloat()) {
						new_component->Inject_Value_Pair(field_key, compIt->value.GetFloat());
					}
					else if (compIt->value.IsBool()) {
						new_component->Inject_Value_Pair(field_key, compIt->value.GetBool());
					}
				}
			}
		}
	}
}

void TemplateDB::LoadTemplateActor(Actor& new_actor,const std::string& template_name)
{
	if (template_files.find(template_name) == template_files.end()) {
		if (!EngineUtils::Resource_File_Exist(folder_path + template_name + ".template")) {
			std::cout << "error: template " << template_name + " is missing";
			exit(0);
		}
		std::shared_ptr<rapidjson::Document> document = std::make_shared<rapidjson::Document>();
		EngineUtils::Read_Json_File(EngineUtils::Get_Resource_File_Path(folder_path + template_name + ".template"), *document);
		template_files[template_name] = document;
	}
	rapidjson::Document& document = *(template_files[template_name]);
	if (document.HasMember("name"))new_actor.name = document["name"].GetString();



	/*if (document.HasMember("transform_position_x"))new_actor.position.x = document["transform_position_x"].GetFloat();
	if (document.HasMember("transform_position_y"))new_actor.position.y = document["transform_position_y"].GetFloat();
	if (document.HasMember("vel_x"))new_actor.velocity.x = document["vel_x"].GetFloat();
	if (document.HasMember("vel_y"))new_actor.velocity.y = document["vel_y"].GetFloat();
	if (document.HasMember("nearby_dialogue"))new_actor.nearby_dialogue = document["nearby_dialogue"].GetString();
	if (document.HasMember("contact_dialogue"))new_actor.contact_dialogue = document["contact_dialogue"].GetString();
	if (document.HasMember("view_image"))new_actor.set_view_image(document["view_image"].GetString());
	if (document.HasMember("view_image_back"))new_actor.set_view_image_back( document["view_image_back"].GetString());
	if (document.HasMember("transform_scale_x"))new_actor.transform_scale.x = document["transform_scale_x"].GetFloat();
	if (document.HasMember("transform_scale_y"))new_actor.transform_scale.y = document["transform_scale_y"].GetFloat();
	if (document.HasMember("transform_rotation_degrees"))new_actor.transform_rotation_degrees = document["transform_rotation_degrees"].GetFloat();
	if (document.HasMember("view_pivot_offset_x"))new_actor.view_pivot_offset.x = document["view_pivot_offset_x"].GetFloat();
	if (document.HasMember("view_pivot_offset_y"))new_actor.view_pivot_offset.y = document["view_pivot_offset_y"].GetFloat();
	if (document.HasMember("render_order"))new_actor.render_order = document["render_order"].GetInt();
	if (document.HasMember("movement_bounce_enabled"))new_actor.movement_bounce_enabled = document["movement_bounce_enabled"].GetBool();
	if (document.HasMember("box_collider_width") && document.HasMember("box_collider_height")) {
		new_actor.set_box_collider(document["box_collider_width"].GetFloat(), document["box_collider_height"].GetFloat());
	}
	if (document.HasMember("box_trigger_width") && document.HasMember("box_trigger_height")) {
		new_actor.set_box_trigger(document["box_trigger_width"].GetFloat(), document["box_trigger_height"].GetFloat());
	}
	if (document.HasMember("view_image_attack"))new_actor.set_view_image_attack(document["view_image_attack"].GetString());
	if (document.HasMember("nearby_dialogue_sfx"))new_actor.nearby_dialogue_sfx = document["nearby_dialogue_sfx"].GetString();*/



	Load_Components(new_actor, document);
}

void TemplateDB::LoadTemplatePlayer(Player& player_template, const std::string& template_name)
{
	if (!EngineUtils::Resource_File_Exist(folder_path + template_name + ".template")) {
		std::cout << "error: template " << template_name << " is missing";
		exit(0);
	}
	rapidjson::Document document;
	EngineUtils::Read_Json_File(EngineUtils::Get_Resource_File_Path(folder_path + template_name + ".template"), document);
	if (document.HasMember("name"))player_template.name = document["name"].GetString();


	/*if (document.HasMember("transform_position_x"))player_template.position.x = document["transform_position_x"].GetFloat();
	if (document.HasMember("transform_position_y"))player_template.position.y = document["transform_position_y"].GetFloat();
	if (document.HasMember("vel_x"))player_template.velocity.x = document["vel_x"].GetFloat();
	if (document.HasMember("vel_y"))player_template.velocity.y = document["vel_y"].GetFloat();
	if (document.HasMember("nearby_dialogue"))player_template.nearby_dialogue = document["nearby_dialogue"].GetString();
	if (document.HasMember("contact_dialogue"))player_template.contact_dialogue = document["contact_dialogue"].GetString();
	if (document.HasMember("view_image"))player_template.set_view_image(document["view_image"].GetString());
	if (document.HasMember("view_image_back"))player_template.set_view_image_back(document["view_image_back"].GetString());
	if (document.HasMember("transform_scale_x"))player_template.transform_scale.x = document["transform_scale_x"].GetFloat();
	if (document.HasMember("transform_scale_y"))player_template.transform_scale.y = document["transform_scale_y"].GetFloat();
	if (document.HasMember("transform_rotation_degrees"))player_template.transform_rotation_degrees = document["transform_rotation_degrees"].GetFloat();
	if (document.HasMember("view_pivot_offset_x"))player_template.view_pivot_offset.x = document["view_pivot_offset_x"].GetFloat();
	if (document.HasMember("view_pivot_offset_y"))player_template.view_pivot_offset.y = document["view_pivot_offset_y"].GetFloat();
	if (document.HasMember("render_order"))player_template.render_order = document["render_order"].GetInt();
	if (document.HasMember("movement_bounce_enabled"))player_template.movement_bounce_enabled = document["movement_bounce_enabled"].GetBool();
	if (document.HasMember("box_collider_width") && document.HasMember("box_collider_height")) {
		player_template.set_box_collider(document["box_collider_width"].GetFloat(), document["box_collider_height"].GetFloat());
	}
	if (document.HasMember("box_trigger_width") && document.HasMember("box_trigger_height")) {
		player_template.set_box_trigger(document["box_trigger_width"].GetFloat(), document["box_trigger_height"].GetFloat());
	}
	if (document.HasMember("view_image_damage"))player_template.set_view_image_damage(document["view_image_damage"].GetString());
	if (document.HasMember("damage_sfx"))player_template.damage_sfx = document["damage_sfx"].GetString();
	if (document.HasMember("step_sfx"))player_template.step_sfx = document["step_sfx"].GetString();*/



	Load_Components(player_template, document);
}
