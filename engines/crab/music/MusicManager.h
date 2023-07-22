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

#include "audio/mixer.h"
#include "audio/decoders/wave.h"
#include "crab/GameParam.h"
#include "crab/common_header.h"
#include "crab/music/musicparam.h"

namespace Crab {

namespace pyrodactyl {
namespace music {
class MusicManager {
	struct EffectAudio {
		Common::File _file;
		Audio::SeekableAudioStream *_stream;
		Audio::SoundHandle *_handle;
	};

	// The background music for our current level
	MusicData _bg;

	Common::HashMap<ChunkKey, EffectAudio *> _effects;
#if 0
	// The sound effects in the game
	std::unordered_map<ChunkKey, Mix_Chunk *> effect;
#endif

	// Data about the audio
	int _freq, _channels, _chunksize;

	Audio::SoundHandle *_musicHandle;

public:
	// The notification sound
	ChunkKey _notify, _repInc, _repDec;

	MusicManager() {
#if 0
		freq = MIX_DEFAULT_FREQUENCY;
		channels = MIX_DEFAULT_CHANNELS;
#endif
		_chunksize = 1024;

		_notify = -1;
		_repInc = -1;
		_repDec = -1;

		(void)_freq;
		(void)_channels;
		(void)_chunksize;

		_musicHandle = nullptr;
	}
	~MusicManager() {}

	bool load(rapidxml::xml_node<char> *node);

	void playMusic(const MusicKey &id);
	void playEffect(const ChunkKey &id, const int &loops);

	static void pause() {
		warning("STUB: MusicManager::pause()");

#if 0
		Mix_PauseMusic();
#endif

	}

	static void resume() {
		warning("STUB: MusicManager::resume()");

#if 0
		Mix_ResumeMusic();
#endif

	}

	static void stop() {
		warning("STUB: MusicManager::stop()");

#if 0
		Mix_HaltMusic();
#endif

	}

	static void volEffects(const int &volume) {
		warning("STUB: MusicManager::stop()");

#if 0
		Mix_Volume(-1, volume);
#endif

	}

	static int volEffects() {
		warning("STUB: MusicManager::volEffects()");
		return 0;
#if 0
		return Mix_Volume(0, -1);
#endif

	}

	static void volMusic(const int &volume) {
		warning("STUB: MusicManager::volMusic() %d", volume);

#if 0
		Mix_VolumeMusic(volume);
#endif

	}

	static int volMusic() {
		warning("STUB: MusicManager::volMusic()");
		return 0;

#if 0
		return Mix_VolumeMusic(-1);
#endif

	}

	void saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);

	void freeMusic();
	void freeChunk();

	void quit() {
		freeMusic();
		freeChunk();
	}
};

} // End of namespace music
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_MUSICMANAGER_H
