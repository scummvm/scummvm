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

#include "room_actor_initializer.h"

#include "common/algorithm.h"

namespace Scooby {
void RoomActorInitializer::initializeRoomActors() {
	// Ghidra 0x00007A8C-0x00007AAF: retain only built-in visibility gates, clear path/loop state, seed ambient
	// movement timing, and clear interaction bit five without disturbing the other seven bits.
	_state.actorVisibilityBlockFlags &= 0x03;
	_state.actorAnimationLoopExitFlags = 0;
	_state.actorPathActiveFlags = 0;
	_state.randomMoveTimer = 100;
	_state.interactionFlags &= 0xDF;

	// Ghidra 0x00007AB0-0x00007AC5: both six-word delay arrays are immediately due at signed value -1.
	Common::fill(_state.actorStagedAnimationDelays.begin(), _state.actorStagedAnimationDelays.end(), -1);
	Common::fill(_state.actorAnimationDelays.begin(), _state.actorAnimationDelays.end(), -1);

	// Ghidra 0x00007AC6-0x00007ADD: clear directional latches, preserve dynamic movement bits, and seed the
	// lead actor's automatic idle-animation countdown.
	_state.shaggyActorWalkingAnimationLatchFlags = 0;
	_state.actorMovementFlags &= 0xFC;
	_state.shaggyActorIdleAnimationTimer = 300;

	// Ghidra 0x00007ADE-0x00007B0D: derive both built-in steady offsets from slot zero's signed position.
	// Interaction bit seven deliberately leaves slot one's previous animation offset unchanged.
	_state.actorAnimationOffsets[0] = static_cast<int16>(_state.actorPositionIndices[0] + 4);
	if ((_state.interactionFlags & 0x80) == 0) {
		_state.actorAnimationOffsets[1] = static_cast<int16>(_state.actorPositionIndices[0] + 4);
	}

	// Ghidra 0x00007B0E-0x00007B41: clear position updates, seed all three local masks for slots zero and one,
	// and begin the inclusive object-table scan with dynamic actor slot two.
	_state.actorPositionUpdateFlags = 0;
	uint8 activeActorFlags = 0x03;
	uint8 animationRestartFlags = 0x03;
	uint8 compactFrameFlags = 0x03;
	int actorSlot = kDynamicActorFirstSlot;

	for (size_t objectIndex = 0; objectIndex < _state.roomObjects.size(); ++objectIndex) {
		RoomObject &roomObject = _state.roomObjects[objectIndex];

		// Ghidra 0x00007B42-0x00007B7B: only fixed-position objects enter activation, but every such object is
		// first marked unassigned before the active-room test and complete 36-descriptor shape scan.
		if ((roomObject.flags & kFixedPositionMask) == 0) {
			continue;
		}

		roomObject.fixedPositionIndex = -1;
		if (roomObject.roomId != _state.roomId) {
			continue;
		}

		int descriptorOffset;
		int descriptorCatalogIndex;
		if (!_descriptorCatalog.tryResolve(roomObject.shapeIndex, descriptorOffset, descriptorCatalogIndex)) {
			continue;
		}

		// Ghidra 0x00007B7C-0x00007BD1: publish the matched descriptor's parallel step, exact logical
		// half-step, compact bit, fixed coordinates and targets, object animation, and zero-based slot index.
		int dynamicActorIndex = actorSlot - kDynamicActorFirstSlot;
		uint8 actorMask = static_cast<uint8>(1 << actorSlot);
		uint32 horizontalStep = _descriptorCatalog.readHorizontalStep(descriptorCatalogIndex);
		activeActorFlags |= actorMask;
		animationRestartFlags |= actorMask;
		if ((roomObject.flags & kCompactActorMask) != 0) {
			compactFrameFlags |= actorMask;
		}

		_state.roomObjectActorHorizontalSteps[dynamicActorIndex] = static_cast<int32>(horizontalStep);
		_state.roomObjectActorVerticalSteps[dynamicActorIndex] = static_cast<int32>(horizontalStep >> 1);
		_state.actorXFixedCoordinates[actorSlot] =
			(_state.actorXFixedCoordinates[actorSlot] & 0xFFFF) | (roomObject.fixedX << 16);
		_state.actorYFixedCoordinates[actorSlot] =
			(_state.actorYFixedCoordinates[actorSlot] & 0xFFFF) | (roomObject.fixedY << 16);
		_state.actorTargetXCoordinates[actorSlot] = roomObject.fixedX;
		_state.actorTargetYCoordinates[actorSlot] = roomObject.fixedY;
		roomObject.fixedPositionIndex = static_cast<int8>(dynamicActorIndex);
		_state.actorAnimationOffsets[actorSlot] = roomObject.tilePatchIndex;
		_state.actorAnimationDescriptorOffsets[actorSlot] = descriptorOffset;

		++actorSlot;
		if (actorSlot == kDynamicActorSlotLimit) {
			break;
		}
	}

	// Ghidra 0x00007BD2-0x00007C03: publish all accumulated masks, clear both pending graphics masks, and
	// block companion slot one exactly when interaction bit seven remains set.
	_state.activeActorFlags = activeActorFlags;
	_state.actorAnimationRestartFlags = animationRestartFlags;
	_state.actorCompactFrameFlags = compactFrameFlags;
	_state.actorTileUploadPendingFlags = 0;
	_state.actorCompactConversionPendingFlags = 0;
	if ((_state.interactionFlags & 0x80) != 0) {
		_state.actorVisibilityBlockFlags |= 0x02;
	}
}
} // namespace Scooby
