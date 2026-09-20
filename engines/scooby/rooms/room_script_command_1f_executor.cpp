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

#include "room_script_command_1f_executor.h"

#include "common/scummsys.h"

namespace Scooby {

void RoomScriptCommand1FExecutor::executeRoomScriptCommand1F(int mode) {
	int32 commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x00004F70-0x00004F73: scan mode bypasses actor resolution and the complete wait loop.
	if (mode != 0) {
		// Ghidra 0x00004F74-0x00004F9F: identities one and two select actor bits zero and one; every other
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

		uint8 actorMask = static_cast<uint8>(1 << actorSlot);
		while (true) {
			// Ghidra 0x00004FA0-0x00004FAD: advance every stream at least once. An active selected movement
			// bit takes the back edge immediately without consulting path state.
			_updateActorAnimationFrames();
			if ((_state.ActorMovementFlags & actorMask) != 0) {
				if (!waitForRoomVerticalBlank()) {
					return;
				}

				continue;
			}

			// Ghidra 0x00004FAE-0x00004FB7: with movement clear, an active selected path bit takes the same
			// back edge; only the distinct both-clear outcome exits.
			if ((_state.ActorPathActiveFlags & actorMask) == 0) {
				break;
			}

			if (!waitForRoomVerticalBlank()) {
				return;
			}
		}
	}

	// Ghidra 0x00004FB8-0x00004FBD: every mode, identity, and wait outcome consumes four bytes.
	_state.RoomScriptStartOffset = commandOffset + 4;
}

bool RoomScriptCommand1FExecutor::waitForRoomVerticalBlank() {
	_handleRoomVBlank();
	if (_presenter.waitForVerticalBlank()) {
		return true;
	}

	_requestSessionExit(SessionExit::HostClosed);
	return false;
}
} // namespace Scooby