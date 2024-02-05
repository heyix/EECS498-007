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
    char buffer[65536];
    rapidjson::FileReadStream stream(file_pointer, buffer, sizeof(buffer));
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

std::string EngineUtils::obtain_word_after_phrase(const std::string& input, const std::string& phrase)
{
    // Find the position of the phrase in the string
    size_t pos = input.find(phrase);

    // If phrase is not found, return an empty string
    if (pos == std::string::npos) return "";

    // Find the starting position of the next word (skip spaces after the phrase)
    pos += phrase.length();
    while (pos < input.size() && std::isspace(input[pos])) {
        ++pos;
    }

    // If we're at the end of the string, return an empty string
    if (pos == input.size()) return "";

    // Find the end position of the word (until a space or the end of the string)
    size_t endPos = pos;
    while (endPos < input.size() && !std::isspace(input[endPos])) {
        ++endPos;
    }

    // Extract and return the word
    return input.substr(pos, endPos - pos);
}


bool EngineUtils::ResourceFileExist(const std::string& filePath)
{
    return std::filesystem::exists(GetResourceFilePath(filePath));
}
uint64_t EngineUtils::create_composite_key(int x, int y) 
{
    // cast to ensure the ints become exactly 32 bits in size.
    uint32_t ux = static_cast<uint32_t>(x);
    uint32_t uy = static_cast<uint32_t>(y);

    // place x into right 32 bits.
    uint64_t result = static_cast<uint64_t>(ux);

    // move x to left 32 bits.
    result = result << 32;

    // place y into right 32 bits.
    result = result | static_cast<uint64_t>(uy);

    return result;
}

uint64_t EngineUtils::create_composite_key(const glm::ivec2& k)
{
    // cast to ensure the ints become exactly 32 bits in size.
    uint32_t ux = static_cast<uint32_t>(k.x);
    uint32_t uy = static_cast<uint32_t>(k.y);

    // place x into right 32 bits.
    uint64_t result = static_cast<uint64_t>(ux);

    // move x to left 32 bits.
    result = result << 32;

    // place y into right 32 bits.
    result = result | static_cast<uint64_t>(uy);

    return result;
}

