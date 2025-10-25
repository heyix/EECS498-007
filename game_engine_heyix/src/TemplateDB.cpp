#include "TemplateDB.h"
#include "EngineUtils.h"
#include "Component.h"
#include "ComponentDB.h"
#include "GameObject.h"

std::string TemplateDB::folder_path = "actor_templates/";

void TemplateDB::Load_Components(GameObject& new_object, rapidjson::Document& document)
{
	if (document.HasMember("components") && document["components"].IsObject()) {
		for (auto it = document["components"].MemberBegin(); it != document["components"].MemberEnd(); ++it) {
			std::string component_key = it->name.GetString();
			std::string component_type = it->value["type"].GetString();
			std::shared_ptr<Component> new_component = new_object.Add_Component_Without_Calling_On_Start(component_key, component_type).lock();
			if (it->value.IsObject()) {
				for (auto compIt = it->value.MemberBegin(); compIt != it->value.MemberEnd(); ++compIt) {
					std::string field_key = compIt->name.GetString();
					if (field_key == "type")continue;
					if (compIt->value.IsString()) {
						new_component->Add_String_Property(field_key, compIt->value.GetString());
					}
					else if (compIt->value.IsInt()) { 
						new_component->Add_Int_Property(field_key, compIt->value.GetInt());
					}
					else if (compIt->value.IsFloat()) {
						new_component->Add_Float_Property(field_key, compIt->value.GetFloat());
					}
					else if (compIt->value.IsBool()) {
						new_component->Add_Bool_Property(field_key, compIt->value.GetBool());
					}
				}
			}
		}
	}
}

void TemplateDB::check_unique_component_type_violation(const rapidjson::Value& components, const std::string& source_name)
{
	std::unordered_map<std::string, int> local_counts;
	for (auto it = components.MemberBegin(); it != components.MemberEnd(); it++) {
		if (!it->value.HasMember("type") || !it->value["type"].IsString()) {
			continue;
		}
		std::string type = it->value["type"].GetString();
		if (!ComponentDB::unique_component_types.count(type))continue;
		local_counts[type]++;
		if (local_counts[type] > 1) {
			std::cerr << "Error: " << source_name
				<< " defines multiple components of unique type '" << type
				<< "' (offending key: '" << it->name.GetString() << "')\n";
			exit(0);
		}
	}
}

void TemplateDB::Load_Template_GameObject(GameObject& new_object, const std::string& template_name)
{
	if (template_files.find(template_name) == template_files.end()) {
		if (!EngineUtils::Resource_File_Exist(folder_path + template_name + ".template")) {
			std::cout << "error: template " << template_name + " is missing";
			exit(0);
		}
		std::shared_ptr<rapidjson::Document> document = std::make_shared<rapidjson::Document>();
		EngineUtils::Read_Json_File(EngineUtils::Get_Resource_File_Path(folder_path + template_name + ".template"), *document);

		if (document->HasMember("components") && (*document)["components"].IsObject()) {
			check_unique_component_type_violation((*document)["components"], "Template '" + template_name + "'");
		}
		template_files[template_name] = document;
	}
	rapidjson::Document& document = *(template_files[template_name]);
	if (document.HasMember("name"))new_object.name = document["name"].GetString();
	Load_Components(new_object, document);
}

void TemplateDB::Initialize_Actor(const rapidjson::Value& actor, std::shared_ptr<GameObject> new_actor)
{
	if (auto it = actor.FindMember("name"); it != actor.MemberEnd())new_actor->name = it->value.GetString();
	if (auto it = actor.FindMember("components"); it != actor.MemberEnd() && it->value.IsObject()) {
		check_unique_component_type_violation(actor["components"], "GameObject '" + new_actor->name + "'");
		for (auto componentIt = it->value.MemberBegin(); componentIt != it->value.MemberEnd(); ++componentIt) {
			std::string componentKey = componentIt->name.GetString();
			const auto& component_object = componentIt->value;
			std::shared_ptr<Component> new_component = new_actor->Get_Component_By_Key(componentKey).lock();
			if (component_object.HasMember("type") && component_object["type"].IsString()) {
				std::string componentType = component_object.FindMember("type")->value.GetString();
				new_component = new_actor->Add_Component_Without_Calling_On_Start(componentKey, componentType).lock();
			}

			for (auto fieldIt = component_object.MemberBegin(); fieldIt != component_object.MemberEnd(); ++fieldIt) {
				std::string fieldName = fieldIt->name.GetString();
				if (fieldName == "type")continue;
				if (fieldIt->value.IsString()) {
					new_component->Add_String_Property(fieldName, fieldIt->value.GetString());
				}
				else if (fieldIt->value.IsInt()) {
					new_component->Add_Int_Property(fieldName, fieldIt->value.GetInt());
				}
				else if (fieldIt->value.IsFloat()) {
					new_component->Add_Float_Property(fieldName, fieldIt->value.GetFloat());
				}
				else if (fieldIt->value.IsBool()) {
					new_component->Add_Bool_Property(fieldName, fieldIt->value.GetBool());
				}
			}
		}
	}
}