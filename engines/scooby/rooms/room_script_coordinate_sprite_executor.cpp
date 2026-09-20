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

#include "room_script_coordinate_sprite_executor.h"

#include "common/scummsys.h"

#include "common/array.h"

#include "scooby/common/span.h"

namespace Scooby {

void RoomScriptCoordinateSpriteExecutor::executeRoomScriptAction17() {
	// Ghidra 0x000042A2-0x000042A9: select unflipped tile 0x7F8.
	_state.ScriptedCoordinateSprite._tileAttributes = 0x07F8;

	// Ghidra 0x000042AA-0x000042BB: replace the exact 0x80-word VRAM write with one eight-tile logical
	// publication, retaining the primary-first/shared-second source order at indices 0x7F8-0x7FF.
	_scene.loadTilesAt(_rom.readBytes(kPrimaryPatternFramesOffset, kPatternPairByteCount), kPatternTileIndex);

	// Ghidra 0x000042BC-0x000042C5: reset the managed offset for coordinate table 0x323BE-0x3244D.
	_state.ScriptedCoordinateSprite._coordinateOffset = 0;

	// Ghidra 0x000042C6-0x000042CD: initialize the signed frame delay.
	_state.ScriptedCoordinateSprite._frameDelay = 7;

	// Ghidra 0x000042CE-0x000042DF: enable terminal coordinate clamping, then sprite publication, and return
	// without changing the existing bit-six frame phase.
	_state.RoomBehaviorFlags |= 0x20;
	_state.RoomBehaviorFlags |= 0x10;
}

void RoomScriptCoordinateSpriteExecutor::executeRoomScriptAction18() {
	// Ghidra 0x000042E0-0x000042E7: select tile 0x7F8 with the original horizontal-flip attribute.
	_state.ScriptedCoordinateSprite._tileAttributes = 0x47F8;

	// Ghidra 0x000042E8-0x00004305: replace both 0x40-word VRAM writes with one exact eight-tile logical
	// publication. The original rewinds A0 after the first write, so indices 0x7F8-0x7FF contain two copies
	// of the same alternate four-tile frame.
	Common::Array<uint8> packedTiles(static_cast<std::size_t>(kPatternFrameByteCount) * 2);
	Span<const uint8> patternFrame =
		_rom.readBytes(kAlternatePatternFrameOffset, kPatternFrameByteCount);
	for (std::size_t i = 0; i < patternFrame.size(); ++i) {
		packedTiles[i] = patternFrame[i];
		packedTiles[i + static_cast<std::size_t>(kPatternFrameByteCount)] = patternFrame[i];
	}
	_scene.loadTilesAt(MakeSpan(packedTiles), kPatternTileIndex);

	// Ghidra 0x00004306-0x0000430F: reset the managed offset for coordinate table 0x323BE-0x3244D.
	_state.ScriptedCoordinateSprite._coordinateOffset = 0;

	// Ghidra 0x00004310-0x00004321: enable terminal coordinate clamping, then sprite publication, and return
	// without changing the existing frame delay or bit-six phase.
	_state.RoomBehaviorFlags |= 0x20;
	_state.RoomBehaviorFlags |= 0x10;
}

void RoomScriptCoordinateSpriteExecutor::executeRoomScriptAction19() {
	// Ghidra 0x00004322-0x0000432B: clear only room-behavior bit four and return.
	_state.RoomBehaviorFlags &= 0xEF;
}
} // namespace Scooby