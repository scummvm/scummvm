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
	 * The effect that is the noise at the end of the title screen's pictures being drawn
	 */
	static constexpr byte TITLE_BURST = 0xF3;

	/**
	 * Works out the sound of an effect as unsigned 8-bit samples
	 * @param effect	The effect number, from 0xF4 to 0xFD
	 * @param arg1		The first of the values the effect is varied by
	 * @param arg2		The second
	 * @param timed		True to make the effects as the Ultima III Upgrade does, which
	 *					times them by a timer rather than by how long the processor takes
	 * @returns			The samples, or nothing if the effect isn't a sound
	 */
	static Common::Array<byte> render(byte effect, byte arg1, byte arg2, bool timed = false);

	/**
	 * Works out the buzz made as a title picture is revealed bit by bit, a
	 * pass of it at a time
	 * @param revealed	Whether each bit of the pass was drawn rather than left out
	 * @param timed		True to make the buzz as the Ultima III Upgrade does
	 * @returns			The samples, which also give how long the pass takes
	 */
	static Common::Array<byte> renderSparkle(const Common::Array<bool> &revealed, bool timed);
};

} // namespace Ultima3
} // namespace Ultima

#endif
