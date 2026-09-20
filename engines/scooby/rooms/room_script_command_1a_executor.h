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

#ifndef SCOOBY_ROOM_SCRIPT_COMMAND_1A_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_COMMAND_1A_EXECUTOR_H

#include "common/scummsys.h"

#include "scooby/assets/rom.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/runtime/runtime_state.h"

// Owns command 0x1A's animated palette-range configuration and publication.

namespace Scooby {

class RoomScriptCommand1AExecutor {
public:
	// Binds command records to the shared palette target and five range-animation slots.
	// rom: verified cartridge containing command fields.
	// scene: logical palette owner replacing immediate CRAM transfers.
	// state: shared palette target and animated-range configuration.
	RoomScriptCommand1AExecutor(const ScoobyDooRom &rom, TileScene &scene,
								RuntimeState &state)
		: _rom(rom), _scene(scene), _state(state) {
	}

	// Configures, disables, or immediately restores one animated palette range.
	// mode: zero scans the ten-byte record; nonzero evaluates its signed selector and control word.
	//
	// Ghidra: executeRoomScriptCommand1A (0x00002626). The selector comparison is signed, and the disable
	// transfer retains its exclusive end and word-width arithmetic.
	void executeRoomScriptCommand1A(int mode);

private:
	const ScoobyDooRom &_rom;
	TileScene &_scene;
	RuntimeState &_state;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_COMMAND_1A_EXECUTOR_H
