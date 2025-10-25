#pragma once
#include <string>
#include <unordered_map>
#include "SDL2/SDL.h"
#include "SDL2_image/SDL_image.h"
#include "glm/glm.hpp"
#include "Renderer.h"

class ImageDB {
private:
	class ImageDrawRequestComparator {
	public:
		bool operator()(ImageDrawRequest& a, ImageDrawRequest& b) {
			return a.sorting_order < b.sorting_order;
		}
	};
private:
	static std::string folder_path;
public:
	static std::unordered_map<std::string, SDL_Texture*> loaded_images;
	static inline std::unordered_map<std::string, glm::ivec2> queried_image_size;
	static SDL_Texture* Load_Image_Texture(const std::string& image_name);
	static void Clean_Loaded_Images();
	static void Get_Image_Resolution(const std::string& image_name,float& width, float& height);
	static void Register_Instantiated_Texture(const std::string& image_name, SDL_Texture* texture, float width, float height);
public:
	static void Lua_Draw_UI(const std::string& image_name, float x, float y);
	static void Lua_Draw_UI_Ex(const std::string& image_name, float x, float y, float r, float g, float b, float a, int sorting_order);
	static void Lua_Draw(const std::string& image_name, float x, float y);
	static void Lua_Draw_Ex(const std::string& image_name, float x, float y, float rotation_degrees, float scale_x, float scale_y, float pivot_x, float pivot_y, float r, float g, float b, float a, int sorting_order);
	static void Lua_Draw_Pixel(float x, float y, float r, float g, float b, float a);
};
