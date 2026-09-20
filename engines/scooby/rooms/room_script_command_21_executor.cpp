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

#include "room_script_command_21_executor.h"

#include "common/scummsys.h"

namespace Scooby {

void RoomScriptCommand21Executor::executeRoomScriptCommand21(int mode) {
	int32 commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x00004944-0x00004947: scan mode bypasses actor resolution and every path-state write.
	if (mode != 0) {
		// Ghidra 0x00004948-0x0000496D: selectors one and two choose slots zero and one. Every other selector
		// retains the wrapped signed object-record offset, sign-extends byte +0x19, and adds two.
		uint16 objectIdentity = _rom.readUInt16(commandOffset + 4);
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
			actorSlot = roomObject.FixedPositionIndex + 2;
		}

		// Ghidra 0x0000496E-0x000049A3: install the inline stream, seed both cadence arrays from the command
		// word's low byte, and restart traversal at byte offset zero for the exact selected slot.
		_state.ActorPathStreamOffsets[static_cast<std::size_t>(actorSlot)] = commandOffset + 8;
		uint8 cadence = static_cast<uint8>(_rom.readUInt16(commandOffset + 6));
		_state.ActorPathCadenceCountdowns[static_cast<std::size_t>(actorSlot)] = static_cast<int8>(
			cadence);
		_state.ActorPathCadenceReloads[static_cast<std::size_t>(actorSlot)] = cadence;
		_state.ActorPathRecordOffsets[static_cast<std::size_t>(actorSlot)] = 0;

		// Ghidra 0x000049A4-0x000049AF: clear a stale one-shot loop exit before setting path traversal.
		uint8 actorMask = static_cast<uint8>(1 << (actorSlot & 0x07));
		_state.ActorAnimationLoopExitFlags &= static_cast<uint8>(~actorMask);
		_state.ActorPathActiveFlags |= actorMask;
	}

	// Ghidra 0x000049B0-0x000049B9: every mode advances over the eight-byte header and signed inline extent.
	_state.RoomScriptStartOffset = commandOffset + 8 + _rom.readInt16(commandOffset + 2);
}
} // namespace Scooby