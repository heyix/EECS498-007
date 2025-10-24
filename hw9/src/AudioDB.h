#pragma once
#include <string>
#include "SDL2_mixer/SDL_mixer.h"
#include<unordered_map>
#include "AudioHelper.h"
class AudioDB {
private:
	static std::string folder_path;
public:
	static std::unordered_map<std::string, Mix_Chunk*> loaded_audio;
public:
	static void Clean_Loaded_Audio();
	static Mix_Chunk* Load_Audio(const std::string& audio_name);
	static void Init_Audio_Channel(int frequency, Uint16 format, int channels, int chunksize);
	static void Play_Audio(int channel, const std::string& audio_name, int loops);
	static void Halt_Audio(int channel);
public:
	static void Lua_Play(int channel, const std::string& clip_name, bool does_loop);
	static void Lua_Halt(int channel);
	static void Lua_Set_Volume(int channel, float volume);
}; 