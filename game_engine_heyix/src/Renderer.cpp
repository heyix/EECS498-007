#include "Renderer.h"
#include "ImageDB.h"
#include "Engine.h"
#include "SDL2_gfxPrimitives.h"
namespace {
	constexpr float kPixelsPerMeter = 100.0f;
	constexpr float kPolyPadPixels = 2.0f;

	std::size_t HashPolygonVertices(const std::vector<Vector2>& verts)
	{
		std::size_t h = 1469598103934665603ull;
		auto mix = [&](uint32_t v) {
			h ^= static_cast<std::size_t>(v);
			h *= 1099511628211ull;
			};

		uint32_t bits = 0;
		for (const Vector2& p : verts) {
			float fx = p.x();
			float fy = p.y();

			std::memcpy(&bits, &fx, sizeof(float));
			mix(bits);
			std::memcpy(&bits, &fy, sizeof(float));
			mix(bits);
		}

		mix(static_cast<uint32_t>(verts.size()));
		return h;
	}

	bool VerticesEqual(const std::vector<Vector2>& a, const std::vector<Vector2>& b)
	{
		if (a.size() != b.size()) return false;
		for (size_t i = 0; i < a.size(); ++i) {
			if (a[i].x() != b[i].x() || a[i].y() != b[i].y())
				return false;
		}
		return true;
	}
}
Renderer::~Renderer()
{
	for (auto& entry : polygon_cache_) {
		if (entry.texture) {
			SDL_DestroyTexture(entry.texture);
			entry.texture = nullptr;
		}
	}
	polygon_cache_.clear();
	polygon_cache_buckets_.clear();

	if (sdl_renderer) {
		SDL_DestroyRenderer(sdl_renderer);
		sdl_renderer = nullptr;
	}
	if (sdl_window) {
		SDL_DestroyWindow(sdl_window);
		sdl_window = nullptr;
	}
	SDL_Quit();
}
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
	Render_All_Scene_Space_Image_Requests();
	Render_All_Polygon_Requests();
	Render_All_UI_Image_Requests();
	Render_All_Text_Requests();
	Render_All_Pixel_Draw_Request();
	Render_All_FRect_Requests();
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

void Renderer::draw_pixel(float x, float y, float r, float g, float b, float a)
{
	pixels_request_queue.push_back({ x,y,r,g,b,a });
}
void Renderer::clear_all_request_queues()
{
	pixels_request_queue.clear();
	text_draw_request_queue.clear();
	frect_draw_request_queue.clear();
	polygon_draw_request_queue.clear();
	ui_image_request_queue.clear();
	scene_space_image_request_queue.clear();
}
void Renderer::Render_All_UI_Image_Requests()
{
	SDL_RenderSetScale(sdl_renderer, 1,1);
	render_image_request_queue(ui_image_request_queue);
}
void Renderer::Render_All_Pixel_Draw_Request()
{
	for (auto& image_request : pixels_request_queue) {
		SDL_Color color{ static_cast<Uint8>(image_request.r),static_cast<Uint8>(image_request.g),static_cast<Uint8>(image_request.b),static_cast<Uint8>(image_request.a) };
		SDL_SetRenderDrawColor(sdl_renderer, color.r, color.g, color.b, color.a);
		SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
		SDL_RenderDrawPoint(sdl_renderer, image_request.x, image_request.y);
		SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_NONE);
	}
	pixels_request_queue.clear();
}
void Renderer::Render_All_Scene_Space_Image_Requests()
{
	float zoom_factor = Engine::instance->running_game->Get_Zoom_Factor();
	SDL_RenderSetScale(sdl_renderer, zoom_factor, zoom_factor);
	render_image_request_queue(scene_space_image_request_queue);
}

void Renderer::Render_All_Text_Requests()
{
	for (auto& text_request : text_draw_request_queue) {
		//if (text_request.zoom_factor != last_zoom_factor) {
		//	SDL_RenderSetScale(sdl_renderer, text_request.zoom_factor, text_request.zoom_factor);
		//	last_zoom_factor = text_request.zoom_factor;
		//}
		SDL_Texture* texture = TextDB::Load_Text_Texture(text_request.font_name, text_request.text_content, text_request.font_size, text_request.font_color, text_request.x, text_request.y);
		float width, height;
		Helper::SDL_QueryTexture(texture, &width, &height);
		SDL_FRect dst_rect = { text_request.x,text_request.y,width,height };
		Helper::SDL_RenderCopy(sdl_renderer, texture, nullptr, &dst_rect);
	}
}

void Renderer::Render_All_FRect_Requests()
{
	for (auto& frect_request :frect_draw_request_queue) {
		//if (frect_request.zoom_factor != last_zoom_factor) {
		//	SDL_RenderSetScale(sdl_renderer, frect_request.zoom_factor, frect_request.zoom_factor);
		//	last_zoom_factor = frect_request.zoom_factor;
		//}
		SDL_SetRenderDrawColor(sdl_renderer, 255, 0, 0, 255);
		SDL_RenderDrawRectF(sdl_renderer, &frect_request.rect);
		SDL_SetRenderDrawColor(sdl_renderer, clear_color_r, clear_color_g, clear_color_b, 255);
	}
}

void Renderer::Render_All_Polygon_Requests()
{
	if (polygon_draw_request_queue.empty())
		return;
	std::stable_sort(polygon_draw_request_queue.begin(), polygon_draw_request_queue.end(), PolygonDrawRequestComparator{});
	SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);

	const float ppm = kPixelsPerMeter;
	const glm::vec2  cam_pos = Engine::instance->running_game->Get_Camera_Position();
	const glm::ivec2 cam_dim = Engine::instance->running_game->Get_Camera_Dimension();
	const float      zoom = Engine::instance->running_game->Get_Zoom_Factor();

	const float ox = cam_dim.x * 0.5f * (1.0f / zoom);
	const float oy = cam_dim.y * 0.5f * (1.0f / zoom);

	std::vector<SDL_FPoint> screenPts;
	std::vector<Sint16> xs;
	std::vector<Sint16> ys;

	for (auto& req : polygon_draw_request_queue)
	{
		const auto& verts = *req.vertices;
		const size_t n = verts.size();
		if (n < 2) continue;

		const Uint8 r = static_cast<Uint8>(req.r);
		const Uint8 g = static_cast<Uint8>(req.g);
		const Uint8 b = static_cast<Uint8>(req.b);
		const Uint8 a = static_cast<Uint8>(req.a);

		if (req.use_cache && req.fill_color && n >= 3)
		{
			if (auto* entry = GetOrCreatePolygonCacheEntry(verts))
			{
				const Vector2& minv = entry->minv;

				const float origin_sx =
					(req.position.x() - cam_pos.x) * ppm + ox;
				const float origin_sy =
					(req.position.y() - cam_pos.y) * ppm + oy;

				const float origin_offset_x = kPolyPadPixels - minv.x() * ppm;
				const float origin_offset_y = kPolyPadPixels - minv.y() * ppm;

				SDL_FRect dest{
					origin_sx - origin_offset_x,
					origin_sy - origin_offset_y,
					entry->texW,
					entry->texH
				};

				SDL_SetTextureColorMod(entry->texture, r, g, b);
				SDL_SetTextureAlphaMod(entry->texture, a);

				const float angle_deg = req.angle_radian * (180.0f / 3.14159265f);

				if (std::fabs(req.angle_radian) < 1e-4f)
				{
					SDL_RenderCopyF(
						sdl_renderer,
						entry->texture,
						nullptr,
						&dest
					);
				}
				else
				{
					SDL_FPoint pivot{
						origin_offset_x,
						origin_offset_y
					};

					SDL_RenderCopyExF(
						sdl_renderer,
						entry->texture,
						nullptr,
						&dest,
						angle_deg,
						&pivot,
						SDL_FLIP_NONE
					);
				}


				continue;
			}
		}

		screenPts.clear();
		screenPts.reserve(n);

		const float c = std::cos(req.angle_radian);
		const float s = std::sin(req.angle_radian);

		for (size_t i = 0; i < n; ++i)
		{
			const float lx = verts[i].x();
			const float ly = verts[i].y();

			const float wx = req.position.x() + (c * lx - s * ly);
			const float wy = req.position.y() + (s * lx + c * ly);

			const float sx = (wx - cam_pos.x) * ppm + ox;
			const float sy = (wy - cam_pos.y) * ppm + oy;

			screenPts.push_back({ sx, sy });
		}

		xs.resize(n);
		ys.resize(n);
		for (size_t i = 0; i < n; ++i) {
			xs[i] = static_cast<Sint16>(std::lround(screenPts[i].x));
			ys[i] = static_cast<Sint16>(std::lround(screenPts[i].y));
		}

		if (req.fill_color && n >= 3) {
			GFX_filledPolygonRGBA(
				sdl_renderer,
				xs.data(), ys.data(), static_cast<int>(n),
				r, g, b, a
			);
		}

		GFX_polygonRGBA(
			sdl_renderer,
			xs.data(), ys.data(), static_cast<int>(n),
			r, g, b, a
		);
	}

	SDL_SetRenderDrawColor(
		sdl_renderer,
		clear_color_r,
		clear_color_g,
		clear_color_b,
		clear_color_a
	);
	SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_NONE);

	polygon_draw_request_queue.clear();
}

void Renderer::render_image_request_queue(std::vector<ImageDrawRequest>& request_queue)
{
	std::stable_sort(request_queue.begin(), request_queue.end(), ImageDrawRequestComparator());
	for (auto& image_request : request_queue) {
		SDL_Texture* texture = ImageDB::Load_Image_Texture(*image_request.image_name);
		if (!image_request.use_ex) {
			float width = 0;
			float height = 0;
			ImageDB::Get_Image_Resolution(*image_request.image_name, width, height);
			SDL_FRect dest_rect = {
				image_request.x,image_request.y,width,height
			};
			SDL_Color color{ static_cast<Uint8>(image_request.r),static_cast<Uint8>(image_request.g),static_cast<Uint8>(image_request.b),static_cast<Uint8>(image_request.a) };

			SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
			SDL_SetTextureAlphaMod(texture, color.a);
			Helper::SDL_RenderCopy(sdl_renderer, texture, nullptr, &dest_rect);
			SDL_SetTextureColorMod(texture, 255, 255, 255);
			SDL_SetTextureAlphaMod(texture, 255);
		}
		else {
			const int pixels_per_meter = 100;
			glm::vec2 final_rendering_position =
				glm::vec2(image_request.x, image_request.y) - Engine::instance->running_game->Get_Camera_Position();

			float width;
			float height;
			ImageDB::Get_Image_Resolution(*image_request.image_name, width, height);
			/* Apply scale */
			SDL_RendererFlip flip_mode = SDL_FLIP_NONE;
			if (image_request.scale_x < 0)
				flip_mode = SDL_FLIP_HORIZONTAL;
			if (image_request.scale_y < 0)
				flip_mode = SDL_FLIP_VERTICAL;

			float x_scale = glm::abs(image_request.scale_x.value());
			float y_scale = glm::abs(image_request.scale_y.value());
			width *= x_scale;
			height *= y_scale;

			SDL_FPoint pivot_point = {
				image_request.pivot_x.value() * width,
				image_request.pivot_y.value() * height
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
			SDL_Color color{ static_cast<Uint8>(image_request.r),static_cast<Uint8>(image_request.g),static_cast<Uint8>(image_request.b),static_cast<Uint8>(image_request.a) };
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
			Helper::SDL_RenderCopyEx(0, "", sdl_renderer, texture, nullptr, &dest_rect, image_request.rotation_degrees.value(), &pivot_point, static_cast<SDL_RendererFlip>(flip_mode));
			SDL_SetTextureColorMod(texture, 255, 255, 255);
			SDL_SetTextureAlphaMod(texture, 255);
		}
	}
	request_queue.clear();
}

Renderer::PolygonCacheEntry* Renderer::GetOrCreatePolygonCacheEntry(const std::vector<Vector2>& verts)
{
	if (verts.size() < 3) return nullptr;

	std::size_t h = HashPolygonVertices(verts);

	auto it = polygon_cache_buckets_.find(h);
	if (it != polygon_cache_buckets_.end()) {
		auto& indexList = it->second;
		for (std::size_t idx : indexList) {
			PolygonCacheEntry& entry = polygon_cache_[idx];
			if (VerticesEqual(entry.vertices, verts)) {
				return &entry;
			}
		}
	}

	Vector2 minv = verts[0];
	Vector2 maxv = verts[0];
	for (size_t i = 1; i < verts.size(); ++i) {
		const Vector2& v = verts[i];
		if (v.x() < minv.x()) minv.x() = v.x();
		if (v.y() < minv.y()) minv.y() = v.y();
		if (v.x() > maxv.x()) maxv.x() = v.x();
		if (v.y() > maxv.y()) maxv.y() = v.y();
	}
	const Vector2 size = maxv - minv;

	int texW = static_cast<int>(std::ceil(size.x() * kPixelsPerMeter + 2.0f * kPolyPadPixels));
	int texH = static_cast<int>(std::ceil(size.y() * kPixelsPerMeter + 2.0f * kPolyPadPixels));

	if (texW <= 0 || texH <= 0) {
		return nullptr;
	}

	SDL_Texture* tex = SDL_CreateTexture(
		sdl_renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET,
		texW,
		texH
	);
	if (!tex) {
		return nullptr;
	}

	SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

	SDL_SetRenderTarget(sdl_renderer, tex);
	SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 0);
	SDL_RenderClear(sdl_renderer);

	const size_t n = verts.size();
	std::vector<Sint16> xs(n), ys(n);

	const float origin_offset_x = kPolyPadPixels - minv.x() * kPixelsPerMeter;
	const float origin_offset_y = kPolyPadPixels - minv.y() * kPixelsPerMeter;

	for (size_t i = 0; i < n; ++i) {
		const Vector2& v = verts[i];
		float px = v.x() * kPixelsPerMeter + origin_offset_x;
		float py = v.y() * kPixelsPerMeter + origin_offset_y;
		xs[i] = static_cast<Sint16>(std::lround(px));
		ys[i] = static_cast<Sint16>(std::lround(py));
	}

	GFX_filledPolygonRGBA(
		sdl_renderer,
		xs.data(), ys.data(), static_cast<int>(n),
		255, 255, 255, 255
	);
	GFX_polygonRGBA(
		sdl_renderer,
		xs.data(), ys.data(), static_cast<int>(n),
		255, 255, 255, 255
	);

	SDL_SetRenderTarget(sdl_renderer, nullptr);

	PolygonCacheEntry entry;
	entry.vertices = verts;
	entry.texture = tex;
	entry.texW = static_cast<float>(texW);
	entry.texH = static_cast<float>(texH);
	entry.minv = minv;

	const std::size_t newIndex = polygon_cache_.size();
	polygon_cache_.push_back(std::move(entry));
	polygon_cache_buckets_[h].push_back(newIndex);

	return &polygon_cache_.back();
}
