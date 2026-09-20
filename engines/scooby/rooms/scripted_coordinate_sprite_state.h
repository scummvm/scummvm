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

#ifndef SCOOBY_SCRIPTED_COORDINATE_SPRITE_STATE_H
#define SCOOBY_SCRIPTED_COORDINATE_SPRITE_STATE_H

#include "common/scummsys.h"

// Holds the mutable globals shared by the optional scripted coordinate sprite.

namespace Scooby {

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
} // namespace Scooby

#endif // SCOOBY_SCRIPTED_COORDINATE_SPRITE_STATE_H
