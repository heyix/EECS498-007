#pragma once
#include "Helper.h"
#include <memory>
#include "SDL2/SDL.h"
#include "TextDB.h"
#include <optional>
#include  <deque>
#include <algorithm>
#include "glm/glm.hpp"


class ImageDrawRequest {
public:
	ImageDrawRequest(const std::string& image_name, const SDL_FRect* src_rect, const SDL_FRect* dest_rect,const SDL_Color& render_color, int sorting_order)
		:image_name(image_name), use_ex(false), flip(SDL_RendererFlip()), render_color(render_color),sorting_order(sorting_order)
	{
		if (src_rect) {
			this->src_rect = *src_rect;
		}
		if (dest_rect) {
			this->dest_rect = *dest_rect;
		}
	}
	ImageDrawRequest( const std::string& image_name, const SDL_FRect* src_rect, const SDL_FRect* dest_rect, const float angle, const SDL_FPoint* center, const SDL_RendererFlip& flip, const SDL_Color& render_color, int sorting_order)
		:image_name(image_name), use_ex(true), angle(angle), render_color(render_color), flip(flip), sorting_order(sorting_order)
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
	ImageDrawRequest(const glm::vec2& point_position,const SDL_Color& render_color)
		:drawed_point_position(point_position),render_color(render_color),sorting_order(0), flip(SDL_RendererFlip())
	{

	}
public:
	std::string image_name;
	std::optional<SDL_FRect> src_rect;
	std::optional<SDL_FRect> dest_rect;
	float angle = 0.0f;
	std::optional<SDL_FPoint> center;
	SDL_RendererFlip flip;
	bool use_ex = false;
	std::optional<glm::vec2> drawed_point_position;
	SDL_Color render_color;
	int sorting_order = 0;
};

class ImageDrawRequestComparator {
public:
	bool operator()(const ImageDrawRequest& a, const ImageDrawRequest& b) {
		return a.sorting_order < b.sorting_order;
	}
};


class TextDrawRequest {
public:
	TextDrawRequest(const std::string& font_name, const std::string& text_content, int font_size, const SDL_Color& font_color, float x, float y, float zoom_factor)
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
	void draw_text(const std::string& font_name, const std::string& text_content, int font_size, const SDL_Color& font_color, float x, float y,float zoom_factor = 1.0f);
	void draw_frect(float zoom_factor, SDL_FRect& rect);
	void draw_ui(const std::string& image_name, float x, float y);
	void draw_ui_ex(const std::string& image_name, float x, float y, float r, float g, float b, float a, int sorting_order);
	void draw(const std::string& image_name, float x, float y);
	void draw_ex(const std::string& image_name, float x, float y, float rotation_degrees, float scale_x, float scale_y, float pivot_x, float pivot_y, float r, float g, float b, float a, int sorting_order);
	void draw_pixel(float x, float y, float r, float g, float b, float a);
public:
	void Render_And_Clear_All_Scene_Space_Image_Requests();
	void Render_And_Clear_All_UI_Image_Requests();
	void Render_And_Clear_All_Pixel_Draw_Request();
	void Render_And_Clear_All_Text_Requests();
	void Render_And_Clear_All_FRect_Requests();
private:
	void render_and_clear_image_request_queue(std::vector<ImageDrawRequest>& request_queue);
public:
	SDL_Window* sdl_window;
	SDL_Renderer* sdl_renderer;
	int clear_color_r = 255;
	int clear_color_g = 255;
	int clear_color_b = 255;
	int clear_color_a = 255;
private:
	std::vector<ImageDrawRequest> scene_space_image_request_queue;
	std::vector<ImageDrawRequest> ui_image_request_queue;
	std::vector<ImageDrawRequest> pixels_request_queue;
	std::deque<TextDrawRequest> text_draw_request_queue;
	std::deque<FRectDrawRequest> frect_draw_request_queue;
};