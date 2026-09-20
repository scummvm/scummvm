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

#include "room_script_command_0b_executor.h"

#include "common/scummsys.h"

namespace Scooby {
void RoomScriptCommand0BExecutor::executeRoomScriptCommand0B(int mode) {
	int32 commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x00002DF2-0x00002DF5: dispatcher mode zero skips every object-state branch.
	if (mode != 0) {
		// Ghidra 0x00002DF6-0x00002E13: mask encoded identity bit 15, retain the low-word table calculation,
		// and preserve both outcomes of fixed-position flag bit one.
		uint16 encodedObjectIdentity = _rom.readUInt16(commandOffset + 2);
		uint16 objectIdentity = static_cast<uint16>(encodedObjectIdentity & 0x7FFF);
		uint16 tableByteOffset = static_cast<uint16>((objectIdentity - 3) *
													 kRoomObjectRecordSize);
		RoomObject &roomObject =
			_state.RoomObjects[static_cast<std::size_t>(tableByteOffset / kRoomObjectRecordSize)];
		int16 tilePatchIndex = _rom.readInt16(commandOffset + 4);
		if ((roomObject.Flags & kFixedPositionMask) != 0) {
			// Ghidra 0x00002E14-0x00002E1D: a fixed-position object replaces only its tile-patch word.
			roomObject.TilePatchIndex = tilePatchIndex;
		} else {
			// Ghidra 0x00002E1E-0x00002E2F: snapshot the prior room ID and retain both command-value outcomes;
			// only a nonzero value replaces the shape word.
			int16 previousRoomId = roomObject.RoomId;
			if (tilePatchIndex != 0) {
				roomObject.ShapeIndex = tilePatchIndex;
			}

			// Ghidra 0x00002E30-0x00002E43: a non-active-room object replaces tile-patch and skips every
			// pending-publication and encoded-identity branch.
			if (previousRoomId != _state.RoomId) {
				roomObject.TilePatchIndex = tilePatchIndex;
			} else {
				bool publishTilePatch = false;
				if (tilePatchIndex == 0) {
					// Ghidra 0x00002E44-0x00002E5B: zero preserves an already-zero tile and skips publication;
					// otherwise set both retirement bits before joining the common path.
					if (roomObject.TilePatchIndex != 0) {
						roomObject.TilePatchIndex = static_cast<int16>(
							static_cast<uint16>(roomObject.TilePatchIndex) | kTilePatchRetirementMask);
						publishTilePatch = true;
					}
				} else {
					// Ghidra 0x00002E5C-0x00002E61: nonzero replaces the complete tile-patch word.
					roomObject.TilePatchIndex = tilePatchIndex;
					publishTilePatch = true;
				}

				if (publishTilePatch) {
					// Ghidra 0x00002E62-0x00002E77: queue flag bit zero, preserve both encoded-sign outcomes,
					// and retain both busy-loop outcomes until asynchronous publication clears it.
					roomObject.Flags |= kTilePatchPendingMask;
					if (static_cast<int16>(encodedObjectIdentity) < 0) {
						while ((roomObject.Flags & kTilePatchPendingMask) != 0) {
							if (!_waitForRoomVerticalBlank()) {
								return;
							}
						}
					}
				}
			}
		}
	}

	// Ghidra 0x00002E78-0x00002E7D: every native branch consumes the six-byte command.
	_state.RoomScriptStartOffset = commandOffset + 6;
}
} // namespace Scooby