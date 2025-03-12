#pragma once
#include "Helper.h"
#include <memory>
#include "SDL2/SDL.h"
#include "ImageDB.h"
#include "TextDB.h"
#include <optional>
#include  <deque>

class ImageDrawRequest {
public:
	ImageDrawRequest(const std::string& image_name, const SDL_FRect* src_rect, const SDL_FRect* dest_rect,float zoom_factor)
		:image_name(image_name), render_actor(false),flip(SDL_RendererFlip()),zoom_factor(zoom_factor)
	{
		if (src_rect) {
			this->src_rect = *src_rect;
		}
		if (dest_rect) {
			this->dest_rect = *dest_rect;
		}
	}
	ImageDrawRequest(int actor_id, const std::string& actor_name, const std::string& image_name, const SDL_FRect* src_rect, const SDL_FRect* dest_rect, const float angle, const SDL_FPoint* center, const SDL_RendererFlip& flip,float zoom_factor)
		:image_name(image_name),  render_actor(true),actor_id(actor_id),actor_name(actor_name),angle(angle),flip(flip),zoom_factor(zoom_factor)
	{
		if (src_rect) {
			this->src_rect = *src_rect;
		}
		if (dest_rect) {
			this->dest_rect = *dest_rect;
		}
		if (center) {
			this->center = *center;
		}
	}
public:
	int actor_id = 0;
	const std::string actor_name;
	const std::string image_name;
	std::optional<SDL_FRect> src_rect;
	std::optional<SDL_FRect> dest_rect;
	const float angle=0.0f;
	std::optional<SDL_FPoint> center;
	const SDL_RendererFlip flip;
	bool render_actor = false;
	float zoom_factor = 1.0f;
};



class TextDrawRequest {
public:
	TextDrawRequest(const std::string& font_name, const std::string& text_content, int font_size, SDL_Color& font_color, float x, float y, float zoom_factor)
		:font_name(font_name),text_content(text_content),font_size(font_size),font_color(font_color),x(x),y(y),zoom_factor(zoom_factor)
	{}
public:
	const std::string font_name;
	const std::string text_content;
	int font_size;
	SDL_Color font_color;
	float x;
	float y;
	float zoom_factor = 1.0f;
};

class FRectDrawRequest {
public:
	FRectDrawRequest(float zoom_factor, SDL_FRect& rect) 
		:zoom_factor(zoom_factor),rect(rect)
	{}
public:
	float zoom_factor = 1.0f;
	SDL_FRect rect;
};




class Renderer {
public:
	template <typename T>
	T* get_pointer(std::optional<T>& opt) {
		return opt ? &(*opt) : nullptr;
	}
public:
	void init_renderer(const char* title, int x, int y, int w, int h, int index, Uint32 window_flags,Uint32 renderer_flags);
	void clear_renderer();
	void render_frame();
	void set_clear_color(int r, int g, int b,int a);
	void draw_text(const std::string& font_name, const std::string& text_content, int font_size, SDL_Color font_color, float x, float y,float zoom_factor = 1.0f);
	void draw_image(const std::string& image_name, const SDL_FRect* src_rect, const SDL_FRect* dest_rect,float zoom_factor = 1.0f);
	void draw_image(int actor_id, const std::string& actor_name, const std::string& image_name, const SDL_FRect* src_rect, const SDL_FRect* dest_rect, const float angle, const SDL_FPoint* center, const SDL_RendererFlip& flip,float zoom_factor = 1.0f);
	void draw_frect(float zoom_factor, SDL_FRect& rect);
public:
	void RenderAndClearAllImageRequests();
	void RenderAndClearAllTextRequests();
	void RenderAndClearAllFRectRequest();
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
	std::deque<FRectDrawRequest> frect_draw_request_queue;
	float last_zoom_factor = 1.0f;
};