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

#ifndef SCOOBY_ROOM_SCRIPT_COORDINATE_SPRITE_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_COORDINATE_SPRITE_EXECUTOR_H

#include "common/scummsys.h"

#include "scooby/assets/rom.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/runtime/runtime_state.h"

// Owns room-script setup and teardown for the optional coordinate-driven sprite.

namespace Scooby {

class RoomScriptCoordinateSpriteExecutor {
public:
	// Binds the scripted sprite lifecycle to its authored patterns, logical scene, and mutable state.
	// rom: verified cartridge containing the coordinate sprite's authored patterns.
	// scene: logical tile scene receiving the eight-tile pattern publication.
	// state: shared behavior flags and scripted-sprite globals.
	RoomScriptCoordinateSpriteExecutor(const ScoobyDooRom &rom, TileScene &scene,
									   RuntimeState &state)
		: _rom(rom), _scene(scene), _state(state) {
	}

	// Starts the primary coordinate-sprite presentation with its authored two-frame sequence.
	//
	// Ghidra: executeRoomScriptAction17 (0x000042A2). The first four packed tiles come from
	// g_abScriptedSpritePrimaryPatternFrame at 0x000321BE-0x0003223D; the second four come from
	// g_abScriptedSpriteSharedPatternFrame at 0x0003223E-0x000322BD.
	void executeRoomScriptAction17();

	// Starts the alternate coordinate-sprite presentation with two copies of its authored frame.
	//
	// Ghidra: executeRoomScriptAction18 (0x000042E0). Both four-tile slots come from
	// g_abScriptedSpriteAlternatePatternFrame at 0x000322BE-0x0003233D: the first transfer advances A0 by
	// 0x80 bytes, then LEA -0x80(A0),A0 restores the same source for the second transfer.
	void executeRoomScriptAction18();

	// Suppresses the optional scripted coordinate sprite while preserving its prepared state.
	//
	// Ghidra: executeRoomScriptAction19 (0x00004322).
	void executeRoomScriptAction19();

private:
	static const int kAlternatePatternFrameOffset = 0x322BE;
	static const int kPatternFrameByteCount = 0x80;
	static const int kPatternPairByteCount = 0x100;
	static const int kPatternTileIndex = 0x7F8;
	static const int kPrimaryPatternFramesOffset = 0x321BE;

	const ScoobyDooRom &_rom;
	TileScene &_scene;
	RuntimeState &_state;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_COORDINATE_SPRITE_EXECUTOR_H
