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
#include "ultima/ultima4/core/debug.h"
#include "ultima/ultima4/core/error.h"
#include "ultima/ultima4/sound/music.h"
#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/filesys/u4file.h"
#include "ultima/ultima4/sound/sound_p.h"

namespace Ultima {
namespace Ultima4 {

using Common::String;
using Std::vector;

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

	vector<ConfigElement> soundConfs = config->getElement("sound").getChildren();
	vector<ConfigElement>::const_iterator i = soundConfs.begin();
	vector<ConfigElement>::const_iterator theEnd = soundConfs.end();
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
	if (!Music::functional || !settings._soundVol)
		return false;

	if (_soundChunk[sound] == NULL) {
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
	if (!Music::functional || !settings._soundVol)
		return;

	if (_soundChunk[sound] == NULL) {
		if (!load(sound)) {
			return;
		}
	}

	play_sys(sound, onlyOnce, specificDurationInTicks);
}

void SoundManager::stop(int channel) {
	stop_sys(channel);
}

} // End of namespace Ultima4
} // End of namespace Ultima
