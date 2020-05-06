/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima4/sound/sound.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/sound/music.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/filesys/u4file.h"
#include "ultima/ultima4/core/utils.h"

namespace Ultima {
namespace Ultima4 {

int soundInit(void) {
	return SoundManager::getInstance()->init();
}

void soundDelete(void) {
	delete SoundManager::getInstance();
}

void soundPlay(Sound sound, bool onlyOnce, int specificDurationInTicks) {
	SoundManager::getInstance()->play(sound, onlyOnce, specificDurationInTicks);
}

void soundStop(int channel) {
	SoundManager::getInstance()->stop(channel);
}

/*-------------------------------------------------------------------*/

SoundManager *SoundManager::_instance = 0;

SoundManager::SoundManager() {
}

SoundManager::~SoundManager() {
	del();
	_instance = 0;
}

SoundManager *SoundManager::getInstance() {
	if (!_instance)
		_instance = new SoundManager();
	return _instance;
}

int SoundManager::init() {
	/*
	 * load sound track filenames from xml config file
	 */
	const Config *config = Config::getInstance();
	_soundFilenames.reserve(SOUND_MAX);
	_soundChunk.resize(SOUND_MAX);

	Std::vector<ConfigElement> soundConfs = config->getElement("sound").getChildren();
	Std::vector<ConfigElement>::const_iterator i = soundConfs.begin();
	Std::vector<ConfigElement>::const_iterator theEnd = soundConfs.end();
	for (; i != theEnd; ++i) {
		if (i->getName() != "track")
			continue;

		_soundFilenames.push_back(i->getString("file"));
	}
	return init_sys();
}

bool SoundManager::load(Sound sound) {
	ASSERT(sound < SOUND_MAX, "Attempted to load an invalid sound in soundLoad()");

	// If music didn't initialize correctly, then we can't play it anyway
	if (!Music::_functional || !settings._soundVol)
		return false;

	if (_soundChunk[sound] == nullptr) {
		Common::String pathname(u4find_sound(_soundFilenames[sound]));
		Common::String basename = pathname.substr(pathname.findLastOf("/") + 1);
		if (!basename.empty())
			return load_sys(sound, pathname);
	}
	return true;
}

void SoundManager::play(Sound sound, bool onlyOnce, int specificDurationInTicks) {

	ASSERT(sound < SOUND_MAX, "Attempted to play an invalid sound in soundPlay()");

	// If music didn't initialize correctly, then we can't play it anyway
	if (!Music::_functional || !settings._soundVol)
		return;

	if (_soundChunk[sound] == nullptr) {
		if (!load(sound)) {
			return;
		}
	}

	play_sys(sound, onlyOnce, specificDurationInTicks);
}

void SoundManager::stop(int channel) {
	stop_sys(channel);
}

bool SoundManager::load_sys(Sound sound, const Common::String &pathname) {
#ifdef TODO
	soundChunk[sound] = Mix_LoadWAV(pathname.c_str());
	if (!soundChunk[sound]) {
		warning("Unable to load sound effect file %s: %s", soundFilenames[sound].c_str(), Mix_GetError());
		return false;
	}
#endif
	return true;
}

void SoundManager::play_sys(Sound sound, bool onlyOnce, int specificDurationInTicks) {
#ifdef TODO
	/**
	 * Use Channel 1 for sound effects
	 */
	if (!onlyOnce || !Mix_Playing(1)) {
		if (Mix_PlayChannelTimed(1, soundChunk[sound], specificDurationInTicks == -1 ? 0 : -1, specificDurationInTicks) == -1)
			fprintf(stderr, "Error playing sound %d: %s\n", sound, Mix_GetError());
	}
#endif
}

void SoundManager::stop_sys(int channel) {
#ifdef TODO
	// If music didn't initialize correctly, then we shouldn't try to stop it
	if (!g_music->functional || !settings.soundVol)
		return;

	if (Mix_Playing(channel))
		Mix_HaltChannel(channel);
#endif
}

int SoundManager::init_sys() {
	return 1;
}

void SoundManager::del_sys() {
}

} // End of namespace Ultima4
} // End of namespace Ultima
