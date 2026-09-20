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

#ifndef SCOOBY_ROOM_SCRIPT_FOREGROUND_OVERRIDE_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_FOREGROUND_OVERRIDE_EXECUTOR_H

#include "common/scummsys.h"

#include "common/func.h"

#include "scooby/assets/rom.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/runtime/runtime_state.h"

// Owns the authored room-script foreground override and its scrolling-mode lifecycle.

namespace Scooby {

class RoomScriptForegroundOverrideExecutor {
public:
	// Binds foreground-override actions to their authored assets and room publication owners.
	// rom: verified cartridge containing the compressed tiles and foreground map.
	// scene: logical tile scene receiving dynamic patterns and the replacement foreground.
	// state: shared interaction flags controlling foreground scrolling and edge streaming.
	// waitForRoomVerticalBlank: callback-aware clocked frame publishing pending actor tiles.
	RoomScriptForegroundOverrideExecutor(const ScoobyDooRom &rom, TileScene &scene,
										 RuntimeState &state,
										 Common::Functor0<bool> &waitForRoomVerticalBlank)
		: _rom(rom), _scene(scene), _state(state), _waitForRoomVerticalBlank(waitForRoomVerticalBlank) {
	}

	// Installs the authored actor-relative foreground and suppresses normal edge replacement.
	//
	// Ghidra: executeRoomScriptAction07 (0x000044BA). Ring-LZ source 0x00031DFA-0x000320CB decodes to 70
	// packed tiles. The 121 words at 0x000320CC-0x000321BD form an 11x11 map whose cells receive the packed
	// dynamic-tile base with original word wrapping. The original shared output workspace at 0xFF4000 becomes
	// an operation-owned decoded array.
	void executeRoomScriptAction07();

	// Restores camera-derived foreground scrolling and normal foreground edge streaming.
	//
	// Ghidra: executeRoomScriptAction08 (0x00004534).
	void executeRoomScriptAction08();

private:
	static const int kForegroundMapColumnCount = 11;
	static const int kForegroundMapOffset = 0x320CC;
	static const int kForegroundMapRowCount = 11;
	static const uint8 kForegroundOverrideMask = 0x40;
	static const int kTileIndexMask = 0x07FF;
	static const int kTileStreamOffset = 0x31DFA;

	const ScoobyDooRom &_rom;
	TileScene &_scene;
	RuntimeState &_state;
	Common::Functor0<bool> &_waitForRoomVerticalBlank;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_FOREGROUND_OVERRIDE_EXECUTOR_H
