#include "Renderer.h"

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
	RenderAndClearAllImageRequests();
	RenderAndClearAllTextRequests();


	Helper::SDL_RenderPresent(sdl_renderer);
}

void Renderer::set_clear_color(int r, int g, int b,int a)
{
	clear_color_r = r;
	clear_color_b = b;
	clear_color_g = g;
	clear_color_a = a;
}

void Renderer::draw_text(const std::string& font_name, const std::string& text_content, int font_size, SDL_Color font_color, float x, float y)
{
	text_draw_request_queue.push_back(TextDrawRequest(font_name, text_content, font_size, font_color, x, y));
}


void Renderer::draw_image(const std::string& image_name, std::shared_ptr<const SDL_FRect> src_rect, std::shared_ptr<const SDL_FRect> dest_rect)
{
	image_draw_request_queue.push_back(ImageDrawRequest(image_name, src_rect, dest_rect));
}

void Renderer::draw_image(int actor_id, const std::string& actor_name, const std::string& image_name, std::shared_ptr<const SDL_FRect> src_rect, std::shared_ptr<const SDL_FRect> dest_rect, const float angle, std::shared_ptr<const SDL_FPoint> center, const SDL_RendererFlip& flip)
{
	image_draw_request_queue.push_back(ImageDrawRequest(actor_id,actor_name,image_name, src_rect, dest_rect, angle, center, flip));
}
void Renderer::RenderAndClearAllImageRequests()
{
	while (!image_draw_request_queue.empty()) {
		auto& image_request = image_draw_request_queue.front();
		SDL_Texture* texture = ImageDB::Load_Image_Texture(image_request.image_name);
		if (!image_request.render_actor) {
			Helper::SDL_RenderCopy(sdl_renderer, texture, image_request.src_rect.get(), image_request.dest_rect.get());
		}
		else {
			Helper::SDL_RenderCopyEx(image_request.actor_id, image_request.actor_name,sdl_renderer, texture, image_request.src_rect.get(), image_request.dest_rect.get(), image_request.angle, image_request.center.get(), image_request.flip);
		}
		image_draw_request_queue.pop_front();
	}
}

void Renderer::RenderAndClearAllTextRequests()
{
	while (!text_draw_request_queue.empty()) {
		auto& text_request = text_draw_request_queue.front();
		SDL_Texture* texture = TextDB::Load_Text_Texture(text_request.font_name, text_request.text_content, text_request.font_size, text_request.font_color, text_request.x, text_request.y);
		float width, height;
		Helper::SDL_QueryTexture(texture, &width, &height);
		SDL_FRect dst_rect = { text_request.x,text_request.y,width,height };
		Helper::SDL_RenderCopy(sdl_renderer, texture, nullptr, &dst_rect);
		text_draw_request_queue.pop_front();
	}
}
