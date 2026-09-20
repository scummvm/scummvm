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

#include "room_script_command_1c_executor.h"

#include "common/scummsys.h"

namespace Scooby {

void RoomScriptCommand1CExecutor::executeRoomScriptCommand1C(int mode) {
	int32 commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x0000255E-0x00002561: scan mode skips all progress, identity, and movement effects.
	if (mode != 0) {
		// Ghidra 0x00002562-0x00002569: execution clears the projected progress bit before selection.
		_state.ProgressStateBytes[0] &= 0xFB;

		// Ghidra 0x0000256A-0x00002593: identities one and two select actor bits zero and one; every other
		// word retains the wrapped signed object-record offset, zero-extends byte +0x19, adds two, and uses
		// the low three bits as the actor bit number.
		uint16 objectIdentity = _rom.readUInt16(commandOffset + 2);
		int actorSlot;
		if (objectIdentity == 1) {
			actorSlot = 0;
		} else if (objectIdentity == 2) {
			actorSlot = 1;
		} else {
			int16 tableByteOffset = static_cast<int16>((objectIdentity - 3) *
													   kRoomObjectRecordSize);
			const RoomObject &roomObject =
				_state.RoomObjects[static_cast<std::size_t>(tableByteOffset / kRoomObjectRecordSize)];
			actorSlot = (static_cast<uint8>(roomObject.FixedPositionIndex) + 2) & 0x07;
		}

		// Ghidra 0x00002594-0x000025A5: both movement-bit outcomes remain explicit; only an active selected
		// actor restores progress byte-zero bit two.
		uint8 actorMask = static_cast<uint8>(1 << actorSlot);
		if ((_state.ActorMovementFlags & actorMask) != 0) {
			_state.ProgressStateBytes[0] |= 0x04;
		}
	}

	// Ghidra 0x000025A6-0x000025AB: every mode and identity branch consumes the four-byte command.
	_state.RoomScriptStartOffset = commandOffset + 4;
}
} // namespace Scooby