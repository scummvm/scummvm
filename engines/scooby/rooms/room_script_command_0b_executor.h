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

#ifndef SCOOBY_ROOM_SCRIPT_COMMAND_0B_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_COMMAND_0B_EXECUTOR_H

#include "common/scummsys.h"

#include "common/func.h"

#include "scooby/assets/rom.h"
#include "scooby/runtime/runtime_state.h"

// Owns command 0x0B's room-object shape and tile-patch publication lifecycle.

namespace Scooby {

class RoomScriptCommand0BExecutor {
public:
	// Binds command records to room-object state and asynchronous room publication.
	// rom: verified cartridge containing command fields.
	// state: shared room-object table and active room identity.
	// waitForRoomVerticalBlank: callback-aware clocked frame completing object tile patches.
	RoomScriptCommand0BExecutor(const ScoobyDooRom &rom, RuntimeState &state,
								Common::Functor0<bool> &waitForRoomVerticalBlank)
		: _rom(rom), _state(state), _waitForRoomVerticalBlank(waitForRoomVerticalBlank) {
	}

	// Updates one object's shape and tile patch, optionally waiting for publication.
	// mode: zero scans past the command; nonzero applies its complete object update.
	//
	// Ghidra: executeRoomScriptCommand0B (0x00002DF2). Encoded identity bit 15 controls synchronous completion
	// independently from the low 15-bit object identity. The callback-aware clocked frame replaces the
	// original asynchronous interrupt progress observed by the busy loop.
	void executeRoomScriptCommand0B(int mode);

private:
	static const uint8 kFixedPositionMask = 0x02;
	static const uint8 kTilePatchPendingMask = 0x01;
	static const uint16 kTilePatchRetirementMask = 0xC000;
	static const int kRoomObjectRecordSize = 0x1A;

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
	Common::Functor0<bool> &_waitForRoomVerticalBlank;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_COMMAND_0B_EXECUTOR_H
