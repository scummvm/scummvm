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

	Audio::SoundHandle *_musicHandle;

public:
	// The notification sound
	ChunkKey _notify, _repInc, _repDec;

	MusicManager() {
		_notify = -1;
		_repInc = -1;
		_repDec = -1;

		_musicHandle = nullptr;
	}
	~MusicManager() {}

	bool load(rapidxml::xml_node<char> *node);

	void syncSettings();

	void playMusic(const MusicKey &id);
	void playEffect(const ChunkKey &id, const int &loops);

	static void pause() {
		g_system->getMixer()->pauseAll(true);
	}

	static void resume() {
		g_system->getMixer()->pauseAll(false);
	}

	static void stop() {
		g_system->getMixer()->stopAll();
	}

	static void volEffects(const int &volume, const bool &unmute = false) {
		if (unmute)
			g_system->getMixer()->muteSoundType(Audio::Mixer::kSFXSoundType, false);
		g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, volume);
	}

	static int volEffects() {
		return g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kSFXSoundType);
	}

	static void volMusic(const int &volume, const bool &unmute = false) {
		if (unmute)
			g_system->getMixer()->muteSoundType(Audio::Mixer::kMusicSoundType, false);
		g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, volume);
	}

	static int volMusic() {
		return g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kMusicSoundType);
	}

	void saveState();

	void freeMusic();
	void freeChunk();

	void quit() {
		g_system->getMixer()->stopAll();

		freeMusic();
		freeChunk();
	}
};

} // End of namespace music
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_MUSICMANAGER_H
