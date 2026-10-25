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

#ifndef SCOOBY_SAVE_GAME_DATA_H
#define SCOOBY_SAVE_GAME_DATA_H

#include "common/array.h"
#include "common/scummsys.h"
#include "common/serializer.h"

// MANAGED-ONLY SAVE GAMES. Nothing in this file exists in the original game.
//
// The original game only offers passwords: the episode, the room and the first 29 progress bytes. On resume,
// the episode's password scripts rebuild the world from those bits. Any state the scripts keep elsewhere is
// lost, most visibly objects that were taken, used or moved without a matching progress bit (reported for the
// air freshener, the doll and the goblet), which can leave the game unwinnable.
//
// A ScummVM save therefore stores the complete persistent world instead:
//   - all 256 progress bytes,
//   - every room-object record (room, position, script-state words, flags and so on),
//   - the inventory order,
//   - the current room and the entry position Shaggy last used to enter it.
//
// Loading still runs the original password resume path (episode opening, resume scripts) with the
// password-equivalent subset, so every other piece of setup happens exactly as before. The saved world is
// then written over the rebuilt one immediately before the saved room is loaded
// (SessionController::runGame), so the room is built from the saved state just as if Shaggy had walked into it.
//
// Saving is only permitted while the room loop is idle (RuntimeState::SaveAllowed), so no script is ever
// part-way through changing this state when it is captured.

namespace Scooby {

class RuntimeState;

// The persistent part of one room-object record (RoomObject). The runtime actor-slot byte at record offset
// +0x19 is not saved: room loading reassigns it from the other fields.
struct SavedRoomObject {
	// Record words +0x00 through +0x16, in record order (RoomObject::readCommandWord).
	static const int kWordCount = 12;

	uint16 Words[kWordCount];
	// Record offset +0x18 (RoomObject::Flags).
	uint8 Flags;

	SavedRoomObject() : Flags(0) {
		for (int i = 0; i < kWordCount; i++) {
			Words[i] = 0;
		}
	}
};

struct SaveGameData {
	// The original password covers this many leading progress bytes; the password resume scripts are run with
	// exactly this subset so they behave as they do for a typed password.
	static const int kPasswordProgressByteCount = 29;
	static const int kProgressByteCount = 256;

	// Set while a loaded save is waiting for the session restart that applies it.
	bool shouldLoadGameData = false;

	int16 episodeIdx = 0;
	int16 roomId = 0;
	// RuntimeState::InitialRoomPositionCoordinateOffset when the save was made: the entry point Shaggy used
	// to enter the saved room, so he reappears where he came in instead of at the room's first position.
	int16 entryPositionOffset = 0;
	Common::Array<uint8> progressBytes = Common::Array<uint8>(kProgressByteCount);
	Common::Array<SavedRoomObject> roomObjects;
	Common::Array<int32> inventoryObjectIndices;

	// Copies the persistent world out of the running state.
	void capture(const RuntimeState &state);

	// Reads or writes the save stream. Returns false for a stream that is not a compatible save; the data is
	// then unusable.
	bool sync(Common::Serializer &s);

	// Writes the saved world over the state rebuilt by the password resume scripts. Returns false (leaving
	// the state untouched) when the save does not match the loaded episode's object table.
	bool applyWorldState(RuntimeState &state) const;
};

} // namespace Scooby

#endif // SCOOBY_SAVE_GAME_DATA_H
