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

#include "scooby/rooms/actor_updater.h"

#include "common/array.h"

#include "actor_facing_animation.h"
#include "scooby/rooms/collision_probe_result.h"
#include "room_object.h"
#include "scooby/rooms/scene_data.h"
#include "scooby/common/contracts.h"
#include "scooby/common/span.h"

namespace Scooby {
void RoomActorUpdater::updateRoomActorsAndInterface() {
	// Ghidra 0x00005E4A-0x00005EE9: service lead-actor path cadence and every path control record.
	updateShaggyActorPath();

	// Ghidra 0x00005EEA-0x0000600B: service the signed interface transition and both cell-table transfers.
	// Ghidra 0x0000600C-0x0000632B: retain path termination, cursor movement, menu selection, and interface
	// close branches. Some suppression paths deliberately bypass the complete lead-actor block.
	bool skipShaggyActor = updateInterfaceTransitionAndCursor();

	// Ghidra 0x0000632C-0x00006D23: update lead movement, camera following, priority, and interface entry.
	if (!skipShaggyActor) {
		updateShaggyActorAndCamera();
	}

	// Ghidra 0x00006D24-0x0000707D: update companion copy, path, wander, and queued movement branches.
	updateScoobyActor();

	// Ghidra 0x0000707E-0x00007239: sample companion priority, service the paired transition layout,
	// interpolate compact scales, and propagate lead scale to the selected transition actor.
	updateActorPrioritiesAndScales();

	// Ghidra 0x0000723A-0x000073D5: service active actor slots two through five without omitting either
	// the path, movement, stationary, transition-skip, or priority-inhibition branches.
	updateDynamicActors();
}

void RoomActorUpdater::updateShaggyActorPath() {
	if (!hasFlag(_state.actorPathActiveFlags, kShaggyActorSlot)) {
		return;
	}

	_state.actorPathCadenceCountdowns[kShaggyActorSlot]--;
	if (_state.actorPathCadenceCountdowns[kShaggyActorSlot] >= 0) {
		return;
	}

	_state.actorPathCadenceCountdowns[kShaggyActorSlot] = static_cast<int8>(
		_state.actorPathCadenceReloads[kShaggyActorSlot]);
	while (true) {
		Common::Point record = readAndAdvancePathRecord(kShaggyActorSlot);
		int16 x = record.x;
		int16 y = record.y;
		if (x >= 0) {
			setActorIntegerCoordinates(kShaggyActorSlot, x, y);
			return;
		}

		if (x == -3) {
			requestActorAnimation(kShaggyActorSlot, y);
			continue;
		}

		if (x == -4 || x == -5) {
			uint16 scale = static_cast<uint16>(y);
			if (_state.actorHorizontalScaleFactors[kShaggyActorSlot] != scale) {
				_state.actorHorizontalScaleFactors[kShaggyActorSlot] = scale;
				_state.actorVerticalScaleFactors[kShaggyActorSlot] = scale;
				_state.actorPositionUpdateFlags |= kShaggyActorMask;
			}

			if (x == -5) {
				continue;
			}

			return;
		}

		if (x == -2) {
			return;
		}

		_state.actorPathRecordOffsets[kShaggyActorSlot] = 0;
	}
}

bool RoomActorUpdater::updateInterfaceTransitionAndCursor() {
	if (_state.roomInterfaceTransitionCountdown < 0) {
		if ((_state.interactionFlags & kInteractionMenuSuppressionMask) != 0) {
			return (_state.actorMovementFlags & kShaggyActorMask) == 0;
		}

		if ((_state.progressStateBytes[0] & kProgressInterfaceSuppressionMask) != 0) {
			return false;
		}

		if (isNewPress(ControllerButtons::C)) { // button C pressed
			publishLowerInterfaceCells(kLowerInterfaceClosingCellsOffset);
			_state.roomInterfaceTransitionCountdown = 0x10;
		}
	}

	if (_state.roomInterfaceTransitionCountdown >= 0) {
		advanceRoomInterfaceTransition();
	}

	if ((_state.displayFlags & kDisplayInterfaceMask) == 0) {
		return false;
	}

	endActorPathAtTerminal(kShaggyActorSlot);
	if (isNewPress(ControllerButtons::A)) {
		closeInteractionInterface();
		return true;
	}

	updateInteractionCursor();
	return true;
}

// transition to/from inventory
void RoomActorUpdater::advanceRoomInterfaceTransition() {
	if ((_state.displayFlags & kDisplayRightInterfaceMask) != 0) {
		_state.roomInterfaceHorizontalScroll += 16;
	} else {
		_state.roomInterfaceHorizontalScroll -= 16;
	}

	_scene.setInterfaceScroll(_state.roomInterfaceHorizontalScroll);

	_state.roomInterfaceTransitionCountdown--;
	if (_state.roomInterfaceTransitionCountdown != 0) {
		return;
	}

	if ((_state.interactionFlags & kInteractionMenuSuppressionMask) == 0) {
		if ((_state.progressStateBytes[0] & kProgressInterfaceSuppressionMask) == 0) {
			publishLowerInterfaceCells(kLowerInterfaceOpeningCellsOffset);
		}
	}

	_state.roomInterfaceTransitionCountdown = -1;
	_state.displayFlags ^= kDisplayRightInterfaceMask;
}

void RoomActorUpdater::publishLowerInterfaceCells(int sourceOffset) {
	Common::Array<uint8> packedRow(kLowerInterfaceColumnCount * sizeof(uint16));
	Span<uint8> packedRowSpan = makeSpan(packedRow);
	for (int row = 0; row < kLowerInterfaceRowCount; row++) {
		for (int column = 0; column < kLowerInterfaceColumnCount; column++) {
			uint16 packedCell = static_cast<uint16>(
				_rom.readUInt16(sourceOffset + row * kLowerInterfaceSourceRowByteStride +
								column * static_cast<int>(sizeof(uint16))) +
				_state.actionPromptTileAttributes);
			writeUInt16BigEndian(packedRowSpan, column * static_cast<int>(sizeof(uint16)), packedCell);
		}

		_scene.loadLayerRows(packedRowSpan, TileLayer::Interface, kLowerInterfaceStartColumn,
							 kLowerInterfaceStartRow + row, kLowerInterfaceColumnCount, 1);
	}
}

void RoomActorUpdater::updateInteractionCursor() {
	if (_state.cursorY >= 0xA8) {
		_state.cursorVerticalStep = 3;
		if ((_state.previousControllerOneInput & kCursorLeftMask) == 0) {
			_state.cursorHorizontalStep = 2;
		} else if ((_state.previousControllerOneInput & kCursorRightMask) == 0) {
			_state.cursorHorizontalStep = 2;
		} else {
			_state.cursorHorizontalStep = 0x28;
		}
	} else {
		_state.cursorHorizontalStep = 2;
		_state.cursorVerticalStep = 2;
	}

	if ((_state.displayFlags & kDisplayTransitionMask) == 0) {
		if ((_state.actorVisibilityBlockFlags & kShaggyActorMask) == 0) {
			if ((_state.roomBehaviorFlags & kRoomBehaviorDuelMask) == 0) {
				if ((_state.transitionFlags & kTransitionHideShaggyMask) == 0) {
					if ((_state.transitionFlags & kTransitionCenteredCursorMask) == 0) {
						requestActorAnimation(kShaggyActorSlot, static_cast<int16>(
																  _state.actorPositionIndices[kShaggyActorSlot] + 4));
					}
				}
			}
		}
	}

	if (_state.isButtonDown(ControllerButtons::Left)) {
		_state.cursorX -= _state.cursorHorizontalStep;
		if (_state.cursorX < -8) {
			_state.cursorX = -8;
		}
	}

	if (_state.isButtonDown(ControllerButtons::Right)) {
		_state.cursorX += _state.cursorHorizontalStep;
		if (_state.cursorX > 0xF7) {
			_state.cursorX = 0xF7;
		}
	}

	if (_state.isButtonDown(ControllerButtons::Up)) {
		if (_state.cursorY >= 0xC0) {
			_state.cursorY = 0xBF;
		} else {
			_state.cursorY -= _state.cursorVerticalStep;
			if (_state.cursorY < 0) {
				_state.cursorY = 0;
			}
		}
	}

	if (_state.isButtonDown(ControllerButtons::Down)) {
		if (_state.cursorY >= 0xA8) {
			if ((_state.previousControllerOneInput & kCursorDownMask) != 0) {
				_state.cursorY = 0xC0;
			}
		}

		_state.cursorY += _state.cursorVerticalStep;
		if (_state.cursorY > 0xCF) {
			_state.cursorY = 0xCF;
		}
	}

	if (_state.cursorY >= 0xA8) {
		updateMenuCommandFromCursor();
		if ((_state.displayFlags & kDisplayRightInterfaceMask) == 0) {
			return;
		}

		if (_state.menuCommand == 5 || _state.menuCommand == 10) {
			return;
		}
	}

	if (!isNewPress(ControllerButtons::B)) {
		return;
	}

	_state.selectedVerb = _state.activeVerbIcon;
	if (_state.selectedVerb == 0) {
		_state.selectedVerb = _state.cachedVerbIcon;
		if (_state.selectedVerb == 0) {
			return;
		}
	}

	if (_state.currentInteraction != 0) {
		_state.displayFlags |= kDisplayTransitionMask;
		return;
	}

	if ((_state.displayFlags & kDisplayTransitionMask) == 0) {
		closeInteractionInterface();
	}
}

void RoomActorUpdater::updateMenuCommandFromCursor() {
	int16 command = 1;
	int16 remainingX = _state.cursorX;
	if ((_state.displayFlags & kDisplayRightInterfaceMask) == 0) {
		remainingX -= 8;
	}

	do {
		remainingX -= 0x28;
		if (remainingX >= 0) {
			command++;
		}
	} while (remainingX >= 0);

	if (command > 6) {
		command = 6;
	}

	if (_state.cursorY > 0xC0) {
		command += 6;
	}

	if ((_state.displayFlags & kDisplayRightInterfaceMask) == 0) {
		if (command > 5) {
			command--;
			if (command > 10) {
				command--;
			}
		}
	} else {
		if (command > 7) {
			command--;
		}

		command--;
	}

	_state.menuCommand = command;
}

void RoomActorUpdater::closeInteractionInterface() {
	_state.displayFlags &= static_cast<uint8>(~kDisplayInterfaceMask);
	_state.pendingVerbIcon = _state.activeVerbIcon;
	_actionMenu.drawSelectedActionIcon(0);
	if (_state.currentInteraction == 0) {
		return;
	}

	// Managed-only guard: Shaggy and Scooby (interactions 1 and 2) have no room-object record, and the original
	// reads the bytes in front of the object table here (cancelling with A while the cursor is on either of
	// them). Show LOOK, the action ActionMenuController::resolveActionIcon gives both actors while hovering.
	if (_state.currentInteraction < kRoomObjectFirstInteraction) {
		_state.pendingVerbIcon = LOOK;
		_actionMenu.drawSelectedActionIcon(0);
		return;
	}

	const RoomObject &roomObject = _state.roomObjects[_state.currentInteraction - kRoomObjectFirstInteraction];
	_state.pendingVerbIcon = static_cast<VerbEnum>(roomObject.suggestedVerb);
	_actionMenu.drawSelectedActionIcon(0);
}

void RoomActorUpdater::updateShaggyActorAndCamera() {
	int16 maximumX = static_cast<int16>(_state.roomWidthTiles * 8 - 1);
	int16 maximumY = static_cast<int16>(_state.roomHeightTiles * 8 - 1);
	if (hasFlag(_state.actorPathActiveFlags, kShaggyActorSlot)) {
		endActorPathAtTerminal(kShaggyActorSlot);
		updateCameraAndInteractionEntry(maximumX, maximumY);
		return;
	}

	if ((_state.roomBehaviorFlags & kRoomBehaviorDuelMask) != 0) {
		if ((_state.duelStateFlags & 0x01) != 0) {
			updateCameraAndInteractionEntry(maximumX, maximumY);
		} else {
			tryOpenInteractionInterface();
		}

		return;
	}

	if ((_state.actorVisibilityBlockFlags & kShaggyActorMask) != 0) {
		return;
	}

	if ((_state.transitionFlags & kTransitionHideShaggyMask) != 0) {
		tryOpenInteractionInterface();
		return;
	}

	if ((_state.displayFlags & kDisplayTransitionMask) != 0 && (_state.actorMovementFlags & kShaggyActorMask) ==
																   0) {
		return;
	}

	int candidateX = _state.actorXFixedCoordinates[kShaggyActorSlot];
	int candidateY = _state.actorYFixedCoordinates[kShaggyActorSlot];
	uint32 stepMagnitude = static_cast<uint32>(static_cast<uint16>(
							   0x100 - _state.actorHorizontalScaleFactors[kShaggyActorSlot])) *
						   0x155u;
	if ((_state.actorMovementFlags & kShaggyActorMask) == 0) {
		_state.shaggyActorHorizontalMovementStep = 0x100;
		_state.shaggyActorVerticalMovementStep = 0x100;
	}

	if ((_state.displayFlags & kDisplayTransitionMask) != 0) {
		advanceShaggyScriptedMovement();
	} else {
		bool advancedScriptedMovement = updateShaggyControlledCandidate(candidateX, candidateY, stepMagnitude,
																	  maximumX, maximumY);
		if (!advancedScriptedMovement) {
			_state.actorMovementFlags &= static_cast<uint8>(~kShaggyActorMask);
			RoomCollisionProbeResult probe = _collisionProbe.probeShaggyActorCollision(candidateX, candidateY,
																					 maximumX, maximumY);
			if (probe.isClear) {
				_state.actorXFixedCoordinates[kShaggyActorSlot] = candidateX;
				_state.actorYFixedCoordinates[kShaggyActorSlot] = candidateY;
			} else {
				adjustShaggyMovementAroundCollision(candidateX, candidateY, maximumX, maximumY,
												  probe.retainedDataRegister);
			}
		}
	}

	updateActorPriority(kShaggyActorSlot);
	updateCameraAndInteractionEntry(maximumX, maximumY);
}

bool RoomActorUpdater::updateShaggyControlledCandidate(int &candidateX, int &candidateY,
													 uint32 stepMagnitude,
													 int16 maximumX, int16 maximumY) {
	bool noDirection = (_state.controllerOneInput & kDirectionMask) == kDirectionMask;
	if (!noDirection) {
		_state.shaggyActorIdleAnimationTimer = 300;
	}

	if (noDirection) {
		if ((_state.shaggyActorWalkingAnimationLatchFlags & 0x02) != 0) {
			moveShaggyLeft(candidateX, stepMagnitude);
		} else if ((_state.shaggyActorWalkingAnimationLatchFlags & 0x01) != 0) {
			moveShaggyRight(candidateX, stepMagnitude, maximumX);
		} else {
			uint32 verticalStep = stepMagnitude >> 1;
			if ((_state.shaggyActorWalkingAnimationLatchFlags & 0x04) != 0) {
				moveShaggyUp(candidateY, verticalStep, maximumY, false);
			} else if ((_state.shaggyActorWalkingAnimationLatchFlags & 0x08) != 0) {
				moveShaggyDown(candidateY, verticalStep, maximumY, false);
			} else {
				if ((_state.actorMovementFlags & kShaggyActorMask) != 0) {
					advanceShaggyScriptedMovement();
					return true;
				}

				updateShaggyIdleAnimation();
			}

			return false;
		}
	} else if (_state.isButtonDown(ControllerButtons::Left)) {
		moveShaggyLeft(candidateX, stepMagnitude);
	} else {
		if ((_state.shaggyActorWalkingAnimationLatchFlags & 0x02) != 0 &&
			_state.actorAnimationOffsets[kShaggyActorSlot] <= 3) {
			_state.shaggyActorWalkingAnimationLatchFlags &= 0xFD;
		}

		if (_state.isButtonDown(ControllerButtons::Right)) {
			moveShaggyRight(candidateX, stepMagnitude, maximumX);
		}
	}

	if ((_state.shaggyActorWalkingAnimationLatchFlags & 0x01) != 0 &&
		_state.actorAnimationOffsets[kShaggyActorSlot] <= 3) {
		_state.shaggyActorWalkingAnimationLatchFlags &= 0xFE;
	}

	if ((_state.shaggyActorWalkingAnimationLatchFlags & 0x03) != 0) {
		return false;
	}

	uint32 verticalMagnitude = stepMagnitude >> 1;
	if (_state.isButtonDown(ControllerButtons::Up)) {
		bool horizontalDirectionHeld = _state.isButtonDown(ControllerButtons::Left) ||
									   _state.isButtonDown(ControllerButtons::Right);
		bool graphicsPending = moveShaggyUp(candidateY, verticalMagnitude, maximumY, horizontalDirectionHeld);
		if (!graphicsPending) {
			return false;
		}
	}

	if (_state.isButtonDown(ControllerButtons::Down)) {
		bool horizontalDirectionHeld = _state.isButtonDown(ControllerButtons::Left) ||
									   _state.isButtonDown(ControllerButtons::Right);
		moveShaggyDown(candidateY, verticalMagnitude, maximumY, horizontalDirectionHeld);
	}

	return false;
}

void RoomActorUpdater::moveShaggyLeft(int &candidateX, uint32 stepMagnitude) {
	uint32 movementMagnitude;
	if (_state.actorAnimationOffsets[kShaggyActorSlot] == 1) {
		_state.actorPositionIndices[kShaggyActorSlot] = 1;
		if ((_state.actorGraphicsReadyFlags & kActorGraphicsReadyMask) == 0) {
			return;
		}

		_state.shaggyActorWalkingAnimationLatchFlags &= 0xFD;
		movementMagnitude = stepMagnitude;
	} else {
		movementMagnitude = stepMagnitude >> 1;
		int16 animationOffset = resolveShaggyWalkingAnimation(
			_state.actorPositionIndices[kShaggyActorSlot], 1);
		if (_state.actorAnimationOffsets[kShaggyActorSlot] != animationOffset) {
			requestActorAnimation(kShaggyActorSlot, animationOffset);
			_state.shaggyActorWalkingAnimationLatchFlags |= 0x02;
			return;
		}
	}

	candidateX = candidateX - static_cast<int>(movementMagnitude);
	if (candidateX < 0) {
		candidateX = 0;
	}
}

void RoomActorUpdater::moveShaggyRight(int &candidateX, uint32 stepMagnitude, int16 maximumX) {
	uint32 movementMagnitude;
	if (_state.actorAnimationOffsets[kShaggyActorSlot] == 0) {
		_state.actorPositionIndices[kShaggyActorSlot] = 0;
		if ((_state.actorGraphicsReadyFlags & kActorGraphicsReadyMask) == 0) {
			return;
		}

		_state.shaggyActorWalkingAnimationLatchFlags &= 0xFE;
		movementMagnitude = stepMagnitude;
	} else {
		movementMagnitude = stepMagnitude >> 1;
		int16 animationOffset = resolveShaggyWalkingAnimation(
			_state.actorPositionIndices[kShaggyActorSlot], 0);
		if (_state.actorAnimationOffsets[kShaggyActorSlot] != animationOffset) {
			requestActorAnimation(kShaggyActorSlot, animationOffset);
			_state.shaggyActorWalkingAnimationLatchFlags |= 0x01;
			return;
		}
	}

	candidateX = candidateX + static_cast<int>(movementMagnitude);
	if (getIntegerCoordinate(candidateX) > maximumX) {
		candidateX = replaceIntegerCoordinate(candidateX, maximumX);
	}
}

bool RoomActorUpdater::moveShaggyUp(int &candidateY, uint32 movementMagnitude, int16 maximumY,
								  bool horizontalDirectionHeld) {
	if (horizontalDirectionHeld || _state.actorAnimationOffsets[kShaggyActorSlot] == 2) {
		_state.actorPositionIndices[kShaggyActorSlot] = 2;
		if ((_state.actorGraphicsReadyFlags & kActorGraphicsReadyMask) == 0) {
			return true;
		}

		_state.shaggyActorWalkingAnimationLatchFlags &= 0xF3;
	} else {
		int16 animationOffset = resolveShaggyWalkingAnimation(
			_state.actorPositionIndices[kShaggyActorSlot], 2);
		movementMagnitude >>= 1;
		if (_state.actorAnimationOffsets[kShaggyActorSlot] != animationOffset) {
			requestActorAnimation(kShaggyActorSlot, animationOffset);
			_state.shaggyActorWalkingAnimationLatchFlags |= 0x04;
			return false;
		}
	}

	if ((_state.transitionFlags & kTransitionForegroundPolarityMask) != 0) {
		candidateY = candidateY + static_cast<int>(movementMagnitude);
		if (getIntegerCoordinate(candidateY) > maximumY) {
			candidateY = replaceIntegerCoordinate(candidateY, maximumY);
		}

		return false;
	}

	candidateY = candidateY - static_cast<int>(movementMagnitude);
	if (getIntegerCoordinate(candidateY) < 0) {
		candidateY = replaceIntegerCoordinate(candidateY, 0);
	}

	return false;
}

void RoomActorUpdater::moveShaggyDown(int &candidateY, uint32 movementMagnitude, int16 maximumY,
									bool horizontalDirectionHeld) {
	if (horizontalDirectionHeld || _state.actorAnimationOffsets[kShaggyActorSlot] == 3) {
		_state.actorPositionIndices[kShaggyActorSlot] = 3;
		if ((_state.actorGraphicsReadyFlags & kActorGraphicsReadyMask) == 0) {
			return;
		}

		_state.shaggyActorWalkingAnimationLatchFlags &= 0xF3;
	} else {
		int16 animationOffset = resolveShaggyWalkingAnimation(
			_state.actorPositionIndices[kShaggyActorSlot], 3);
		movementMagnitude >>= 1;
		if (_state.actorAnimationOffsets[kShaggyActorSlot] != animationOffset) {
			requestActorAnimation(kShaggyActorSlot, animationOffset);
			_state.shaggyActorWalkingAnimationLatchFlags |= 0x08;
			return;
		}
	}

	if ((_state.transitionFlags & kTransitionForegroundPolarityMask) != 0) {
		candidateY = candidateY - static_cast<int>(movementMagnitude);
		if (getIntegerCoordinate(candidateY) < 0) {
			candidateY = replaceIntegerCoordinate(candidateY, 0);
		}

		return;
	}

	candidateY = candidateY + static_cast<int>(movementMagnitude);
	if (getIntegerCoordinate(candidateY) > maximumY) {
		candidateY = replaceIntegerCoordinate(candidateY, maximumY);
	}
}

void RoomActorUpdater::updateShaggyIdleAnimation() {
	_state.shaggyActorIdleAnimationTimer--;
	if (_state.shaggyActorIdleAnimationTimer < 0) {
		_state.shaggyActorIdleAnimationTimer = 300;
		requestActorAnimation(kShaggyActorSlot, static_cast<int16>(
												  _state.actorPositionIndices[kShaggyActorSlot] + 0x20));
		return;
	}

	int16 animationOffset = _state.actorAnimationOffsets[kShaggyActorSlot];
	int16 desiredOffset = static_cast<int16>(_state.actorPositionIndices[kShaggyActorSlot] + 4);
	if (animationOffset < 0x20 || animationOffset >= 0x24 ||
		(_state.actorAnimationHoldFlags & kActorAnimationHoldMask) != 0) {
		if (animationOffset != desiredOffset && (_state.actorAnimationRestartFlags & kShaggyActorMask) == 0) {
			requestActorAnimation(kShaggyActorSlot, desiredOffset);
		}
	}
}

void RoomActorUpdater::adjustShaggyMovementAroundCollision(int &candidateX, int &candidateY,
														 int16 maximumX,
														 int16 maximumY,
														 int16 retainedDataRegister) {
	int16 directionIndex = getControllerDirectionIndex(retainedDataRegister);
	if (directionIndex <= 1) {
		return;
	}

	if (directionIndex <= 3) {
		return;
	}

	if (directionIndex <= 5) {
		int16 movementStep = -0x100;
		for (int attempt = 0; attempt < 4; attempt++) {
			candidateY = replaceIntegerCoordinate(candidateY, static_cast<int16>(
																  getIntegerCoordinate(candidateY) - 1));
			if (_collisionProbe.probeShaggyActorCollision(candidateX, candidateY, maximumX, maximumY).isClear) {
				beginShaggyVerticalCollisionAdjustment(candidateX, candidateY, movementStep);
				return;
			}
		}

		candidateY = replaceIntegerCoordinate(candidateY, static_cast<int16>(
															  getIntegerCoordinate(candidateY) + 4));
		movementStep = 0x100;
		for (int attempt = 0; attempt < 4; attempt++) {
			candidateY = replaceIntegerCoordinate(candidateY, static_cast<int16>(
																  getIntegerCoordinate(candidateY) + 1));
			if (_collisionProbe.probeShaggyActorCollision(candidateX, candidateY, maximumX, maximumY).isClear) {
				beginShaggyVerticalCollisionAdjustment(candidateX, candidateY, movementStep);
				return;
			}
		}

		return;
	}

	int16 horizontalStep = -0x100;
	for (int attempt = 0; attempt < 3; attempt++) {
		candidateX = replaceIntegerCoordinate(candidateX, static_cast<int16>(
															  getIntegerCoordinate(candidateX) - 1));
		if (_collisionProbe.probeShaggyActorCollision(candidateX, candidateY, maximumX, maximumY).isClear) {
			beginShaggyHorizontalCollisionAdjustment(candidateX, candidateY, horizontalStep);
			return;
		}
	}

	candidateX = replaceIntegerCoordinate(candidateX, static_cast<int16>(
														  getIntegerCoordinate(candidateX) + 3));
	horizontalStep = 0x100;
	for (int attempt = 0; attempt < 3; attempt++) {
		candidateX = replaceIntegerCoordinate(candidateX, static_cast<int16>(
															  getIntegerCoordinate(candidateX) + 1));
		if (_collisionProbe.probeShaggyActorCollision(candidateX, candidateY, maximumX, maximumY).isClear) {
			beginShaggyHorizontalCollisionAdjustment(candidateX, candidateY, horizontalStep);
			return;
		}
	}
}

void RoomActorUpdater::beginShaggyVerticalCollisionAdjustment(int candidateX, int candidateY,
															int16 movementStep) {
	_state.shaggyActorVerticalMovementStep = movementStep;
	_state.shaggyActorHorizontalMovementStep = 0;
	_state.actorXFixedCoordinates[kShaggyActorSlot] = candidateX;
	_state.actorTargetXCoordinates[kShaggyActorSlot] = getIntegerCoordinate(candidateX);
	_state.actorMovementFlags |= kShaggyActorMask;
	_state.actorTargetYCoordinates[kShaggyActorSlot] = getIntegerCoordinate(candidateY);
	advanceShaggyScriptedMovement();
}

void RoomActorUpdater::beginShaggyHorizontalCollisionAdjustment(int candidateX, int candidateY,
															  int16 movementStep) {
	_state.shaggyActorHorizontalMovementStep = movementStep;
	_state.shaggyActorVerticalMovementStep = 0;
	_state.actorTargetXCoordinates[kShaggyActorSlot] = getIntegerCoordinate(candidateX);
	_state.actorMovementFlags |= kShaggyActorMask;
	_state.actorYFixedCoordinates[kShaggyActorSlot] = candidateY;
	_state.actorTargetYCoordinates[kShaggyActorSlot] = getIntegerCoordinate(candidateY);
	advanceShaggyScriptedMovement();
}

void RoomActorUpdater::advanceShaggyScriptedMovement() {
	_state.shaggyActorIdleAnimationTimer = 300;
	int32 horizontalDelta = scaleMovementStep(_state.actorHorizontalScaleFactors[kShaggyActorSlot], 0x155,
											  _state.shaggyActorHorizontalMovementStep);
	int32 verticalDelta = scaleMovementStep(_state.actorVerticalScaleFactors[kShaggyActorSlot], 0xAA,
											_state.shaggyActorVerticalMovementStep);
	_state.actorXFixedCoordinates[kShaggyActorSlot] = _state.actorXFixedCoordinates[kShaggyActorSlot] +
													horizontalDelta;
	_state.actorYFixedCoordinates[kShaggyActorSlot] = _state.actorYFixedCoordinates[kShaggyActorSlot] +
													verticalDelta;

	int16 actorX = getIntegerCoordinate(_state.actorXFixedCoordinates[kShaggyActorSlot]);
	int16 targetX = _state.actorTargetXCoordinates[kShaggyActorSlot];
	if (_state.shaggyActorHorizontalMovementStep >= 0) {
		if (actorX >= targetX) {
			_state.actorXFixedCoordinates[kShaggyActorSlot] = replaceIntegerCoordinate(
				_state.actorXFixedCoordinates[kShaggyActorSlot], targetX);
			_state.shaggyActorHorizontalMovementStep = 0;
		}
	} else if (actorX <= targetX) {
		_state.shaggyActorHorizontalMovementStep = 0;
	}

	int16 actorY = getIntegerCoordinate(_state.actorYFixedCoordinates[kShaggyActorSlot]);
	int16 targetY = _state.actorTargetYCoordinates[kShaggyActorSlot];
	if (_state.shaggyActorVerticalMovementStep >= 0) {
		if (actorY >= targetY) {
			_state.actorYFixedCoordinates[kShaggyActorSlot] = replaceIntegerCoordinate(
				_state.actorYFixedCoordinates[kShaggyActorSlot], targetY);
			_state.shaggyActorVerticalMovementStep = 0;
		}
	} else if (actorY <= targetY) {
		_state.actorYFixedCoordinates[kShaggyActorSlot] = replaceIntegerCoordinate(
			_state.actorYFixedCoordinates[kShaggyActorSlot], targetY);
		_state.shaggyActorVerticalMovementStep = 0;
	}

	if (_state.shaggyActorHorizontalMovementStep != 0 || _state.shaggyActorVerticalMovementStep != 0) {
		return;
	}

	if (_state.shaggyActorMovementPointIndex >= 0) {
		_state.shaggyActorMovementPointIndex--;
		if (_state.shaggyActorMovementPointIndex >= 0) {
			beginNextShaggyMovementPoint(_state.shaggyActorMovementPoints[_state.shaggyActorMovementPointIndex]);
			return;
		}
	}

	_state.actorMovementFlags &= static_cast<uint8>(~kShaggyActorMask);
	if ((_state.displayFlags & kDisplayTransitionMask) != 0) {
		requestActorAnimation(kShaggyActorSlot, static_cast<int16>(
												  _state.actorPositionIndices[kShaggyActorSlot] + 4));
	}
}

void RoomActorUpdater::beginNextShaggyMovementPoint(const ActorMovementPoint &point) {
	_state.actorTargetXCoordinates[kShaggyActorSlot] = point.targetX;
	_state.actorTargetYCoordinates[kShaggyActorSlot] = point.targetY;
	_state.shaggyActorHorizontalMovementStep = point.horizontalStep;
	_state.shaggyActorVerticalMovementStep = point.verticalStep;
	int16 relativeAnimation = static_cast<int16>(
		_state.actorAnimationOffsets[kShaggyActorSlot] - _state.actorPositionIndices[kShaggyActorSlot]);
	_state.actorAnimationOffsets[kShaggyActorSlot] = relativeAnimation == 0
													   ? resolveShaggyWalkingAnimation(
															 _state.actorPositionIndices[kShaggyActorSlot],
															 point.positionIndex)
													   : static_cast<int16>(point.positionIndex +
																			relativeAnimation);
	_state.actorPositionIndices[kShaggyActorSlot] = point.positionIndex;
	_state.actorAnimationRestartFlags |= kShaggyActorMask;
}

void RoomActorUpdater::updateCameraAndInteractionEntry(int16 maximumX, int16 maximumY) {
	if ((_state.roomBehaviorFlags & kRoomBehaviorCameraLockMask) == 0) {
		updateCameraX(maximumX);
		updateCameraY(maximumY);
	}

	tryOpenInteractionInterface();
}

void RoomActorUpdater::updateCameraX(int16 maximumX) {
	int16 actorX = getIntegerCoordinate(_state.actorXFixedCoordinates[kShaggyActorSlot]);
	int16 relativeX = static_cast<int16>(actorX - _state.cameraX);
	int16 nextCameraX;
	if (relativeX <= 0x4B) {
		nextCameraX = static_cast<int16>(actorX - 0x4B);
		if (nextCameraX < 0) {
			nextCameraX = 0;
		}
	} else if (relativeX >= 0xB5) {
		nextCameraX = static_cast<int16>(actorX - 0xB4);
		int16 maximumCameraX = static_cast<int16>(maximumX - 0xFF);
		if (nextCameraX > maximumCameraX) {
			nextCameraX = maximumCameraX;
		}
	} else {
		return;
	}

	uint16 previousTile = static_cast<uint16>(_state.cameraX) >> 3;
	_state.cameraX = nextCameraX;
	uint16 nextTile = static_cast<uint16>(nextCameraX) >> 3;
	if (previousTile == nextTile) {
		return;
	}

	_state.tileStreamingEdgeFlags |= previousTile < nextTile
										 ? static_cast<uint8>(0x01)
										 : static_cast<uint8>(0x02);
}

void RoomActorUpdater::updateCameraY(int16 maximumY) {
	int16 actorY = getIntegerCoordinate(_state.actorYFixedCoordinates[kShaggyActorSlot]);
	int16 relativeY = static_cast<int16>(actorY - _state.cameraY);
	int16 nextCameraY;
	if (relativeY <= 0x32) {
		nextCameraY = static_cast<int16>(actorY - 0x32);
		if (nextCameraY < 0) {
			nextCameraY = 0;
		}
	} else if (relativeY >= 0x66) {
		nextCameraY = static_cast<int16>(actorY - 0x65);
		int16 maximumCameraY = static_cast<int16>(maximumY - 0x97);
		if (nextCameraY > maximumCameraY) {
			nextCameraY = maximumCameraY;
		}
	} else {
		return;
	}

	uint16 previousTile = static_cast<uint16>(_state.cameraY) >> 3;
	_state.cameraY = nextCameraY;
	uint16 nextTile = static_cast<uint16>(nextCameraY) >> 3;
	if (previousTile == nextTile) {
		return;
	}

	_state.tileStreamingEdgeFlags |= previousTile < nextTile
										 ? static_cast<uint8>(0x08)
										 : static_cast<uint8>(0x04);
}

void RoomActorUpdater::tryOpenInteractionInterface() {
	if ((_state.displayFlags & kDisplayTransitionMask) != 0) {
		return;
	}

	if ((_state.progressStateBytes[0] & kProgressInterfaceSuppressionMask) != 0) {
		return;
	}

	bool shouldOpen = isNewPress(ControllerButtons::A);
	if (!shouldOpen) {
		shouldOpen = isNewPress(ControllerButtons::B);
	}

	// Managed-only mouse support: the pointer opens the cursor interface like an A/B press would
	// (input/room_mouse_controller.h).
	if (!shouldOpen) {
		shouldOpen = _state.mouseOpenInterfaceRequest;
	}

	if (!shouldOpen) {
		return;
	}

	_state.displayFlags |= kDisplayInterfaceMask;
	_state.interactionFlags &= 0xFE;
	_state.cursorHorizontalStep = 2;
	_state.cursorVerticalStep = 2;
	_state.activeVerbIcon = NONE;
	_state.previousInteraction = 0;
	_state.secondaryInteraction = 0;

	if ((_state.transitionFlags & kTransitionCenteredCursorMask) != 0) {
		setCenteredCursor();
		return;
	}

	if ((_state.actorVisibilityBlockFlags & kShaggyActorMask) == 0) {
		if ((_state.transitionFlags & kTransitionHideShaggyMask) != 0) {
			setCenteredCursor();
			return;
		}

		if ((_state.roomBehaviorFlags & kRoomBehaviorDuelMask) == 0) {
			requestActorAnimation(kShaggyActorSlot, static_cast<int16>(
													  _state.actorPositionIndices[kShaggyActorSlot] + 4));
		}
	}

	_state.cursorX = static_cast<int16>(
		getIntegerCoordinate(_state.actorXFixedCoordinates[kShaggyActorSlot]) - _state.cameraX);
	_state.cursorY = static_cast<int16>(
		getIntegerCoordinate(_state.actorYFixedCoordinates[kShaggyActorSlot]) - _state.cameraY - 0x2C);
}

void RoomActorUpdater::setCenteredCursor() {
	_state.cursorX = 0x80;
	_state.cursorY = 0x50;
}

void RoomActorUpdater::updateScoobyActor() {
	if ((_state.videoFlags & kVideoScoobyUpdateBlockMask) != 0) {
		return;
	}

	// Scooby walks beside Shaggy. Used in Maze room.
	if ((_state.videoFlags & kVideoScoobyShaggySideBySideWalking) != 0) {
		_state.actorXFixedCoordinates[kScoobyActorSlot] = replaceIntegerCoordinate(
			_state.actorXFixedCoordinates[kScoobyActorSlot],
			getIntegerCoordinate(_state.actorXFixedCoordinates[kShaggyActorSlot]));
		_state.actorYFixedCoordinates[kScoobyActorSlot] = replaceIntegerCoordinate(
			_state.actorYFixedCoordinates[kScoobyActorSlot],
			getIntegerCoordinate(_state.actorYFixedCoordinates[kShaggyActorSlot]));
		// Ghidra 0x00006D4E-0x00006D8E: controller input is active-low, so all four direction bits set means no
		// direction is held. With no direction held, or with the interaction display open, select the lead
		// pose's steady animation (+4); otherwise select its walking animation (+0).
		int16 animationOffset = _state.actorPositionIndices[kShaggyActorSlot];
		if ((_state.controllerOneInput & kDirectionMask) == kDirectionMask ||
			(_state.displayFlags & kDisplayInterfaceMask) != 0) {
			animationOffset += 4;
		}

		if (_state.actorAnimationOffsets[kScoobyActorSlot] != animationOffset) {
			requestActorAnimation(kScoobyActorSlot, animationOffset);
		}

		return;
	}

	if (hasFlag(_state.actorPathActiveFlags, kScoobyActorSlot)) {
		updateStandardActorPath(kScoobyActorSlot);
		return;
	}

	if ((_state.actorMovementFlags & kScoobyActorMask) == 0) {
		if (_state.randomMoveTimer >= 0) {
			_state.randomMoveTimer--;
		}

		return;
	}

	if ((_state.interactionFlags & 0x20) != 0) {
		if ((_state.actorAnimationRestartFlags & kScoobyActorMask) == 0) {
			if ((_state.actorAnimationHoldFlags & kScoobyActorMask) != 0) {
				_state.actorAnimationOffsets[kScoobyActorSlot] = static_cast<int16>(
					_state.randomMoveAnimation + _state.randomMoveDirection);
				_state.actorPositionIndices[kScoobyActorSlot] = _state.randomMoveDirection;
				_state.actorAnimationDelays[kScoobyActorSlot] = -1;
				_state.actorAnimationRestartFlags |= kScoobyActorMask;
				_state.actorMovementFlags &= static_cast<uint8>(~kScoobyActorMask);
				_state.interactionFlags &= 0xDF;
			}
		}

		return;
	}

	advanceScoobyScriptedMovement();
}

void RoomActorUpdater::advanceScoobyScriptedMovement() {
	int32 horizontalDelta = scaleMovementStep(_state.actorHorizontalScaleFactors[kScoobyActorSlot],
											  0x12A,
											  _state.scoobyActorHorizontalMovementStep);
	int32 verticalDelta = scaleMovementStep(_state.actorVerticalScaleFactors[kScoobyActorSlot], 0x95,
											_state.scoobyActorVerticalMovementStep);
	_state.actorXFixedCoordinates[kScoobyActorSlot] =
		_state.actorXFixedCoordinates[kScoobyActorSlot] + horizontalDelta;
	_state.actorYFixedCoordinates[kScoobyActorSlot] =
		_state.actorYFixedCoordinates[kScoobyActorSlot] + verticalDelta;

	int16 actorX = getIntegerCoordinate(_state.actorXFixedCoordinates[kScoobyActorSlot]);
	int16 targetX = _state.actorTargetXCoordinates[kScoobyActorSlot];
	if (_state.scoobyActorHorizontalMovementStep >= 0) {
		if (actorX >= targetX) {
			_state.actorXFixedCoordinates[kScoobyActorSlot] = replaceIntegerCoordinate(
				_state.actorXFixedCoordinates[kScoobyActorSlot], targetX);
			_state.scoobyActorHorizontalMovementStep = 0;
		}
	} else if (actorX <= targetX) {
		_state.scoobyActorHorizontalMovementStep = 0;
	}

	int16 actorY = getIntegerCoordinate(_state.actorYFixedCoordinates[kScoobyActorSlot]);
	int16 targetY = _state.actorTargetYCoordinates[kScoobyActorSlot];
	if (_state.scoobyActorVerticalMovementStep >= 0) {
		if (actorY >= targetY) {
			_state.actorYFixedCoordinates[kScoobyActorSlot] = replaceIntegerCoordinate(
				_state.actorYFixedCoordinates[kScoobyActorSlot], targetY);
			_state.scoobyActorVerticalMovementStep = 0;
		}
	} else if (actorY <= targetY) {
		_state.actorYFixedCoordinates[kScoobyActorSlot] = replaceIntegerCoordinate(
			_state.actorYFixedCoordinates[kScoobyActorSlot], targetY);
		_state.scoobyActorVerticalMovementStep = 0;
	}

	if (_state.scoobyActorHorizontalMovementStep != 0 || _state.scoobyActorVerticalMovementStep != 0) {
		return;
	}

	if (_state.scoobyActorMovementPointIndex >= 0) {
		_state.scoobyActorMovementPointIndex--;
		if (_state.scoobyActorMovementPointIndex >= 0) {
			beginNextScoobyMovementPoint(
				_state.scoobyActorMovementPoints[_state.scoobyActorMovementPointIndex]);
			return;
		}
	}

	if ((_state.displayFlags & kDisplayTransitionMask) != 0) {
		requestActorAnimation(kScoobyActorSlot, static_cast<int16>(
													   _state.actorPositionIndices[kScoobyActorSlot] + 4));
		_state.actorMovementFlags &= static_cast<uint8>(~kScoobyActorMask);
		return;
	}

	_state.interactionFlags |= 0x20;
	_state.actorAnimationOffsets[kScoobyActorSlot] =
		resolveScoobyTurnAnimation(
			_state.actorPositionIndices[kScoobyActorSlot], _state.randomMoveDirection);
	_state.actorAnimationDelays[kScoobyActorSlot] = -1;
	_state.actorAnimationRestartFlags |= kScoobyActorMask;
}

void RoomActorUpdater::beginNextScoobyMovementPoint(const ActorMovementPoint &point) {
	_state.actorTargetXCoordinates[kScoobyActorSlot] = point.targetX;
	_state.actorTargetYCoordinates[kScoobyActorSlot] = point.targetY;
	_state.scoobyActorHorizontalMovementStep = point.horizontalStep;
	_state.scoobyActorVerticalMovementStep = point.verticalStep;
	int16 relativeAnimation = static_cast<int16>(
		_state.actorAnimationOffsets[kScoobyActorSlot] - _state.actorPositionIndices[kScoobyActorSlot]);
	_state.actorAnimationOffsets[kScoobyActorSlot] = relativeAnimation == 0
															? resolveScoobyWalkingAnimation(
																  _state.actorPositionIndices[kScoobyActorSlot],
																  point.positionIndex)
															: static_cast<int16>(point.positionIndex +
																				 relativeAnimation);
	_state.actorPositionIndices[kScoobyActorSlot] = point.positionIndex;
	_state.actorAnimationDelays[kScoobyActorSlot] = -1;
	_state.actorAnimationRestartFlags |= kScoobyActorMask;
}

void RoomActorUpdater::updateActorPrioritiesAndScales() {
	updateActorPriority(kScoobyActorSlot);
	if ((_state.transitionFlags & kTransitionCompanionCopyMask) != 0) {
		_state.actorHighPriorityFlags = static_cast<uint8>((_state.actorHighPriorityFlags & 0xF0) |
														   kShaggyAndScoobyActorMask);
		_state.actorXFixedCoordinates[2] = _state.actorXFixedCoordinates[kShaggyActorSlot];
		_state.actorYFixedCoordinates[2] = _state.actorYFixedCoordinates[kShaggyActorSlot];
		if (getIntegerCoordinate(_state.actorXFixedCoordinates[kShaggyActorSlot]) >= 0xF3) {
			_state.actorVerticalScaleFactors[2] = 0;
			_state.actorHorizontalScaleFactors[2] = 0x80;
		} else {
			_state.actorVerticalScaleFactors[2] = 0x80;
			_state.actorHorizontalScaleFactors[2] = 0;
		}

		_state.actorYFixedCoordinates[2] = replaceIntegerCoordinate(_state.actorYFixedCoordinates[2],
																	static_cast<int16>(
																		getIntegerCoordinate(
																			_state.actorYFixedCoordinates[kShaggyActorSlot]) -
																		0x14));
		int16 pairedX = static_cast<int16>(
			getIntegerCoordinate(_state.actorXFixedCoordinates[kShaggyActorSlot]) + 0x14);
		if (pairedX > 0x160) {
			pairedX = 0x160;
		}

		_state.actorXFixedCoordinates[2] = replaceIntegerCoordinate(_state.actorXFixedCoordinates[2], pairedX);
		if (getIntegerCoordinate(_state.actorXFixedCoordinates[3]) == 0x150) {
			_state.actorXFixedCoordinates[3] =
				replaceIntegerCoordinate(_state.actorXFixedCoordinates[3], 0x150);
		}
	}

	int16 retainedPreviousScaleRow = static_cast<int16>(
		_state.roomWidthTiles * static_cast<int>(sizeof(int16)));
	for (int actor = 5; actor >= 0; actor--) {
		if (hasFlag(_state.actorCompactFrameFlags, actor)) {
			if ((_state.transitionFlags & kTransitionSkipCompactScaleMask) == 0) {
				updateCompactActorScale(actor, retainedPreviousScaleRow);
			}
		}
	}

	if ((_state.roomBehaviorFlags & kRoomBehaviorTransitionScaleMask) != 0) {
		uint16 transitionActor = _state.transitionActorIndex;
		_state.actorHorizontalScaleFactors[transitionActor] = _state.actorHorizontalScaleFactors[kShaggyActorSlot];
		_state.actorVerticalScaleFactors[transitionActor] = _state.actorVerticalScaleFactors[kShaggyActorSlot];
	}
}

void RoomActorUpdater::updateCompactActorScale(int actor, int16 &retainedPreviousScaleRow) {
	int16 actorY = getIntegerCoordinate(_state.actorYFixedCoordinates[actor]);
	int actorTileY = static_cast<uint16>(actorY) >> 3;
	int16 previousScale = 0;
	int16 row = 0;
	do {
		int16 scale = _rom.readInt16(_state.roomCoordinateDataOffset +
									 row * static_cast<int>(sizeof(int16)));
		if (scale >= 0) {
			previousScale = scale;
			retainedPreviousScaleRow = row;
		}
	} while (++row < actorTileY);

	int16 nextScale = 0;
	int16 nextRow = static_cast<int16>(_state.roomHeightTiles);
	for (; row < static_cast<int16>(_state.roomHeightTiles); row++) {
		int16 scale = _rom.readInt16(_state.roomCoordinateDataOffset +
									 row * static_cast<int>(sizeof(int16)));
		if (scale >= 0) {
			nextScale = scale;
			nextRow = row;
			break;
		}
	}

	int16 scaleDifference = static_cast<int16>((nextScale - previousScale) << 4);
	int16 remainingPixels = static_cast<int16>(nextRow * 8 + 7 - actorY);
	int16 rowSpanPixels = static_cast<int16>((nextRow - retainedPreviousScaleRow) * 8);
	int16 interpolatedScale = static_cast<int16>(
		nextScale * 0x10 - (scaleDifference * remainingPixels) / rowSpanPixels);
	uint8 actorMask = this->actorMask(actor);
	if (_state.actorHorizontalScaleFactors[actor] != static_cast<uint16>(interpolatedScale)) {
		_state.actorHorizontalScaleFactors[actor] = static_cast<uint16>(interpolatedScale);
		_state.actorPositionUpdateFlags |= actorMask;
	}

	if (_state.actorVerticalScaleFactors[actor] != static_cast<uint16>(interpolatedScale)) {
		_state.actorVerticalScaleFactors[actor] = static_cast<uint16>(interpolatedScale);
		_state.actorPositionUpdateFlags |= actorMask;
	}
}

void RoomActorUpdater::updateDynamicActors() {
	for (int actor = 2; actor < 6; actor++) {
		if ((_state.transitionFlags & kTransitionCompanionCopyMask) != 0 && actor < 4) {
			continue;
		}

		if (!hasFlag(_state.activeActorFlags, actor)) {
			continue;
		}

		if (hasFlag(_state.actorMovementFlags, actor)) {
			advanceDynamicActorMovement(actor);
		} else if (hasFlag(_state.actorPathActiveFlags, actor)) {
			updateStandardActorPath(actor);
		}

		if (getIntegerCoordinate(_state.actorXFixedCoordinates[actor]) == _state.actorTargetXCoordinates[actor] &&
			getIntegerCoordinate(_state.actorYFixedCoordinates[actor]) == _state.actorTargetYCoordinates[actor]) {
			_state.actorMovementFlags &= static_cast<uint8>(~actorMask(actor));
		}

		if (!hasFlag(_state.actorPriorityUpdateBlockFlags, actor)) {
			updateActorPriority(actor);
		}
	}
}

void RoomActorUpdater::advanceDynamicActorMovement(int actor) {
	int movementIndex = actor - 2;
	int16 targetX = _state.actorTargetXCoordinates[actor];
	int16 actorX = getIntegerCoordinate(_state.actorXFixedCoordinates[actor]);
	if (actorX != targetX) {
		int32 targetFixedX = composePackedTargetCoordinate(_state.actorTargetXCoordinates, actor,
														   static_cast<uint16>(_state.actorTargetYCoordinates[kShaggyActorSlot]));
		if (actorX < targetX) {
			_state.actorXFixedCoordinates[actor] =
				_state.actorXFixedCoordinates[actor] + _state.roomObjectActorHorizontalSteps[movementIndex];
			if (_state.actorXFixedCoordinates[actor] >= targetFixedX) {
				_state.actorXFixedCoordinates[actor] = targetFixedX;
			}
		} else {
			_state.actorXFixedCoordinates[actor] =
				_state.actorXFixedCoordinates[actor] - _state.roomObjectActorHorizontalSteps[movementIndex];
			if (_state.actorXFixedCoordinates[actor] <= targetFixedX) {
				_state.actorXFixedCoordinates[actor] = targetFixedX;
			}
		}
	}

	int16 targetY = _state.actorTargetYCoordinates[actor];
	int16 actorY = getIntegerCoordinate(_state.actorYFixedCoordinates[actor]);
	if (actorY == targetY) {
		return;
	}

	int32 targetFixedY = composePackedTargetCoordinate(_state.actorTargetYCoordinates, actor,
													   static_cast<uint16>(static_cast<
																			   uint32>(
																			   _state.actorPathStreamOffsets[kShaggyActorSlot]) >>
																		   16));
	if (actorY < targetY) {
		_state.actorYFixedCoordinates[actor] =
			_state.actorYFixedCoordinates[actor] + _state.roomObjectActorVerticalSteps[movementIndex];
		if (_state.actorYFixedCoordinates[actor] >= targetFixedY) {
			_state.actorYFixedCoordinates[actor] = targetFixedY;
		}
	} else {
		_state.actorYFixedCoordinates[actor] =
			_state.actorYFixedCoordinates[actor] - _state.roomObjectActorVerticalSteps[movementIndex];
		if (_state.actorYFixedCoordinates[actor] <= targetFixedY) {
			_state.actorYFixedCoordinates[actor] = targetFixedY;
		}
	}
}

void RoomActorUpdater::updateStandardActorPath(int actor) {
	_state.actorPathCadenceCountdowns[actor]--;
	if (_state.actorPathCadenceCountdowns[actor] >= 0) {
		return;
	}

	_state.actorPathCadenceCountdowns[actor] = static_cast<int8>(_state.actorPathCadenceReloads[actor]);
	while (true) {
		Common::Point record = readAndAdvancePathRecord(actor);
		int16 x = record.x;
		int16 y = record.y;
		if (x >= 0) {
			setActorIntegerCoordinates(actor, x, y);
			return;
		}

		if (x == -3) {
			requestActorAnimation(actor, y);
			continue;
		}

		_state.actorPathRecordOffsets[actor] = 0;
		if (x == -2) {
			clearActorPath(actor);
			return;
		}

		if (hasFlag(_state.actorAnimationLoopExitFlags, actor)) {
			clearActorPath(actor);
			return;
		}
	}
}

void RoomActorUpdater::endActorPathAtTerminal(int actor) {
	if (!hasFlag(_state.actorPathActiveFlags, actor)) {
		return;
	}

	int32 streamOffset = _state.actorPathStreamOffsets[actor];
	int16 previousRecordOffset = static_cast<int16>(_state.actorPathRecordOffsets[actor] - 4);
	int16 previousX = _rom.readInt16(streamOffset + previousRecordOffset);
	if (previousX == -2) {
		clearActorPath(actor);
	} else if (previousX == -1) {
		if (hasFlag(_state.actorAnimationLoopExitFlags, actor)) {
			clearActorPath(actor);
		}
	}
}

Common::Point RoomActorUpdater::readAndAdvancePathRecord(int actor) {
	int16 recordOffset = static_cast<int16>(_state.actorPathRecordOffsets[actor]);
	_state.actorPathRecordOffsets[actor] += 4;
	int32 sourceOffset = _state.actorPathStreamOffsets[actor] + recordOffset;
	return Common::Point(_rom.readInt16(sourceOffset), _rom.readInt16(sourceOffset + static_cast<int>(sizeof(int16))));
}

void RoomActorUpdater::clearActorPath(int actor) {
	uint8 actorMask = this->actorMask(actor);
	_state.actorPathActiveFlags &= static_cast<uint8>(~actorMask);
	_state.actorAnimationLoopExitFlags &= static_cast<uint8>(~actorMask);
}

void RoomActorUpdater::updateActorPriority(int actor) {
	int x = static_cast<uint16>(getIntegerCoordinate(_state.actorXFixedCoordinates[actor])) >> 3;
	int y = static_cast<uint16>(getIntegerCoordinate(_state.actorYFixedCoordinates[actor])) >> 3;
	int16 byteOffset = static_cast<int16>(
		y * _state.roomWidthTiles * static_cast<int>(sizeof(int16)) +
		x * static_cast<int>(sizeof(int16)));
	SCOOBY_ASSERT(_state.activeRoomScene, "UpdateActorPriority requires an active room scene.");

	// The original reads past the decoded collision map into the shared 0xFFE000 workspace when an actor
	// stands beyond the room's right or bottom edge (for example, while a script walks it off-screen).
	// Those stale bytes have no authored meaning, so leave the actor's current priority unchanged instead
	// of indexing outside the managed map.
	const Common::Array<uint8> &collisionCells = _state.activeRoomScene->collisionCells;
	if (byteOffset < 0 ||
		static_cast<size_t>(byteOffset) + sizeof(uint16) > collisionCells.size()) {
		return;
	}

	int16 collisionCell = static_cast<int16>(readUInt16BigEndian(collisionCells, byteOffset));
	uint8 actorMask = this->actorMask(actor);
	if (collisionCell < 0) {
		_state.actorHighPriorityFlags |= actorMask;
	} else {
		_state.actorHighPriorityFlags &= static_cast<uint8>(~actorMask);
	}
}

void RoomActorUpdater::requestActorAnimation(int actor, int16 animationOffset) {
	_state.actorAnimationOffsets[actor] = animationOffset;
	_state.actorAnimationRestartFlags |= actorMask(actor);
}

void RoomActorUpdater::setActorIntegerCoordinates(int actor, int16 x, int16 y) {
	_state.actorXFixedCoordinates[actor] = replaceIntegerCoordinate(_state.actorXFixedCoordinates[actor], x);
	_state.actorYFixedCoordinates[actor] = replaceIntegerCoordinate(_state.actorYFixedCoordinates[actor], y);
}

bool RoomActorUpdater::isNewPress(ControllerButtons button) const {
	return _state.isButtonDown(button) && (_state.previousControllerOneInput & static_cast<uint8>(button)) != 0;
}

int32 RoomActorUpdater::scaleMovementStep(uint16 scale, uint16 multiplier,
										  int16 movementStep) {
	uint32 magnitude = static_cast<uint32>(static_cast<uint16>(0x100 - scale)) *
					   multiplier;
	return static_cast<int16>(magnitude >> 8) * movementStep;
}

uint8 RoomActorUpdater::actorMask(int actor) {
	return static_cast<uint8>(1 << actor);
}

int32 RoomActorUpdater::composePackedTargetCoordinate(const Common::Array<int16> &targets,
													  int actor,
													  uint16 followingWord) {
	uint16 fractionalWord = actor + 1 < static_cast<int>(targets.size())
								? static_cast<uint16>(targets[actor + 1])
								: followingWord;
	return static_cast<int32>((static_cast<uint32>(static_cast<uint16>(targets[actor])) << 16) |
							  fractionalWord);
}

bool RoomActorUpdater::hasFlag(uint8 flags, int actor) {
	return (flags & actorMask(actor)) != 0;
}

int16 RoomActorUpdater::getIntegerCoordinate(int32 fixedCoordinate) {
	return static_cast<int16>(fixedCoordinate >> 16);
}

int32 RoomActorUpdater::replaceIntegerCoordinate(int32 fixedCoordinate,
												 int16 integerCoordinate) {
	return (fixedCoordinate & 0xFFFF) | (integerCoordinate << 16);
}

int16 RoomActorUpdater::getControllerDirectionIndex(int16 retainedDataRegister) {
	// Ghidra 0x0000750A-0x00007539: Left wins over every simultaneous Right input, then Up wins over Down.
	if (_state.isButtonDown(ControllerButtons::Left)) {
		if (_state.isButtonDown(ControllerButtons::Up)) {
			return 1;
		}

		if (_state.isButtonDown(ControllerButtons::Down)) {
			return 3;
		}

		return 4;
	}

	// Ghidra 0x0000753A-0x00007569: with Left released, preserve Right+Up, Right+Down, and Right-only.
	if (_state.isButtonDown(ControllerButtons::Right)) {
		if (_state.isButtonDown(ControllerButtons::Up)) {
			return 2;
		}

		if (_state.isButtonDown(ControllerButtons::Down)) {
			return 0;
		}

		return 5;
	}

	// Ghidra 0x0000756A-0x00007589: retain independent Up and Down results and the untouched incoming-D0
	// return when all four direction bits are released.
	if (_state.isButtonDown(ControllerButtons::Up)) {
		return 6;
	}

	if (_state.isButtonDown(ControllerButtons::Down)) {
		return 7;
	}

	return retainedDataRegister;
}
} // namespace Scooby
