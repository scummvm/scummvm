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
 */

#ifndef SHERLOCK_SOUND_H
#define SHERLOCK_SOUND_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Sherlock {

class SherlockEngine;

enum WaitType {
	WAIT_RETURN_IMMEDIATELY = 0, WAIT_FINISH = 1, WAIT_KBD_OR_FINISH = 2
};

class Sound {
private:
	SherlockEngine *_vm;
public:
	bool _sfxEnabled;
	bool _musicEnabled;
	bool _voicesEnabled;
	bool _playingEpilogue;
	bool _music;
public:
	Sound(SherlockEngine *vm);

	void loadSound(const Common::String &name, int priority);
	void playSound(const Common::String &name, WaitType waitType = WAIT_RETURN_IMMEDIATELY);
	void cacheSound(const Common::String &name, int index);
	void playCachedSound(int index);
	void clearCache();
	void stopSound();
	int loadSong(int songNumber);
	void startSong();
	void freeSong();
	
	void playMusic(const Common::String &name);
	void stopMusic();
};

} // End of namespace Sherlock

#endif
