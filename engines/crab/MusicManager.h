#pragma once

#include "GameParam.h"
#include "common_header.h"
#include "musicparam.h"

namespace pyrodactyl {
namespace music {
class MusicManager {
	// The background music for our current level
	MusicData bg;

	// The sound effects in the game
	std::unordered_map<ChunkKey, Mix_Chunk *> effect;

	// Data about the audio
	int freq, channels, chunksize;

public:
	// The notification sound
	ChunkKey notify, rep_inc, rep_dec;

	MusicManager() {
		freq = MIX_DEFAULT_FREQUENCY;
		channels = MIX_DEFAULT_CHANNELS;
		chunksize = 1024;

		notify = -1;
		rep_inc = -1;
		rep_dec = -1;
	}
	~MusicManager() {}

	bool Load(rapidxml::xml_node<char> *node);

	void PlayMusic(const MusicKey &id);
	void PlayEffect(const ChunkKey &id, const int &loops);

	static void Pause() { Mix_PauseMusic(); }
	static void Resume() { Mix_ResumeMusic(); }
	static void Stop() { Mix_HaltMusic(); }

	static void VolEffects(const int &volume) { Mix_Volume(-1, volume); }
	static int VolEffects() { return Mix_Volume(0, -1); }

	static void VolMusic(const int &volume) { Mix_VolumeMusic(volume); }
	static int VolMusic() { return Mix_VolumeMusic(-1); }

	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);

	void FreeMusic();
	void FreeChunk();
	void Quit() {
		FreeMusic();
		FreeChunk();
	}
};

extern MusicManager gMusicManager;
} // End of namespace music
} // End of namespace pyrodactyl