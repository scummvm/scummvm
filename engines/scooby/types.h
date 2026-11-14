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

#ifndef SCOOBY_TYPES_H
#define SCOOBY_TYPES_H

namespace Scooby {

enum VerbEnum : uint16 {
	NONE = 0,
	PULL = 1,
	TAKE = 2,
	OPEN = 3,
	SHUT = 4,
	USE =  5,
	GIVE = 6,
	PUSH = 7,
	TALK = 8,
	EAT =  9,
	LOOK = 10,
	NO_SUGGESTED_VERB = 11
};

enum class SessionExit {
	// The reset path requests reconstruction from the cartridge entry sequence.
	RestartRequested,

	// The native host requested process shutdown.
	HostClosed
};

enum class HorizontalDisplayMode {
	// Displays 32 eight-pixel tiles through a centered 256-pixel viewport.
	H32,

	// Displays 40 eight-pixel tiles across the complete 320-pixel frame.
	H40
};

// color components quantized to levels (0-7)
struct PaletteColor {
	uint8 redLevel;
	uint8 greenLevel;
	uint8 blueLevel;

	PaletteColor() : redLevel(0), greenLevel(0), blueLevel(0) {
	}

	PaletteColor(uint8 redLevel_, uint8 greenLevel_, uint8 blueLevel_)
		: redLevel(redLevel_), greenLevel(greenLevel_), blueLevel(blueLevel_) {
	}
};

class ScriptedCoordinateSpriteState {
public:
	ScriptedCoordinateSpriteState() : _coordinateOffset(0), _frameDelay(0), _tileAttributes(0) {
	}

	// Ghidra g_pScriptedSpriteCoordinateCursor at 0xFF08A6-0xFF08A9.
	int32 _coordinateOffset;

	// Ghidra g_bScriptedSpriteFrameDelay at 0xFF0ACE.
	int8 _frameDelay;

	// Ghidra g_wScriptedSpriteTileAttributes at 0xFF08A4-0xFF08A5.
	uint16 _tileAttributes;
};
} // End of namespace Scooby

#endif // SCOOBY_TYPES_H
