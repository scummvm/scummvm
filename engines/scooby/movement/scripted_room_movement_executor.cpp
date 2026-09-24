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

#include "scripted_room_movement_executor.h"

#include "actor_directional_animation_catalog.h"
#include "scooby/common/contracts.h"

namespace Scooby {

bool ScriptedRoomMovementExecutor::moveRoomObjectToPosition(uint16 objectIdentity, int16 targetX,
															int16 targetY, int16 movementMode,
															int16 animationOffset,
															uint16 encodedObjectIdentity) {
	// Ghidra 0x00004B24-0x00004B4B: select the 0x1A-byte typed record, replace both target words, and retain
	// the signed-negative attached-actor return after restoring all three saved input words.
	RoomObject &roomObject = _state.RoomObjects[static_cast<uint16>(objectIdentity - 3)];
	roomObject.FixedX = targetX;
	roomObject.FixedY = targetY;
	if (roomObject.FixedPositionIndex < 0) {
		return true;
	}

	// Ghidra 0x00004B4C-0x00004B6B: zero-extend the dynamic index, map it to actor slots two through five,
	// retain modulo-eight bit selection, and snapshot the slot's current horizontal 16.16 step.
	uint8 dynamicActorIndex = static_cast<uint8>(roomObject.FixedPositionIndex);
	int actorSlot = dynamicActorIndex + 2;
	uint8 actorMask = static_cast<uint8>(1 << (actorSlot & 0x07));
	int32 baseHorizontalStep = _state.RoomObjectActorHorizontalSteps[dynamicActorIndex];

	if (movementMode < 0) {
		// Ghidra 0x00004B6C-0x00004BAB: direct placement replaces only integer coordinate halves. A negative
		// animation returns; otherwise restart at delay -1 and wait for published actor graphics.
		_state.ActorXFixedCoordinates[actorSlot] =
			replaceIntegerCoordinate(_state.ActorXFixedCoordinates[actorSlot], targetX);
		_state.ActorYFixedCoordinates[actorSlot] =
			replaceIntegerCoordinate(_state.ActorYFixedCoordinates[actorSlot], targetY);
		if (animationOffset < 0) {
			return true;
		}

		_state.ActorAnimationOffsets[actorSlot] = animationOffset;
		_state.ActorAnimationDelays[actorSlot] = -1;
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

	// Ghidra 0x00004BAC-0x00004BB7: modes zero and one retain one base step. Larger modes execute the complete
	// DBF loop, adding the base once for every counter value from mode-2 through zero.
	int32 horizontalStep = baseHorizontalStep;
	int16 repeatCounter = static_cast<int16>(movementMode - 2);
	if (repeatCounter >= 0) {
		do {
			horizontalStep = horizontalStep + baseHorizontalStep;
			repeatCounter--;
		} while (repeatCounter != -1);
	}

	// Ghidra 0x00004BB8-0x00004BC1: publish the wrapped horizontal result and its logical-right-shifted vertical
	// half-step for the selected dynamic slot.
	_state.RoomObjectActorHorizontalSteps[dynamicActorIndex] = horizontalStep;
	_state.RoomObjectActorVerticalSteps[dynamicActorIndex] =
		static_cast<int32>(static_cast<uint32>(horizontalStep) >> 1);

	// Ghidra 0x00004BC2-0x00004BD3: restore the saved target words into actor target slots two through five.
	_state.ActorTargetXCoordinates[actorSlot] = targetX;
	_state.ActorTargetYCoordinates[actorSlot] = targetY;

	// Ghidra 0x00004BD4-0x00004BFB: a negative animation skips only animation replacement; every nonnegative
	// value starts at delay -1 and requests restart before movement becomes active.
	if (animationOffset >= 0) {
		_state.ActorAnimationOffsets[actorSlot] = animationOffset;
		_state.ActorAnimationDelays[actorSlot] = -1;
		_state.ActorAnimationRestartFlags |= actorMask;
	}

	// Ghidra 0x00004BFC-0x00004C0F: set movement active unconditionally. Encoded bit 15 returns without
	// waiting, while preserving every movement and optional animation side effect above.
	_state.ActorMovementFlags |= actorMask;
	if (static_cast<int16>(encodedObjectIdentity) < 0) {
		return true;
	}

	// Ghidra 0x00004C10-0x00004C1F: update animations at least once and retain the back edge until the room
	// callback advances the actor to its target and clears the selected movement bit.
	while (true) {
		_updateActorAnimationFrames();
		if ((_state.ActorMovementFlags & actorMask) == 0) {
			return true;
		}

		if (!_waitForRoomVerticalBlank()) {
			return false;
		}
	}
}

bool ScriptedRoomMovementExecutor::moveLeadActorToRoomPosition(int16 targetX, int16 targetY,
															   int16 movementScale,
															   int16 animationBias,
															   uint16 encodedObjectIdentity) {
	// Ghidra 0x00005002-0x0000501B: clear stale lead movement and return only when both signed integer
	// coordinates already equal the requested destination.
	_state.ActorMovementFlags &= static_cast<uint8>(~kLeadActorMask);
	if (getIntegerCoordinate(_state.ActorXFixedCoordinates[0]) == targetX &&
		getIntegerCoordinate(_state.ActorYFixedCoordinates[0]) == targetY) {
		return true;
	}

	// Ghidra 0x0000501C-0x00005043: progress bit four, a clear direct trace, or no visible waypoint all
	// tail-enter direct movement with a freshly reset queued-point index.
	if ((_state.ProgressStateBytes[0] & 0x10) != 0 || !traceLeadPathToTarget(targetX, targetY)) {
		return moveLeadActorDirectly(targetX, targetY, movementScale, animationBias, encodedObjectIdentity,
									 true);
	}

	uint16 visibleWaypoints = classifyLeadWaypointVisibility();
	if (visibleWaypoints == 0) {
		return moveLeadActorDirectly(targetX, targetY, movementScale, animationBias, encodedObjectIdentity,
									 true);
	}

	int waypointDataOffset = _state.RoomMovementDataOffset + kRouteWaypointDataOffset;
	int16 firstWaypointRawX = _rom.readInt16(waypointDataOffset);
	int16 firstWaypointRawY = _rom.readInt16(waypointDataOffset + 2);
	int16 secondWaypointRawX = _rom.readInt16(waypointDataOffset + 4);
	int16 secondWaypointRawY = _rom.readInt16(waypointDataOffset + 6);
	int16 firstWaypointX = static_cast<int16>(firstWaypointRawX << 3);
	int16 firstWaypointY = static_cast<int16>(firstWaypointRawY << 3);
	int16 secondWaypointX = static_cast<int16>(secondWaypointRawX << 3);
	int16 secondWaypointY = static_cast<int16>(secondWaypointRawY << 3);

	// Ghidra 0x00005044-0x000050EB: when both waypoints are visible, compare their raw signed squared
	// magnitudes with 32-bit wrapping, then retain both selected-waypoint-to-target trace outcomes.
	if (visibleWaypoints == 3) {
		int firstSquaredMagnitude = firstWaypointRawX * firstWaypointRawX + firstWaypointRawY *
																				firstWaypointRawY;
		int secondSquaredMagnitude =
			secondWaypointRawX * secondWaypointRawX + secondWaypointRawY * secondWaypointRawY;
		if (firstSquaredMagnitude < secondSquaredMagnitude) {
			if (tracePathFromSecondWaypoint(targetX, targetY)) {
				return moveLeadActorDirectly(targetX, targetY, movementScale, animationBias,
											 encodedObjectIdentity,
											 true);
			}

			_state.LeadActorMovementPoints[0].TargetX = targetX;
			_state.LeadActorMovementPoints[0].TargetY = targetY;
			buildLeadRouteFromSecondWaypoint(targetX, targetY, movementScale);
			_state.LeadActorMovementPointIndex = 1;
			return moveLeadActorDirectly(secondWaypointX, secondWaypointY, movementScale, animationBias,
										 encodedObjectIdentity, false);
		}

		if (tracePathFromFirstWaypoint(targetX, targetY)) {
			return moveLeadActorDirectly(targetX, targetY, movementScale, animationBias, encodedObjectIdentity,
										 true);
		}

		_state.LeadActorMovementPoints[0].TargetX = targetX;
		_state.LeadActorMovementPoints[0].TargetY = targetY;
		buildLeadRouteFromFirstWaypoint(targetX, targetY, movementScale);
		_state.LeadActorMovementPointIndex = 1;
		return moveLeadActorDirectly(firstWaypointX, firstWaypointY, movementScale, animationBias,
									 encodedObjectIdentity, false);
	}

	// Ghidra 0x000050EC-0x00005145: visibility mask one first tries a two-segment route through waypoint one,
	// then preserves the blocked-first/clear-second three-segment route and its reverse queue order.
	if (visibleWaypoints == 1) {
		if (!tracePathFromFirstWaypoint(targetX, targetY)) {
			_state.LeadActorMovementPoints[0].TargetX = targetX;
			_state.LeadActorMovementPoints[0].TargetY = targetY;
			buildLeadRouteFromFirstWaypoint(targetX, targetY, movementScale);
			_state.LeadActorMovementPointIndex = 1;
			return moveLeadActorDirectly(firstWaypointX, firstWaypointY, movementScale, animationBias,
										 encodedObjectIdentity, false);
		}

		if (tracePathFromSecondWaypoint(targetX, targetY)) {
			return moveLeadActorDirectly(targetX, targetY, movementScale, animationBias, encodedObjectIdentity,
										 true);
		}

		_state.LeadActorMovementPoints[0].TargetX = targetX;
		_state.LeadActorMovementPoints[0].TargetY = targetY;
		buildLeadRouteFromSecondWaypoint(targetX, targetY, movementScale);
		_state.LeadActorMovementPoints[1].TargetX = secondWaypointX;
		_state.LeadActorMovementPoints[1].TargetY = secondWaypointY;
		appendLeadRouteFromFirstWaypoint(secondWaypointX, secondWaypointY, movementScale);
		_state.LeadActorMovementPointIndex = 2;
		return moveLeadActorDirectly(firstWaypointX, firstWaypointY, movementScale, animationBias,
									 encodedObjectIdentity, false);
	}

	// Ghidra 0x00005146-0x0000519B: every remaining nonzero mask (including the produced mask two) mirrors the
	// prior branch through waypoint two, preserving both its two-segment and three-segment outcomes.
	if (!tracePathFromSecondWaypoint(targetX, targetY)) {
		_state.LeadActorMovementPoints[0].TargetX = targetX;
		_state.LeadActorMovementPoints[0].TargetY = targetY;
		buildLeadRouteFromSecondWaypoint(targetX, targetY, movementScale);
		_state.LeadActorMovementPointIndex = 1;
		return moveLeadActorDirectly(secondWaypointX, secondWaypointY, movementScale, animationBias,
									 encodedObjectIdentity, false);
	}

	if (tracePathFromFirstWaypoint(targetX, targetY)) {
		return moveLeadActorDirectly(targetX, targetY, movementScale, animationBias, encodedObjectIdentity,
									 true);
	}

	_state.LeadActorMovementPoints[0].TargetX = targetX;
	_state.LeadActorMovementPoints[0].TargetY = targetY;
	buildLeadRouteFromFirstWaypoint(targetX, targetY, movementScale);
	_state.LeadActorMovementPoints[1].TargetX = firstWaypointX;
	_state.LeadActorMovementPoints[1].TargetY = firstWaypointY;
	appendLeadRouteFromSecondWaypoint(firstWaypointX, firstWaypointY, movementScale);
	_state.LeadActorMovementPointIndex = 2;
	return moveLeadActorDirectly(secondWaypointX, secondWaypointY, movementScale, animationBias,
								 encodedObjectIdentity, false);
}

void ScriptedRoomMovementExecutor::buildLeadRouteFromFirstWaypoint(int16 targetX, int16 targetY,
																   int16 movementScale) {
	// Ghidra 0x0000519C-0x000051CB: stage the first waypoint as wrapped signed 16.16 coordinates with zero
	// fractional halves.
	int waypointOffset = _state.RoomMovementDataOffset + kRouteWaypointDataOffset;
	int originXFixed = static_cast<int16>(_rom.readInt16(waypointOffset) << 3) << 16;
	int originYFixed = static_cast<int16>(_rom.readInt16(waypointOffset + sizeof(int16))) << 3 << 16;

	// Ghidra 0x000051CC-0x000051CF: calculate the three movement outputs from the staged origin.
	VelocityResult velocity = computeMovementVelocityFromStagedOrigin(
		originXFixed, originYFixed, targetX, targetY,
		movementScale);

	// Ghidra 0x000051D0-0x000051E3: publish the outputs into queue slot zero without replacing its target.
	_state.LeadActorMovementPoints[0].HorizontalStep = velocity.HorizontalStep;
	_state.LeadActorMovementPoints[0].VerticalStep = velocity.VerticalStep;
	_state.LeadActorMovementPoints[0].PositionIndex = velocity.PositionIndex;
}

void ScriptedRoomMovementExecutor::buildLeadRouteFromSecondWaypoint(int16 targetX, int16 targetY,
																	int16 movementScale) {
	// Ghidra 0x000051E4-0x00005215: stage the second waypoint as wrapped signed 16.16 coordinates with zero
	// fractional halves.
	int waypointOffset = _state.RoomMovementDataOffset + kRouteWaypointDataOffset + 4;
	int originXFixed = static_cast<int16>(_rom.readInt16(waypointOffset) << 3) << 16;
	int originYFixed = static_cast<int16>(_rom.readInt16(waypointOffset + sizeof(int16))) << 3 << 16;

	// Ghidra 0x00005216-0x00005219: calculate the three movement outputs from the staged origin.
	VelocityResult velocity = computeMovementVelocityFromStagedOrigin(
		originXFixed, originYFixed, targetX, targetY,
		movementScale);

	// Ghidra 0x0000521A-0x0000522D: publish the outputs into queue slot zero without replacing its target.
	_state.LeadActorMovementPoints[0].HorizontalStep = velocity.HorizontalStep;
	_state.LeadActorMovementPoints[0].VerticalStep = velocity.VerticalStep;
	_state.LeadActorMovementPoints[0].PositionIndex = velocity.PositionIndex;
}

void ScriptedRoomMovementExecutor::appendLeadRouteFromFirstWaypoint(int16 targetX, int16 targetY,
																	int16 movementScale) {
	// Ghidra 0x0000522E-0x0000525D: stage the first waypoint as wrapped signed 16.16 coordinates with zero
	// fractional halves.
	int waypointOffset = _state.RoomMovementDataOffset + kRouteWaypointDataOffset;
	int originXFixed = static_cast<int16>(_rom.readInt16(waypointOffset) << 3) << 16;
	int originYFixed = static_cast<int16>(_rom.readInt16(waypointOffset + sizeof(int16))) << 3 << 16;

	// Ghidra 0x0000525E-0x00005261: calculate the three movement outputs from the staged origin.
	VelocityResult velocity = computeMovementVelocityFromStagedOrigin(
		originXFixed, originYFixed, targetX, targetY,
		movementScale);

	// Ghidra 0x00005262-0x00005275: publish the outputs into queue slot one without replacing its target.
	_state.LeadActorMovementPoints[1].HorizontalStep = velocity.HorizontalStep;
	_state.LeadActorMovementPoints[1].VerticalStep = velocity.VerticalStep;
	_state.LeadActorMovementPoints[1].PositionIndex = velocity.PositionIndex;
}

void ScriptedRoomMovementExecutor::appendLeadRouteFromSecondWaypoint(int16 targetX, int16 targetY,
																	 int16 movementScale) {
	// Ghidra 0x00005276-0x000052A7: stage the second waypoint as wrapped signed 16.16 coordinates with zero
	// fractional halves.
	int waypointOffset = _state.RoomMovementDataOffset + kRouteWaypointDataOffset + 4;
	int originXFixed = static_cast<int16>(_rom.readInt16(waypointOffset) << 3) << 16;
	int originYFixed = static_cast<int16>(_rom.readInt16(waypointOffset + sizeof(int16))) << 3 << 16;

	// Ghidra 0x000052A8-0x000052AB: calculate the three movement outputs from the staged origin.
	VelocityResult velocity = computeMovementVelocityFromStagedOrigin(
		originXFixed, originYFixed, targetX, targetY,
		movementScale);

	// Ghidra 0x000052AC-0x000052BF: publish the outputs into queue slot one without replacing its target.
	_state.LeadActorMovementPoints[1].HorizontalStep = velocity.HorizontalStep;
	_state.LeadActorMovementPoints[1].VerticalStep = velocity.VerticalStep;
	_state.LeadActorMovementPoints[1].PositionIndex = velocity.PositionIndex;
}

ScriptedRoomMovementExecutor::VelocityResult
ScriptedRoomMovementExecutor::computeMovementVelocityFromStagedOrigin(
	int originXFixed, int originYFixed, int16 targetX, int16 targetY, int16 movementScale) {
	// Ghidra 0x0000548E-0x000054C1: preserve inputs, calculate wrapping word deltas and magnitudes, and classify
	// direction with the original signed comparison of the magnitude words.
	int16 horizontalDelta =
		static_cast<int16>(targetX - static_cast<int16>(originXFixed >> 16));
	int16 verticalDelta = static_cast<int16>(targetY - static_cast<int16>(originYFixed >>
																		  16));
	verticalDelta <<= 1;
	uint16 horizontalMagnitude =
		static_cast<uint16>(horizontalDelta < 0 ? -horizontalDelta : horizontalDelta);
	uint16 verticalMagnitude = static_cast<uint16>(verticalDelta < 0
													   ? -verticalDelta
													   : verticalDelta);
	int16 positionIndex;
	if (static_cast<int16>(verticalMagnitude) >= static_cast<int16>(horizontalMagnitude)) {
		positionIndex = verticalDelta < 0 ? static_cast<int16>(2) : static_cast<int16>(3);
	} else {
		positionIndex = horizontalDelta < 0 ? static_cast<int16>(1) : static_cast<int16>(0);
	}

	// Ghidra 0x000054C2-0x000054D9: shift the scale as one wrapping word and zero-extend both magnitudes before
	// converting them into 24.8 unsigned division numerators.
	uint16 shiftedScale = static_cast<uint16>(movementScale << 8);
	uint32 scaledHorizontalMagnitude = static_cast<uint32>(horizontalMagnitude) << 8;
	uint32 scaledVerticalMagnitude = static_cast<uint32>(verticalMagnitude) << 8;
	int16 horizontalStep;
	int16 verticalStep;

	// Ghidra 0x000054DA-0x000054ED: select zero-axis branches first; otherwise compare the signed low quotient
	// words produced by both unsigned divisions.
	if (scaledVerticalMagnitude == 0) {
		// Ghidra 0x0000554E-0x00005563: a zero wrapped Y magnitude produces a signed unit X step.
		horizontalStep = horizontalDelta < 0
							 ? static_cast<int16>(-0x100)
							 : static_cast<int16>(0x100);
		verticalStep = 0;
	} else if (scaledHorizontalMagnitude == 0) {
		// Ghidra 0x00005536-0x0000554D: a zero wrapped X magnitude produces a signed unit Y step.
		horizontalStep = 0;
		verticalStep = verticalDelta < 0 ? static_cast<int16>(-0x100) : static_cast<int16>(0x100);
	} else {
		uint16 horizontalQuotient =
			divideUnsignedWordPreservingOverflow(scaledHorizontalMagnitude, shiftedScale);
		uint16 verticalQuotient = divideUnsignedWordPreservingOverflow(
			scaledVerticalMagnitude, shiftedScale);
		if (static_cast<int16>(verticalQuotient) > static_cast<int16>(horizontalQuotient)) {
			// Ghidra 0x00005510-0x00005535: normalize X against the dominant Y quotient and restore signs.
			horizontalStep = static_cast<int16>(
				divideUnsignedWordPreservingOverflow(scaledHorizontalMagnitude, verticalQuotient));
			verticalStep = 0x100;
			if (horizontalDelta < 0) {
				horizontalStep = static_cast<int16>(-horizontalStep);
			}

			if (verticalDelta < 0) {
				verticalStep = static_cast<int16>(-verticalStep);
			}
		} else {
			// Ghidra 0x000054EE-0x0000550F: normalize Y against the dominant X quotient and restore signs.
			horizontalStep = 0x100;
			verticalStep = static_cast<int16>(
				divideUnsignedWordPreservingOverflow(scaledVerticalMagnitude, horizontalQuotient));
			if (horizontalDelta < 0) {
				horizontalStep = static_cast<int16>(-horizontalStep);
			}

			if (verticalDelta < 0) {
				verticalStep = static_cast<int16>(-verticalStep);
			}
		}
	}

	// Ghidra 0x00005564-0x00005589: restore the original scale, retain signed-word multiplication and
	// truncation, publish both steps, and return the D4.w/D5.w/D6.w outputs to the route builder.
	horizontalStep *= movementScale;
	verticalStep *= movementScale;
	_state.LeadActorHorizontalMovementStep = horizontalStep;
	_state.LeadActorVerticalMovementStep = verticalStep;
	return VelocityResult(horizontalStep, verticalStep, positionIndex);
}

uint16 ScriptedRoomMovementExecutor::divideUnsignedWordPreservingOverflow(uint32 dividend,
																		  uint16 divisor) {
	// The original M68K DIVU.W traps on a zero divisor; authored movement data never supplies one, so this
	// project fails fast rather than emulating the trap (exceptions are disallowed).
	SDM_ASSERT(divisor != 0, "DivideUnsignedWordPreservingOverflow requires a nonzero divisor.");
	uint32 quotient = dividend / divisor;
	// DIVU leaves the destination register unchanged when its quotient does not fit in the low word.
	return quotient <= 0xFFFFu ? static_cast<uint16>(quotient) : static_cast<uint16>(dividend);
}

bool ScriptedRoomMovementExecutor::tracePathFromFirstWaypoint(int16 targetX, int16 targetY) {
	// Ghidra 0x000052C0-0x000052CF: resolve the active movement block's first signed waypoint pair.
	int waypointOffset = _state.RoomMovementDataOffset + kRouteWaypointDataOffset;
	int16 waypointX = _rom.readInt16(waypointOffset);
	int16 waypointY = _rom.readInt16(waypointOffset + sizeof(int16));

	// Ghidra 0x000052D0-0x000052EF: apply both wrapping word shifts and clear both fractional words.
	int originXFixed = static_cast<int16>(waypointX << 3) << 16;
	int originYFixed = static_cast<int16>(waypointY << 3) << 16;

	// Ghidra 0x000052F0-0x000052F3: preserve the tail branch into the separately owned trace continuation.
	return tracePathFromStagedOrigin(originXFixed, originYFixed, targetX, targetY);
}

bool ScriptedRoomMovementExecutor::tracePathFromSecondWaypoint(int16 targetX, int16 targetY) {
	// Ghidra 0x000052F4-0x00005305: resolve the active movement block's second signed waypoint pair.
	int waypointOffset = _state.RoomMovementDataOffset + kRouteWaypointDataOffset + 4;
	int16 waypointX = _rom.readInt16(waypointOffset);
	int16 waypointY = _rom.readInt16(waypointOffset + sizeof(int16));

	// Ghidra 0x00005306-0x00005325: apply both wrapping word shifts and clear both fractional words.
	int originXFixed = static_cast<int16>(waypointX << 3) << 16;
	int originYFixed = static_cast<int16>(waypointY << 3) << 16;

	// Ghidra 0x00005326-0x00005329: preserve the tail branch into the separately owned trace continuation.
	return tracePathFromStagedOrigin(originXFixed, originYFixed, targetX, targetY);
}

uint16 ScriptedRoomMovementExecutor::classifyLeadWaypointVisibility() {
	// Ghidra 0x0000532A-0x00005339: clear the result and select the first of exactly two waypoint pairs.
	uint16 visibility = 0;
	int waypointOffset = _state.RoomMovementDataOffset + kRouteWaypointDataOffset;
	for (int waypointIndex = 0; waypointIndex < 2; ++waypointIndex) {
		// Ghidra 0x0000533A-0x00005359: preserve the result, bit index, and cursor around the staged trace, then
		// set only the bit whose waypoint has a clear lead-to-waypoint path.
		int16 targetX = static_cast<int16>(_rom.readInt16(waypointOffset) << 3);
		int16 targetY = static_cast<int16>(_rom.readInt16(waypointOffset + sizeof(int16))
										   << 3);
		if (!traceLeadPathToTarget(targetX, targetY)) {
			visibility |= 1 << waypointIndex;
		}

		// Ghidra 0x0000535A-0x00005363: advance one four-byte pair and retain both DBF iterations.
		waypointOffset += 2 * static_cast<int>(sizeof(int16));
	}

	return visibility;
}

bool ScriptedRoomMovementExecutor::traceLeadPathToTarget(int16 targetX, int16 targetY) {
	// Ghidra 0x00005364-0x00005377: stage both complete lead coordinates and preserve the original fallthrough
	// into the separately owned trace continuation.
	return tracePathFromStagedOrigin(_state.ActorXFixedCoordinates[0], _state.ActorYFixedCoordinates[0],
									 targetX,
									 targetY);
}

bool ScriptedRoomMovementExecutor::tracePathFromStagedOrigin(int originXFixed, int originYFixed,
															 int16 targetX, int16 targetY) {
	// Ghidra 0x00005378-0x0000539D: derive wrapped signed word deltas and their native NEG.W magnitudes.
	int16 deltaX = static_cast<int16>(targetX - getIntegerCoordinate(originXFixed));
	int16 deltaY = static_cast<int16>(targetY - getIntegerCoordinate(originYFixed));
	int16 magnitudeX = deltaX < 0 ? static_cast<int16>(-deltaX) : deltaX;
	int16 magnitudeY = deltaY < 0 ? static_cast<int16>(-deltaY) : deltaY;

	int stepX;
	int stepY;
	if (deltaX != 0 && deltaY != 0) {
		// Ghidra 0x0000539E-0x000053E3: normalize the signed-minor axis with unsigned DIVU.W semantics, select
		// exactly one signed 16.16 unit axis, and preserve equal-magnitude quotient overflow.
		if (magnitudeY > magnitudeX) {
			stepX = calculateNormalizedMinorStep(magnitudeX, magnitudeY);
			stepY = 0x10000;
		} else {
			stepX = 0x10000;
			stepY = calculateNormalizedMinorStep(magnitudeY, magnitudeX);
		}

		if (deltaX < 0) {
			stepX = -stepX;
		}

		if (deltaY < 0) {
			stepY = -stepY;
		}
	} else {
		// Ghidra 0x000053E4-0x0000540F: axis-aligned paths use zero on the matching axis and one signed
		// fixed-point unit on the other; coincident endpoints retain the original positive Y unit.
		stepX = deltaX == 0 ? 0 : (deltaX < 0 ? -0x10000 : 0x10000);
		stepY = deltaX == 0 ? (deltaY < 0 ? -0x10000 : 0x10000) : 0;
	}

	// Ghidra 0x00005410-0x0000542D: copy the complete origin into collision scratch and probe before moving.
	int stagedXFixed = originXFixed;
	int stagedYFixed = originYFixed;
	while (true) {
		if (_collisionProbe.probeStagedCollision(getIntegerCoordinate(stagedXFixed),
												 getIntegerCoordinate(stagedYFixed))) {
			// Ghidra 0x00005470-0x00005479: restore caller-owned registers and return blocked in D3.
			return true;
		}

		// Ghidra 0x0000542E-0x00005469: advance both wrapped longwords, then require X and Y to reach or pass
		// their signed targets in sequence before leaving the loop.
		stagedXFixed = stagedXFixed + stepX;
		stagedYFixed = stagedYFixed + stepY;
		int16 stagedX = getIntegerCoordinate(stagedXFixed);
		if (stepX < 0 ? targetX < stagedX : targetX > stagedX) {
			continue;
		}

		int16 stagedY = getIntegerCoordinate(stagedYFixed);
		if (stepY < 0 ? targetY < stagedY : targetY > stagedY) {
			continue;
		}

		// Ghidra 0x0000546A-0x0000546F: restore caller-owned D2 and return clear in D3.
		return false;
	}
}

ScriptedRoomMovementExecutor::VelocityResult ScriptedRoomMovementExecutor::computeLeadMovementVelocity(
	int16 targetX, int16 targetY, int16 movementScale) {
	// Ghidra 0x0000547A-0x0000548D: stage both complete lead coordinates and retain the fallthrough into the
	// separately owned velocity calculation with target and scale inputs unchanged.
	return computeMovementVelocityFromStagedOrigin(_state.ActorXFixedCoordinates[0],
												   _state.ActorYFixedCoordinates[0],
												   targetX, targetY, movementScale);
}

bool ScriptedRoomMovementExecutor::moveLeadActorDirectly(int16 targetX, int16 targetY,
														 int16 movementScale, int16 animationBias,
														 uint16 encodedObjectIdentity,
														 bool resetMovementPointIndex) {
	// Ghidra 0x0000558A-0x00005591: only the full entry resets the queued movement-point index; routed internal
	// entry 0x00005592 retains the caller's prepared reverse-order queue.
	if (resetMovementPointIndex) {
		_state.LeadActorMovementPointIndex = 0;
	}

	// Ghidra 0x00005592-0x000055AF: install the target, preserve and clear display bit three, then stage
	// current lead coordinates and calculate this segment's velocity through its separate original owner.
	_state.ActorTargetXCoordinates[0] = targetX;
	_state.ActorTargetYCoordinates[0] = targetY;
	uint8 savedDisplayBit = static_cast<uint8>(_state.DisplayFlags & 0x08);
	_state.DisplayFlags &= static_cast<uint8>(~0x08);
	VelocityResult velocity = computeLeadMovementVelocity(targetX, targetY, movementScale);
	int16 positionIndex = velocity.PositionIndex;

	// Ghidra 0x000055B0-0x000055D9: zero bias uses the authored previous-to-next directional matrix; every
	// nonzero signed bias wraps with the new position before publishing the selected animation.
	_state.ActorAnimationOffsets[0] =
		animationBias == 0
			? ResolveLead(_state.ActorPositionIndices[0], positionIndex)
			: static_cast<int16>(positionIndex + animationBias);

	// Ghidra 0x000055DA-0x00005613: restart immediately, wait for both restart consumption and graphics
	// publication, then mark lead movement active. Native interrupt-owned progress is serviced per back edge.
	_state.ActorPositionIndices[0] = positionIndex;
	_state.ActorAnimationDelays[0] = -1;
	_state.ActorAnimationRestartFlags |= kLeadActorMask;
	while (true) {
		_updateActorAnimationFrames();
		if ((_state.ActorAnimationRestartFlags & kLeadActorMask) == 0 &&
			(_state.ActorGraphicsReadyFlags & kLeadActorMask) != 0) {
			break;
		}

		if (!_waitForRoomVerticalBlank()) {
			return false;
		}
	}

	_state.ActorMovementFlags |= kLeadActorMask;

	// Ghidra 0x00005614-0x00005629: a negative encoded identity leaves movement asynchronous. Every nonnegative
	// identity waits at least once until the room callback clears the complete routed movement.
	if (static_cast<int16>(encodedObjectIdentity) >= 0) {
		while (true) {
			_updateActorAnimationFrames();
			if ((_state.ActorMovementFlags & kLeadActorMask) == 0) {
				break;
			}

			if (!_waitForRoomVerticalBlank()) {
				return false;
			}
		}

		// Ghidra 0x0000562A-0x00005653: select position plus four, restart at delay -1, and wait only for
		// restart consumption before continuing to the normal restoration path.
		_state.ActorAnimationOffsets[0] = static_cast<int16>(positionIndex + 4);
		_state.ActorAnimationDelays[0] = -1;
		_state.ActorAnimationRestartFlags |= kLeadActorMask;
		while (true) {
			_updateActorAnimationFrames();
			if ((_state.ActorAnimationRestartFlags & kLeadActorMask) == 0) {
				break;
			}

			if (!_waitForRoomVerticalBlank()) {
				return false;
			}
		}
	}

	// Ghidra 0x00005654-0x00005669: normal return replaces only display bit three with its saved value,
	// retaining every other callback-owned display change made during the waits.
	_state.DisplayFlags = static_cast<uint8>((_state.DisplayFlags & ~0x08) | savedDisplayBit);
	return true;
}

bool ScriptedRoomMovementExecutor::finishCompanionMovement(int16 targetX, int16 targetY,
														   int16 movementMode,
														   int16 destinationBias,
														   uint16 encodedObjectIdentity) {
	// Ghidra 0x0000566A-0x0000566D: start the complete companion route and retain its D6 position result.
	MovementResult result = movePlayerToRoomPosition(targetX, targetY, movementMode, destinationBias);
	if (!result.Completed) {
		return false;
	}

	// Ghidra 0x0000566E-0x00005683: a negative encoded identity returns asynchronously. Every nonnegative value
	// updates animations at least once and waits until callback progress clears movement.
	if (static_cast<int16>(encodedObjectIdentity) < 0) {
		return true;
	}

	while (true) {
		_updateActorAnimationFrames();
		if ((_state.ActorMovementFlags & kCompanionActorMask) == 0) {
			break;
		}

		if (!_waitForRoomVerticalBlank()) {
			return false;
		}
	}

	// Ghidra 0x00005684-0x000056AD: wrap the retained position plus four, restart companion animation at delay
	// -1, then update at least once and wait only for callback consumption of its restart request.
	_state.ActorAnimationOffsets[1] = static_cast<int16>(result.PositionIndex + 4);
	_state.ActorAnimationDelays[1] = -1;
	_state.ActorAnimationRestartFlags |= kCompanionActorMask;
	while (true) {
		_updateActorAnimationFrames();
		if ((_state.ActorAnimationRestartFlags & kCompanionActorMask) == 0) {
			break;
		}

		if (!_waitForRoomVerticalBlank()) {
			return false;
		}
	}

	// Ghidra 0x000056AE-0x000056AF: return after the final restart has been consumed.
	return true;
}

MovementResult ScriptedRoomMovementExecutor::movePlayerToRoomPosition(
	int16 targetX, int16 targetY,
	int16 movementScale,
	int16 animationBias) {
	// Ghidra 0x000056B0-0x000056D1: clear stale companion movement and interaction bit five, then return only
	// when both signed integer coordinates already equal the target. Incoming D6 is incidental here.
	_state.ActorMovementFlags &= static_cast<uint8>(~kCompanionActorMask);
	_state.InteractionFlags &= 0xDF;
	if (getIntegerCoordinate(_state.ActorXFixedCoordinates[1]) == targetX &&
		getIntegerCoordinate(_state.ActorYFixedCoordinates[1]) == targetY) {
		return MovementResult(true, _state.ActorPositionIndices[1]);
	}

	// Ghidra 0x000056D2-0x000056F9: progress bit four, a clear direct trace, or no visible waypoint selects the
	// full direct-movement entry and resets the companion queue index.
	if ((_state.ProgressStateBytes[0] & 0x10) != 0 || !traceCompanionPathToTarget(targetX, targetY)) {
		return moveCompanionActorDirectly(targetX, targetY, movementScale, animationBias, true);
	}

	uint16 visibleWaypoints = classifyCompanionWaypointVisibility();
	if (visibleWaypoints == 0) {
		return moveCompanionActorDirectly(targetX, targetY, movementScale, animationBias, true);
	}

	int waypointDataOffset = _state.RoomMovementDataOffset + kRouteWaypointDataOffset;
	int16 firstWaypointRawX = _rom.readInt16(waypointDataOffset);
	int16 firstWaypointRawY = _rom.readInt16(waypointDataOffset + 2);
	int16 secondWaypointRawX = _rom.readInt16(waypointDataOffset + 4);
	int16 secondWaypointRawY = _rom.readInt16(waypointDataOffset + 6);
	int16 firstWaypointX = static_cast<int16>(firstWaypointRawX << 3);
	int16 firstWaypointY = static_cast<int16>(firstWaypointRawY << 3);
	int16 secondWaypointX = static_cast<int16>(secondWaypointRawX << 3);
	int16 secondWaypointY = static_cast<int16>(secondWaypointRawY << 3);

	// Ghidra 0x000056FA-0x0000579F: when both waypoints are visible, compare their raw signed squared
	// magnitudes with 32-bit wrapping and retain both selected-waypoint-to-target trace outcomes.
	if (visibleWaypoints == 3) {
		int firstSquaredMagnitude = firstWaypointRawX * firstWaypointRawX + firstWaypointRawY *
																				firstWaypointRawY;
		int secondSquaredMagnitude =
			secondWaypointRawX * secondWaypointRawX + secondWaypointRawY * secondWaypointRawY;
		if (firstSquaredMagnitude < secondSquaredMagnitude) {
			if (tracePathFromSecondWaypoint(targetX, targetY)) {
				return moveCompanionActorDirectly(targetX, targetY, movementScale, animationBias, true);
			}

			_state.CompanionActorMovementPoints[0].TargetX = targetX;
			_state.CompanionActorMovementPoints[0].TargetY = targetY;
			buildCompanionRouteFromSecondWaypoint(targetX, targetY, movementScale);
			_state.CompanionActorMovementPointIndex = 1;
			return moveCompanionActorDirectly(secondWaypointX, secondWaypointY, movementScale, animationBias,
											  false);
		}

		if (tracePathFromFirstWaypoint(targetX, targetY)) {
			return moveCompanionActorDirectly(targetX, targetY, movementScale, animationBias, true);
		}

		_state.CompanionActorMovementPoints[0].TargetX = targetX;
		_state.CompanionActorMovementPoints[0].TargetY = targetY;
		buildCompanionRouteFromFirstWaypoint(targetX, targetY, movementScale);
		_state.CompanionActorMovementPointIndex = 1;
		return moveCompanionActorDirectly(firstWaypointX, firstWaypointY, movementScale, animationBias, false);
	}

	// Ghidra 0x000057A2-0x000057FB: visibility mask one first tries a two-segment route through waypoint one,
	// then preserves the blocked-first/clear-second three-segment route in reverse queue order.
	if (visibleWaypoints == 1) {
		if (!tracePathFromFirstWaypoint(targetX, targetY)) {
			_state.CompanionActorMovementPoints[0].TargetX = targetX;
			_state.CompanionActorMovementPoints[0].TargetY = targetY;
			buildCompanionRouteFromFirstWaypoint(targetX, targetY, movementScale);
			_state.CompanionActorMovementPointIndex = 1;
			return moveCompanionActorDirectly(firstWaypointX, firstWaypointY, movementScale, animationBias,
											  false);
		}

		if (tracePathFromSecondWaypoint(targetX, targetY)) {
			return moveCompanionActorDirectly(targetX, targetY, movementScale, animationBias, true);
		}

		_state.CompanionActorMovementPoints[0].TargetX = targetX;
		_state.CompanionActorMovementPoints[0].TargetY = targetY;
		buildCompanionRouteFromSecondWaypoint(targetX, targetY, movementScale);
		_state.CompanionActorMovementPoints[1].TargetX = secondWaypointX;
		_state.CompanionActorMovementPoints[1].TargetY = secondWaypointY;
		appendCompanionRouteFromFirstWaypoint(secondWaypointX, secondWaypointY, movementScale);
		_state.CompanionActorMovementPointIndex = 2;
		return moveCompanionActorDirectly(firstWaypointX, firstWaypointY, movementScale, animationBias, false);
	}

	// Ghidra 0x000057FC-0x00005851: every remaining nonzero mask (including the produced mask two) mirrors the
	// prior branch through waypoint two, retaining both two-segment and three-segment outcomes.
	if (!tracePathFromSecondWaypoint(targetX, targetY)) {
		_state.CompanionActorMovementPoints[0].TargetX = targetX;
		_state.CompanionActorMovementPoints[0].TargetY = targetY;
		buildCompanionRouteFromSecondWaypoint(targetX, targetY, movementScale);
		_state.CompanionActorMovementPointIndex = 1;
		return moveCompanionActorDirectly(secondWaypointX, secondWaypointY, movementScale, animationBias,
										  false);
	}

	if (tracePathFromFirstWaypoint(targetX, targetY)) {
		return moveCompanionActorDirectly(targetX, targetY, movementScale, animationBias, true);
	}

	_state.CompanionActorMovementPoints[0].TargetX = targetX;
	_state.CompanionActorMovementPoints[0].TargetY = targetY;
	buildCompanionRouteFromFirstWaypoint(targetX, targetY, movementScale);
	_state.CompanionActorMovementPoints[1].TargetX = firstWaypointX;
	_state.CompanionActorMovementPoints[1].TargetY = firstWaypointY;
	appendCompanionRouteFromSecondWaypoint(firstWaypointX, firstWaypointY, movementScale);
	_state.CompanionActorMovementPointIndex = 2;
	return moveCompanionActorDirectly(secondWaypointX, secondWaypointY, movementScale, animationBias, false);
}

void ScriptedRoomMovementExecutor::buildCompanionRouteFromFirstWaypoint(
	int16 targetX, int16 targetY,
	int16 movementScale) {
	// Ghidra 0x00005852-0x00005881: stage the first waypoint as wrapped signed 16.16 coordinates with zero
	// fractional halves.
	int waypointOffset = _state.RoomMovementDataOffset + kRouteWaypointDataOffset;
	int originXFixed = static_cast<int16>(_rom.readInt16(waypointOffset) << 3) << 16;
	int originYFixed = static_cast<int16>(_rom.readInt16(waypointOffset + sizeof(int16))) << 3 << 16;

	// Ghidra 0x00005882-0x00005885: calculate all three outputs from the staged origin.
	VelocityResult velocity =
		computeCompanionVelocityFromStagedOrigin(originXFixed, originYFixed, targetX, targetY, movementScale);

	// Ghidra 0x00005886-0x00005899: publish the outputs into queue slot zero without replacing its target.
	_state.CompanionActorMovementPoints[0].HorizontalStep = velocity.HorizontalStep;
	_state.CompanionActorMovementPoints[0].VerticalStep = velocity.VerticalStep;
	_state.CompanionActorMovementPoints[0].PositionIndex = velocity.PositionIndex;
}

void ScriptedRoomMovementExecutor::buildCompanionRouteFromSecondWaypoint(
	int16 targetX, int16 targetY,
	int16 movementScale) {
	// Ghidra 0x0000589A-0x000058CB: stage the second waypoint as wrapped signed 16.16 coordinates with zero
	// fractional halves.
	int waypointOffset = _state.RoomMovementDataOffset + kRouteWaypointDataOffset + 4;
	int originXFixed = static_cast<int16>(_rom.readInt16(waypointOffset) << 3) << 16;
	int originYFixed = static_cast<int16>(_rom.readInt16(waypointOffset + sizeof(int16))) << 3 << 16;

	// Ghidra 0x000058CC-0x000058CF: calculate all three outputs from the staged origin.
	VelocityResult velocity =
		computeCompanionVelocityFromStagedOrigin(originXFixed, originYFixed, targetX, targetY, movementScale);

	// Ghidra 0x000058D0-0x000058E3: publish the outputs into queue slot zero without replacing its target.
	_state.CompanionActorMovementPoints[0].HorizontalStep = velocity.HorizontalStep;
	_state.CompanionActorMovementPoints[0].VerticalStep = velocity.VerticalStep;
	_state.CompanionActorMovementPoints[0].PositionIndex = velocity.PositionIndex;
}

void ScriptedRoomMovementExecutor::appendCompanionRouteFromFirstWaypoint(
	int16 targetX, int16 targetY,
	int16 movementScale) {
	// Ghidra 0x000058E4-0x00005913: stage the first waypoint as wrapped signed 16.16 coordinates with zero
	// fractional halves.
	int waypointOffset = _state.RoomMovementDataOffset + kRouteWaypointDataOffset;
	int originXFixed = static_cast<int16>(_rom.readInt16(waypointOffset) << 3) << 16;
	int originYFixed = static_cast<int16>(_rom.readInt16(waypointOffset + sizeof(int16))) << 3 << 16;

	// Ghidra 0x00005914-0x00005917: calculate all three outputs from the staged origin.
	VelocityResult velocity =
		computeCompanionVelocityFromStagedOrigin(originXFixed, originYFixed, targetX, targetY, movementScale);

	// Ghidra 0x00005918-0x0000592B: publish the outputs into queue slot one without replacing its target.
	_state.CompanionActorMovementPoints[1].HorizontalStep = velocity.HorizontalStep;
	_state.CompanionActorMovementPoints[1].VerticalStep = velocity.VerticalStep;
	_state.CompanionActorMovementPoints[1].PositionIndex = velocity.PositionIndex;
}

void ScriptedRoomMovementExecutor::appendCompanionRouteFromSecondWaypoint(
	int16 targetX, int16 targetY,
	int16 movementScale) {
	// Ghidra 0x0000592C-0x0000595D: stage the second waypoint as wrapped signed 16.16 coordinates with zero
	// fractional halves.
	int waypointOffset = _state.RoomMovementDataOffset + kRouteWaypointDataOffset + 4;
	int originXFixed = static_cast<int16>(_rom.readInt16(waypointOffset) << 3) << 16;
	int originYFixed = static_cast<int16>(_rom.readInt16(waypointOffset + sizeof(int16))) << 3 << 16;

	// Ghidra 0x0000595E-0x00005961: calculate all three outputs from the staged origin.
	VelocityResult velocity =
		computeCompanionVelocityFromStagedOrigin(originXFixed, originYFixed, targetX, targetY, movementScale);

	// Ghidra 0x00005962-0x00005975: publish the outputs into queue slot one without replacing its target.
	_state.CompanionActorMovementPoints[1].HorizontalStep = velocity.HorizontalStep;
	_state.CompanionActorMovementPoints[1].VerticalStep = velocity.VerticalStep;
	_state.CompanionActorMovementPoints[1].PositionIndex = velocity.PositionIndex;
}

uint16 ScriptedRoomMovementExecutor::classifyCompanionWaypointVisibility() {
	// Ghidra 0x00005976-0x00005985: clear the result and select the first of exactly two waypoint pairs.
	uint16 visibility = 0;
	int waypointOffset = _state.RoomMovementDataOffset + kRouteWaypointDataOffset;
	for (int waypointIndex = 0; waypointIndex < 2; ++waypointIndex) {
		// Ghidra 0x00005986-0x000059A5: preserve the result, bit index, and cursor around the staged trace,
		// then set only the bit whose waypoint has a clear companion-to-waypoint path.
		int16 targetX = static_cast<int16>(_rom.readInt16(waypointOffset) << 3);
		int16 targetY = static_cast<int16>(_rom.readInt16(waypointOffset + sizeof(int16))
										   << 3);
		if (!traceCompanionPathToTarget(targetX, targetY)) {
			visibility |= 1 << waypointIndex;
		}

		// Ghidra 0x000059A6-0x000059AF: advance one four-byte pair and retain both DBF iterations.
		waypointOffset += 2 * static_cast<int>(sizeof(int16));
	}

	return visibility;
}

bool ScriptedRoomMovementExecutor::traceCompanionPathToTarget(int16 targetX, int16 targetY) {
	// Ghidra 0x000059B0-0x000059C7: stage both complete companion coordinates and preserve the original tail
	// branch into the separately owned trace continuation.
	return tracePathFromStagedOrigin(_state.ActorXFixedCoordinates[1], _state.ActorYFixedCoordinates[1],
									 targetX,
									 targetY);
}

ScriptedRoomMovementExecutor::VelocityResult ScriptedRoomMovementExecutor::computeCompanionMovementVelocity(
	int16 targetX, int16 targetY, int16 movementScale) {
	// Ghidra 0x000059C8-0x000059DB: stage both complete companion coordinates and retain the fallthrough into
	// the separately owned velocity calculation with target and scale inputs unchanged.
	return computeCompanionVelocityFromStagedOrigin(_state.ActorXFixedCoordinates[1],
													_state.ActorYFixedCoordinates[1], targetX, targetY,
													movementScale);
}

ScriptedRoomMovementExecutor::VelocityResult
ScriptedRoomMovementExecutor::computeCompanionVelocityFromStagedOrigin(
	int originXFixed, int originYFixed, int16 targetX, int16 targetY, int16 movementScale) {
	// Ghidra 0x000059DC-0x00005A0F: preserve inputs, calculate wrapping word deltas and magnitudes, and classify
	// direction with the original signed comparison of the magnitude words.
	int16 horizontalDelta =
		static_cast<int16>(targetX - static_cast<int16>(originXFixed >> 16));
	int16 verticalDelta = static_cast<int16>(targetY - static_cast<int16>(originYFixed >>
																		  16));
	verticalDelta <<= 1;
	uint16 horizontalMagnitude =
		static_cast<uint16>(horizontalDelta < 0 ? -horizontalDelta : horizontalDelta);
	uint16 verticalMagnitude = static_cast<uint16>(verticalDelta < 0
													   ? -verticalDelta
													   : verticalDelta);
	int16 positionIndex;
	if (static_cast<int16>(verticalMagnitude) >= static_cast<int16>(horizontalMagnitude)) {
		positionIndex = verticalDelta < 0 ? static_cast<int16>(2) : static_cast<int16>(3);
	} else {
		positionIndex = horizontalDelta < 0 ? static_cast<int16>(1) : static_cast<int16>(0);
	}

	// Ghidra 0x00005A10-0x00005A27: shift the scale as one wrapping word and zero-extend both magnitudes before
	// converting them into 24.8 unsigned division numerators.
	uint16 shiftedScale = static_cast<uint16>(movementScale << 8);
	uint32 scaledHorizontalMagnitude = static_cast<uint32>(horizontalMagnitude) << 8;
	uint32 scaledVerticalMagnitude = static_cast<uint32>(verticalMagnitude) << 8;
	int16 horizontalStep;
	int16 verticalStep;

	// Ghidra 0x00005A28-0x00005A3B: select zero-axis branches first; otherwise compare the signed low quotient
	// words produced by both unsigned divisions.
	if (scaledVerticalMagnitude == 0) {
		// Ghidra 0x00005A9C-0x00005AB1: a zero wrapped Y magnitude produces a signed unit X step.
		horizontalStep = horizontalDelta < 0
							 ? static_cast<int16>(-0x100)
							 : static_cast<int16>(0x100);
		verticalStep = 0;
	} else if (scaledHorizontalMagnitude == 0) {
		// Ghidra 0x00005A84-0x00005A9B: a zero wrapped X magnitude produces a signed unit Y step.
		horizontalStep = 0;
		verticalStep = verticalDelta < 0 ? static_cast<int16>(-0x100) : static_cast<int16>(0x100);
	} else {
		uint16 horizontalQuotient =
			divideUnsignedWordPreservingOverflow(scaledHorizontalMagnitude, shiftedScale);
		uint16 verticalQuotient = divideUnsignedWordPreservingOverflow(
			scaledVerticalMagnitude, shiftedScale);
		if (static_cast<int16>(verticalQuotient) > static_cast<int16>(horizontalQuotient)) {
			// Ghidra 0x00005A5E-0x00005A83: normalize X against the dominant Y quotient and restore signs.
			horizontalStep = static_cast<int16>(
				divideUnsignedWordPreservingOverflow(scaledHorizontalMagnitude, verticalQuotient));
			verticalStep = 0x100;
			if (horizontalDelta < 0) {
				horizontalStep = static_cast<int16>(-horizontalStep);
			}

			if (verticalDelta < 0) {
				verticalStep = static_cast<int16>(-verticalStep);
			}
		} else {
			// Ghidra 0x00005A3C-0x00005A5D: normalize Y against the dominant X quotient and restore signs.
			horizontalStep = 0x100;
			verticalStep = static_cast<int16>(
				divideUnsignedWordPreservingOverflow(scaledVerticalMagnitude, horizontalQuotient));
			if (horizontalDelta < 0) {
				horizontalStep = static_cast<int16>(-horizontalStep);
			}

			if (verticalDelta < 0) {
				verticalStep = static_cast<int16>(-verticalStep);
			}
		}
	}

	// Ghidra 0x00005AB2-0x00005AD7: restore the original scale, retain signed-word multiplication and
	// truncation, publish both steps, and return the D4.w/D5.w/D6.w outputs to the route builder.
	horizontalStep *= movementScale;
	verticalStep *= movementScale;
	_state.CompanionActorHorizontalMovementStep = horizontalStep;
	_state.CompanionActorVerticalMovementStep = verticalStep;
	return VelocityResult(horizontalStep, verticalStep, positionIndex);
}

MovementResult ScriptedRoomMovementExecutor::moveCompanionActorDirectly(
	int16 targetX, int16 targetY,
	int16 movementScale,
	int16 animationBias,
	bool resetMovementPointIndex) {
	// Ghidra 0x00005AD8-0x00005ADF: only the full entry resets the companion queue; routed internal entry
	// 0x00005AE0 retains the prepared reverse-order movement points.
	if (resetMovementPointIndex) {
		_state.CompanionActorMovementPointIndex = 0;
	}

	// Ghidra 0x00005AE0-0x00005AEF: install the companion target and calculate this segment's three velocity
	// outputs through the separate original owner.
	_state.ActorTargetXCoordinates[1] = targetX;
	_state.ActorTargetYCoordinates[1] = targetY;
	VelocityResult velocity = computeCompanionMovementVelocity(targetX, targetY, movementScale);
	int16 positionIndex = velocity.PositionIndex;

	// Ghidra 0x00005AF0-0x00005B1F: zero bias uses the authored previous-to-next directional matrix; every
	// nonzero signed bias wraps with the new position before publishing both animation and position.
	_state.ActorAnimationOffsets[1] =
		animationBias == 0
			? ResolveCompanion(_state.ActorPositionIndices[1], positionIndex)
			: static_cast<int16>(positionIndex + animationBias);
	_state.ActorPositionIndices[1] = positionIndex;

	// Ghidra 0x00005B20-0x00005B4B: restart immediately, update at least once, and wait until restart
	// consumption and graphics publication are both complete. Service native callback progress per back edge.
	_state.ActorAnimationDelays[1] = -1;
	_state.ActorAnimationRestartFlags |= kCompanionActorMask;
	while (true) {
		_updateActorAnimationFrames();
		if ((_state.ActorAnimationRestartFlags & kCompanionActorMask) == 0 &&
			(_state.ActorGraphicsReadyFlags & kCompanionActorMask) != 0) {
			break;
		}

		if (!_waitForRoomVerticalBlank()) {
			return MovementResult(false, positionIndex);
		}
	}

	// Ghidra 0x00005B4C-0x00005B55: mark companion movement active and return D6's semantic position.
	_state.ActorMovementFlags |= kCompanionActorMask;
	return MovementResult(true, positionIndex);
}

int ScriptedRoomMovementExecutor::replaceIntegerCoordinate(int fixedCoordinate, int16 integerCoordinate) {
	return (fixedCoordinate & 0x0000FFFF) | (integerCoordinate << 16);
}

int16 ScriptedRoomMovementExecutor::getIntegerCoordinate(int fixedCoordinate) {
	return static_cast<int16>(fixedCoordinate >> 16);
}

int ScriptedRoomMovementExecutor::calculateNormalizedMinorStep(int16 minorMagnitude,
															   int16 majorMagnitude) {
	// The DIVU.W instructions at 0x000053B0 and 0x000053C4 leave the destination unchanged on quotient overflow;
	// the preceding two long shifts make that retained dividend's low word zero.
	uint32 dividend = static_cast<uint32>(static_cast<uint16>(minorMagnitude)) << 16;
	uint16 divisor = static_cast<uint16>(majorMagnitude);
	return dividend >= (static_cast<uint32>(divisor) << 16)
			   ? static_cast<int>(dividend & 0xFFFFu)
			   : static_cast<int>(static_cast<uint16>(dividend / divisor));
}
} // namespace Scooby