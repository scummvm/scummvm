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

// MANAGED-ONLY SAVE GAMES (see save_game_data.h).

#include "save_game_data.h"

#include "common/endian.h"
#include "common/textconsole.h"

#include "scooby/runtime/runtime_state.h"

namespace Scooby {

namespace {

const uint32 kSaveMagic = MKTAG('S', 'C', 'B', 'Y');

// Version history:
//   1: full world state (progress bytes, room objects, inventory, room, entry position).
const Common::Serializer::Version kSaveVersion = 1;

const int kEpisodeCount = 2;
// Generous bounds that reject a corrupt stream before it allocates.
const uint16 kMaxRoomObjects = 1024;
const uint16 kMaxInventoryObjects = 256;

// Progress byte zero: bit one marks a room/interface transition in progress (Action 0x1D sets it,
// FinishRoomStartup clears it); bits two and three are the command 0x1C/0x1D actor holds.
const uint8 kEngineOwnedByteZeroMask = 0x0E;

} // namespace

void SaveGameData::capture(const RuntimeState &state) {
	episodeIdx = state.EpisodeIndex;
	roomId = state.RoomId;
	entryPositionOffset = state.InitialRoomPositionCoordinateOffset;

	progressBytes.resize(kProgressByteCount);
	for (int i = 0; i < kProgressByteCount; i++) {
		progressBytes[i] = state.ProgressStateBytes[i];
	}

	roomObjects.resize(state.RoomObjects.size());
	for (uint i = 0; i < state.RoomObjects.size(); i++) {
		const RoomObject &roomObject = state.RoomObjects[i];
		SavedRoomObject &saved = roomObjects[i];
		for (int w = 0; w < SavedRoomObject::kWordCount; w++) {
			saved.Words[w] = roomObject.readCommandWord(w * 2);
		}

		saved.Flags = roomObject.Flags;
	}

	inventoryObjectIndices = state.InventoryObjectIndices;
}

bool SaveGameData::sync(Common::Serializer &s) {
	uint32 magic = kSaveMagic;
	s.syncAsUint32BE(magic);
	if (magic != kSaveMagic) {
		warning("Scooby: not a compatible save (it predates the full-state save format)");
		return false;
	}

	if (!s.syncVersion(kSaveVersion)) {
		warning("Scooby: save version %u is newer than this engine supports", s.getVersion());
		return false;
	}

	s.syncAsSint16LE(episodeIdx);
	s.syncAsSint16LE(roomId);
	s.syncAsSint16LE(entryPositionOffset);

	progressBytes.resize(kProgressByteCount);
	s.syncBytes(progressBytes.data(), kProgressByteCount);

	uint16 objectCount = static_cast<uint16>(roomObjects.size());
	s.syncAsUint16LE(objectCount);
	if (s.isLoading()) {
		if (objectCount > kMaxRoomObjects) {
			return false;
		}

		roomObjects.resize(objectCount);
	}

	for (uint i = 0; i < objectCount; i++) {
		SavedRoomObject &saved = roomObjects[i];
		for (int w = 0; w < SavedRoomObject::kWordCount; w++) {
			s.syncAsUint16LE(saved.Words[w]);
		}

		s.syncAsByte(saved.Flags);
	}

	uint16 inventoryCount = static_cast<uint16>(inventoryObjectIndices.size());
	s.syncAsUint16LE(inventoryCount);
	if (s.isLoading()) {
		if (inventoryCount > kMaxInventoryObjects) {
			return false;
		}

		inventoryObjectIndices.resize(inventoryCount);
	}

	for (uint i = 0; i < inventoryCount; i++) {
		uint16 objectIndex = static_cast<uint16>(inventoryObjectIndices[i]);
		s.syncAsUint16LE(objectIndex);
		inventoryObjectIndices[i] = objectIndex;
	}

	if (s.err()) {
		return false;
	}

	if (s.isLoading()) {
		if (episodeIdx < 0 || episodeIdx >= kEpisodeCount) {
			return false;
		}

		for (uint i = 0; i < inventoryObjectIndices.size(); i++) {
			if (inventoryObjectIndices[i] >= static_cast<int32>(objectCount)) {
				return false;
			}
		}
	}

	return true;
}

bool SaveGameData::applyWorldState(RuntimeState &state) const {
	if (roomObjects.size() != state.RoomObjects.size()) {
		warning("Scooby: save holds %u room objects but the episode has %u; keeping the password state",
				roomObjects.size(), state.RoomObjects.size());
		return false;
	}

	// Byte zero bits one to three are engine-owned flags, not progress: the room/interface transition the
	// resume scripts started (cleared by FinishRoomStartup) and the command 0x1C/0x1D actor holds. They are
	// always clear in a save made at idle, but the resume path is still mid-transition here, so keep them.
	state.ProgressStateBytes[0] = static_cast<uint8>((progressBytes[0] & ~kEngineOwnedByteZeroMask) |
													 (state.ProgressStateBytes[0] & kEngineOwnedByteZeroMask));
	for (int i = 1; i < kProgressByteCount; i++) {
		state.ProgressStateBytes[i] = progressBytes[i];
	}

	for (uint i = 0; i < roomObjects.size(); i++) {
		RoomObject &roomObject = state.RoomObjects[i];
		const SavedRoomObject &saved = roomObjects[i];
		for (int w = 0; w < SavedRoomObject::kWordCount; w++) {
			roomObject.writeCommandWord(w * 2, saved.Words[w]);
		}

		// FixedPositionIndex (record +0x19) is left alone: loading the saved room reassigns it.
		roomObject.Flags = saved.Flags;
	}

	state.InventoryObjectIndices = inventoryObjectIndices;
	state.InitialRoomPositionCoordinateOffset = entryPositionOffset;
	return true;
}

} // namespace Scooby
