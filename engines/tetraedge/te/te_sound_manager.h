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

#include "audio/mixer.h"
#include "common/str.h"
#include "common/path.h"

#ifndef TETRAEDGE_TE_TE_SOUND_MANAGER_H
#define TETRAEDGE_TE_TE_SOUND_MANAGER_H

namespace Tetraedge {

class TeMusic;

class TeSoundManager {
public:
	TeSoundManager();

	void playFreeSound(const Common::Path &path);
	void playFreeSound(const Common::Path &path, float vol, const Common::String &channel);
	void stopFreeSound(const Common::String &channel);

	void setChannelVolume(const Common::String &channel, float vol);
	float getChannelVolume(const Common::String &channel);
	void update();

	Common::Array<TeMusic *> &musics() { return _musics; }

private:
	Common::HashMap<Common::String, Audio::SoundHandle> _handles;

	Common::Array<TeMusic *> _musics;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_SOUND_MANAGER_H
