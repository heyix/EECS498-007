#pragma once
#include "Helper.h"
#include <memory>
#include "SDL2/SDL.h"
#include "ImageDB.h"
#include "TextDB.h"
#include  <deque>

class ImageDrawRequest {
public:
	ImageDrawRequest(const std::string& image_name, std::shared_ptr<const SDL_FRect> src_rect, std::shared_ptr<const SDL_FRect> dest_rect)
		:image_name(image_name), src_rect(src_rect),dest_rect(dest_rect),render_actor(false),flip(SDL_RendererFlip()),center(nullptr)
	{
	}
	ImageDrawRequest(int actor_id, const std::string& actor_name, const std::string& image_name, std::shared_ptr<const SDL_FRect> src_rect, std::shared_ptr<const SDL_FRect> dest_rect, const float angle, std::shared_ptr<const SDL_FPoint> center, const SDL_RendererFlip& flip)
		:image_name(image_name), src_rect(src_rect),dest_rect(dest_rect), render_actor(true),actor_id(actor_id),actor_name(actor_name),angle(angle),center(center),flip(flip)
	{
	}
public:
	int actor_id = 0;
	const std::string actor_name;
	const std::string image_name;
	std::shared_ptr<const SDL_FRect> src_rect = nullptr;
	std::shared_ptr<const SDL_FRect> dest_rect = nullptr;
	const float angle=0.0f;
	std::shared_ptr<const SDL_FPoint> center = nullptr;
	const SDL_RendererFlip flip;
	bool render_actor = false;
};



class TextDrawRequest {
public:
	TextDrawRequest(const std::string& font_name, const std::string& text_content, int font_size, SDL_Color& font_color, float x, float y)
		:font_name(font_name),text_content(text_content),font_size(font_size),font_color(font_color),x(x),y(y)
	{}
public:
	const std::string font_name;
	const std::string text_content;
	int font_size;
	SDL_Color font_color;
	float x;
	float y;
};





class Renderer {
public:
	void init_renderer(const char* title, int x, int y, int w, int h, int index, Uint32 window_flags,Uint32 renderer_flags);
	void clear_renderer();
	void render_frame();
	void set_clear_color(int r, int g, int b,int a);
	void draw_text(const std::string& font_name, const std::string& text_content, int font_size, SDL_Color font_color, float x, float y);
	void draw_image(const std::string& image_name, std::shared_ptr<const SDL_FRect> src_rect, std::shared_ptr<const SDL_FRect> dest_rect);
	void draw_image(int actor_id, const std::string& actor_name, const std::string& image_name, std::shared_ptr<const SDL_FRect> src_rect, std::shared_ptr<const SDL_FRect> dest_rect, const float angle, std::shared_ptr<const SDL_FPoint> center, const SDL_RendererFlip& flip);
public:
	void RenderAndClearAllImageRequests();
	void RenderAndClearAllTextRequests();
public:
	SDL_Window* sdl_window;
	SDL_Renderer* sdl_renderer;
	int clear_color_r = 255;
	int clear_color_g = 255;
	int clear_color_b = 255;
	int clear_color_a = 255;
private:
	std::deque<ImageDrawRequest> image_draw_request_queue;
	std::deque<TextDrawRequest> text_draw_request_queue;
};