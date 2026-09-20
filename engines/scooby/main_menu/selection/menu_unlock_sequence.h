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

#ifndef SCOOBY_MENU_UNLOCK_SEQUENCE_H
#define SCOOBY_MENU_UNLOCK_SEQUENCE_H

#include "common/scummsys.h"

#include "common/array.h"

#include "scooby/input/controller_buttons.h"

// Tracks the exact held-button sequence stored at ROM 0x00010118 by Ghidra
// SelectMainMenuOption at 0x00008D6A.

namespace Scooby {

class MenuUnlockSequence {
public:
	// Advances on the exact next held input, retains progress across release or a held prior
	// step, and resets on every other nonempty input.
	// activeLowInput: current controller-one byte in the original active-low representation.
	// Returns true when the eleventh and final input is reached.
	bool Observe(uint8 activeLowInput) {
		uint8 expectedInput = static_cast<uint8>(~kInputSequence[static_cast<std::size_t>(
			_progress)]);
		if (activeLowInput == expectedInput) {
			_progress++;
			return _progress == static_cast<int>(kInputSequence.size());
		}

		if (_progress != 0) {
			uint8 priorInput =
				static_cast<uint8>(~kInputSequence[static_cast<std::size_t>(_progress - 1)]);
			if (activeLowInput == priorInput) {
				return false;
			}
		}

		if (activeLowInput != 0xFF) {
			_progress = 0;
		}

		return false;
	}

private:
	// Original executable input bytes at 0x00010118-0x00010122.
	static const Common::Array<uint8> kInputSequence;

	// Replaces Ghidra g_wMainMenuUnlockSequenceProgress at 0xFF08BA. Original RAM has no
	// image-backed initial value; SelectMainMenuOption explicitly initializes progress to zero
	// before its first read.
	int _progress = 0;
};
} // namespace Scooby

#endif // SCOOBY_MENU_UNLOCK_SEQUENCE_H
