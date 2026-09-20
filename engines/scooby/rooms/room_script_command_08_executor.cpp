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

#include "room_script_command_08_executor.h"

#include "common/scummsys.h"

namespace Scooby {

void RoomScriptCommand08Executor::executeRoomScriptCommand08(int mode) {
	int32 commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x00002C56-0x00002C59: mode zero skips every update and retains the common cursor advance.
	if (mode != 0) {
		uint16 flags = _rom.readUInt16(commandOffset + 10);
		uint16 sourceValue;

		// Ghidra 0x00002C5A-0x00002C83: flag bit two selects one progress byte and tests the requested bit
		// modulo eight, preserving both zero and one source-value outcomes.
		if ((flags & 0x04) != 0) {
			int16 sourceByteOffset = _rom.readInt16(commandOffset + 6);
			int sourceBitMask = 1 << (_rom.readUInt16(commandOffset + 8) & 0x07);
			sourceValue = (_state.ProgressStateBytes[static_cast<std::size_t>(sourceByteOffset)] &
						   sourceBitMask) != 0
							  ? static_cast<uint16>(1)
							  : static_cast<uint16>(0);
		}
		// Ghidra 0x00002C84-0x00002CAD: flag bit three selects any even-aligned room-object word after the
		// wrapped stride-plus-field sum, including a field offset that enters a neighboring record.
		else if ((flags & 0x08) != 0) {
			RoomObjectWordAddress source =
				resolveRoomObjectWordAddress(_rom.readUInt16(commandOffset + 8),
											 _rom.readUInt16(commandOffset + 6));
			sourceValue = source.TargetRoomObject.readCommandWord(source.ByteOffset);
		} else {
			// Ghidra 0x00002CAE-0x00002CC9: otherwise retain the immediate word and both flag-bit-seven
			// outcomes; scaling advances the shared recovered random state before replacing the source.
			sourceValue = _rom.readUInt16(commandOffset + 6);
			if ((flags & 0x80) != 0) {
				sourceValue = _random.scaleNextRandomValue(sourceValue);
			}
		}

		if ((flags & 0x01) != 0) {
			int16 destinationByteOffset = _rom.readInt16(commandOffset + 2);
			uint16 destinationBit = _rom.readUInt16(commandOffset + 4);
			if ((flags & 0x10) != 0) {
				// Ghidra 0x00002CCA-0x00002CFD: destination flag bit four selects byte-bit assignment; preserve
				// independent nonzero set and zero clear outcomes with bit-number modulo eight.
				uint8 destinationBitMask = static_cast<uint8>(1 << (destinationBit & 0x07));
				if (sourceValue != 0) {
					_state.ProgressStateBytes[static_cast<std::size_t>(destinationByteOffset)] |=
						destinationBitMask;
				} else {
					_state.ProgressStateBytes[static_cast<std::size_t>(destinationByteOffset)] &=
						static_cast<uint8>(~destinationBitMask);
				}
			} else {
				// Ghidra 0x00002CFE-0x00002D0F: preserve zero and nonzero normalization, the low-six-bit
				// dynamic word shift, and big-endian XOR at the command-selected progress destination.
				int shiftCount = destinationBit & 0x3F;
				uint16 xorMask = (sourceValue != 0 && shiftCount < 16)
									 ? static_cast<uint16>(1 << shiftCount)
									 : static_cast<uint16>(0);
				_state.xorProgressStateWord(destinationByteOffset, xorMask);
			}
		} else {
			// Ghidra 0x00002D10-0x00002D39: resolve the complete wrapped and aligned object-word destination,
			// read its prior bit pattern, and retain the flag-bit-four assignment outcome.
			RoomObjectWordAddress destination =
				resolveRoomObjectWordAddress(_rom.readUInt16(commandOffset + 4),
											 _rom.readUInt16(commandOffset + 2));
			uint16 destinationValue = destination.TargetRoomObject.readCommandWord(
				destination.ByteOffset);
			if ((flags & 0x10) != 0) {
				destinationValue = sourceValue;
			}
			// Ghidra 0x00002D3A-0x00002D47: flag bit five subtracts with original word wrapping.
			else if ((flags & 0x20) != 0) {
				destinationValue = static_cast<uint16>(destinationValue - sourceValue);
			}
			// Ghidra 0x00002D48-0x00002D53: flag bit six ORs both complete word bit patterns.
			else if ((flags & 0x40) != 0) {
				destinationValue |= sourceValue;
			} else {
				// Ghidra 0x00002D54-0x00002D57: the remaining operation adds with word wrapping.
				destinationValue = static_cast<uint16>(destinationValue + sourceValue);
			}

			destination.TargetRoomObject.writeCommandWord(destination.ByteOffset, destinationValue);
		}
	}

	// Ghidra 0x00002D58-0x00002D5D: every native return path consumes the complete six-word record.
	_state.RoomScriptStartOffset = commandOffset + 12;
}

RoomScriptCommand08Executor::RoomObjectWordAddress RoomScriptCommand08Executor::resolveRoomObjectWordAddress(
	uint16 objectIdentity, uint16 fieldOffset) {
	uint16 tableByteOffset =
		static_cast<uint16>(fieldOffset + (objectIdentity - 3) * kRoomObjectRecordSize);
	tableByteOffset &= 0xFFFE;
	return RoomObjectWordAddress(
		_state.RoomObjects[static_cast<std::size_t>(tableByteOffset / kRoomObjectRecordSize)],
		tableByteOffset % kRoomObjectRecordSize);
}
} // namespace Scooby