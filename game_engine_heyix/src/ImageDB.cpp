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
