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

#ifndef ULTIMA3_SOUND_EFFECTS_H
#define ULTIMA3_SOUND_EFFECTS_H

#include "common/array.h"

namespace Ultima {
namespace Ultima3 {

/**
 * Recreates the sound effects the original made by switching the PC speaker
 * on and off in timing loops
 */
class SoundEffects {
public:
	static constexpr int SAMPLE_RATE = 44100;

	/**
	 * Works out the sound of an effect as unsigned 8-bit samples
	 * @param effect	The effect number, from 0xF4 to 0xFD
	 * @param arg1		The first of the values the effect is varied by
	 * @param arg2		The second
	 * @returns			The samples, or nothing if the effect isn't a sound
	 */
	static Common::Array<byte> render(byte effect, byte arg1, byte arg2);
};

} // namespace Ultima3
} // namespace Ultima

#endif
