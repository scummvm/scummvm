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

#include "scooby/rooms/script/command_executor.h"

#include "common/scummsys.h"

// Owns room-object relocation across actor slots, inventory order, and action-prompt publication.

namespace Scooby {

void RoomScriptCommandExecutor::scriptCommand05MoveObject(int mode) {
	int32 commandOffset = _state.roomScriptCurrentOffset;

	// Ghidra 0x000028A0-0x000028C3: scan mode and masked identities zero, one, and two each take their own
	// branch to the common epilogue; every other identity retains the original word-width table offset.
	if (mode != 0) {
		uint16 encodedObjectId = _rom.readUInt16(commandOffset + 2);
		uint16 objectId = static_cast<uint16>(encodedObjectId & 0x7FFF);
		if (objectId > 2) {
			// Ghidra 0x000028C4-0x000028DD: resolve the 0x1A-byte record through the original low-word
			// multiply and select its fixed-position or portable-object ecosystem from flag bit one.
			int objectIndex = objectId - 3;
			RoomObject &roomObject = _state.roomObjects[static_cast<size_t>(objectIndex)];
			int16 newRoomId = _rom.readInt16(commandOffset + 4);
			if ((roomObject.flags & kFixedPositionMask) != 0) {
				if (!relocateFixedObject(roomObject, newRoomId)) {
					return;
				}
			} else if (relocatePortableObject(roomObject, objectIndex, encodedObjectId, newRoomId)) {
				return;
			}
		}
	}

	// Ghidra 0x00002ACA-0x00002ACF: every native return path consumes the command and its two arguments.
	_state.roomScriptCurrentOffset = commandOffset + 6;
}

bool RoomScriptCommandExecutor::relocateFixedObject(RoomObject &roomObject, int16 newRoomId) {
	int16 currentRoomId = _state.roomId;

	// Ghidra 0x000028DE-0x00002907: detach an actor only when its object belongs to the active room. Dynamic
	// byte-bit selection is modulo eight, including the stored-index branch, before the index returns to -1.
	if (roomObject.roomId == currentRoomId) {
		uint8 previousActorMask =
			static_cast<uint8>(1 << ((static_cast<uint8>(roomObject.fixedPositionIndex) + 2) &
									 0x07));
		_state.activeActorFlags &= static_cast<uint8>(~previousActorMask);
		_state.actorTileUploadPendingFlags &= static_cast<uint8>(~previousActorMask);
		roomObject.fixedPositionIndex = -1;
	}

	// Ghidra 0x00002908-0x0000292F: publish the new room, return when it is not active, or choose the first
	// inactive actor slot from two through five; reaching sentinel slot six is a distinct no-slot exit.
	roomObject.roomId = newRoomId;
	if (newRoomId != currentRoomId) {
		return true;
	}

	int actorSlot = kDynamicActorFirstSlot;
	while ((_state.activeActorFlags & (1 << actorSlot)) != 0) {
		actorSlot++;
		if (actorSlot == kDynamicActorSlotLimit) {
			return true;
		}
	}

	// Ghidra 0x00002930-0x0000294B: scan all 36 descriptor pointers by their leading shape word. Exhausting
	// the DBF catalog without a match is a separate return and does not activate the selected free slot.
	int descriptorOffset = 0;
	int descriptorCatalogIndex = -1;
	if (!_actorDescriptorCatalog.tryResolve(roomObject.shapeIndex, descriptorOffset, descriptorCatalogIndex)) {
		return true;
	}

	uint32 horizontalStep = _actorDescriptorCatalog.readHorizontalStep(descriptorCatalogIndex);

	// Ghidra 0x0000294C-0x000029E3: initialize the selected slot, replacing the original overlapping step
	// aliases and preallocated graphics-work pointers with typed movement state and on-demand tile storage.
	int dynamicActorIndex = actorSlot - kDynamicActorFirstSlot;
	uint8 actorMask = static_cast<uint8>(1 << actorSlot);
	_state.roomObjectActorHorizontalSteps[static_cast<size_t>(dynamicActorIndex)] =
		static_cast<int32>(horizontalStep);
	_state.roomObjectActorVerticalSteps[static_cast<size_t>(dynamicActorIndex)] =
		static_cast<int32>(horizontalStep >> 1);
	_state.actorAnimationDescriptorOffsets[static_cast<size_t>(actorSlot)] = descriptorOffset;
	_state.actorAnimationOffsets[static_cast<size_t>(actorSlot)] = roomObject.tilePatchIndex;
	_state.actorXFixedCoordinates[static_cast<size_t>(actorSlot)] = replaceIntegerCoordinate(
		_state.actorXFixedCoordinates[static_cast<size_t>(actorSlot)], roomObject.fixedX);
	_state.actorYFixedCoordinates[static_cast<size_t>(actorSlot)] = replaceIntegerCoordinate(
		_state.actorYFixedCoordinates[static_cast<size_t>(actorSlot)], roomObject.fixedY);
	_state.actorAnimationDelays[static_cast<size_t>(actorSlot)] = -1;
	_state.actorTargetXCoordinates[static_cast<size_t>(actorSlot)] = roomObject.fixedX;
	_state.actorTargetYCoordinates[static_cast<size_t>(actorSlot)] = roomObject.fixedY;
	roomObject.fixedPositionIndex = static_cast<int8>(dynamicActorIndex);
	_state.actorCompactFrameFlags &= static_cast<uint8>(~actorMask);
	if ((roomObject.flags & kCompactActorMask) != 0) {
		_state.actorCompactFrameFlags |= actorMask;
	}

	_state.actorTileUploadPendingFlags &= static_cast<uint8>(~actorMask);
	_state.actorCompactConversionPendingFlags &= static_cast<uint8>(~actorMask);
	_state.actorGraphicsReadyFlags &= static_cast<uint8>(~actorMask);
	_state.actorAnimationRestartFlags |= actorMask;
	_state.activeActorFlags |= actorMask;

	// Ghidra 0x000029E4-0x000029FD: advance at least once and retain the graphics-ready back edge. The
	// original interrupt publishes pending graphics asynchronously, so service the room callback before each
	// managed continuation and repeat the final room assignment after publication.
	while (true) {
		_actorAnimator.updateActorAnimationFrames();
		if ((_state.actorGraphicsReadyFlags & actorMask) != 0) {
			break;
		}

		if (!waitForRoomVerticalBlank()) {
			return false;
		}
	}

	roomObject.roomId = newRoomId;
	return true;
}

bool RoomScriptCommandExecutor::relocatePortableObject(RoomObject &roomObject, int objectIndex,
														 uint16 encodedObjectId,
														 int16 newRoomId) {
	int16 previousRoomId = roomObject.roomId;

	// Ghidra 0x000029FE-0x00002A2B: when a positive-identity visible patch leaves the active room, mark its
	// tile index and object flags before waiting one retrace. Each room, sign, and zero-patch branch remains.
	if (previousRoomId == _state.roomId) {
		if (static_cast<int16>(encodedObjectId) >= 0) {
			if (roomObject.tilePatchIndex != 0) {
				roomObject.tilePatchIndex =
					static_cast<int16>(static_cast<uint16>(roomObject.tilePatchIndex) | 0x8000);
				roomObject.flags |= kObjectTilePatchPendingMask;
				if (!waitForRoomVerticalBlank()) {
					return true;
				}
			}
		}
	}

	// Ghidra 0x00002A2C-0x00002A43: publish the new room and mark a positive-identity object entering the
	// active room. A different room or negative identity independently skips the flag update.
	roomObject.roomId = newRoomId;
	if (newRoomId == _state.roomId) {
		if (static_cast<int16>(encodedObjectId) >= 0) {
			roomObject.flags |= kObjectTilePatchPendingMask;
		}
	}

	// Ghidra 0x00002A44-0x00002A69: inventory departure scans until the exact object index, then shifts every
	// later entry left through the original sentinel position. The typed list removes that sentinel.
	if (previousRoomId == kInventoryRoomId) {
		size_t inventoryIndex = 0;
		while (_state.inventoryObjectIndices[inventoryIndex] != objectIndex) {
			inventoryIndex++;
		}

		while (inventoryIndex < _state.inventoryObjectIndices.size() - 1) {
			_state.inventoryObjectIndices[inventoryIndex] = _state.inventoryObjectIndices[inventoryIndex + 1];
			inventoryIndex++;
		}

		_state.inventoryObjectIndices.pop_back();
		if (newRoomId != kInventoryRoomId) {
			return refreshActionPromptsAfterRelocation(encodedObjectId);
		}
	}

	// Ghidra 0x00002A6A-0x00002AA5: a non-inventory destination skips insertion. Otherwise append after the
	// sentinel scan and preserve the original unusual page update comparison against MenuPage + 1.
	if (newRoomId != kInventoryRoomId) {
		return false;
	}

	size_t inventoryObjectCount = _state.inventoryObjectIndices.size();
	_state.inventoryObjectIndices.push_back(objectIndex);
	int16 inventoryPage = static_cast<int16>(inventoryObjectCount >> 2);
	if (static_cast<int16>(_state.menuPage + 1) != inventoryPage) {
		_state.menuPage = inventoryPage;
	}

	return refreshActionPromptsAfterRelocation(encodedObjectId);
}

bool RoomScriptCommandExecutor::refreshActionPromptsAfterRelocation(uint16 encodedObjectId) {
	// Ghidra 0x00002AA6-0x00002AC9: interaction bit two and a negative encoded identity independently skip
	// all three calls. Otherwise preserve WaitForVerticalBlank, CommitActionPromptUpdate, then refresh.
	if ((_state.interactionFlags & kSuppressActionPromptRefreshMask) != 0) {
		return false;
	}

	if (static_cast<int16>(encodedObjectId) < 0) {
		return false;
	}

	if (!waitForRoomVerticalBlank()) {
		return true;
	}

	_actionMenu.commitActionPromptUpdate();
	_actionMenu.refreshActionPrompts();
	return false;
}

} // namespace Scooby
