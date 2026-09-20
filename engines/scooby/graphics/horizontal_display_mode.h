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

#ifndef SCOOBY_HORIZONTAL_DISPLAY_MODE_H
#define SCOOBY_HORIZONTAL_DISPLAY_MODE_H

// Identifies the authored horizontal cell count independently from the
// backing plane dimensions.
//
// Genesis VDP register 12 selects H32 or H40. The managed host always
// publishes a 320x224 frame; H32 centers 256 visible pixels within it, while
// H40 uses the complete 320-pixel width. Ghidra RunStartupSequence selects
// H40 with value 0x81 at 0x00000A1E and restores H32 with value 0x00 at
// 0x00000A84 before entering the remaining runtime.

namespace Scooby {

enum class HorizontalDisplayMode {
	// Displays 32 eight-pixel tiles through a centered 256-pixel viewport.
	H32,

	// Displays 40 eight-pixel tiles across the complete 320-pixel frame.
	H40
};
} // namespace Scooby

#endif // SCOOBY_HORIZONTAL_DISPLAY_MODE_H
