#include "EngineUtils.h"
std::string EngineUtils::folderPath= "resources/";

void EngineUtils::ReadJsonFile(const std::string& path, rapidjson::Document& out_document)
{
	FILE* file_pointer = nullptr;
#ifdef _WIN32
	fopen_s(&file_pointer, path.c_str(), "rb");
#else
	file_pointer = fopen(path.c_str(), "rb");
#endif
	std::unique_ptr<char[]> buffer=std::make_unique<char[]>(65536);
	rapidjson::FileReadStream stream(file_pointer, buffer.get(), sizeof(char) * 65536);
	out_document.ParseStream(stream);
	std::fclose(file_pointer);

	if (out_document.HasParseError()) {
		rapidjson::ParseErrorCode errorCode = out_document.GetParseError();
		std::cout << "error parsing json at [" << path << "]" << std::endl;
		exit(0);
	}
}

//by chatgpt
void EngineUtils::ModifyStringInJsonFile(const std::string& filePath, const std::string& key, const std::string& value)
{
    // Open and read the file into a string
    std::ifstream inFile(filePath);
    std::string jsonStr((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close(); // Close the file as soon as the reading is done

    // Parse the JSON string into a document
    rapidjson::Document document;
    document.Parse(jsonStr.c_str());

    // Prepare the key and value for the document
    rapidjson::Value k(key.c_str(), document.GetAllocator());
    rapidjson::Value v(value.c_str(), document.GetAllocator());

    // Check if the key exists
    auto memberIterator = document.FindMember(k);
    if (memberIterator != document.MemberEnd()) {
        // If the key exists, update its value
        memberIterator->value.SetString(value.c_str(), document.GetAllocator());
    }
    else {
        // If the key does not exist, add a new member
        document.AddMember(k, v, document.GetAllocator());
    }

    // Write the modified document back to the file
    std::ofstream outFile(filePath);
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    outFile << buffer.GetString();
    outFile.close();
}
//by chatgpt
void EngineUtils::ModifyIntInJsonFile(const std::string& filePath, const std::string& key, int value)
{
    // Open and read the file into a string
    std::ifstream inFile(filePath);
    std::string jsonStr((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close(); // Close the file as soon as reading is done

    // Parse the JSON string into a document
    rapidjson::Document document;
    document.Parse(jsonStr.c_str());

    // Prepare the key for the document. The value is prepared when needed since it's a primitive type
    rapidjson::Value k(key.c_str(), document.GetAllocator());

    // Check if the key exists
    auto memberIterator = document.FindMember(k);
    if (memberIterator != document.MemberEnd()) {
        // If the key exists, update its value
        memberIterator->value.SetInt(value);
    }
    else {
        // If the key does not exist, add a new member
        rapidjson::Value v(value); // Prepare the value here, no allocator needed
        document.AddMember(k, v, document.GetAllocator());
    }

    // Write the modified document back to the file
    std::ofstream outFile(filePath);
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    outFile << buffer.GetString();
    outFile.close();
}

bool EngineUtils::ResourceFileExist(const std::string& filePath)
{
    return std::filesystem::exists(GetResourceFilePath(filePath));
}

