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

#ifndef SCOOBY_ROOM_SCRIPT_COMMAND_09_EXECUTOR_H
#define SCOOBY_ROOM_SCRIPT_COMMAND_09_EXECUTOR_H

#include "common/scummsys.h"

#include "common/func.h"

#include "scooby/assets/rom.h"
#include "scooby/runtime/runtime_state.h"

// Owns command 0x09's room-object byte mutation and action-icon handover.

namespace Scooby {

class RoomScriptCommand09Executor {
public:
	// Binds command records to the typed room-object table and the canonical icon renderer.
	// rom: verified cartridge containing command fields.
	// state: shared room-object and action-icon state.
	// drawSelectedActionIcon: recovered action-icon renderer called for the current interaction.
	RoomScriptCommand09Executor(const ScoobyDooRom &rom, RuntimeState &state,
								Common::Functor1<int, void> &drawSelectedActionIcon)
		: _rom(rom), _state(state), _drawSelectedActionIcon(drawSelectedActionIcon) {
	}

	// Replaces an indexed room-object byte and retires any active action icon.
	// mode: zero scans past the command; nonzero applies its object and icon updates.
	//
	// Ghidra: executeRoomScriptCommand09 (0x00002D5E). The signed command displacement is relative to record
	// field +0x16 and may select any byte in an adjacent typed record. The canonical icon renderer and its
	// hardware handover preserve the original resolved A1 address.
	void executeRoomScriptCommand09(int mode);

private:
	static const int kActionIconByteOffset = 0x16;
	static const int kRoomObjectRecordSize = 0x1A;

	// Preserves a mutable reference to the resolved room-object record and its command-byte offset, replacing
	// the C# "(RoomObject RoomObject, int ByteOffset)" tuple return.
	struct RoomObjectByteLocation {
		RoomObject &TargetRoomObject;
		int ByteOffset;

		RoomObjectByteLocation(RoomObject &targetRoomObject, int byteOffset)
			: TargetRoomObject(targetRoomObject), ByteOffset(byteOffset) {
		}
	};

	RoomObjectByteLocation resolveRoomObjectByte(uint16 objectIdentity, int8 displacement);

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
	Common::Functor1<int, void> &_drawSelectedActionIcon;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_SCRIPT_COMMAND_09_EXECUTOR_H
