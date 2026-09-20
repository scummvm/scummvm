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

#include "room_script_command_0f_executor.h"

#include "common/scummsys.h"

namespace Scooby {

void RoomScriptCommand0FExecutor::executeRoomScriptCommand0F(int mode) {
	int32 commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x00004C20-0x00004C23: scan mode skips every effect and joins the fixed-width epilogue.
	if (mode != 0) {
		uint16 encodedTargetIdentity = _rom.readUInt16(commandOffset + 2);
		uint16 targetIdentity = static_cast<uint16>(encodedTargetIdentity & 0x7FFF);

		// Ghidra 0x00004C24-0x00004C79: target one or two selects its active descriptor directly. Every other
		// identity retains the wrapped room-object lookup and complete 36-entry shape scan; catalog exhaustion
		// skips movement without bypassing the record epilogue.
		int animationDescriptorOffset = 0;
		if (tryResolveTargetAnimationDescriptor(targetIdentity, animationDescriptorOffset)) {
			// Ghidra 0x00004C7A-0x00004CFF: retain all three source-identity branches, the type-two record
			// call for every room-object source, and both fixed-position and shape-relative outcomes.
			RelativeDestination target = resolveRelativeDestination(commandOffset, animationDescriptorOffset);
			int16 movementArgument = _rom.readInt16(commandOffset + 6);
			int16 movementModeOrScale = _rom.readInt16(commandOffset + 10);

			switch (targetIdentity) {
			case 1:
				// Ghidra 0x00004D00-0x00004D37: lead movement receives the encoded target context, then
				// actor slot zero copies the source room object's position selector. Managed host closure
				// retains the native non-returning handover and bypasses that post-movement copy.
				if (!_movement.moveLeadActorToRoomPosition(target.X, target.Y, movementModeOrScale,
														   movementArgument, encodedTargetIdentity)) {
					_requestMovementHostClose();
					return;
				}

				_state.ActorPositionIndices[0] = resolveRoomObject(_rom.readUInt16(commandOffset + 4)).PositionIndex;
				break;
			case 2:
				// Ghidra 0x00004D38-0x00004D63: companion direct movement remains distinct, then actor slot
				// one independently copies the source room object's position selector.
				if (!_movement
						 .moveCompanionActorDirectly(target.X, target.Y, movementModeOrScale, movementArgument,
													 true)
						 .Completed) {
					_requestMovementHostClose();
					return;
				}

				_state.ActorPositionIndices[1] = resolveRoomObject(_rom.readUInt16(commandOffset + 4)).PositionIndex;
				break;
			default:
				// Ghidra 0x00004D64-0x00004D79: every other masked target preserves argument order, movement
				// mode, encoded target context, and the common retained-register restoration.
				if (!_movement.moveRoomObjectToPosition(targetIdentity, target.X, target.Y, movementModeOrScale,
														movementArgument, encodedTargetIdentity)) {
					_requestMovementHostClose();
					return;
				}

				break;
			}
		}
	}

	// Ghidra 0x00004D7A-0x00004D7F: every mode, catalog outcome, source, and target consumes 12 bytes.
	_state.RoomScriptStartOffset = commandOffset + kRecordSize;
}

bool RoomScriptCommand0FExecutor::tryResolveTargetAnimationDescriptor(uint16 targetIdentity,
																	  int &descriptorOffset) {
	if (targetIdentity == 1) {
		descriptorOffset = _state.ActorAnimationDescriptorOffsets[0];
		return true;
	}

	if (targetIdentity == 2) {
		descriptorOffset = _state.ActorAnimationDescriptorOffsets[1];
		return true;
	}

	RoomObject &targetObject = resolveRoomObject(targetIdentity);
	int catalogIndex = -1;
	return _actorDescriptorCatalog.tryResolve(targetObject.ShapeIndex, descriptorOffset, catalogIndex);
}

RoomScriptCommand0FExecutor::RelativeDestination RoomScriptCommand0FExecutor::resolveRelativeDestination(
	int commandOffset, int animationDescriptorOffset) {
	uint16 sourceIdentity = _rom.readUInt16(commandOffset + 4);
	int16 relativeX = _rom.readInt16(commandOffset + 8);
	if (sourceIdentity == 1) {
		// Ghidra 0x00004C7A-0x00004C97: source one adds the relative X with word wrap and retains actor
		// zero's current integer Y without introducing a separate vertical command offset.
		return RelativeDestination(
			static_cast<int16>(relativeX + getIntegerCoordinate(_state.ActorXFixedCoordinates[0])),
			getIntegerCoordinate(_state.ActorYFixedCoordinates[0]));
	}

	if (sourceIdentity == 2) {
		// Ghidra 0x00004C98-0x00004CAD: source two repeats the independent actor-one coordinate branch.
		return RelativeDestination(
			static_cast<int16>(relativeX + getIntegerCoordinate(_state.ActorXFixedCoordinates[1])),
			getIntegerCoordinate(_state.ActorYFixedCoordinates[1]));
	}

	// Ghidra 0x00004CAE-0x00004CCD: every other source retains wrapped object-table selection, adds its
	// room-position selector to relative X, and calls the distinct type-two descriptor-record function.
	RoomObject &sourceObject = resolveRoomObject(sourceIdentity);
	int16 positionOffset = static_cast<int16>(relativeX + sourceObject.PositionIndex);
	RelativeDestination descriptor = findInteractionTypeTwoRecord(animationDescriptorOffset, positionOffset);
	if ((sourceObject.Flags & kFixedPositionMask) != 0) {
		// Ghidra 0x00004CD0-0x00004CDD: fixed placement doubles only descriptor X with word wrap, adds
		// authored fixed X, and replaces descriptor Y with authored fixed Y.
		return RelativeDestination(static_cast<int16>((descriptor.X << 1) + sourceObject.FixedX),
								   sourceObject.FixedY);
	}

	// Ghidra 0x00004CDE-0x00004CFF: the portable branch resolves the one-based episode shape pointer and adds
	// its signed +8/+10 origin to both descriptor coordinates with word wrapping.
	int32 episodeDescriptorOffset = _state.ActiveEpisodeDescriptorOffset;
	int shapePointerTableOffset =
		static_cast<int>(_rom.readUInt32(episodeDescriptorOffset + kEpisodeShapePointerTableField));
	int16 shapePointerByteOffset = static_cast<int16>((sourceObject.ShapeIndex - 1) << 2);
	int shapeOffset = static_cast<int>(_rom.readUInt32(shapePointerTableOffset + shapePointerByteOffset) +
									   _rom.readUInt32(episodeDescriptorOffset + kEpisodeShapeDataBaseField));
	return RelativeDestination(static_cast<int16>(descriptor.X + _rom.readInt16(shapeOffset + 8)),
							   static_cast<int16>(descriptor.Y + _rom.readInt16(shapeOffset + 10)));
}

RoomObject &RoomScriptCommand0FExecutor::resolveRoomObject(uint16 objectIdentity) {
	int16 tableByteOffset = static_cast<int16>((objectIdentity - 3) * kRoomObjectRecordSize);
	return _state.RoomObjects[static_cast<std::size_t>(tableByteOffset / kRoomObjectRecordSize)];
}

int16 RoomScriptCommand0FExecutor::getIntegerCoordinate(int fixedCoordinate) {
	return static_cast<int16>(fixedCoordinate >> 16);
}

RoomScriptCommand0FExecutor::RelativeDestination RoomScriptCommand0FExecutor::findInteractionTypeTwoRecord(
	int animationDescriptorOffset, int16 positionOffset) {
	// Ghidra 0x0000197E-0x0000198D: double the signed selector with word wrapping, add the descriptor's
	// word-table offset, resolve its signed list offset, and preserve the immutable descriptor base.
	int16 positionTableEntryOffset = static_cast<int16>(
		_rom.readInt16(animationDescriptorOffset + static_cast<int>(sizeof(int16))) + (positionOffset << 1));
	int interactionEntryOffset =
		animationDescriptorOffset + _rom.readInt16(animationDescriptorOffset + positionTableEntryOffset);

	// Ghidra 0x0000198E-0x0000199B: advance by one four-byte entry until the unbounded native scan finds
	// interaction type two. ROM bounds remain the only managed malformed-data failure boundary.
	while (_rom.readInt16(interactionEntryOffset) != 2) {
		interactionEntryOffset += static_cast<int>(sizeof(int32));
	}

	// Ghidra 0x0000199C-0x000019AF: restore the descriptor base semantically, resolve the selected signed
	// record offset, and return its signed X/Y words from +0x0E/+0x10.
	int interactionRecordOffset =
		animationDescriptorOffset + _rom.readInt16(
										interactionEntryOffset + static_cast<int>(sizeof(int16)));
	return RelativeDestination(_rom.readInt16(interactionRecordOffset + 0x0E),
							   _rom.readInt16(interactionRecordOffset + 0x10));
}
} // namespace Scooby