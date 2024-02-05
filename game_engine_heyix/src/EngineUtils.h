#ifndef ENGINE_UTILS_H
#define ENGINE_UTILS_H
#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/filereadstream.h"
#include "rapidjson/include/rapidjson/prettywriter.h" 
#include <string>
#include <iostream>
#include <memory>
#include <fstream> 
#include <filesystem>
#include "glm/glm.hpp"
#include "Actor.h"

class EngineUtils
{
public:
	struct ActorPointerComparator {
		bool operator()(const Actor* const& a, const Actor* const& b) const {
			return a->ID < b->ID;
		}
	};
public:
	static std::string folderPath;
public:
	static void ReadJsonFile(const std::string& path, rapidjson::Document& out_document);
	static std::string GetResourceFilePath(const std::string& filename) { return folderPath + filename; }
	static void ModifyStringInJsonFile(const std::string& filePath, const std::string& key, const std::string& value);
	static void ModifyIntInJsonFile(const std::string& filePath, const std::string& key, int value);
	static bool ResourceFileExist(const std::string& filePath);
	static std::string obtain_word_after_phrase(const std::string& input, const std::string& phrase);
	static uint64_t create_composite_key(int x, int y);
	static uint64_t create_composite_key(const glm::ivec2& k);
};
#endif