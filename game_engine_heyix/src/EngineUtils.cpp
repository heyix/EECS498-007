#include "EngineUtils.h"
std::string EngineUtils::folderPath = "resources/";


void EngineUtils::Read_Json_File(const std::string& path, rapidjson::Document& out_document)
{
	FILE* file_pointer = nullptr;
#ifdef _WIN32
	fopen_s(&file_pointer, path.c_str(), "rb");
#else
	file_pointer = fopen(path.c_str(), "rb");
#endif
	std::unique_ptr<char[]> buffer = std::make_unique<char[]>(65536);
	rapidjson::FileReadStream stream(file_pointer, buffer.get(), sizeof(char)*65536);
	out_document.ParseStream(stream);
	std::fclose(file_pointer);

	if (out_document.HasParseError()) {
		rapidjson::ParseErrorCode errorCode = out_document.GetParseError();
		std::cout << "error parsing json at [" << path << "]" << std::endl;
		exit(0);
	}
}

bool EngineUtils::Resource_File_Exist(const std::string& filePath)
{
	return std::filesystem::exists(Get_Resource_File_Path(filePath));
}

uint64_t EngineUtils::Create_Composite_Key(int x, int y)
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

uint64_t EngineUtils::Create_Composite_Key(const glm::ivec2& k)
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

std::string EngineUtils::Obtain_Word_After_Phrase(const std::string& input, const std::string& phrase)
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
