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
#include "crab/musicparam.h"

namespace Crab {

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

} // End of namespace Crab

#endif // CRAB_MUSICMANAGER_H
