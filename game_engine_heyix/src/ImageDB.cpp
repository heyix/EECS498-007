#include "ImageDB.h"
#include "Engine.h"
#include "EngineUtils.h"

std::string ImageDB::folder_path = "images/";
std::unordered_map<std::string, SDL_Texture*>  ImageDB::loaded_images;

SDL_Texture* ImageDB::Load_Image_Texture(const std::string& image_name)
{
	if (loaded_images.find(image_name) != loaded_images.end()) {
		return loaded_images[image_name];
	}
	if (!EngineUtils::Resource_File_Exist(folder_path + image_name + ".png")) {
		std::cout << "error: missing image " + image_name;
		exit(0);
	}
	SDL_Texture* texture = IMG_LoadTexture(Engine::instance->renderer->sdl_renderer, (EngineUtils::Get_Resource_File_Path(folder_path + image_name + ".png")).c_str());
	loaded_images[image_name] = texture;
	return texture;
}

void ImageDB::Clean_Loaded_Images()
{
	for (auto& p : loaded_images) {
		if (p.second) {
			SDL_DestroyTexture(p.second);
		}
	}
	loaded_images.clear();
}

void ImageDB::Get_Image_Resolution(const std::string& image_name, float& width, float& height)
{
	if (auto it = queried_image_size.find(image_name); it != queried_image_size.end()) {
		glm::ivec2& target = it->second;
		width = target.x;
		height = target.y;
		return;
	}
	SDL_Texture* view_texture = ImageDB::Load_Image_Texture(image_name);
	Helper::SDL_QueryTexture(view_texture, &width, &height);
	queried_image_size[image_name] = { width,height };
}

void ImageDB::Lua_Draw_UI(const std::string& image_name, float x, float y)
{
	Engine::instance->renderer->draw_ui(image_name, x, y);
}

void ImageDB::Lua_Draw_UI_Ex(const std::string& image_name, float x, float y, float r, float g, float b, float a, int sorting_order)
{
	Engine::instance->renderer->draw_ui_ex(image_name, x, y, r, g, b, a, sorting_order);
}

void ImageDB::Lua_Draw(const std::string& image_name, float x, float y)
{
	Engine::instance->renderer->draw(image_name, x, y);
}

void ImageDB::Lua_Draw_Ex(const std::string& image_name, float x, float y, float rotation_degrees, float scale_x, float scale_y, float pivot_x, float pivot_y, float r, float g, float b, float a, int sorting_order)
{
	Engine::instance->renderer->draw_ex(image_name, x, y, rotation_degrees, scale_x, scale_y, pivot_x, pivot_y, r, g, b, a, sorting_order);
}

void ImageDB::Lua_Draw_Pixel(float x, float y, float r, float g, float b, float a)
{
	Engine::instance->renderer->draw_pixel(x, y, r, g, b, a);
}
