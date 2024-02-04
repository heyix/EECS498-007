#pragma once
#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/filereadstream.h"
#include "rapidjson/include/rapidjson/prettywriter.h" 
#include <string>
#include <iostream>
#include <memory>
#include <fstream> 
#include <filesystem>


class EngineUtils
{
public:
	static std::string folderPath;
public:
	static void ReadJsonFile(const std::string& path, rapidjson::Document& out_document);
	static std::string GetResourceFilePath(const std::string& filename) { return folderPath + filename; }
	static void ModifyStringInJsonFile(const std::string& filePath, const std::string& key, const std::string& value);
	static void ModifyIntInJsonFile(const std::string& filePath, const std::string& key, int value);
	static bool ResourceFileExist(const std::string& filePath);
};
