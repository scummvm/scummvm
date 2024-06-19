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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/engines/m4/midi.cpp $
 * $Id: midi.cpp 47705 2010-01-30 09:21:07Z dreammaster $
 *
 */

#include "common/stream.h"

#include "prisoner/prisoner.h"
#include "prisoner/resource.h"
#include "prisoner/midi.h"

namespace Prisoner {

void PrisonerEngine::playMusic(int16 musicIndex) {
	if (musicIndex >= 0) {
		MusicSlot &musicSlot = _musics[musicIndex];
		stopSound(musicIndex);
		// If no other volume was set use the default one
		if (!musicSlot.volumeFlag)
			setMusicVolume(musicIndex, 100);
		musicSlot.isPlaying = true;
		musicSlot.shouldResume = false;
		MidiResource *midiResource = _res->get<MidiResource>(musicSlot.resourceCacheSlot);
		_midi->playMusic(midiResource, musicSlot.volume, true);
	}
}

void PrisonerEngine::stopMusic(int16 musicIndex) {
	if (musicIndex >= 0) {
		MusicSlot &musicSlot = _musics[musicIndex];
		_midi->stopMusic();
		musicSlot.isPlaying = false;
		musicSlot.shouldResume = false;
	}
}

bool PrisonerEngine::isMusicPlaying(int16 musicIndex) {
	if (musicIndex >= 0) {
		MusicSlot &musicSlot = _musics[musicIndex];
		return musicSlot.isPlaying;
	} else
		return false;
}

void PrisonerEngine::setMusicVolume(int16 musicIndex, uint volume) {
	if (musicIndex >= 0) {
		MusicSlot &musicSlot = _musics[musicIndex];
		musicSlot.volume = volume;
		musicSlot.volumeFlag = true;
		// Update the volume if the sound is playing
		if (musicSlot.isPlaying) {
			_midi->setVolume(volume);
		}
	}
}

int16 PrisonerEngine::loadMusic(Common::String &pakName, int16 pakSlot, bool moduleWide) {
	int16 musicIndex = _musics.getFreeSlot();
	MusicSlot &musicSlot = _musics[musicIndex];
	musicSlot.resourceCacheSlot = _res->load<MidiResource>(pakName, pakSlot, 12);
	musicSlot.moduleWide = moduleWide;
	musicSlot.shouldResume = false;
	musicSlot.volumeFlag = false;
	return musicIndex;
}

void PrisonerEngine::unloadMusic(int16 musicIndex) {
	if (musicIndex >= 0) {
		MusicSlot &musicSlot = _musics[musicIndex];
		stopMusic(musicIndex);
		_res->unload(musicSlot.resourceCacheSlot);
		musicSlot.resourceCacheSlot = -1;
	}
}

void PrisonerEngine::unloadMusics(bool all) {
	for (int16 musicIndex = 0; musicIndex < kMaxMusics; musicIndex++) {
		MusicSlot &musicSlot = _musics[musicIndex];
		if (musicSlot.resourceCacheSlot != -1 && (all || !musicSlot.moduleWide)) {
			stopMusic(musicIndex);
			_res->unload(musicSlot.resourceCacheSlot);
			musicSlot.resourceCacheSlot = -1;
		}
	}
}

} // End of namespace Prisoner
