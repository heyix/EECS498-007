#pragma once
#include <string>
#include <unordered_map>
#include "SDL2/SDL.h"
#include "SDL2_image/SDL_image.h"
#include "glm/glm.hpp"

class ImageDB {
private:
	static std::string folder_path;
public:
	static std::unordered_map<std::string, SDL_Texture*> loaded_images;
	static inline std::unordered_map<std::string, glm::ivec2> queried_image_size;
	static SDL_Texture* Load_Image_Texture(const std::string& image_name);
	static void Clean_Loaded_Images();
	static void Get_Image_Resolution(const std::string& image_name,float& width, float& height);
};
