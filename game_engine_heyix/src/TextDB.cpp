#include "TextDB.h"
#include "EngineUtils.h"
#include "Engine.h"
#include "Renderer.h"
std::string TextDB::folder_path = "fonts/";

void TextDB::Clean_Loaded_Fonts_And_Texture_Then_Quit()
{
	if (last_text_texture) {
		SDL_DestroyTexture(last_text_texture);
		last_text_texture = nullptr;
	}

	for (auto& p : loaded_fonts) {
		for (auto& font : p.second) {
			TTF_CloseFont(font.second);
		}
	}
	loaded_fonts.clear();

	TTF_Quit();
}

void TextDB::Init_TextDB()
{
	TTF_Init();
}

TTF_Font* TextDB::Load_Font(const std::string& font_name, int font_size)
{
	auto it = loaded_fonts.find(font_name);
	if (it !=loaded_fonts.end()) {
		auto it2 = it->second.find(font_size);
		if (it2 != it->second.end()) {
			return it2->second;
		}
	}
	if (!EngineUtils::Resource_File_Exist(folder_path + font_name + ".ttf")) {
		std::cout << "error: font " << font_name << " missing";
		exit(0);
	}
	TTF_Font* result = TTF_OpenFont((EngineUtils::Get_Resource_File_Path(folder_path + font_name + ".ttf")).c_str(), font_size);
	loaded_fonts[font_name][font_size] = result;
	return result;
}

void TextDB::Lua_Draw(const std::string& str_content, float x, float y, const std::string& font_name, int font_size, int r, int g, int b, int a)
{
	Engine::instance->renderer->draw_text(font_name, str_content, font_size, { static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b), static_cast<Uint8>(a) }, x, y);
}

SDL_Texture* TextDB::Load_Text_Texture(const std::string& font_name, const std::string& text_content, int font_size, SDL_Color& font_color, int x, int y)
{
	if (last_text_texture) {
		SDL_DestroyTexture(last_text_texture);
		last_text_texture = nullptr;
	}

	TTF_Font* font = Load_Font(font_name, font_size);

	SDL_Surface* text_surface =
		TTF_RenderText_Solid(font, text_content.c_str(), font_color);

	SDL_Texture* text_texture =
		SDL_CreateTextureFromSurface(
			Engine::instance->renderer->sdl_renderer,
			text_surface
		);

	SDL_FreeSurface(text_surface);

	last_text_texture = text_texture;
	return text_texture;
}

