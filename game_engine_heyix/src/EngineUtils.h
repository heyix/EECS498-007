#pragma once
#include "glm/glm.hpp"
#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/filereadstream.h"
#include "rapidjson/include/rapidjson/prettywriter.h"
#include <string>
#include <memory>
#include <iostream>
#include <filesystem>
#include "Actor.h"

class EngineUtils {
public:
	class ActorPointerComparator {
	public:
		bool operator()(const std::shared_ptr<Actor> a,const std::shared_ptr<Actor> b)const {
			return a->ID < b->ID;
		}
	};
	class ActorRenderOrderComparator {
	public:
		bool operator()(const std::pair<std::shared_ptr<Actor>, SDL_FRect>& p1, const std::pair<std::shared_ptr<Actor>, SDL_FRect>& p2)const {
			std::shared_ptr<Actor> a = p1.first;
			std::shared_ptr<Actor> b = p2.first;
			float order_a = a->position.y;
			float order_b = b->position.y;
			if (a->render_order.has_value()) {
				order_a = a->render_order.value();
			}
			if (b->render_order.has_value()) {
				order_b = b->render_order.value();
			}
			if (order_a == order_b) {
				return a->ID < b->ID;
			}
			return order_a < order_b;
		}
	};
public:
	static std::string folderPath;
public:
	static void Read_Json_File(const std::string& path, rapidjson::Document& out_document);
	static std::string Get_Resource_File_Path(const std::string& filename) { return folderPath + filename; }
	static bool Resource_File_Exist(const std::string& filePath);
	static uint64_t Create_Composite_Key(int x, int y);
	static uint64_t Create_Composite_Key(const glm::ivec2& k);
	static std::string Obtain_Word_After_Phrase(const std::string& input, const std::string& phrase);
	static bool AABB_Collision(glm::vec2& position1, glm::vec2& position2, glm::vec2& box1, glm::vec2& box2);
	static bool Compare_Float_Equal(float f1, float f2,float epsilon = 0.001f);
};
