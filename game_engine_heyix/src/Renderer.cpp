#include "Renderer.h"
#include "ImageDB.h"
#include "Engine.h"

void Renderer::init_renderer(const char* title, int x, int y, int w, int h, int index, Uint32 window_flags, Uint32 renderer_flags)
{
	sdl_window = Helper::SDL_CreateWindow(title, x, y, w, h, window_flags);
	sdl_renderer = Helper::SDL_CreateRenderer(sdl_window, index, renderer_flags);
}

void Renderer::clear_renderer()
{
	SDL_SetRenderDrawColor(sdl_renderer, clear_color_r, clear_color_g, clear_color_b, clear_color_a);
	SDL_RenderClear(sdl_renderer);
}

void Renderer::render_frame()
{
	Render_And_Clear_All_Scene_Space_Image_Requests();
	Render_And_Clear_All_UI_Image_Requests();
	Render_And_Clear_All_Text_Requests();
	Render_And_Clear_All_Pixel_Draw_Request();
	Render_And_Clear_All_FRect_Requests();
	SDL_RenderSetScale(sdl_renderer, 1, 1);

	Helper::SDL_RenderPresent(sdl_renderer);
}

void Renderer::set_clear_color(int r, int g, int b,int a)
{
	clear_color_r = r;
	clear_color_b = b;
	clear_color_g = g;
	clear_color_a = a;
}

void Renderer::draw_text(const std::string& font_name, const std::string& text_content, int font_size, const SDL_Color& font_color, float x, float y, float zoom_factor)
{
	text_draw_request_queue.push_back(TextDrawRequest(font_name, text_content, font_size, font_color, x, y,zoom_factor));
}


void Renderer::draw_frect(float zoom_factor, SDL_FRect& rect)
{
	frect_draw_request_queue.push_back(FRectDrawRequest(zoom_factor, rect));

}
void Renderer::draw_ui(const std::string& image_name, float x, float y)
{
	draw_ui_ex(image_name, x, y, 255, 255, 255, 255, 0);
}
void Renderer::draw_ui_ex(const std::string& image_name, float x, float y, float r, float g, float b, float a, int sorting_order)
{
	float width = 0;
	float height = 0;
	ImageDB::Get_Image_Resolution(image_name, width, height);
	SDL_FRect dest_rect = {
		x,y,width,height
	};
	SDL_Color color{ static_cast<Uint8>(r),static_cast<Uint8>(g),static_cast<Uint8>(b),static_cast<Uint8>(a) };
	ui_image_request_queue.push_back({ image_name,nullptr,&dest_rect,color,sorting_order });
}
void Renderer::draw(const std::string& image_name, float x, float y)
{
		draw_ex(image_name, x, y, 0, 1, 1, 0.5f, 0.5f, 255, 255, 255, 255, 0);
}
void Renderer::draw_ex(const std::string& image_name, float x, float y, float rotation_degrees, float scale_x, float scale_y, float pivot_x, float pivot_y, float r, float g, float b, float a, int sorting_order)
{
	const int pixels_per_meter = 100;
	glm::vec2 final_rendering_position =
		glm::vec2(x, y) - Engine::instance->running_game->Get_Camera_Position();

	float width;
	float height;
	ImageDB::Get_Image_Resolution(image_name, width, height);

	/* Apply scale */
	SDL_RendererFlip flip_mode = SDL_FLIP_NONE;
	if (scale_x < 0)
		flip_mode = SDL_FLIP_HORIZONTAL;
	if (scale_y < 0)
		flip_mode = SDL_FLIP_VERTICAL;

	float x_scale = glm::abs(scale_x);
	float y_scale = glm::abs(scale_y);
	width *= x_scale;
	height *= y_scale;

	SDL_FPoint pivot_point = {
		pivot_x * width,
		pivot_y * height
	};

	glm::ivec2 cam_dimensions = Engine::instance->running_game->Get_Camera_Dimension();
	float zoom_factor = Engine::instance->running_game->Get_Zoom_Factor();
	float pos_x = (
		final_rendering_position.x * pixels_per_meter +
		cam_dimensions.x * 0.5f * (1.0f / zoom_factor) -
		pivot_point.x
		);
	float pos_y = (
		final_rendering_position.y * pixels_per_meter +
		cam_dimensions.y * 0.5f * (1.0f / zoom_factor) -
		pivot_point.y
		);
	const SDL_FRect dest_rect{ pos_x,pos_y,width,height };
	SDL_Color color{ static_cast<Uint8>(r),static_cast<Uint8>(g),static_cast<Uint8>(b),static_cast<Uint8>(a) };
	scene_space_image_request_queue.push_back({ image_name,nullptr,&dest_rect,rotation_degrees,&pivot_point,static_cast<SDL_RendererFlip>(flip_mode),color,sorting_order});
}
void Renderer::draw_pixel(float x, float y, float r, float g, float b, float a)
{
	SDL_Color color{ static_cast<Uint8>(r),static_cast<Uint8>(g),static_cast<Uint8>(b),static_cast<Uint8>(a) };
	pixels_request_queue.push_back({ {x,y},color });
}
void Renderer::Render_And_Clear_All_UI_Image_Requests()
{
	SDL_RenderSetScale(sdl_renderer, 1,1);
	render_and_clear_image_request_queue(ui_image_request_queue);
}
void Renderer::Render_And_Clear_All_Pixel_Draw_Request()
{
	for (auto& image_request : pixels_request_queue) {
		SDL_Color& color = image_request.render_color;
		glm::vec2& point_position = image_request.drawed_point_position.value();
		SDL_SetRenderDrawColor(sdl_renderer, color.r, color.g, color.b, color.a);
		SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
		SDL_RenderDrawPoint(sdl_renderer, point_position.x, point_position.y);
		SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_NONE);
	}
	pixels_request_queue.clear();
}
void Renderer::Render_And_Clear_All_Scene_Space_Image_Requests()
{
	float zoom_factor = Engine::instance->running_game->Get_Zoom_Factor();
	SDL_RenderSetScale(sdl_renderer, zoom_factor, zoom_factor);
	render_and_clear_image_request_queue(scene_space_image_request_queue);
}

void Renderer::Render_And_Clear_All_Text_Requests()
{
	while (!text_draw_request_queue.empty()) {
		auto& text_request = text_draw_request_queue.front();
		//if (text_request.zoom_factor != last_zoom_factor) {
		//	SDL_RenderSetScale(sdl_renderer, text_request.zoom_factor, text_request.zoom_factor);
		//	last_zoom_factor = text_request.zoom_factor;
		//}
		SDL_Texture* texture = TextDB::Load_Text_Texture(text_request.font_name, text_request.text_content, text_request.font_size, text_request.font_color, text_request.x, text_request.y);
		float width, height;
		Helper::SDL_QueryTexture(texture, &width, &height);
		SDL_FRect dst_rect = { text_request.x,text_request.y,width,height };
		Helper::SDL_RenderCopy(sdl_renderer, texture, nullptr, &dst_rect);
		text_draw_request_queue.pop_front();
	}
}

void Renderer::Render_And_Clear_All_FRect_Requests()
{
	while (!frect_draw_request_queue.empty()) {
		auto& frect_request = frect_draw_request_queue.front();
		//if (frect_request.zoom_factor != last_zoom_factor) {
		//	SDL_RenderSetScale(sdl_renderer, frect_request.zoom_factor, frect_request.zoom_factor);
		//	last_zoom_factor = frect_request.zoom_factor;
		//}
		SDL_SetRenderDrawColor(sdl_renderer, 255, 0, 0, 255);
		SDL_RenderDrawRectF(sdl_renderer, &frect_request.rect);
		SDL_SetRenderDrawColor(sdl_renderer, clear_color_r, clear_color_g, clear_color_b, 255);
		frect_draw_request_queue.pop_front();
	}
}

void Renderer::render_and_clear_image_request_queue(std::vector<ImageDrawRequest>& request_queue)
{
	std::stable_sort(request_queue.begin(), request_queue.end(), ImageDrawRequestComparator());
	for (auto& image_request : request_queue) {
		SDL_Color& color = image_request.render_color;
		SDL_Texture* texture = ImageDB::Load_Image_Texture(image_request.image_name);
		if (!image_request.use_ex) {
			SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
			SDL_SetTextureAlphaMod(texture, color.a);
			Helper::SDL_RenderCopy(sdl_renderer, texture, get_pointer(image_request.src_rect), get_pointer(image_request.dest_rect));
			SDL_SetTextureColorMod(texture, 255, 255, 255);
			SDL_SetTextureAlphaMod(texture, 255);
		}
		else {
			//if (image_request.dest_rect.has_value())
			//{
			//	const SDL_FRect& dr = image_request.dest_rect.value();
			//	std::cout << image_request.image_name << ' ';
			//	std::cout
			//		<< Helper::GetFrameNumber() << ":" << -1 << ": "
			//		<< "dstrect "
			//		<< dr.x << " " << dr.y << " "
			//		<< dr.w << " " << dr.h << " "
			//		<< "angle " << image_request.angle << " ";
			//	// If there is a center, print it
			//	if (image_request.center.has_value())
			//	{
			//		const SDL_FPoint& c = image_request.center.value();
			//		std::cout << "center " << c.x << " " << c.y << " ";
			//	}
			//	// Flip is an enum, so cast to int for printing
			//	std::cout
			//		<< "flip " << static_cast<int>(image_request.flip) << " "
			//		<< "renderscale 1 1"
			//		<< std::endl;
			//}
			SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
			SDL_SetTextureAlphaMod(texture, color.a);
			Helper::SDL_RenderCopyEx(0, "", sdl_renderer, texture, get_pointer(image_request.src_rect), get_pointer(image_request.dest_rect), image_request.angle, get_pointer(image_request.center), image_request.flip);
			SDL_SetTextureColorMod(texture, 255, 255, 255);
			SDL_SetTextureAlphaMod(texture, 255);
		}
	}
	request_queue.clear();
}
