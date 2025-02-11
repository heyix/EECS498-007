#ifndef AUDIOHELPER_H
#define AUDIOHELPER_H

#define AUDIO_HELPER_VERSION 0.8

#include <iostream>
#include <filesystem>

/* WARNING : You may need to adjust the following include paths if your headers / file structures are different. */
/* Here is the instructor solution folder structure (if we make $(ProjectDir) a include directory, these paths are valid. */
/* https://bit.ly/3OClfHc */
#include "SDL2_mixer/SDL_mixer.h"
#include "Helper.h"

class AudioHelper {
	public:
		static inline Mix_Chunk* Mix_LoadWAV(const char* file)
		{
			if (!IsAutograderMode())
				return ::Mix_LoadWAV(file);
			else
			{
				if (std::filesystem::exists(file))
					return &autograder_dummy_sound;
				else
					return nullptr;
			}
		}
		
		static inline int Mix_PlayChannel(int channel, Mix_Chunk *chunk, int loops)
		{
			std::cout << "(Mix_PlayChannel(" << channel << ",?," << loops << ") called on frame " << Helper::GetFrameNumber() << ")" << std::endl;
			
			if (!IsAutograderMode())
				return ::Mix_PlayChannel(channel, chunk, loops);
			else
			{
				return channel;
			}
		}

		static inline int Mix_OpenAudio(int frequency, Uint16 format, int channels, int chunksize)
		{
			if (!IsAutograderMode())
				return ::Mix_OpenAudio(frequency, format, channels, chunksize);
			else
				return 0;
		}

		static inline int Mix_AllocateChannels(int numchans)
		{
			if (!IsAutograderMode())
				return ::Mix_AllocateChannels(numchans);
			else
				return numchans;
		}
				
		static inline void Mix_Pause(int channel)
		{
			std::cout << "(Mix_Pause(" << channel << ") called on frame " << Helper::GetFrameNumber() << ")" << std::endl;
			
			if (!IsAutograderMode())
				::Mix_Pause(channel);
		}
		static inline void Mix_Resume(int channel)
		{
			std::cout << "(Mix_Resume(" << channel << ") called on frame " << Helper::GetFrameNumber() << ")" << std::endl;
			
			if (!IsAutograderMode())
				::Mix_Resume(channel);
		}
		
		static inline int Mix_HaltChannel(int channel)
		{
			std::cout << "(Mix_HaltChannel(" << channel << ") called on frame " << Helper::GetFrameNumber() << ")" << std::endl;
			
			if (!IsAutograderMode())
				return ::Mix_HaltChannel(channel);
			else
				return 0;
		}
		
		static inline int Mix_Volume(int channel, int volume)
		{
			std::cout << "(Mix_Volume(" << channel << "," << volume << ") called on frame " << Helper::GetFrameNumber() << ")" << std::endl;
			
			if (!IsAutograderMode())
				return ::Mix_Volume(channel, volume);
			else
				return 0;
		}			
		
		static inline void Mix_CloseAudio(void)
		{
			if (!IsAutograderMode())
				::Mix_CloseAudio();
		}
	
	private:
		static inline Mix_Chunk autograder_dummy_sound;

		/* It's best for everyone if the autograder doesn't actually play any audio (it will still check that you try though). */
		/* Imagine poor Professor Kloosterman hearing the autograder all day as it spams various sounds in my office. */
		static inline bool IsAutograderMode() {
			/* Visual C++ does not like std::getenv */
#ifdef _WIN32
			char* val = nullptr;
			size_t length = 0;
			_dupenv_s(&val, &length, "AUTOGRADER");
			if (val) {
				free(val);
				return true;
			}
			else {
				return false;
			}
#else
			const char* autograder_mode_env_variable = std::getenv("AUTOGRADER");
			if (autograder_mode_env_variable)
				return true;
			else
				return false;
#endif

			return false;
		}
};

#endif
