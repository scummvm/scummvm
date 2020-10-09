/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef GRIM_SOUND_H
#define GRIM_SOUND_H

#include "engines/grim/savegame.h"

namespace Grim {

class EMISound;

class SoundPlayer {
public:
	bool startVoice(const char *soundName, int volume = 127, int pan = 64);
	bool getSoundStatus(const char *soundName);
	void stopSound(const char *soundName);
	int32 getPosIn16msTicks(const char *soundName);

	void setVolume(const char *soundName, int volume);
	void setPan(const char *soundName, int pan);

	void setMusicState(int stateId);
	void flushTracks();
// Save/Load:
	void restoreState(SaveGame *savedState);
	void saveState(SaveGame *savedState);
};

extern SoundPlayer *g_sound;

} // end of namespace Grim

#endif
