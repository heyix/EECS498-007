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
#include <unordered_map>

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
		const Vector2& position, float angle_radian,
		float r, float g, float b, float a, bool fill_color, bool use_cache)
		: vertices(&vertices), position(position), angle_radian(angle_radian), r(r), g(g), b(b), a(a), fill_color(fill_color), use_cache(use_cache) {}
	PolygonDrawRequest(std::vector<Vector2>&& vertices,
		const Vector2& position, float angle_radian,
		float r, float g, float b, float a, bool fill_color, bool use_cache)
		: owned_vertices(std::move(vertices)), vertices(&owned_vertices), position(position), angle_radian(angle_radian), r(r), g(g), b(b), a(a), fill_color(fill_color), use_cache(use_cache) {}

public:
	std::vector<Vector2> owned_vertices;
	const std::vector<Vector2>* vertices;
	Vector2 position;
	float angle_radian;
	float r, g, b, a;
	bool fill_color;
	bool use_cache;
};

class Renderer {
public:
	template <typename T>
	T* get_pointer(std::optional<T>& opt) {
		return opt ? &(*opt) : nullptr;
	}
private:
	struct PolygonCacheEntry {
		std::vector<Vector2> vertices;
		SDL_Texture* texture = nullptr;
		float texW = 0.0f;
		float texH = 0.0f;
		Vector2 minv;
	};
public:
	void init_renderer(const char* title, int x, int y, int w, int h, int index, Uint32 window_flags,Uint32 renderer_flags);
	void clear_renderer();
	void render_frame();
	void set_clear_color(int r, int g, int b,int a);
	void draw_text(const std::string& font_name, const std::string& text_content, int font_size, const SDL_Color& font_color, float x, float y,float zoom_factor = 1.0f);
	void draw_frect(float zoom_factor, SDL_FRect& rect);
	void draw_pixel(float x, float y, float r, float g, float b, float a);

	void clear_all_request_queues();

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
	template<typename T>
	void draw_polygon(T&& vertices, const Vector2& position, float angle_radian, float r, float g, float b, float a, bool fill_color = true) {
		polygon_draw_request_queue.emplace_back(std::forward<T>(vertices), position, angle_radian, r, g, b, a, fill_color, true);
	}
	template<typename T>
	void draw_polygon_world(T&& worldVertices, float r, float g, float b, float a, bool fill_color = true) {
		polygon_draw_request_queue.emplace_back(std::forward<T>(worldVertices), Vector2(0.0f, 0.0f), 0.0f, r, g, b, a, fill_color, false);
	}
	template<typename T>
	void draw_polygon_nocache(T&& vertices,const Vector2& position,float angle_radian,float r, float g, float b, float a,bool fill_color = true) {
		polygon_draw_request_queue.emplace_back(std::forward<T>(vertices), position, angle_radian, r, g, b, a, fill_color, false);
	}
private:
	void Render_All_Scene_Space_Image_Requests();
	void Render_All_UI_Image_Requests();
	void Render_All_Pixel_Draw_Request();
	void Render_All_Text_Requests();
	void Render_All_FRect_Requests();
	void Render_All_Polygon_Requests();
private:
	void render_image_request_queue(std::vector<ImageDrawRequest>& request_queue);
	PolygonCacheEntry* GetOrCreatePolygonCacheEntry(const std::vector<Vector2>& verts);
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
	std::vector<TextDrawRequest> text_draw_request_queue;
	std::vector<FRectDrawRequest> frect_draw_request_queue;
	std::vector<PolygonDrawRequest> polygon_draw_request_queue;

	std::vector<PolygonCacheEntry> polygon_cache_;
	std::unordered_map<std::size_t, std::vector<std::size_t>> polygon_cache_buckets_;
};