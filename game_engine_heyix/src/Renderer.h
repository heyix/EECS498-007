#pragma once
#include "Helper.h"
#include <memory>
#include "SDL2/SDL.h"
#include "TextDB.h"
#include <optional>
#include  <deque>
#include <algorithm>
#include "glm/glm.hpp"
#include "Vector2.h"
#include <utility>

class ImageDrawRequest {
public:
	ImageDrawRequest(float x, float y, float r, float g, float b, float a) 
		:image_name(nullptr),x(x),y(y),r(r),g(g),b(b),a(a),sorting_order(0)
	{
	}
	ImageDrawRequest(const std::string& image_name, float x, float y, float r, float g, float b, float a, int sorting_order) 
		:image_name(&image_name),x(x),y(y),r(r),g(g),b(b),a(a),sorting_order(sorting_order),use_ex(false)
	{
	}
	ImageDrawRequest(const std::string& image_name, float x, float y, float rotation_degrees, float scale_x, float scale_y, float pivot_x, float pivot_y, float r, float g, float b, float a, int sorting_order) 
		:image_name(&image_name),x(x),y(y), rotation_degrees(rotation_degrees), scale_x(scale_x), scale_y(scale_y), pivot_x(pivot_x), pivot_y(pivot_y), r(r),g(g),b(b),a(a),sorting_order(sorting_order),use_ex(true)
	{
	}
	ImageDrawRequest(std::string&& new_image_name, float x, float y, float r, float g, float b, float a, int sorting_order)
		:owned_image(std::make_unique<std::string>(std::move(new_image_name))), image_name(owned_image.get()), x(x), y(y), r(r), g(g), b(b), a(a), sorting_order(sorting_order), use_ex(false)
	{
	}
	ImageDrawRequest(std::string&& new_image_name, float x, float y, float rotation_degrees, float scale_x, float scale_y, float pivot_x, float pivot_y, float r, float g, float b, float a, int sorting_order)
		:owned_image(std::make_unique<std::string>(std::move(new_image_name))), image_name(owned_image.get()), x(x), y(y), rotation_degrees(rotation_degrees), scale_x(scale_x), scale_y(scale_y), pivot_x(pivot_x), pivot_y(pivot_y), r(r), g(g), b(b), a(a), sorting_order(sorting_order), use_ex(true)
	{
	}
private:
	std::unique_ptr<std::string> owned_image = nullptr;
public:
	const std::string* image_name;
	float x; 
	float y; 
	std::optional<float> rotation_degrees; 
	std::optional<float> scale_x;
	std::optional<float> scale_y; 
	std::optional<float> pivot_x;
	std::optional<float> pivot_y;
	float r;
	float g;
	float b;
	float a;
	std::optional<int> sorting_order;
	bool use_ex = false;
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

class PolygonDrawRequest {
public:
	PolygonDrawRequest(const std::vector<Vector2>& vertices,
		float r, float g, float b, float a)
		: vertices(vertices), r(r), g(g), b(b), a(a) {}
public:
	std::vector<Vector2> vertices;
	float r, g, b, a;
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
	void draw_pixel(float x, float y, float r, float g, float b, float a);
	void draw_polygon(const std::vector<Vector2>& vertices, float r, float g, float b, float a);

	template<typename T>
	void draw_ui(T&& image_name, float x, float y)
	{
		draw_ui_ex(std::forward<T>(image_name), x, y, 255, 255, 255, 255, 0);
	}
	template<typename T>
	void draw_ui_ex(T&& image_name, float x, float y, float r, float g, float b, float a, int sorting_order)
	{
		ui_image_request_queue.emplace_back(std::forward<T>(image_name),x,y,r,g,b,a,sorting_order);
	}
	template<typename T>
	void draw(T&& image_name, float x, float y)
	{
		draw_ex(std::forward<T>(image_name), x, y, 0, 1, 1, 0.5f, 0.5f, 255, 255, 255, 255, 0);
	}
	template<typename T>
	void draw_ex(T&& image_name, float x, float y, float rotation_degrees, float scale_x, float scale_y, float pivot_x, float pivot_y, float r, float g, float b, float a, int sorting_order)
	{
		scene_space_image_request_queue.emplace_back( std::forward<T>(image_name), x, y, rotation_degrees,scale_x, scale_y, pivot_x, pivot_y,  r,  g,  b,  a, sorting_order );
	}
public:
	void Render_And_Clear_All_Scene_Space_Image_Requests();
	void Render_And_Clear_All_UI_Image_Requests();
	void Render_And_Clear_All_Pixel_Draw_Request();
	void Render_And_Clear_All_Text_Requests();
	void Render_And_Clear_All_FRect_Requests();
	void Render_And_Clear_All_Polygon_Requests();
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
	std::deque<PolygonDrawRequest> polygon_draw_request_queue;
};