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

#include "room_actor_updater.h"

#include "common/array.h"

#include "companion_actor_position_transition_animation_catalog.h"
#include "room_collision_probe_result.h"
#include "room_object.h"
#include "room_scene_data.h"
#include "scooby/common/contracts.h"
#include "scooby/common/span.h"
#include "scooby/movement/actor_directional_animation_catalog.h"

namespace Scooby {
namespace {
void WriteUInt16BigEndian(Span<uint8> bytes, int offset, uint16 value) {
	bytes[static_cast<std::size_t>(offset)] = static_cast<uint8>(value >> 8);
	bytes[static_cast<std::size_t>(offset + 1)] = static_cast<uint8>(value);
}

inline uint16 ReadBigEndian16(const Common::Array<uint8> &bytes, int offset) {
	std::size_t o = static_cast<std::size_t>(offset);
	return static_cast<uint16>((uint16(bytes[o]) << 8) | uint16(bytes[o + 1]));
}
} // namespace

void RoomActorUpdater::updateRoomActorsAndInterface() {
	// Ghidra 0x00005E4A-0x00005EE9: service lead-actor path cadence and every path control record.
	updateLeadActorPath();

	// Ghidra 0x00005EEA-0x0000600B: service the signed interface transition and both cell-table transfers.
	// Ghidra 0x0000600C-0x0000632B: retain path termination, cursor movement, menu selection, and interface
	// close branches. Some suppression paths deliberately bypass the complete lead-actor block.
	bool skipLeadActor = updateInterfaceTransitionAndCursor();

	// Ghidra 0x0000632C-0x00006D23: update lead movement, camera following, priority, and interface entry.
	if (!skipLeadActor) {
		updateLeadActorAndCamera();
	}

	// Ghidra 0x00006D24-0x0000707D: update companion copy, path, ambient, and queued movement branches.
	updateCompanionActor();

	// Ghidra 0x0000707E-0x00007239: sample companion priority, service the paired transition layout,
	// interpolate compact scales, and propagate lead scale to the selected transition actor.
	updateActorPrioritiesAndScales();

	// Ghidra 0x0000723A-0x000073D5: service active actor slots two through five without omitting either
	// the path, movement, stationary, transition-skip, or priority-inhibition branches.
	updateDynamicActors();
}

void RoomActorUpdater::updateLeadActorPath() {
	if (!hasFlag(_state.ActorPathActiveFlags, kLeadActorSlot)) {
		return;
	}

	_state.ActorPathCadenceCountdowns[kLeadActorSlot] = static_cast<int8>(
		_state.ActorPathCadenceCountdowns[kLeadActorSlot] - 1);
	if (_state.ActorPathCadenceCountdowns[kLeadActorSlot] >= 0) {
		return;
	}

	_state.ActorPathCadenceCountdowns[kLeadActorSlot] = static_cast<int8>(
		_state.ActorPathCadenceReloads[kLeadActorSlot]);
	while (true) {
		PathRecord record = readAndAdvancePathRecord(kLeadActorSlot);
		int16 x = record.X;
		int16 y = record.Y;
		if (x >= 0) {
			setActorIntegerCoordinates(kLeadActorSlot, x, y);
			return;
		}

		if (x == -3) {
			requestActorAnimation(kLeadActorSlot, y);
			continue;
		}

		if (x == -4 || x == -5) {
			uint16 scale = static_cast<uint16>(y);
			if (_state.ActorHorizontalScaleFactors[kLeadActorSlot] != scale) {
				_state.ActorHorizontalScaleFactors[kLeadActorSlot] = scale;
				_state.ActorVerticalScaleFactors[kLeadActorSlot] = scale;
				_state.ActorPositionUpdateFlags |= kLeadActorMask;
			}

			if (x == -5) {
				continue;
			}

			return;
		}

		if (x == -2) {
			return;
		}

		_state.ActorPathRecordOffsets[kLeadActorSlot] = 0;
	}
}

bool RoomActorUpdater::updateInterfaceTransitionAndCursor() {
	if (_state.RoomInterfaceTransitionCountdown < 0) {
		if ((_state.InteractionFlags & kInteractionMenuSuppressionMask) != 0) {
			return (_state.ActorMovementFlags & kLeadActorMask) == 0;
		}

		if ((_state.ProgressStateBytes[0] & kProgressInterfaceSuppressionMask) != 0) {
			return false;
		}

		if (isNewPress(kInterfaceTransitionButtonMask)) {
			publishLowerInterfaceCells(kLowerInterfaceClosingCellsOffset);
			_state.RoomInterfaceTransitionCountdown = 0x10;
		}
	}

	if (_state.RoomInterfaceTransitionCountdown >= 0) {
		advanceRoomInterfaceTransition();
	}

	if ((_state.DisplayFlags & kDisplayInterfaceMask) == 0) {
		return false;
	}

	endActorPathAtTerminal(kLeadActorSlot);
	if (isNewPress(kNewInterfaceButtonMask)) {
		closeInteractionInterface();
		return true;
	}

	updateInteractionCursor();
	return true;
}

void RoomActorUpdater::advanceRoomInterfaceTransition() {
	if ((_state.DisplayFlags & kDisplayRightInterfaceMask) != 0) {
		_state.RoomInterfaceHorizontalScroll = static_cast<int16>(
			_state.RoomInterfaceHorizontalScroll + 0x10);
	} else {
		_state.RoomInterfaceHorizontalScroll = static_cast<int16>(
			_state.RoomInterfaceHorizontalScroll - 0x10);
	}

	_state.RoomInterfaceTransitionCountdown = static_cast<int8>(
		_state.RoomInterfaceTransitionCountdown - 1);
	if (_state.RoomInterfaceTransitionCountdown != 0) {
		return;
	}

	if ((_state.InteractionFlags & kInteractionMenuSuppressionMask) == 0) {
		if ((_state.ProgressStateBytes[0] & kProgressInterfaceSuppressionMask) == 0) {
			publishLowerInterfaceCells(kLowerInterfaceOpeningCellsOffset);
		}
	}

	_state.RoomInterfaceTransitionCountdown = -1;
	_state.DisplayFlags ^= kDisplayRightInterfaceMask;
}

void RoomActorUpdater::publishLowerInterfaceCells(int sourceOffset) {
	Common::Array<uint8> packedRow(kLowerInterfaceColumnCount * sizeof(uint16));
	Span<uint8> packedRowSpan = MakeSpan(packedRow);
	for (int row = 0; row < kLowerInterfaceRowCount; row++) {
		for (int column = 0; column < kLowerInterfaceColumnCount; column++) {
			uint16 packedCell = static_cast<uint16>(
				_rom.readUInt16(sourceOffset + row * kLowerInterfaceSourceRowByteStride +
								column * static_cast<int>(sizeof(uint16))) +
				_state.ActionPromptTileAttributes);
			WriteUInt16BigEndian(packedRowSpan, column * static_cast<int>(sizeof(uint16)), packedCell);
		}

		_scene.loadLayerRows(packedRowSpan, TileLayer::Interface, kLowerInterfaceStartColumn,
							 kLowerInterfaceStartRow + row, kLowerInterfaceColumnCount, 1);
	}
}

void RoomActorUpdater::updateInteractionCursor() {
	if (_state.CursorY >= 0xA8) {
		_state.CursorVerticalStep = 3;
		if ((_state.PreviousControllerOneInput & kCursorLeftMask) == 0) {
			_state.CursorHorizontalStep = 2;
		} else if ((_state.PreviousControllerOneInput & kCursorRightMask) == 0) {
			_state.CursorHorizontalStep = 2;
		} else {
			_state.CursorHorizontalStep = 0x28;
		}
	} else {
		_state.CursorHorizontalStep = 2;
		_state.CursorVerticalStep = 2;
	}

	if ((_state.DisplayFlags & kDisplayTransitionMask) == 0) {
		if ((_state.ActorVisibilityBlockFlags & kLeadActorMask) == 0) {
			if ((_state.RoomBehaviorFlags & kRoomBehaviorDuelMask) == 0) {
				if ((_state.TransitionFlags & kTransitionHideLeadMask) == 0) {
					if ((_state.TransitionFlags & kTransitionCenteredCursorMask) == 0) {
						requestActorAnimation(kLeadActorSlot, static_cast<int16>(
																  _state.ActorPositionIndices[kLeadActorSlot] + 4));
					}
				}
			}
		}
	}

	if ((_state.ControllerOneInput & kCursorLeftMask) == 0) {
		_state.CursorX = static_cast<int16>(_state.CursorX - _state.CursorHorizontalStep);
		if (_state.CursorX < -8) {
			_state.CursorX = -8;
		}
	}

	if ((_state.ControllerOneInput & kCursorRightMask) == 0) {
		_state.CursorX = static_cast<int16>(_state.CursorX + _state.CursorHorizontalStep);
		if (_state.CursorX > 0xF7) {
			_state.CursorX = 0xF7;
		}
	}

	if ((_state.ControllerOneInput & kCursorUpMask) == 0) {
		if (_state.CursorY >= 0xC0) {
			_state.CursorY = 0xBF;
		} else {
			_state.CursorY = static_cast<int16>(_state.CursorY - _state.CursorVerticalStep);
			if (_state.CursorY < 0) {
				_state.CursorY = 0;
			}
		}
	}

	if ((_state.ControllerOneInput & kCursorDownMask) == 0) {
		if (_state.CursorY >= 0xA8) {
			if ((_state.PreviousControllerOneInput & kCursorDownMask) != 0) {
				_state.CursorY = 0xC0;
			}
		}

		_state.CursorY = static_cast<int16>(_state.CursorY + _state.CursorVerticalStep);
		if (_state.CursorY > 0xCF) {
			_state.CursorY = 0xCF;
		}
	}

	if (_state.CursorY >= 0xA8) {
		updateMenuCommandFromCursor();
		if ((_state.DisplayFlags & kDisplayRightInterfaceMask) == 0) {
			return;
		}

		if (_state.MenuCommand == 5 || _state.MenuCommand == 10) {
			return;
		}
	}

	if (!isNewPress(kActionButtonMask)) {
		return;
	}

	_state.InteractionMode = _state.ActiveActionIcon;
	if (_state.InteractionMode == 0) {
		_state.InteractionMode = _state.CachedActionIcon;
		if (_state.InteractionMode == 0) {
			return;
		}
	}

	if (_state.CurrentInteraction != 0) {
		_state.DisplayFlags |= kDisplayTransitionMask;
		return;
	}

	if ((_state.DisplayFlags & kDisplayTransitionMask) == 0) {
		closeInteractionInterface();
	}
}

void RoomActorUpdater::updateMenuCommandFromCursor() {
	int16 command = 1;
	int16 remainingX = _state.CursorX;
	if ((_state.DisplayFlags & kDisplayRightInterfaceMask) == 0) {
		remainingX = static_cast<int16>(remainingX - 8);
	}

	do {
		remainingX = static_cast<int16>(remainingX - 0x28);
		if (remainingX >= 0) {
			command = static_cast<int16>(command + 1);
		}
	} while (remainingX >= 0);

	if (command > 6) {
		command = 6;
	}

	if (_state.CursorY > 0xC0) {
		command = static_cast<int16>(command + 6);
	}

	if ((_state.DisplayFlags & kDisplayRightInterfaceMask) == 0) {
		if (command > 5) {
			command = static_cast<int16>(command - 1);
			if (command > 10) {
				command = static_cast<int16>(command - 1);
			}
		}
	} else {
		if (command > 7) {
			command = static_cast<int16>(command - 1);
		}

		command = static_cast<int16>(command - 1);
	}

	_state.MenuCommand = command;
}

void RoomActorUpdater::closeInteractionInterface() {
	_state.DisplayFlags &= static_cast<uint8>(~kDisplayInterfaceMask);
	_state.PendingActionIcon = _state.ActiveActionIcon;
	_drawSelectedActionIcon(0);
	if (_state.CurrentInteraction == 0) {
		return;
	}

	const RoomObject &roomObject = _state.RoomObjects[_state.CurrentInteraction - kRoomObjectFirstInteraction];
	_state.PendingActionIcon = roomObject.ActionIcon;
	_drawSelectedActionIcon(0);
}

void RoomActorUpdater::updateLeadActorAndCamera() {
	int16 maximumX = static_cast<int16>(_state.RoomWidthTiles * 8 - 1);
	int16 maximumY = static_cast<int16>(_state.RoomHeightTiles * 8 - 1);
	if (hasFlag(_state.ActorPathActiveFlags, kLeadActorSlot)) {
		endActorPathAtTerminal(kLeadActorSlot);
		updateCameraAndInteractionEntry(maximumX, maximumY);
		return;
	}

	if ((_state.RoomBehaviorFlags & kRoomBehaviorDuelMask) != 0) {
		if ((_state.DuelStateFlags & 0x01) != 0) {
			updateCameraAndInteractionEntry(maximumX, maximumY);
		} else {
			tryOpenInteractionInterface();
		}

		return;
	}

	if ((_state.ActorVisibilityBlockFlags & kLeadActorMask) != 0) {
		return;
	}

	if ((_state.TransitionFlags & kTransitionHideLeadMask) != 0) {
		tryOpenInteractionInterface();
		return;
	}

	if ((_state.DisplayFlags & kDisplayTransitionMask) != 0 && (_state.ActorMovementFlags & kLeadActorMask) ==
																   0) {
		return;
	}

	int candidateX = _state.ActorXFixedCoordinates[kLeadActorSlot];
	int candidateY = _state.ActorYFixedCoordinates[kLeadActorSlot];
	uint32 stepMagnitude = static_cast<uint32>(static_cast<uint16>(
							   0x100 - _state.ActorHorizontalScaleFactors[kLeadActorSlot])) *
						   0x155u;
	if ((_state.ActorMovementFlags & kLeadActorMask) == 0) {
		_state.LeadActorHorizontalMovementStep = 0x100;
		_state.LeadActorVerticalMovementStep = 0x100;
	}

	if ((_state.DisplayFlags & kDisplayTransitionMask) != 0) {
		advanceLeadScriptedMovement();
	} else {
		bool advancedScriptedMovement = updateLeadControlledCandidate(candidateX, candidateY, stepMagnitude,
																	  maximumX, maximumY);
		if (!advancedScriptedMovement) {
			_state.ActorMovementFlags &= static_cast<uint8>(~kLeadActorMask);
			RoomCollisionProbeResult probe = _collisionProbe.probeLeadActorCollision(candidateX, candidateY,
																					 maximumX, maximumY);
			if (probe.IsClear) {
				_state.ActorXFixedCoordinates[kLeadActorSlot] = candidateX;
				_state.ActorYFixedCoordinates[kLeadActorSlot] = candidateY;
			} else {
				adjustLeadMovementAroundCollision(candidateX, candidateY, maximumX, maximumY,
												  probe.RetainedDataRegister);
			}
		}
	}

	updateActorPriority(kLeadActorSlot);
	updateCameraAndInteractionEntry(maximumX, maximumY);
}

bool RoomActorUpdater::updateLeadControlledCandidate(int &candidateX, int &candidateY,
													 uint32 stepMagnitude,
													 int16 maximumX, int16 maximumY) {
	bool noDirection = (_state.ControllerOneInput & kDirectionMask) == kDirectionMask;
	if (!noDirection) {
		_state.LeadActorIdleAnimationTimer = 300;
	}

	if (noDirection) {
		if ((_state.LeadActorDirectionalAnimationLatchFlags & 0x02) != 0) {
			moveLeadLeft(candidateX, stepMagnitude);
		} else if ((_state.LeadActorDirectionalAnimationLatchFlags & 0x01) != 0) {
			moveLeadRight(candidateX, stepMagnitude, maximumX);
		} else {
			uint32 verticalStep = stepMagnitude >> 1;
			if ((_state.LeadActorDirectionalAnimationLatchFlags & 0x04) != 0) {
				moveLeadUp(candidateY, verticalStep, maximumY, false);
			} else if ((_state.LeadActorDirectionalAnimationLatchFlags & 0x08) != 0) {
				moveLeadDown(candidateY, verticalStep, maximumY, false);
			} else {
				if ((_state.ActorMovementFlags & kLeadActorMask) != 0) {
					advanceLeadScriptedMovement();
					return true;
				}

				updateLeadIdleAnimation();
			}

			return false;
		}
	} else if ((_state.ControllerOneInput & kCursorLeftMask) == 0) {
		moveLeadLeft(candidateX, stepMagnitude);
	} else {
		if ((_state.LeadActorDirectionalAnimationLatchFlags & 0x02) != 0 &&
			_state.ActorAnimationOffsets[kLeadActorSlot] <= 3) {
			_state.LeadActorDirectionalAnimationLatchFlags &= 0xFD;
		}

		if ((_state.ControllerOneInput & kCursorRightMask) == 0) {
			moveLeadRight(candidateX, stepMagnitude, maximumX);
		}
	}

	if ((_state.LeadActorDirectionalAnimationLatchFlags & 0x01) != 0 &&
		_state.ActorAnimationOffsets[kLeadActorSlot] <= 3) {
		_state.LeadActorDirectionalAnimationLatchFlags &= 0xFE;
	}

	if ((_state.LeadActorDirectionalAnimationLatchFlags & 0x03) != 0) {
		return false;
	}

	uint32 verticalMagnitude = stepMagnitude >> 1;
	if ((_state.ControllerOneInput & kCursorUpMask) == 0) {
		bool horizontalDirectionHeld = (_state.ControllerOneInput & kCursorLeftMask) == 0 ||
									   (_state.ControllerOneInput & kCursorRightMask) == 0;
		bool graphicsPending = moveLeadUp(candidateY, verticalMagnitude, maximumY, horizontalDirectionHeld);
		if (!graphicsPending) {
			return false;
		}
	}

	if ((_state.ControllerOneInput & kCursorDownMask) == 0) {
		bool horizontalDirectionHeld = (_state.ControllerOneInput & kCursorLeftMask) == 0 ||
									   (_state.ControllerOneInput & kCursorRightMask) == 0;
		moveLeadDown(candidateY, verticalMagnitude, maximumY, horizontalDirectionHeld);
	}

	return false;
}

void RoomActorUpdater::moveLeadLeft(int &candidateX, uint32 stepMagnitude) {
	uint32 movementMagnitude;
	if (_state.ActorAnimationOffsets[kLeadActorSlot] == 1) {
		_state.ActorPositionIndices[kLeadActorSlot] = 1;
		if ((_state.ActorGraphicsReadyFlags & kActorGraphicsReadyMask) == 0) {
			return;
		}

		_state.LeadActorDirectionalAnimationLatchFlags &= 0xFD;
		movementMagnitude = stepMagnitude;
	} else {
		movementMagnitude = stepMagnitude >> 1;
		int16 animationOffset = ResolveLead(
			_state.ActorPositionIndices[kLeadActorSlot], 1);
		if (_state.ActorAnimationOffsets[kLeadActorSlot] != animationOffset) {
			requestActorAnimation(kLeadActorSlot, animationOffset);
			_state.LeadActorDirectionalAnimationLatchFlags |= 0x02;
			return;
		}
	}

	candidateX = candidateX - static_cast<int>(movementMagnitude);
	if (candidateX < 0) {
		candidateX = 0;
	}
}

void RoomActorUpdater::moveLeadRight(int &candidateX, uint32 stepMagnitude, int16 maximumX) {
	uint32 movementMagnitude;
	if (_state.ActorAnimationOffsets[kLeadActorSlot] == 0) {
		_state.ActorPositionIndices[kLeadActorSlot] = 0;
		if ((_state.ActorGraphicsReadyFlags & kActorGraphicsReadyMask) == 0) {
			return;
		}

		_state.LeadActorDirectionalAnimationLatchFlags &= 0xFE;
		movementMagnitude = stepMagnitude;
	} else {
		movementMagnitude = stepMagnitude >> 1;
		int16 animationOffset = ResolveLead(
			_state.ActorPositionIndices[kLeadActorSlot], 0);
		if (_state.ActorAnimationOffsets[kLeadActorSlot] != animationOffset) {
			requestActorAnimation(kLeadActorSlot, animationOffset);
			_state.LeadActorDirectionalAnimationLatchFlags |= 0x01;
			return;
		}
	}

	candidateX = candidateX + static_cast<int>(movementMagnitude);
	if (getIntegerCoordinate(candidateX) > maximumX) {
		candidateX = replaceIntegerCoordinate(candidateX, maximumX);
	}
}

bool RoomActorUpdater::moveLeadUp(int &candidateY, uint32 movementMagnitude, int16 maximumY,
								  bool horizontalDirectionHeld) {
	if (horizontalDirectionHeld || _state.ActorAnimationOffsets[kLeadActorSlot] == 2) {
		_state.ActorPositionIndices[kLeadActorSlot] = 2;
		if ((_state.ActorGraphicsReadyFlags & kActorGraphicsReadyMask) == 0) {
			return true;
		}

		_state.LeadActorDirectionalAnimationLatchFlags &= 0xF3;
	} else {
		int16 animationOffset = ResolveLead(
			_state.ActorPositionIndices[kLeadActorSlot], 2);
		movementMagnitude >>= 1;
		if (_state.ActorAnimationOffsets[kLeadActorSlot] != animationOffset) {
			requestActorAnimation(kLeadActorSlot, animationOffset);
			_state.LeadActorDirectionalAnimationLatchFlags |= 0x04;
			return false;
		}
	}

	if ((_state.TransitionFlags & kTransitionForegroundPolarityMask) != 0) {
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

void RoomActorUpdater::moveLeadDown(int &candidateY, uint32 movementMagnitude, int16 maximumY,
									bool horizontalDirectionHeld) {
	if (horizontalDirectionHeld || _state.ActorAnimationOffsets[kLeadActorSlot] == 3) {
		_state.ActorPositionIndices[kLeadActorSlot] = 3;
		if ((_state.ActorGraphicsReadyFlags & kActorGraphicsReadyMask) == 0) {
			return;
		}

		_state.LeadActorDirectionalAnimationLatchFlags &= 0xF3;
	} else {
		int16 animationOffset = ResolveLead(
			_state.ActorPositionIndices[kLeadActorSlot], 3);
		movementMagnitude >>= 1;
		if (_state.ActorAnimationOffsets[kLeadActorSlot] != animationOffset) {
			requestActorAnimation(kLeadActorSlot, animationOffset);
			_state.LeadActorDirectionalAnimationLatchFlags |= 0x08;
			return;
		}
	}

	if ((_state.TransitionFlags & kTransitionForegroundPolarityMask) != 0) {
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

void RoomActorUpdater::updateLeadIdleAnimation() {
	_state.LeadActorIdleAnimationTimer = static_cast<int16>(_state.LeadActorIdleAnimationTimer - 1);
	if (_state.LeadActorIdleAnimationTimer < 0) {
		_state.LeadActorIdleAnimationTimer = 300;
		requestActorAnimation(kLeadActorSlot, static_cast<int16>(
												  _state.ActorPositionIndices[kLeadActorSlot] + 0x20));
		return;
	}

	int16 animationOffset = _state.ActorAnimationOffsets[kLeadActorSlot];
	int16 desiredOffset = static_cast<int16>(_state.ActorPositionIndices[kLeadActorSlot] + 4);
	if (animationOffset < 0x20 || animationOffset >= 0x24 ||
		(_state.ActorAnimationHoldFlags & kActorAnimationHoldMask) != 0) {
		if (animationOffset != desiredOffset && (_state.ActorAnimationRestartFlags & kLeadActorMask) == 0) {
			requestActorAnimation(kLeadActorSlot, desiredOffset);
		}
	}
}

void RoomActorUpdater::adjustLeadMovementAroundCollision(int &candidateX, int &candidateY,
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
			if (_collisionProbe.probeLeadActorCollision(candidateX, candidateY, maximumX, maximumY).IsClear) {
				beginLeadVerticalCollisionAdjustment(candidateX, candidateY, movementStep);
				return;
			}
		}

		candidateY = replaceIntegerCoordinate(candidateY, static_cast<int16>(
															  getIntegerCoordinate(candidateY) + 4));
		movementStep = 0x100;
		for (int attempt = 0; attempt < 4; attempt++) {
			candidateY = replaceIntegerCoordinate(candidateY, static_cast<int16>(
																  getIntegerCoordinate(candidateY) + 1));
			if (_collisionProbe.probeLeadActorCollision(candidateX, candidateY, maximumX, maximumY).IsClear) {
				beginLeadVerticalCollisionAdjustment(candidateX, candidateY, movementStep);
				return;
			}
		}

		return;
	}

	int16 horizontalStep = -0x100;
	for (int attempt = 0; attempt < 3; attempt++) {
		candidateX = replaceIntegerCoordinate(candidateX, static_cast<int16>(
															  getIntegerCoordinate(candidateX) - 1));
		if (_collisionProbe.probeLeadActorCollision(candidateX, candidateY, maximumX, maximumY).IsClear) {
			beginLeadHorizontalCollisionAdjustment(candidateX, candidateY, horizontalStep);
			return;
		}
	}

	candidateX = replaceIntegerCoordinate(candidateX, static_cast<int16>(
														  getIntegerCoordinate(candidateX) + 3));
	horizontalStep = 0x100;
	for (int attempt = 0; attempt < 3; attempt++) {
		candidateX = replaceIntegerCoordinate(candidateX, static_cast<int16>(
															  getIntegerCoordinate(candidateX) + 1));
		if (_collisionProbe.probeLeadActorCollision(candidateX, candidateY, maximumX, maximumY).IsClear) {
			beginLeadHorizontalCollisionAdjustment(candidateX, candidateY, horizontalStep);
			return;
		}
	}
}

void RoomActorUpdater::beginLeadVerticalCollisionAdjustment(int candidateX, int candidateY,
															int16 movementStep) {
	_state.LeadActorVerticalMovementStep = movementStep;
	_state.LeadActorHorizontalMovementStep = 0;
	_state.ActorXFixedCoordinates[kLeadActorSlot] = candidateX;
	_state.ActorTargetXCoordinates[kLeadActorSlot] = getIntegerCoordinate(candidateX);
	_state.ActorMovementFlags |= kLeadActorMask;
	_state.ActorTargetYCoordinates[kLeadActorSlot] = getIntegerCoordinate(candidateY);
	advanceLeadScriptedMovement();
}

void RoomActorUpdater::beginLeadHorizontalCollisionAdjustment(int candidateX, int candidateY,
															  int16 movementStep) {
	_state.LeadActorHorizontalMovementStep = movementStep;
	_state.LeadActorVerticalMovementStep = 0;
	_state.ActorTargetXCoordinates[kLeadActorSlot] = getIntegerCoordinate(candidateX);
	_state.ActorMovementFlags |= kLeadActorMask;
	_state.ActorYFixedCoordinates[kLeadActorSlot] = candidateY;
	_state.ActorTargetYCoordinates[kLeadActorSlot] = getIntegerCoordinate(candidateY);
	advanceLeadScriptedMovement();
}

void RoomActorUpdater::advanceLeadScriptedMovement() {
	_state.LeadActorIdleAnimationTimer = 300;
	int32 horizontalDelta = scaleMovementStep(_state.ActorHorizontalScaleFactors[kLeadActorSlot], 0x155,
											  _state.LeadActorHorizontalMovementStep);
	int32 verticalDelta = scaleMovementStep(_state.ActorVerticalScaleFactors[kLeadActorSlot], 0xAA,
											_state.LeadActorVerticalMovementStep);
	_state.ActorXFixedCoordinates[kLeadActorSlot] = _state.ActorXFixedCoordinates[kLeadActorSlot] +
													horizontalDelta;
	_state.ActorYFixedCoordinates[kLeadActorSlot] = _state.ActorYFixedCoordinates[kLeadActorSlot] +
													verticalDelta;

	int16 actorX = getIntegerCoordinate(_state.ActorXFixedCoordinates[kLeadActorSlot]);
	int16 targetX = _state.ActorTargetXCoordinates[kLeadActorSlot];
	if (_state.LeadActorHorizontalMovementStep >= 0) {
		if (actorX >= targetX) {
			_state.ActorXFixedCoordinates[kLeadActorSlot] = replaceIntegerCoordinate(
				_state.ActorXFixedCoordinates[kLeadActorSlot], targetX);
			_state.LeadActorHorizontalMovementStep = 0;
		}
	} else if (actorX <= targetX) {
		_state.LeadActorHorizontalMovementStep = 0;
	}

	int16 actorY = getIntegerCoordinate(_state.ActorYFixedCoordinates[kLeadActorSlot]);
	int16 targetY = _state.ActorTargetYCoordinates[kLeadActorSlot];
	if (_state.LeadActorVerticalMovementStep >= 0) {
		if (actorY >= targetY) {
			_state.ActorYFixedCoordinates[kLeadActorSlot] = replaceIntegerCoordinate(
				_state.ActorYFixedCoordinates[kLeadActorSlot], targetY);
			_state.LeadActorVerticalMovementStep = 0;
		}
	} else if (actorY <= targetY) {
		_state.ActorYFixedCoordinates[kLeadActorSlot] = replaceIntegerCoordinate(
			_state.ActorYFixedCoordinates[kLeadActorSlot], targetY);
		_state.LeadActorVerticalMovementStep = 0;
	}

	if (_state.LeadActorHorizontalMovementStep != 0 || _state.LeadActorVerticalMovementStep != 0) {
		return;
	}

	if (_state.LeadActorMovementPointIndex >= 0) {
		_state.LeadActorMovementPointIndex = static_cast<int16>(_state.LeadActorMovementPointIndex - 1);
		if (_state.LeadActorMovementPointIndex >= 0) {
			beginNextLeadMovementPoint(_state.LeadActorMovementPoints[_state.LeadActorMovementPointIndex]);
			return;
		}
	}

	_state.ActorMovementFlags &= static_cast<uint8>(~kLeadActorMask);
	if ((_state.DisplayFlags & kDisplayTransitionMask) != 0) {
		requestActorAnimation(kLeadActorSlot, static_cast<int16>(
												  _state.ActorPositionIndices[kLeadActorSlot] + 4));
	}
}

void RoomActorUpdater::beginNextLeadMovementPoint(const ActorMovementPoint &point) {
	_state.ActorTargetXCoordinates[kLeadActorSlot] = point.TargetX;
	_state.ActorTargetYCoordinates[kLeadActorSlot] = point.TargetY;
	_state.LeadActorHorizontalMovementStep = point.HorizontalStep;
	_state.LeadActorVerticalMovementStep = point.VerticalStep;
	int16 relativeAnimation = static_cast<int16>(
		_state.ActorAnimationOffsets[kLeadActorSlot] - _state.ActorPositionIndices[kLeadActorSlot]);
	_state.ActorAnimationOffsets[kLeadActorSlot] = relativeAnimation == 0
													   ? ResolveLead(
															 _state.ActorPositionIndices[kLeadActorSlot],
															 point.PositionIndex)
													   : static_cast<int16>(point.PositionIndex +
																			relativeAnimation);
	_state.ActorPositionIndices[kLeadActorSlot] = point.PositionIndex;
	_state.ActorAnimationRestartFlags |= kLeadActorMask;
}

void RoomActorUpdater::updateCameraAndInteractionEntry(int16 maximumX, int16 maximumY) {
	if ((_state.RoomBehaviorFlags & kRoomBehaviorCameraLockMask) == 0) {
		updateCameraX(maximumX);
		updateCameraY(maximumY);
	}

	tryOpenInteractionInterface();
}

void RoomActorUpdater::updateCameraX(int16 maximumX) {
	int16 actorX = getIntegerCoordinate(_state.ActorXFixedCoordinates[kLeadActorSlot]);
	int16 relativeX = static_cast<int16>(actorX - _state.CameraX);
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

	uint16 previousTile = static_cast<uint16>(_state.CameraX) >> 3;
	_state.CameraX = nextCameraX;
	uint16 nextTile = static_cast<uint16>(nextCameraX) >> 3;
	if (previousTile == nextTile) {
		return;
	}

	_state.TileStreamingEdgeFlags |= previousTile < nextTile
										 ? static_cast<uint8>(0x01)
										 : static_cast<uint8>(0x02);
}

void RoomActorUpdater::updateCameraY(int16 maximumY) {
	int16 actorY = getIntegerCoordinate(_state.ActorYFixedCoordinates[kLeadActorSlot]);
	int16 relativeY = static_cast<int16>(actorY - _state.CameraY);
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

	uint16 previousTile = static_cast<uint16>(_state.CameraY) >> 3;
	_state.CameraY = nextCameraY;
	uint16 nextTile = static_cast<uint16>(nextCameraY) >> 3;
	if (previousTile == nextTile) {
		return;
	}

	_state.TileStreamingEdgeFlags |= previousTile < nextTile
										 ? static_cast<uint8>(0x08)
										 : static_cast<uint8>(0x04);
}

void RoomActorUpdater::tryOpenInteractionInterface() {
	if ((_state.DisplayFlags & kDisplayTransitionMask) != 0) {
		return;
	}

	if ((_state.ProgressStateBytes[0] & kProgressInterfaceSuppressionMask) != 0) {
		return;
	}

	bool shouldOpen = isNewPress(kNewInterfaceButtonMask);
	if (!shouldOpen) {
		shouldOpen = isNewPress(kActionButtonMask);
	}

	if (!shouldOpen) {
		return;
	}

	_state.DisplayFlags |= kDisplayInterfaceMask;
	_state.InteractionFlags &= 0xFE;
	_state.CursorHorizontalStep = 2;
	_state.CursorVerticalStep = 2;
	_state.ActiveActionIcon = 0;
	_state.PreviousInteraction = 0;
	_state.SecondaryInteraction = 0;

	if ((_state.TransitionFlags & kTransitionCenteredCursorMask) != 0) {
		setCenteredCursor();
		return;
	}

	if ((_state.ActorVisibilityBlockFlags & kLeadActorMask) == 0) {
		if ((_state.TransitionFlags & kTransitionHideLeadMask) != 0) {
			setCenteredCursor();
			return;
		}

		if ((_state.RoomBehaviorFlags & kRoomBehaviorDuelMask) == 0) {
			requestActorAnimation(kLeadActorSlot, static_cast<int16>(
													  _state.ActorPositionIndices[kLeadActorSlot] + 4));
		}
	}

	_state.CursorX = static_cast<int16>(
		getIntegerCoordinate(_state.ActorXFixedCoordinates[kLeadActorSlot]) - _state.CameraX);
	_state.CursorY = static_cast<int16>(
		getIntegerCoordinate(_state.ActorYFixedCoordinates[kLeadActorSlot]) - _state.CameraY - 0x2C);
}

void RoomActorUpdater::setCenteredCursor() {
	_state.CursorX = 0x80;
	_state.CursorY = 0x50;
}

void RoomActorUpdater::updateCompanionActor() {
	if ((_state.VideoFlags & kVideoCompanionUpdateBlockMask) != 0) {
		return;
	}

	if ((_state.VideoFlags & kVideoCompanionCopyMask) != 0) {
		_state.ActorXFixedCoordinates[kCompanionActorSlot] = replaceIntegerCoordinate(
			_state.ActorXFixedCoordinates[kCompanionActorSlot],
			getIntegerCoordinate(_state.ActorXFixedCoordinates[kLeadActorSlot]));
		_state.ActorYFixedCoordinates[kCompanionActorSlot] = replaceIntegerCoordinate(
			_state.ActorYFixedCoordinates[kCompanionActorSlot],
			getIntegerCoordinate(_state.ActorYFixedCoordinates[kLeadActorSlot]));
		int16 animationOffset = _state.ActorPositionIndices[kLeadActorSlot];
		if ((_state.ControllerOneInput & kDirectionMask) != kDirectionMask ||
			(_state.DisplayFlags & kDisplayInterfaceMask) != 0) {
			animationOffset = static_cast<int16>(animationOffset + 4);
		}

		if (_state.ActorAnimationOffsets[kCompanionActorSlot] != animationOffset) {
			requestActorAnimation(kCompanionActorSlot, animationOffset);
		}

		return;
	}

	if (hasFlag(_state.ActorPathActiveFlags, kCompanionActorSlot)) {
		updateStandardActorPath(kCompanionActorSlot);
		return;
	}

	if ((_state.ActorMovementFlags & kCompanionActorMask) == 0) {
		if (_state.RandomMoveTimer >= 0) {
			_state.RandomMoveTimer = static_cast<int16>(_state.RandomMoveTimer - 1);
		}

		return;
	}

	if ((_state.InteractionFlags & 0x20) != 0) {
		if ((_state.ActorAnimationRestartFlags & kCompanionActorMask) == 0) {
			if ((_state.ActorAnimationHoldFlags & kCompanionActorMask) != 0) {
				_state.ActorAnimationOffsets[kCompanionActorSlot] = static_cast<int16>(
					_state.RandomMoveAnimation + _state.RandomMoveDirection);
				_state.ActorPositionIndices[kCompanionActorSlot] = _state.RandomMoveDirection;
				_state.ActorAnimationDelays[kCompanionActorSlot] = -1;
				_state.ActorAnimationRestartFlags |= kCompanionActorMask;
				_state.ActorMovementFlags &= static_cast<uint8>(~kCompanionActorMask);
				_state.InteractionFlags &= 0xDF;
			}
		}

		return;
	}

	advanceCompanionScriptedMovement();
}

void RoomActorUpdater::advanceCompanionScriptedMovement() {
	int32 horizontalDelta = scaleMovementStep(_state.ActorHorizontalScaleFactors[kCompanionActorSlot],
											  0x12A,
											  _state.CompanionActorHorizontalMovementStep);
	int32 verticalDelta = scaleMovementStep(_state.ActorVerticalScaleFactors[kCompanionActorSlot], 0x95,
											_state.CompanionActorVerticalMovementStep);
	_state.ActorXFixedCoordinates[kCompanionActorSlot] =
		_state.ActorXFixedCoordinates[kCompanionActorSlot] + horizontalDelta;
	_state.ActorYFixedCoordinates[kCompanionActorSlot] =
		_state.ActorYFixedCoordinates[kCompanionActorSlot] + verticalDelta;

	int16 actorX = getIntegerCoordinate(_state.ActorXFixedCoordinates[kCompanionActorSlot]);
	int16 targetX = _state.ActorTargetXCoordinates[kCompanionActorSlot];
	if (_state.CompanionActorHorizontalMovementStep >= 0) {
		if (actorX >= targetX) {
			_state.ActorXFixedCoordinates[kCompanionActorSlot] = replaceIntegerCoordinate(
				_state.ActorXFixedCoordinates[kCompanionActorSlot], targetX);
			_state.CompanionActorHorizontalMovementStep = 0;
		}
	} else if (actorX <= targetX) {
		_state.CompanionActorHorizontalMovementStep = 0;
	}

	int16 actorY = getIntegerCoordinate(_state.ActorYFixedCoordinates[kCompanionActorSlot]);
	int16 targetY = _state.ActorTargetYCoordinates[kCompanionActorSlot];
	if (_state.CompanionActorVerticalMovementStep >= 0) {
		if (actorY >= targetY) {
			_state.ActorYFixedCoordinates[kCompanionActorSlot] = replaceIntegerCoordinate(
				_state.ActorYFixedCoordinates[kCompanionActorSlot], targetY);
			_state.CompanionActorVerticalMovementStep = 0;
		}
	} else if (actorY <= targetY) {
		_state.ActorYFixedCoordinates[kCompanionActorSlot] = replaceIntegerCoordinate(
			_state.ActorYFixedCoordinates[kCompanionActorSlot], targetY);
		_state.CompanionActorVerticalMovementStep = 0;
	}

	if (_state.CompanionActorHorizontalMovementStep != 0 || _state.CompanionActorVerticalMovementStep != 0) {
		return;
	}

	if (_state.CompanionActorMovementPointIndex >= 0) {
		_state.CompanionActorMovementPointIndex = static_cast<int16>(
			_state.CompanionActorMovementPointIndex - 1);
		if (_state.CompanionActorMovementPointIndex >= 0) {
			beginNextCompanionMovementPoint(
				_state.CompanionActorMovementPoints[_state.CompanionActorMovementPointIndex]);
			return;
		}
	}

	if ((_state.DisplayFlags & kDisplayTransitionMask) != 0) {
		requestActorAnimation(kCompanionActorSlot, static_cast<int16>(
													   _state.ActorPositionIndices[kCompanionActorSlot] + 4));
		_state.ActorMovementFlags &= static_cast<uint8>(~kCompanionActorMask);
		return;
	}

	_state.InteractionFlags |= 0x20;
	_state.ActorAnimationOffsets[kCompanionActorSlot] =
		ResolveCompanionPositionTransition(
			_state.ActorPositionIndices[kCompanionActorSlot], _state.RandomMoveDirection);
	_state.ActorAnimationDelays[kCompanionActorSlot] = -1;
	_state.ActorAnimationRestartFlags |= kCompanionActorMask;
}

void RoomActorUpdater::beginNextCompanionMovementPoint(const ActorMovementPoint &point) {
	_state.ActorTargetXCoordinates[kCompanionActorSlot] = point.TargetX;
	_state.ActorTargetYCoordinates[kCompanionActorSlot] = point.TargetY;
	_state.CompanionActorHorizontalMovementStep = point.HorizontalStep;
	_state.CompanionActorVerticalMovementStep = point.VerticalStep;
	int16 relativeAnimation = static_cast<int16>(
		_state.ActorAnimationOffsets[kCompanionActorSlot] - _state.ActorPositionIndices[kCompanionActorSlot]);
	_state.ActorAnimationOffsets[kCompanionActorSlot] = relativeAnimation == 0
															? ResolveCompanion(
																  _state.ActorPositionIndices[kCompanionActorSlot],
																  point.PositionIndex)
															: static_cast<int16>(point.PositionIndex +
																				 relativeAnimation);
	_state.ActorPositionIndices[kCompanionActorSlot] = point.PositionIndex;
	_state.ActorAnimationDelays[kCompanionActorSlot] = -1;
	_state.ActorAnimationRestartFlags |= kCompanionActorMask;
}

void RoomActorUpdater::updateActorPrioritiesAndScales() {
	updateActorPriority(kCompanionActorSlot);
	if ((_state.TransitionFlags & kTransitionCompanionCopyMask) != 0) {
		_state.ActorHighPriorityFlags = static_cast<uint8>((_state.ActorHighPriorityFlags & 0xF0) |
														   0x03);
		_state.ActorXFixedCoordinates[2] = _state.ActorXFixedCoordinates[kLeadActorSlot];
		_state.ActorYFixedCoordinates[2] = _state.ActorYFixedCoordinates[kLeadActorSlot];
		if (getIntegerCoordinate(_state.ActorXFixedCoordinates[kLeadActorSlot]) >= 0xF3) {
			_state.ActorVerticalScaleFactors[2] = 0;
			_state.ActorHorizontalScaleFactors[2] = 0x80;
		} else {
			_state.ActorVerticalScaleFactors[2] = 0x80;
			_state.ActorHorizontalScaleFactors[2] = 0;
		}

		_state.ActorYFixedCoordinates[2] = replaceIntegerCoordinate(_state.ActorYFixedCoordinates[2],
																	static_cast<int16>(
																		getIntegerCoordinate(
																			_state.ActorYFixedCoordinates[kLeadActorSlot]) -
																		0x14));
		int16 pairedX = static_cast<int16>(
			getIntegerCoordinate(_state.ActorXFixedCoordinates[kLeadActorSlot]) + 0x14);
		if (pairedX > 0x160) {
			pairedX = 0x160;
		}

		_state.ActorXFixedCoordinates[2] = replaceIntegerCoordinate(_state.ActorXFixedCoordinates[2], pairedX);
		if (getIntegerCoordinate(_state.ActorXFixedCoordinates[3]) == 0x150) {
			_state.ActorXFixedCoordinates[3] =
				replaceIntegerCoordinate(_state.ActorXFixedCoordinates[3], 0x150);
		}
	}

	int16 retainedPreviousScaleRow = static_cast<int16>(
		_state.RoomWidthTiles * static_cast<int>(sizeof(int16)));
	for (int actor = 5; actor >= 0; actor--) {
		if (hasFlag(_state.ActorCompactFrameFlags, actor)) {
			if ((_state.TransitionFlags & kTransitionSkipCompactScaleMask) == 0) {
				updateCompactActorScale(actor, retainedPreviousScaleRow);
			}
		}
	}

	if ((_state.RoomBehaviorFlags & kRoomBehaviorTransitionScaleMask) != 0) {
		uint16 transitionActor = _state.TransitionActorIndex;
		_state.ActorHorizontalScaleFactors[transitionActor] = _state.ActorHorizontalScaleFactors[kLeadActorSlot];
		_state.ActorVerticalScaleFactors[transitionActor] = _state.ActorVerticalScaleFactors[kLeadActorSlot];
	}
}

void RoomActorUpdater::updateCompactActorScale(int actor, int16 &retainedPreviousScaleRow) {
	int16 actorY = getIntegerCoordinate(_state.ActorYFixedCoordinates[actor]);
	int actorTileY = static_cast<uint16>(actorY) >> 3;
	int16 previousScale = 0;
	int16 row = 0;
	do {
		int16 scale = _rom.readInt16(_state.RoomCoordinateDataOffset +
									 row * static_cast<int>(sizeof(int16)));
		if (scale >= 0) {
			previousScale = scale;
			retainedPreviousScaleRow = row;
		}
	} while (++row < actorTileY);

	int16 nextScale = 0;
	int16 nextRow = static_cast<int16>(_state.RoomHeightTiles);
	for (; row < static_cast<int16>(_state.RoomHeightTiles); row++) {
		int16 scale = _rom.readInt16(_state.RoomCoordinateDataOffset +
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
	if (_state.ActorHorizontalScaleFactors[actor] != static_cast<uint16>(interpolatedScale)) {
		_state.ActorHorizontalScaleFactors[actor] = static_cast<uint16>(interpolatedScale);
		_state.ActorPositionUpdateFlags |= actorMask;
	}

	if (_state.ActorVerticalScaleFactors[actor] != static_cast<uint16>(interpolatedScale)) {
		_state.ActorVerticalScaleFactors[actor] = static_cast<uint16>(interpolatedScale);
		_state.ActorPositionUpdateFlags |= actorMask;
	}
}

void RoomActorUpdater::updateDynamicActors() {
	for (int actor = 2; actor < 6; actor++) {
		if ((_state.TransitionFlags & kTransitionCompanionCopyMask) != 0 && actor < 4) {
			continue;
		}

		if (!hasFlag(_state.ActiveActorFlags, actor)) {
			continue;
		}

		if (hasFlag(_state.ActorMovementFlags, actor)) {
			advanceDynamicActorMovement(actor);
		} else if (hasFlag(_state.ActorPathActiveFlags, actor)) {
			updateStandardActorPath(actor);
		}

		if (getIntegerCoordinate(_state.ActorXFixedCoordinates[actor]) == _state.ActorTargetXCoordinates[actor] &&
			getIntegerCoordinate(_state.ActorYFixedCoordinates[actor]) == _state.ActorTargetYCoordinates[actor]) {
			_state.ActorMovementFlags &= static_cast<uint8>(~actorMask(actor));
		}

		if (!hasFlag(_state.ActorPriorityUpdateBlockFlags, actor)) {
			updateActorPriority(actor);
		}
	}
}

void RoomActorUpdater::advanceDynamicActorMovement(int actor) {
	int movementIndex = actor - 2;
	int16 targetX = _state.ActorTargetXCoordinates[actor];
	int16 actorX = getIntegerCoordinate(_state.ActorXFixedCoordinates[actor]);
	if (actorX != targetX) {
		int32 targetFixedX = composePackedTargetCoordinate(_state.ActorTargetXCoordinates, actor,
														   static_cast<uint16>(_state.ActorTargetYCoordinates[0]));
		if (actorX < targetX) {
			_state.ActorXFixedCoordinates[actor] =
				_state.ActorXFixedCoordinates[actor] + _state.RoomObjectActorHorizontalSteps[movementIndex];
			if (_state.ActorXFixedCoordinates[actor] >= targetFixedX) {
				_state.ActorXFixedCoordinates[actor] = targetFixedX;
			}
		} else {
			_state.ActorXFixedCoordinates[actor] =
				_state.ActorXFixedCoordinates[actor] - _state.RoomObjectActorHorizontalSteps[movementIndex];
			if (_state.ActorXFixedCoordinates[actor] <= targetFixedX) {
				_state.ActorXFixedCoordinates[actor] = targetFixedX;
			}
		}
	}

	int16 targetY = _state.ActorTargetYCoordinates[actor];
	int16 actorY = getIntegerCoordinate(_state.ActorYFixedCoordinates[actor]);
	if (actorY == targetY) {
		return;
	}

	int32 targetFixedY = composePackedTargetCoordinate(_state.ActorTargetYCoordinates, actor,
													   static_cast<uint16>(static_cast<
																			   uint32>(
																			   _state.ActorPathStreamOffsets[0]) >>
																		   16));
	if (actorY < targetY) {
		_state.ActorYFixedCoordinates[actor] =
			_state.ActorYFixedCoordinates[actor] + _state.RoomObjectActorVerticalSteps[movementIndex];
		if (_state.ActorYFixedCoordinates[actor] >= targetFixedY) {
			_state.ActorYFixedCoordinates[actor] = targetFixedY;
		}
	} else {
		_state.ActorYFixedCoordinates[actor] =
			_state.ActorYFixedCoordinates[actor] - _state.RoomObjectActorVerticalSteps[movementIndex];
		if (_state.ActorYFixedCoordinates[actor] <= targetFixedY) {
			_state.ActorYFixedCoordinates[actor] = targetFixedY;
		}
	}
}

void RoomActorUpdater::updateStandardActorPath(int actor) {
	_state.ActorPathCadenceCountdowns[actor] = static_cast<int8>(_state.ActorPathCadenceCountdowns[actor] - 1);
	if (_state.ActorPathCadenceCountdowns[actor] >= 0) {
		return;
	}

	_state.ActorPathCadenceCountdowns[actor] = static_cast<int8>(_state.ActorPathCadenceReloads[actor]);
	while (true) {
		PathRecord record = readAndAdvancePathRecord(actor);
		int16 x = record.X;
		int16 y = record.Y;
		if (x >= 0) {
			setActorIntegerCoordinates(actor, x, y);
			return;
		}

		if (x == -3) {
			requestActorAnimation(actor, y);
			continue;
		}

		_state.ActorPathRecordOffsets[actor] = 0;
		if (x == -2) {
			clearActorPath(actor);
			return;
		}

		if (hasFlag(_state.ActorAnimationLoopExitFlags, actor)) {
			clearActorPath(actor);
			return;
		}
	}
}

void RoomActorUpdater::endActorPathAtTerminal(int actor) {
	if (!hasFlag(_state.ActorPathActiveFlags, actor)) {
		return;
	}

	int32 streamOffset = _state.ActorPathStreamOffsets[actor];
	int16 previousRecordOffset = static_cast<int16>(_state.ActorPathRecordOffsets[actor] - 4);
	int16 previousX = _rom.readInt16(streamOffset + previousRecordOffset);
	if (previousX == -2) {
		clearActorPath(actor);
	} else if (previousX == -1) {
		if (hasFlag(_state.ActorAnimationLoopExitFlags, actor)) {
			clearActorPath(actor);
		}
	}
}

RoomActorUpdater::PathRecord RoomActorUpdater::readAndAdvancePathRecord(int actor) {
	int16 recordOffset = static_cast<int16>(_state.ActorPathRecordOffsets[actor]);
	_state.ActorPathRecordOffsets[actor] = static_cast<uint16>(_state.ActorPathRecordOffsets[actor] + 4);
	int32 sourceOffset = _state.ActorPathStreamOffsets[actor] + recordOffset;
	PathRecord record;
	record.X = _rom.readInt16(sourceOffset);
	record.Y = _rom.readInt16(sourceOffset + static_cast<int>(sizeof(int16)));
	return record;
}

void RoomActorUpdater::clearActorPath(int actor) {
	uint8 actorMask = this->actorMask(actor);
	_state.ActorPathActiveFlags &= static_cast<uint8>(~actorMask);
	_state.ActorAnimationLoopExitFlags &= static_cast<uint8>(~actorMask);
}

void RoomActorUpdater::updateActorPriority(int actor) {
	int x = static_cast<uint16>(getIntegerCoordinate(_state.ActorXFixedCoordinates[actor])) >> 3;
	int y = static_cast<uint16>(getIntegerCoordinate(_state.ActorYFixedCoordinates[actor])) >> 3;
	int16 byteOffset = static_cast<int16>(
		y * _state.RoomWidthTiles * static_cast<int>(sizeof(int16)) +
		x * static_cast<int>(sizeof(int16)));
	SDM_ASSERT(_state.ActiveRoomScene, "UpdateActorPriority requires an active room scene.");
	int16 collisionCell = static_cast<int16>(
		ReadBigEndian16(_state.ActiveRoomScene->CollisionCells, byteOffset));
	uint8 actorMask = this->actorMask(actor);
	if (collisionCell < 0) {
		_state.ActorHighPriorityFlags |= actorMask;
	} else {
		_state.ActorHighPriorityFlags &= static_cast<uint8>(~actorMask);
	}
}

void RoomActorUpdater::requestActorAnimation(int actor, int16 animationOffset) {
	_state.ActorAnimationOffsets[actor] = animationOffset;
	_state.ActorAnimationRestartFlags |= actorMask(actor);
}

void RoomActorUpdater::setActorIntegerCoordinates(int actor, int16 x, int16 y) {
	_state.ActorXFixedCoordinates[actor] = replaceIntegerCoordinate(_state.ActorXFixedCoordinates[actor], x);
	_state.ActorYFixedCoordinates[actor] = replaceIntegerCoordinate(_state.ActorYFixedCoordinates[actor], y);
}

bool RoomActorUpdater::isNewPress(uint8 buttonMask) const {
	return (_state.ControllerOneInput & buttonMask) == 0 && (_state.PreviousControllerOneInput & buttonMask) !=
																0;
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
	if ((_state.ControllerOneInput & kCursorLeftMask) == 0) {
		if ((_state.ControllerOneInput & kCursorUpMask) == 0) {
			return 1;
		}

		if ((_state.ControllerOneInput & kCursorDownMask) == 0) {
			return 3;
		}

		return 4;
	}

	// Ghidra 0x0000753A-0x00007569: with Left released, preserve Right+Up, Right+Down, and Right-only.
	if ((_state.ControllerOneInput & kCursorRightMask) == 0) {
		if ((_state.ControllerOneInput & kCursorUpMask) == 0) {
			return 2;
		}

		if ((_state.ControllerOneInput & kCursorDownMask) == 0) {
			return 0;
		}

		return 5;
	}

	// Ghidra 0x0000756A-0x00007589: retain independent Up and Down results and the untouched incoming-D0
	// return when all four direction bits are released.
	if ((_state.ControllerOneInput & kCursorUpMask) == 0) {
		return 6;
	}

	if ((_state.ControllerOneInput & kCursorDownMask) == 0) {
		return 7;
	}

	return retainedDataRegister;
}
} // namespace Scooby