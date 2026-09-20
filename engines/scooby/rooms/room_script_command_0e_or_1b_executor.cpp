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

#include "room_script_command_0e_or_1b_executor.h"

#include "common/scummsys.h"

namespace Scooby {

void RoomScriptCommand0EOr1BExecutor::executeRoomScriptCommand0EOr1B(int mode) {
	int32 commandOffset = _state.RoomScriptStartOffset;
	uint16 command = _rom.readUInt16(commandOffset);

	// Ghidra 0x000049BA-0x000049CB: scan mode bypasses every effect. Execution masks identity bit 15 and
	// preserves identity zero as a no-op.
	if (mode != 0) {
		uint16 encodedObjectIdentity = _rom.readUInt16(commandOffset + 2);
		uint16 objectIdentity = static_cast<uint16>(encodedObjectIdentity & 0x7FFF);
		if (objectIdentity != 0) {
			// Ghidra 0x000049CC-0x000049D7: establish the active room-position source and select the
			// identity-one branch. The managed source is read only when indexed command 0x0E consumes it.
			switch (objectIdentity) {
			case 1: {
				// Ghidra 0x000049D8-0x000049FB: identity one independently selects indexed or immediate
				// coordinates before loading its animation or movement argument.
				TargetCoordinates target = readTargetCoordinates(commandOffset, command);
				int16 movementMode = _rom.readInt16(commandOffset + 4);
				int16 movementArgument = _rom.readInt16(commandOffset + 6);

				// Ghidra 0x000049FC-0x00004A43: negative movement mode directly replaces actor-zero integer
				// coordinates. A negative animation skips restart; otherwise retain both graphics-ready loop
				// outcomes and its asynchronous publication handover.
				if (movementMode < 0) {
					if (!placeActorDirectly(0, target.X, target.Y, movementArgument, -1, 0x01)) {
						return;
					}
				} else {
					// Ghidra 0x00004A44-0x00004A4F: nonnegative movement mode preserves the complete
					// lead-actor movement boundary and the original encoded identity context. Managed host
					// closure bypasses the native record epilogue as a non-returning handover.
					if (!_movement.moveLeadActorToRoomPosition(
							target.X, target.Y, movementMode, movementArgument,
							encodedObjectIdentity)) {
						_requestMovementHostClose();
						return;
					}
				}

				break;
			}
			case 2: {
				// Ghidra 0x00004A50-0x00004A79: identity two repeats both coordinate-source branches
				// independently for the companion actor.
				TargetCoordinates target = readTargetCoordinates(commandOffset, command);
				int16 movementMode = _rom.readInt16(commandOffset + 4);
				int16 movementArgument = _rom.readInt16(commandOffset + 6);

				// Ghidra 0x00004A7A-0x00004AC1: negative movement mode directly replaces actor-one integer
				// coordinates. Its nonnegative animation starts at delay one and retains both graphics-ready
				// outcomes instead of inheriting actor zero's delay.
				if (movementMode < 0) {
					if (!placeActorDirectly(1, target.X, target.Y, movementArgument, 1, 0x02)) {
						return;
					}
				} else {
					// Ghidra 0x00004AC2-0x00004ACD: nonnegative movement mode preserves the complete companion
					// movement and finish boundary, including encoded bit 15.
					if (!_movement.finishCompanionMovement(target.X, target.Y, movementMode,
														   movementArgument,
														   encodedObjectIdentity)) {
						_requestMovementHostClose();
						return;
					}
				}

				break;
			}
			default: {
				// Ghidra 0x00004ACE-0x00004B0D: every other nonzero identity retains its masked 16-bit value,
				// movement mode, independent coordinate-source branch, animation argument, and encoded
				// identity context at the room-object movement boundary.
				TargetCoordinates target = readTargetCoordinates(commandOffset, command);
				if (!_movement.moveRoomObjectToPosition(objectIdentity, target.X, target.Y,
														_rom.readInt16(commandOffset + 4),
														_rom.readInt16(commandOffset + 6),
														encodedObjectIdentity)) {
					_requestMovementHostClose();
					return;
				}

				break;
			}
			}
		}
	}

	// Ghidra 0x00004B0E-0x00004B23: restore the retained register and preserve both record-width returns;
	// command 0x0E consumes ten bytes while every other mapped command consumes twelve.
	_state.RoomScriptStartOffset =
		commandOffset + (command == kIndexedPositionCommand ? kIndexedRecordSize : kImmediateRecordSize);
}

RoomScriptCommand0EOr1BExecutor::TargetCoordinates RoomScriptCommand0EOr1BExecutor::readTargetCoordinates(
	int commandOffset, uint16 command) {
	if (command != kIndexedPositionCommand) {
		return TargetCoordinates(_rom.readInt16(commandOffset + 8), _rom.readInt16(commandOffset + 10));
	}

	int16 positionByteOffset = static_cast<int16>(_rom.readUInt16(commandOffset + 8) << 2);
	int32 positionOffset = _state.RoomPositionCoordinateTableOffset + positionByteOffset;
	return TargetCoordinates(static_cast<int16>(_rom.readInt16(positionOffset) << 3),
							 static_cast<int16>(_rom.readInt16(positionOffset + 2) << 3));
}

bool RoomScriptCommand0EOr1BExecutor::placeActorDirectly(int actorSlot, int16 targetX,
														 int16 targetY,
														 int16 animationOffset,
														 int16 initialDelay,
														 uint8 actorMask) {
	_state.ActorXFixedCoordinates[static_cast<std::size_t>(actorSlot)] =
		replaceIntegerCoordinate(_state.ActorXFixedCoordinates[static_cast<std::size_t>(actorSlot)], targetX);
	_state.ActorYFixedCoordinates[static_cast<std::size_t>(actorSlot)] =
		replaceIntegerCoordinate(_state.ActorYFixedCoordinates[static_cast<std::size_t>(actorSlot)], targetY);
	if (animationOffset < 0) {
		return true;
	}

	_state.ActorAnimationOffsets[static_cast<std::size_t>(actorSlot)] = animationOffset;
	_state.ActorAnimationDelays[static_cast<std::size_t>(actorSlot)] = initialDelay;
	_state.ActorAnimationRestartFlags |= actorMask;
	while (true) {
		_updateActorAnimationFrames();
		if ((_state.ActorGraphicsReadyFlags & actorMask) != 0) {
			return true;
		}

		if (!_waitForRoomVerticalBlank()) {
			return false;
		}
	}
}

int RoomScriptCommand0EOr1BExecutor::replaceIntegerCoordinate(int fixedCoordinate,
															  int16 integerCoordinate) {
	return (fixedCoordinate & 0x0000FFFF) | (static_cast<int>(integerCoordinate) << 16);
}
} // namespace Scooby