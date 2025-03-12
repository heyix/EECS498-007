#include "TextDB.h"
#include "EngineUtils.h"
#include "Engine.h"
std::string TextDB::folder_path = "fonts/";
TTF_Font* TextDB::loaded_font = nullptr;
std::unordered_map<std::string, SDL_Texture*> TextDB::loaded_text_texture;

void TextDB::Clean_Loaded_Fonts_And_Texture_Then_Quit()
{
	for (auto& p : loaded_text_texture) {
		if (p.second) {
			SDL_DestroyTexture(p.second);
		}
	}
	loaded_text_texture.clear();
	TTF_CloseFont(loaded_font);
	TTF_Quit();
}

void TextDB::Init_TextDB()
{
	TTF_Init();
}

TTF_Font* TextDB::Load_Font(const std::string& font_name, int font_size)
{
	if (loaded_font != nullptr) {
		return loaded_font;
	}
	if (!EngineUtils::Resource_File_Exist(folder_path + font_name + ".ttf")) {
		std::cout << "error: font " << font_name << " missing";
		exit(0);
	}
	loaded_font = TTF_OpenFont((EngineUtils::Get_Resource_File_Path(folder_path + font_name + ".ttf")).c_str(), font_size);
	return loaded_font;
}

SDL_Texture* TextDB::Load_Text_Texture(const std::string& font_name, const std::string& text_content, int font_size, SDL_Color& font_color, int x, int y)
{
	if (loaded_text_texture.find(text_content) != loaded_text_texture.end()) {
		return loaded_text_texture[text_content];
	}
	TTF_Font* font = Load_Font(font_name, font_size);
	SDL_Surface* text_surface = TTF_RenderText_Solid(font, text_content.c_str(), font_color);
	SDL_Texture* text_texture = SDL_CreateTextureFromSurface(Engine::instance->renderer->sdl_renderer, text_surface);
	SDL_FreeSurface(text_surface);
	loaded_text_texture[text_content] = text_texture;
	return text_texture;
}

