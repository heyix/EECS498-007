#pragma once
#include <string>
#include "SDL2/SDL.h"
#include "SDL2_ttf/SDL_ttf.h"
#include "unordered_map"
class TextDB {
private:
	static std::string folder_path;
public:
	static inline std::unordered_map<std::string,std::unordered_map<int,TTF_Font*>> loaded_fonts;
	static inline std::unordered_map<TTF_Font*, std::unordered_map<std::string, SDL_Texture*>> loaded_text_texture;
public:
	static void Clean_Loaded_Fonts_And_Texture_Then_Quit();
	static void Init_TextDB();
	static SDL_Texture* Load_Text_Texture(const std::string& font_name, const std::string& text_content, int font_size, SDL_Color& font_color, int x, int y);
	static TTF_Font* Load_Font(const std::string& font_name, int font_size);
public:
	static void Lua_Draw(const std::string& str_content, float x, float y, const std::string& font_name, int font_size, int r, int g, int b, int a);
};