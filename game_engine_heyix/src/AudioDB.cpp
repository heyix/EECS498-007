#include "AudioDB.h"
std::string AudioDB::folder_path = "audio/";
std::unordered_map<std::string, Mix_Chunk*> AudioDB::loaded_audio;
void AudioDB::Clean_Loaded_Audio()
{
}

Mix_Chunk* AudioDB::Load_Audio(const std::string& audio_name)
{
	if (loaded_audio.find(audio_name) != loaded_audio.end()) {
		return loaded_audio[audio_name];
	}
	Mix_Chunk* wav = nullptr;
	Mix_Chunk* ogg = nullptr;
	if (EngineUtils::Resource_File_Exist(folder_path + audio_name + ".ogg")) {
		ogg = AudioHelper::Mix_LoadWAV((EngineUtils::Get_Resource_File_Path(folder_path + audio_name + ".ogg")).c_str());
	}
	if (EngineUtils::Resource_File_Exist(folder_path + audio_name + ".wav")) {
		wav = AudioHelper::Mix_LoadWAV((EngineUtils::Get_Resource_File_Path(folder_path + audio_name + ".wav")).c_str());
	}
	if (wav == nullptr && ogg == nullptr) {
		std::cout << "error: failed to play audio clip " + audio_name;
		exit(0);
	}
	if (wav != nullptr) {
		loaded_audio[audio_name] = wav;
	}
	else {
		loaded_audio[audio_name] = ogg;
	}
	return loaded_audio[audio_name];
}

void AudioDB::Init_Audio_Channel(int frequency, Uint16 format, int channels, int chunksize)
{
	AudioHelper::Mix_OpenAudio(frequency, format, channels, chunksize);
}

void AudioDB::Play_Audio(int channel,std::string audio_name,int loops)
{
	AudioHelper::Mix_PlayChannel(channel, Load_Audio(audio_name), loops);
}

void AudioDB::Halt_Audio(int channel)
{
	AudioHelper::Mix_HaltChannel(channel);
}
