/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_MUSICMANAGER_H
#define CRAB_MUSICMANAGER_H

#include "crab/GameParam.h"
#include "crab/common_header.h"
#include "crab/music/musicparam.h"

namespace Crab {

namespace pyrodactyl {
namespace music {
class MusicManager {
	// The background music for our current level
	MusicData bg;

#if 0
	// The sound effects in the game
	std::unordered_map<ChunkKey, Mix_Chunk *> effect;
#endif

	// Data about the audio
	int freq, channels, chunksize;

public:
	// The notification sound
	ChunkKey notify, rep_inc, rep_dec;

	MusicManager() {
#if 0
		freq = MIX_DEFAULT_FREQUENCY;
		channels = MIX_DEFAULT_CHANNELS;
#endif
		chunksize = 1024;

		notify = -1;
		rep_inc = -1;
		rep_dec = -1;

		(void)freq;
		(void)channels;
		(void)chunksize;
	}
	~MusicManager() {}

	bool Load(rapidxml::xml_node<char> *node);

	void PlayMusic(const MusicKey &id);
	void PlayEffect(const ChunkKey &id, const int &loops);

	static void Pause() {
		warning("STUB: MusicManager::Pause()");

#if 0
		Mix_PauseMusic();
#endif

	}

	static void Resume() {
		warning("STUB: MusicManager::Resume()");

#if 0
		Mix_ResumeMusic();
#endif

	}

	static void Stop() {
		warning("STUB: MusicManager::Stop()");

#if 0
		Mix_HaltMusic();
#endif

	}

	static void VolEffects(const int &volume) {
		warning("STUB: MusicManager::Stop()");

#if 0
		Mix_Volume(-1, volume);
#endif

	}

	static int VolEffects() {
		warning("STUB: MusicManager::VolEffects()");
		return 0;
#if 0
		return Mix_Volume(0, -1);
#endif

	}

	static void VolMusic(const int &volume) {
		warning("STUB: MusicManager::VolMusic() %d", volume);

#if 0
		Mix_VolumeMusic(volume);
#endif

	}

	static int VolMusic() {
		warning("STUB: MusicManager::VolMusic()");
		return 0;

#if 0
		return Mix_VolumeMusic(-1);
#endif

	}

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

} // End of namespace Crab

#endif // CRAB_MUSICMANAGER_H
