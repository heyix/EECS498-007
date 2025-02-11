#pragma once
#include <string>
#include "SDL2_mixer/SDL_mixer.h"
#include<unordered_map>
#include "EngineUtils.h"
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
	static void Play_Audio(int channel, std::string audio_name, int loops);
	static void Halt_Audio(int channel);
};