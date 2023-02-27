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

#ifndef MM_MM1_SOUND_H
#define MM_MM1_SOUND_H

#include "mm/shared/xeen/sound.h"

namespace MM {
namespace MM1 {

enum SoundId {
	SOUND_1 = 1, SOUND_2 = 2, SOUND_3 = 3, SOUND_4 = 4,
	SOUND_5 = 5, SOUND_8 = 8, SOUND_9 = 9
};

class Sound : public Shared::Xeen::Sound {
public:
	Sound(Audio::Mixer *mixer) : Shared::Xeen::Sound(mixer) {}

	static void sound(SoundId soundNum);
	static void sound2(SoundId soundNum);
	static void stopSound();
};

} // namespace MM1
} // namespace MM

#endif
