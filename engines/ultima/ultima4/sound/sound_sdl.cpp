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

#include "ultima/ultima4/sound/sound_p.h"
#include "ultima/ultima4/sound/sound.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/core/debug.h"
#include "ultima/ultima4/core/error.h"
#include "ultima/ultima4/sound/music.h"
#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/filesys/u4file.h"

namespace Ultima {
namespace Ultima4 {

bool SoundManager::load_sys(Sound sound, const Common::String &pathname) {
#ifdef TODO
	soundChunk[sound] = Mix_LoadWAV(pathname.c_str());
	if (!soundChunk[sound]) {
		errorWarning("Unable to load sound effect file %s: %s", soundFilenames[sound].c_str(), Mix_GetError());
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
	if (!musicMgr->functional || !settings.soundVol)
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
