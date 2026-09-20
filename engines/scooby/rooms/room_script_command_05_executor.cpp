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

#include "room_script_command_05_executor.h"

#include "common/scummsys.h"

namespace Scooby {

void RoomScriptCommand05Executor::executeRoomScriptCommand05(int mode) {
	int32 commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x000028A0-0x000028C3: scan mode and masked identities zero, one, and two each take their own
	// branch to the common epilogue; every other identity retains the original word-width table offset.
	if (mode != 0) {
		uint16 encodedObjectIdentity = _rom.readUInt16(commandOffset + 2);
		uint16 objectIdentity = static_cast<uint16>(encodedObjectIdentity & 0x7FFF);
		switch (objectIdentity) {
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		default: {
			// Ghidra 0x000028C4-0x000028DD: resolve the 0x1A-byte record through the original low-word
			// multiply and select its fixed-position or portable-object ecosystem from flag bit one.
			uint16 tableByteOffset = static_cast<uint16>((objectIdentity - 3) *
														 kRoomObjectRecordSize);
			int objectIndex = tableByteOffset / kRoomObjectRecordSize;
			RoomObject &roomObject = _state.RoomObjects[static_cast<std::size_t>(objectIndex)];
			int16 newRoomId = _rom.readInt16(commandOffset + 4);
			if ((roomObject.Flags & kFixedPositionMask) != 0) {
				if (!relocateFixedObject(roomObject, newRoomId)) {
					return;
				}
			} else if (relocatePortableObject(roomObject, objectIndex, encodedObjectIdentity, newRoomId)) {
				return;
			}

			break;
		}
		}
	}

	// Ghidra 0x00002ACA-0x00002ACF: every native return path consumes the command and its two arguments.
	_state.RoomScriptStartOffset = commandOffset + 6;
}

bool RoomScriptCommand05Executor::relocateFixedObject(RoomObject &roomObject, int16 newRoomId) {
	int16 currentRoomId = _state.RoomId;

	// Ghidra 0x000028DE-0x00002907: detach an actor only when its object belongs to the active room. Dynamic
	// byte-bit selection is modulo eight, including the stored-index branch, before the index returns to -1.
	if (roomObject.RoomId == currentRoomId) {
		uint8 previousActorMask =
			static_cast<uint8>(1 << ((static_cast<uint8>(roomObject.FixedPositionIndex) + 2) &
									 0x07));
		_state.ActiveActorFlags &= static_cast<uint8>(~previousActorMask);
		_state.ActorTileUploadPendingFlags &= static_cast<uint8>(~previousActorMask);
		roomObject.FixedPositionIndex = -1;
	}

	// Ghidra 0x00002908-0x0000292F: publish the new room, return when it is not active, or choose the first
	// inactive actor slot from two through five; reaching sentinel slot six is a distinct no-slot exit.
	roomObject.RoomId = newRoomId;
	if (newRoomId != currentRoomId) {
		return true;
	}

	int actorSlot = kDynamicActorFirstSlot;
	while ((_state.ActiveActorFlags & (1 << actorSlot)) != 0) {
		actorSlot++;
		if (actorSlot == kDynamicActorSlotLimit) {
			return true;
		}
	}

	// Ghidra 0x00002930-0x0000294B: scan all 36 descriptor pointers by their leading shape word. Exhausting
	// the DBF catalog without a match is a separate return and does not activate the selected free slot.
	int descriptorOffset = 0;
	int descriptorCatalogIndex = -1;
	if (!_actorDescriptorCatalog.tryResolve(roomObject.ShapeIndex, descriptorOffset, descriptorCatalogIndex)) {
		return true;
	}

	uint32 horizontalStep = _actorDescriptorCatalog.readHorizontalStep(descriptorCatalogIndex);

	// Ghidra 0x0000294C-0x000029E3: initialize the selected slot, replacing the original overlapping step
	// aliases and preallocated graphics-work pointers with typed movement state and on-demand tile storage.
	int dynamicActorIndex = actorSlot - kDynamicActorFirstSlot;
	uint8 actorMask = static_cast<uint8>(1 << actorSlot);
	_state.RoomObjectActorHorizontalSteps[static_cast<std::size_t>(dynamicActorIndex)] =
		static_cast<int32>(horizontalStep);
	_state.RoomObjectActorVerticalSteps[static_cast<std::size_t>(dynamicActorIndex)] =
		static_cast<int32>(horizontalStep >> 1);
	_state.ActorAnimationDescriptorOffsets[static_cast<std::size_t>(actorSlot)] = descriptorOffset;
	_state.ActorAnimationOffsets[static_cast<std::size_t>(actorSlot)] = roomObject.TilePatchIndex;
	_state.ActorXFixedCoordinates[static_cast<std::size_t>(actorSlot)] = replaceIntegerCoordinate(
		_state.ActorXFixedCoordinates[static_cast<std::size_t>(actorSlot)], roomObject.FixedX);
	_state.ActorYFixedCoordinates[static_cast<std::size_t>(actorSlot)] = replaceIntegerCoordinate(
		_state.ActorYFixedCoordinates[static_cast<std::size_t>(actorSlot)], roomObject.FixedY);
	_state.ActorAnimationDelays[static_cast<std::size_t>(actorSlot)] = -1;
	_state.ActorTargetXCoordinates[static_cast<std::size_t>(actorSlot)] = roomObject.FixedX;
	_state.ActorTargetYCoordinates[static_cast<std::size_t>(actorSlot)] = roomObject.FixedY;
	roomObject.FixedPositionIndex = static_cast<int8>(dynamicActorIndex);
	_state.ActorCompactFrameFlags &= static_cast<uint8>(~actorMask);
	if ((roomObject.Flags & kCompactActorMask) != 0) {
		_state.ActorCompactFrameFlags |= actorMask;
	}

	_state.ActorTileUploadPendingFlags &= static_cast<uint8>(~actorMask);
	_state.ActorCompactConversionPendingFlags &= static_cast<uint8>(~actorMask);
	_state.ActorGraphicsReadyFlags &= static_cast<uint8>(~actorMask);
	_state.ActorAnimationRestartFlags |= actorMask;
	_state.ActiveActorFlags |= actorMask;

	// Ghidra 0x000029E4-0x000029FD: advance at least once and retain the graphics-ready back edge. The
	// original interrupt publishes pending graphics asynchronously, so service the room callback before each
	// managed continuation and repeat the final room assignment after publication.
	while (true) {
		_updateActorAnimationFrames();
		if ((_state.ActorGraphicsReadyFlags & actorMask) != 0) {
			break;
		}

		if (!_waitForRoomVerticalBlank()) {
			return false;
		}
	}

	roomObject.RoomId = newRoomId;
	return true;
}

bool RoomScriptCommand05Executor::relocatePortableObject(RoomObject &roomObject, int objectIndex,
														 uint16 encodedObjectIdentity,
														 int16 newRoomId) {
	int16 previousRoomId = roomObject.RoomId;

	// Ghidra 0x000029FE-0x00002A2B: when a positive-identity visible patch leaves the active room, mark its
	// tile index and object flags before waiting one retrace. Each room, sign, and zero-patch branch remains.
	if (previousRoomId == _state.RoomId) {
		if (static_cast<int16>(encodedObjectIdentity) >= 0) {
			if (roomObject.TilePatchIndex != 0) {
				roomObject.TilePatchIndex =
					static_cast<int16>(static_cast<uint16>(roomObject.TilePatchIndex) | 0x8000);
				roomObject.Flags |= kObjectTilePatchPendingMask;
				if (!_waitForRoomVerticalBlank()) {
					return true;
				}
			}
		}
	}

	// Ghidra 0x00002A2C-0x00002A43: publish the new room and mark a positive-identity object entering the
	// active room. A different room or negative identity independently skips the flag update.
	roomObject.RoomId = newRoomId;
	if (newRoomId == _state.RoomId) {
		if (static_cast<int16>(encodedObjectIdentity) >= 0) {
			roomObject.Flags |= kObjectTilePatchPendingMask;
		}
	}

	// Ghidra 0x00002A44-0x00002A69: inventory departure scans until the exact object index, then shifts every
	// later entry left through the original sentinel position. The typed list removes that sentinel.
	if (previousRoomId == kInventoryRoomId) {
		std::size_t inventoryIndex = 0;
		while (_state.InventoryObjectIndices[inventoryIndex] != objectIndex) {
			inventoryIndex++;
		}

		while (inventoryIndex < _state.InventoryObjectIndices.size() - 1) {
			_state.InventoryObjectIndices[inventoryIndex] = _state.InventoryObjectIndices[inventoryIndex + 1];
			inventoryIndex++;
		}

		_state.InventoryObjectIndices.pop_back();
		if (newRoomId != kInventoryRoomId) {
			return refreshActionPromptsAfterRelocation(encodedObjectIdentity);
		}
	}

	// Ghidra 0x00002A6A-0x00002AA5: a non-inventory destination skips insertion. Otherwise append after the
	// sentinel scan and preserve the original unusual page update comparison against MenuPage + 1.
	if (newRoomId != kInventoryRoomId) {
		return false;
	}

	std::size_t inventoryObjectCount = _state.InventoryObjectIndices.size();
	_state.InventoryObjectIndices.push_back(objectIndex);
	int16 inventoryPage = static_cast<int16>(inventoryObjectCount >> 2);
	if (static_cast<int16>(_state.MenuPage + 1) != inventoryPage) {
		_state.MenuPage = inventoryPage;
	}

	return refreshActionPromptsAfterRelocation(encodedObjectIdentity);
}

bool RoomScriptCommand05Executor::refreshActionPromptsAfterRelocation(uint16 encodedObjectIdentity) {
	// Ghidra 0x00002AA6-0x00002AC9: interaction bit two and a negative encoded identity independently skip
	// all three calls. Otherwise preserve WaitForVerticalBlank, CommitActionPromptUpdate, then refresh.
	if ((_state.InteractionFlags & kSuppressActionPromptRefreshMask) != 0) {
		return false;
	}

	if (static_cast<int16>(encodedObjectIdentity) < 0) {
		return false;
	}

	if (!_waitForRoomVerticalBlank()) {
		return true;
	}

	_commitActionPromptUpdate();
	_refreshActionPrompts();
	return false;
}

int RoomScriptCommand05Executor::replaceIntegerCoordinate(int coordinate, int16 integer) {
	return (coordinate & 0x0000FFFF) | (static_cast<int>(integer) << 16);
}
} // namespace Scooby