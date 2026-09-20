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

#include "room_script_command_19_executor.h"

#include "common/scummsys.h"

namespace Scooby {

void RoomScriptCommand19Executor::executeRoomScriptCommand19(int mode) {
	int32 commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x00002B0E-0x00002B11: scan mode skips every actor-selection and publication effect.
	if (mode != 0) {
		// Ghidra 0x00002B12-0x00002B37: identities one and two select actor bits zero and one directly; every
		// other word retains the signed low-word object offset and byte +0x19 modulo-eight branch.
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

		// Ghidra 0x00002B38-0x00002B4D: request the selected actor's current frame and update at least once.
		// Original interrupts publish staged graphics asynchronously, so service the installed room callback
		// after each failed ready test before retaining the native back edge.
		uint8 actorMask = static_cast<uint8>(1 << actorSlot);
		_state.ActorPositionUpdateFlags |= actorMask;
		while (true) {
			_updateActorAnimationFrames();
			if ((_state.ActorGraphicsReadyFlags & actorMask) != 0) {
				break;
			}

			if (!_waitForRoomVerticalBlank()) {
				return;
			}
		}

		// Ghidra 0x00002B4E-0x00002B53: clear only the selected visibility block after publication.
		_state.ActorVisibilityBlockFlags &= static_cast<uint8>(~actorMask);
	}

	// Ghidra 0x00002B54-0x00002B59: every native mode and selector branch consumes four bytes.
	_state.RoomScriptStartOffset = commandOffset + 4;
}
} // namespace Scooby