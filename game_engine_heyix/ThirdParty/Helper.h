/* Hey Student --
 * Take a look at some of the public static variables below-- you may use them to help you debug.
 */

#ifndef INPUTHELPER_H
#define INPUTHELPER_H

#define HELPER_VERSION 0.91

#include <unordered_map>
#include <queue>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <random>

 /* WARNING : You may need to adjust the following include paths if your headers / file structures are different. */
 /* Here is the instructor solution folder structure (if we make $(ProjectDir) a include directory, these paths are valid. */
 /* https://bit.ly/3OClfHc */

#include "SDL2_image/SDL_image.h"
#include "SDL2/SDL.h"

enum InputStatus { NOT_INITIALIZED, INPUT_FILE_MISSING, INPUT_FILE_PRESENT };
enum RenderLoggerStatus { RL_NOT_INITIALIZED, RL_NOT_ENABLED, RL_ENABLED };

/* The Helper class contains mostly static functions / data, and doesn't need to be instanced. */
/* Call the public static functions below via Helper::<function>() */
class Helper {
public:
	/* Turn RECORDING_MODE on to record inputs as you play. */
	/* The input file may be fed back in to replay your game session (autograder does this). */
	inline static const bool RECORDING_MODE = false;
	inline static const char* USER_INPUT_FILENAME = "sdl_user_input.txt";

	/* The Helper.h function works differently (and thus your program works differently) */
	/* Depending on whether or not an autograder is testing it. */
	inline static bool _autograder_mode = false;

	/* One way the autograder gauges success is by comparing your "frames" (renderings) to */
	/* that of a staff solution program fed the exact same input. These are placed into a "frames" folder. */
	inline static std::string frame_directory_relative_path = "frames";

	/* The frame_number advances with every call to Helper::SDL_RenderPresent() */
	static inline int frame_number = 0;
	static inline Uint32 current_frame_start_timestamp = 0;
	static int GetFrameNumber() { return frame_number; }

	static SDL_Window* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags)
	{
		if (IsAutograderMode())
		{
			x = 0;
			y = 0;
		}

		return ::SDL_CreateWindow(title, x, y, w, h, flags);
	}

	static SDL_Renderer* SDL_CreateRenderer(SDL_Window* window, int index, Uint32 flags)
	{
		if (IsAutograderMode())
			flags &= ~SDL_RENDERER_PRESENTVSYNC; // VSync is disabled to let frames render faster in the autograder.

		SDL_Renderer* renderer = ::SDL_CreateRenderer(window, index, flags);

		if (renderer == nullptr)
			std::cerr << "Failed to create renderer : " << SDL_GetError() << std::endl;

		return renderer;
	}

	/* Wrapper that will inject input events into the SDL Event Queue if a user input file is found */
	/* This is what enables playback of inputs and game session replay. */
	static int SDL_PollEvent(SDL_Event* e)
	{
		SDL_ConsiderInputFile();
		return ::SDL_PollEvent(e);
	}

	/* Wrapper that renders to screen while also persisting to a .BMP file */
	static void SDL_RenderPresent(SDL_Renderer* renderer)
	{
		if (renderer == nullptr)
		{
			std::cout << "ERROR : The renderer pointer passed to Helper::SDL_RenderPresent() is a nullptr." << std::endl;
			exit(0);
		}

		if (input_status == NOT_INITIALIZED)
		{
			std::cout << "ERROR : Please do not attempt to render (Helper::SDL_RenderPresent()) before you've entered the game loop (IE, begun calling Helper::SDL_PollEvent()." << std::endl;
			exit(0);
		}

		static bool initialized = false;
		static SDL_Surface* saving_surface = nullptr;

		if (RECORDING_MODE || _autograder_mode)
		{
			if (!initialized)
			{
				/* Check for existence of frames folder and establish it if necessary. */
				if (!std::filesystem::exists(frame_directory_relative_path))
				{
					std::filesystem::create_directory(frame_directory_relative_path);
				}

				/* Create reusable surface. */
				int height, width;
				SDL_GetRendererOutputSize(renderer, &width, &height);
				saving_surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 24, SDL_PIXELFORMAT_RGB24);

				current_frame_start_timestamp = SDL_GetTicks();
				frame_number = 0;
				initialized = true;
			}

			/* Read the current renderer's data and persist it as a .bmp file to disk (BMP format is fast-to-write compared to PNG). */
			if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGB24, saving_surface->pixels, saving_surface->pitch) != 0) {
				SDL_Log("SDL_RenderReadPixels() failed: %s", SDL_GetError());
			}
			std::stringstream filenameStream;
			filenameStream << "frame_" << std::setw(5) << std::setfill('0') << frame_number << ".bmp";
			std::string output_file_name = filenameStream.str();
			std::string output_file_path = frame_directory_relative_path + "/" + output_file_name;

			if (SDL_SaveBMP(saving_surface, output_file_path.c_str()) != 0) {
				SDL_Log("SDL_SaveBMP() failed: %s", SDL_GetError());
			}
		}

		/* Present and then wait for the next frame to begin */
		if (!_autograder_mode)
			::SDL_RenderPresent(renderer); // The autograder doesn't need to render to window.
		SDL_Delay();
		frame_number++;
	}

	/* FEATURE : Render Logger */
	/* Create a "RENDERLOGGER" environmental variable, run your engine, and check render_logger.txt. */
	/* Use to compare to test case render_logger.txt files to see what goes wrong in your render. */
	static inline RenderLoggerStatus render_logger_mode = RL_NOT_INITIALIZED;
	static inline std::ofstream render_logging_file;
	static void CheckForRenderLoggerInit()
	{
		/* Check environmental variable on first call. */
		if (render_logger_mode == RL_NOT_INITIALIZED)
		{
			if (IsLoggingMode())
			{
				render_logger_mode = RL_ENABLED;
				std::ofstream file("render_logger.txt", std::ios::out); // delete the file if it exists.
				render_logging_file.open("render_logger.txt", std::ios::app);

				if (!render_logging_file.is_open())
				{
					std::cerr << "Error : Failed to open render_logger.txt for writing." << std::endl;
				}

				render_logging_file << "== RENDER LOGGER ==" << std::endl;
				render_logging_file << "Study the following SDL_RenderCopyEx() calls to debug render-related issues." << std::endl;
				render_logging_file << "Enable render logger mode on your computer by setting the RENDERLOGGER environmental variable." << std::endl;
				render_logging_file << "frame:actor_id:actor_name" << std::endl << std::endl;
			}
			else
			{
				render_logger_mode = RL_NOT_ENABLED;
			}
		}
	}

	static void SDL_RenderCopyEx(int actor_id, const std::string& actor_name, SDL_Renderer* renderer, SDL_Texture* texture, const SDL_FRect* srcrect, const SDL_FRect* dstrect, const float angle, const SDL_FPoint* center, const SDL_RendererFlip flip)
	{
		SDL_Rect* srcrect_i_ptr = nullptr;
		SDL_Rect* dstrect_i_ptr = nullptr;
		SDL_Point* center_i_ptr = nullptr;

		SDL_Rect srcrect_i, dstrect_i;

		SDL_Point center_i;

		if (srcrect != nullptr)
		{
			srcrect_i = { static_cast<int>(srcrect->x), static_cast<int>(srcrect->y), static_cast<int>(srcrect->w), static_cast<int>(srcrect->h) };
			srcrect_i_ptr = &srcrect_i;
		}
		
		if (dstrect != nullptr)
		{
			dstrect_i = { static_cast<int>(dstrect->x), static_cast<int>(dstrect->y), static_cast<int>(dstrect->w), static_cast<int>(dstrect->h) };
			dstrect_i_ptr = &dstrect_i;
		}

		if (center != nullptr)
		{
			center_i = { static_cast<int>(center->x), static_cast<int>(center->y) };
			center_i_ptr = &center_i;
		}

		/* Perform the render like normal. */
		::SDL_RenderCopyEx(renderer, texture, srcrect_i_ptr, dstrect_i_ptr, angle, center_i_ptr, flip);

		CheckForRenderLoggerInit();

		/* Log render operation to file if necessary */
		if (render_logger_mode == RL_ENABLED)
		{
			float x_scale = 1;
			float y_scale = 1;
			SDL_RenderGetScale(renderer, &x_scale, &y_scale);

			render_logging_file << GetFrameNumber() << ":" << actor_id << ":" << actor_name;
			if (dstrect != nullptr)
				render_logging_file << " dstrect " << dstrect->x << " " << dstrect->y << " " << dstrect->w << " " << dstrect->h;
			
			render_logging_file << " angle " << angle;
			if (center != nullptr)
				render_logging_file << " center " << center->x << " " << center->y;
			
			render_logging_file << " flip " << flip << " renderscale " << x_scale << " " << y_scale << std::endl;
		}
	}

	/* This encourages students to keep their data in float form as long as possible. We handle truncating to ints at the very end for them, as necessary. */
	static void SDL_RenderCopy(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_FRect * srcrect, const SDL_FRect * dstrect)
	{
		SDL_Rect* srcrect_i_ptr = nullptr;
		SDL_Rect* dstrect_i_ptr = nullptr;

		SDL_Rect srcrect_i, dstrect_i;

		if (srcrect != nullptr)
		{
			srcrect_i = { static_cast<int>(srcrect->x), static_cast<int>(srcrect->y), static_cast<int>(srcrect->w), static_cast<int>(srcrect->h) };
			srcrect_i_ptr = &srcrect_i;
		}

		if (dstrect != nullptr)
		{
			dstrect_i = { static_cast<int>(dstrect->x), static_cast<int>(dstrect->y), static_cast<int>(dstrect->w), static_cast<int>(dstrect->h) };
			dstrect_i_ptr = &dstrect_i;
		}

		::SDL_RenderCopy(renderer, texture, srcrect_i_ptr, dstrect_i_ptr);
	}

	static void SDL_QueryTexture(SDL_Texture* texture, float* w, float* h)
	{
		if (texture == nullptr)
			return;

		int w_i, h_i;
		::SDL_QueryTexture(texture, NULL, NULL, &w_i, &h_i);

		if (w != nullptr)
			*w = static_cast<float>(w_i);

		if (h != nullptr)
			*h = static_cast<float>(h_i);
	}

private:
	static inline std::unordered_map<int, std::queue<SDL_Event>> frame_to_user_input;
	static inline InputStatus input_status = NOT_INITIALIZED;
	static inline std::ofstream recording_file;

	/* Do not use SDL_GetKeyboardState(), as it will not observe the input file. */
	static void SDL_ConsiderInputFile()
	{
		/* Lazy Initialize */
		if (input_status == NOT_INITIALIZED)
		{
			LoadSDLEventsFromInputFile();
		}

		if (input_status == INPUT_FILE_PRESENT)
		{
			if (frame_to_user_input.find(frame_number) != frame_to_user_input.end())
			{
				while (!frame_to_user_input[frame_number].empty())
				{
					SDL_PushEvent(&(frame_to_user_input[frame_number].front()));
					frame_to_user_input[frame_number].pop();
				}
			}
		}

		/* Recording mode (primarily for course staff usage) */
		if (RECORDING_MODE && input_status != INPUT_FILE_PRESENT && !_autograder_mode) {

			/* Peek at incoming events so we may record them before the student code consumes them. */
			SDL_PumpEvents();
			SDL_Event incoming_events[100];
			int num_events = SDL_PeepEvents(incoming_events, 100, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);

			/* Filter for relevant events */
			std::vector<SDL_Event> relevant_events;
			for (int i = 0; i < num_events; i++) {

				Uint32 event_type = incoming_events[i].type;
				if (event_type == SDL_KEYUP || event_type == SDL_KEYDOWN || event_type == SDL_MOUSEMOTION || event_type == SDL_MOUSEBUTTONDOWN || event_type == SDL_MOUSEBUTTONUP || event_type == SDL_MOUSEWHEEL || event_type == SDL_QUIT)
					relevant_events.push_back(incoming_events[i]);
			}

			/* If there are any events we care about, write them to file. */
			if (relevant_events.size() > 0)
			{
				if (!recording_file.is_open())
				{
					recording_file.open("recorded_sdl_user_input.txt");
				}

				/* Every line begins with a frame number */
				recording_file << frame_number << ";";

				/* Each frame may have multiple events we need to package up. */
				for (int i = 0; i < relevant_events.size(); i++) {

					/* Every event begins with an event type */
					Uint32 event_type = relevant_events[i].type;
					recording_file << event_type << ",";

					/* Each event could be of multiple types, with differing numbers of parameters. */
					if (event_type == SDL_KEYDOWN || event_type == SDL_KEYUP)
					{
						SDL_Scancode keycode = relevant_events[i].key.keysym.scancode;
						recording_file << keycode;
					}
					else if (event_type == SDL_MOUSEMOTION)
					{
						Sint32 x = relevant_events[i].motion.x;
						Sint32 y = relevant_events[i].motion.y;
						recording_file << x << "," << y;
					}
					else if (event_type == SDL_MOUSEBUTTONDOWN)
					{
						int button_index = static_cast<int>(relevant_events[i].button.button);
						recording_file << button_index;
					}
					else if (event_type == SDL_MOUSEBUTTONUP)
					{
						int button_index = static_cast<int>(relevant_events[i].button.button);
						recording_file << button_index;
					}
					else if (event_type == SDL_MOUSEWHEEL)
					{
						float scroll_amount = relevant_events[i].wheel.preciseY;
						recording_file << scroll_amount;
					}

					/* Every event ends with a semicolon */
					recording_file << ";";
				}

				/* Every line ends with newline, denoting the end of the frame. */
				recording_file << std::endl;
			}

			// Program will auto-close recording_user_input.txt file when it exits.
		}
	}

	static bool IsEnvVariableSet(const char* env_variable_name)
	{
		/* Visual C++ does not like std::getenv */
#ifdef _WIN32
		char* val = nullptr;
		size_t length = 0;
		_dupenv_s(&val, &length, env_variable_name);
		if (val) {
			free(val);
			return true;
		}
		else {
			return false;
		}
#else
		const char* autograder_mode_env_variable = std::getenv(env_variable_name);
		if (autograder_mode_env_variable)
			return true;
		else
			return false;
#endif

		return false;
	}

	static bool IsAutograderMode() {
		return IsEnvVariableSet("AUTOGRADER");
	}

	static bool IsLoggingMode() {
		return IsEnvVariableSet("RENDERLOGGER");
	}

	/* The engine will aim for 60fps (16ms per frame) during a normal play session. */
	/* If the engine detects it is being autograded, it will run as fast as possible. */
	static void SDL_Delay() {

		if (_autograder_mode)
		{
			//::SDL_Delay(1); Don't bother delaying at all. Gotta go fast when autograding.
		}
		else
		{
			Uint32 current_frame_end_timestamp = SDL_GetTicks();  // Record end time of the frame
			Uint32 current_frame_duration_milliseconds = current_frame_end_timestamp - current_frame_start_timestamp;
			Uint32 desired_frame_duration_milliseconds = 16;

			int delay_ticks = std::max(static_cast<int>(desired_frame_duration_milliseconds) - static_cast<int>(current_frame_duration_milliseconds), 1);

			::SDL_Delay(delay_ticks);
		}

		current_frame_start_timestamp = SDL_GetTicks();  // Record start time of the frame
	}

	static void LoadSDLEventsFromInputFile()
	{
		if (IsAutograderMode())
			_autograder_mode = true;

		if (!std::filesystem::exists(USER_INPUT_FILENAME))
		{
			input_status = INPUT_FILE_MISSING;
			return;
		}

		std::ifstream infile(USER_INPUT_FILENAME);
		std::string line;

		while (std::getline(infile, line)) {
			std::istringstream iss(line);
			std::string eventStr, frameStr;

			std::getline(iss, frameStr, ';');
			int frameNumber = std::stoi(frameStr);

			if (frame_to_user_input.find(frameNumber) == frame_to_user_input.end())
				frame_to_user_input[frameNumber] = std::queue<SDL_Event>();

			std::queue<SDL_Event> & new_queue = frame_to_user_input[frameNumber];

			while (std::getline(iss, eventStr, ';')) {

				/* Identify event type */
				std::istringstream eventStream(eventStr);
				std::string event_type_string;
				std::getline(eventStream, event_type_string, ',');

				// Remove any '\r' carriage returns we might find
				// (may be needed for stoi to work on osx / linux when sdl_user_input.txt has been authored on windows)
				// (depends on how student chooses to download the test cases)
				event_type_string.erase(std::remove(event_type_string.begin(), event_type_string.end(), '\r'), event_type_string.end());

				if (event_type_string == "")
					continue;

				Uint32 event_type = std::stoi(event_type_string);
				SDL_Event fabricated_sdl_event;
				fabricated_sdl_event.type = event_type;

				/* Handle unpacking for different event types */

				if (event_type == SDL_KEYUP || event_type == SDL_KEYDOWN)
				{
					std::string keycode;
					std::getline(eventStream, keycode, ',');
					if (keycode == "")
						continue;

					fabricated_sdl_event.key.keysym.scancode = static_cast<SDL_Scancode>(std::stoi(keycode));
				}
				else if (event_type == SDL_MOUSEMOTION)
				{
					std::string x_str;
					std::getline(eventStream, x_str, ',');
					std::string y_str;
					std::getline(eventStream, y_str, ',');

					if (x_str == "" || y_str == "")
						continue;

					fabricated_sdl_event.motion.x = static_cast<Sint32>(std::stoi(x_str));
					fabricated_sdl_event.motion.y = static_cast<Sint32>(std::stoi(y_str));
				}
				else if (event_type == SDL_MOUSEBUTTONDOWN || event_type == SDL_MOUSEBUTTONUP)
				{
					std::string mouse_button_index_str;
					std::getline(eventStream, mouse_button_index_str, ',');

					if (mouse_button_index_str == "")
						continue;

					fabricated_sdl_event.button.button = static_cast<Uint8>(std::stoi(mouse_button_index_str));
				}
				else if (event_type == SDL_MOUSEWHEEL)
				{
					std::string mouse_wheel_movement_str;
					std::getline(eventStream, mouse_wheel_movement_str, ','); 

					if (mouse_wheel_movement_str == "")
						continue;

					fabricated_sdl_event.wheel.preciseY = std::stof(mouse_wheel_movement_str);
				}

				new_queue.push(fabricated_sdl_event);
			}
		}

		input_status = INPUT_FILE_PRESENT;
	}
};

/* Utilized for homework 9 */
class RandomEngine
{
	std::default_random_engine engine;
	std::uniform_real_distribution<float> distribution;

public:
	/* Please look up the correct seed value for your usage purposes. Find it in the homework9 assignment spec. */
	RandomEngine(float min, float max, int seed_look_up_in_assignment_spec_pls)
	{
		Configure(min, max, seed_look_up_in_assignment_spec_pls);
	}

	/* Please look up the correct seed value for your usage purposes. Find it in the homework9 assignment spec. */
	void Configure(float min, float max, int seed_look_up_in_assignment_spec_pls)
	{
		engine = std::default_random_engine(seed_look_up_in_assignment_spec_pls);
		distribution = std::uniform_real_distribution<float>(min, max);
	}

	/* If you use this constructor, don't forget to configure the engine later. */
	RandomEngine() {}

	/* Get a random number in the specified range. */
	inline float Sample()
	{
		return distribution(engine);
	}
};

#endif
