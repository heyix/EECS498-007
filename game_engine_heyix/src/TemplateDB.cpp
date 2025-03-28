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
			std::shared_ptr<Component> new_component = new_object.Add_Component_Without_Calling_On_Start(component_key, component_type);
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

void TemplateDB::Load_Template_GameObject(GameObject& new_object, const std::string& template_name)
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
	if (document.HasMember("name"))new_object.name = document["name"].GetString();
	Load_Components(new_object, document);
}