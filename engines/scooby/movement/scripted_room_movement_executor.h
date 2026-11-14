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

#ifndef SCOOBY_SCRIPTED_ROOM_MOVEMENT_EXECUTOR_H
#define SCOOBY_SCRIPTED_ROOM_MOVEMENT_EXECUTOR_H

#include "common/scummsys.h"

#include "common/func.h"

#include "scooby/assets/rom.h"
#include "scooby/rooms/actor_animator.h"
#include "scooby/rooms/collision_probe.h"
#include "scooby/runtime/runtime_state.h"

// Owns the original actor and room-object movement boundaries shared by room-script opcodes.

namespace Scooby {

struct MovementResult {
	bool completed;
	int16 positionIndex;

	MovementResult() : completed(false), positionIndex(0) {
	}

	MovementResult(bool newCompleted, int16 newPositionIndex) : completed(newCompleted),
														        positionIndex(newPositionIndex) {
	}
};

class ScriptedRoomMovementExecutor {
public:
	// Binds scripted movement to shared actor state and the callback that advances native waits.
	// rom: verified cartridge containing the active room's immutable route waypoints.
	// state: shared room-object, actor-coordinate, animation, and movement state.
	// collisionProbe: shared owner of decoded room collision queries.
	// actorAnimator: owner of the recovered actor-animation update used by synchronous waits.
	// waitForRoomVerticalBlank: callback-aware clocked frame that publishes and advances movement.
	ScriptedRoomMovementExecutor(const ScoobyDooRom &rom, RuntimeState &state,
								 const RoomCollisionProbe &collisionProbe,
								 RoomActorAnimator &actorAnimator,
								 Common::Functor0<bool> &waitForRoomVerticalBlank)
		: _rom(rom), _state(state), _collisionProbe(collisionProbe),
		  _actorAnimator(actorAnimator),
		  _waitForRoomVerticalBlank(waitForRoomVerticalBlank) {
	}

	// Moves one room object and its attached dynamic actor to a scripted target.
	// objectId: one-based object identity after the caller masks encoded bit 15.
	// targetX/targetY: signed target coordinates written to the mutable room-object record.
	// movementMode: negative places directly; nonnegative scales and starts dynamic movement.
	// animationOffset: negative retains animation; nonnegative selects and restarts it.
	// encodedObjectId: original command identity whose sign suppresses movement waiting.
	// Returns false when host closure interrupts a synchronous movement wait.
	//
	// Ghidra: moveRoomObjectToPosition (0x00004B24). The complete body is 0x00004B24-0x00004C1F. Mutable
	// record fields +0x12/+0x14 are replaced before the signed attached-actor guard. Native VBlank advances
	// graphics publication and movement during the two busy waits; the managed room callback and shared frame
	// clock supply those updates explicitly.
	bool moveRoomObjectToPosition(uint16 objectId, int16 targetX, int16 targetY,
								  int16 movementMode, int16 animationOffset,
								  uint16 encodedObjectId);

	// Chooses and starts a direct or waypoint-routed lead-actor movement.
	// targetX/targetY: signed final coordinates in room pixels.
	// movementScale: signed movement multiplier forwarded to each selected route segment.
	// animationBias: zero derives a directional animation; nonzero biases the selected position.
	// encodedObjectId: original command identity whose sign suppresses completion waiting.
	// Returns false when host closure interrupts a callback-aware movement wait.
	//
	// Ghidra: moveLeadActorToRoomPosition (0x00005002). The complete body is 0x00005002-0x0000519B. The two
	// signed waypoint pairs follow the eight-entry wander section in g_pRoomMovementData. Visibility bits
	// independently describe clear actor-to-waypoint paths; every clear/blocked waypoint-to-target combination
	// is preserved before the direct-movement tail.
	bool moveShaggyActorToRoomPosition(int16 targetX, int16 targetY, int16 movementScale,
									 int16 animationBias, uint16 encodedObjectId);

	// Starts a companion move and optionally waits through movement and its final animation.
	// targetX/targetY: signed final coordinates in room pixels.
	// movementMode: signed movement mode or scale forwarded to route selection.
	// destinationBias: animation or destination bias forwarded to route selection.
	// encodedObjectId: a negative encoded word suppresses completion waiting.
	// Returns false when host closure interrupts one of the clocked waits.
	//
	// Ghidra: finishCompanionMovement (0x0000566A). The complete body is 0x0000566A-0x000056AF. Actual
	// movement paths retain the directional position in native D6 across the movement wait; the managed result
	// preserves that semantic value without rereading callback-mutated actor state. The callee's same-coordinate
	// return leaves incidental caller residue in D6, which is deliberately not represented as managed state.
	// Both busy loops update animations at least once and service the installed room callback on every
	// continuing back edge.
	bool finishScoobyMovement(int16 targetX, int16 targetY, int16 movementMode,
								 int16 destinationBias, uint16 encodedObjectId);

	// Chooses and starts a direct or waypoint-routed companion-actor movement.
	// targetX/targetY: signed final coordinates in room pixels.
	// movementScale: signed movement multiplier forwarded to each selected route segment.
	// animationBias: zero derives a directional animation; nonzero biases the selected position.
	// Returns the host-completion state and semantic directional position selected for the first segment.
	//
	// Ghidra: movePlayerToRoomPosition (0x000056B0). The complete body is 0x000056B0-0x00005851. Both signed
	// waypoint pairs follow the eight wander entries in g_pRoomMovementData. Route points are queued in
	// reverse consumption order under g_nCompanionActorMovementPointIndex at 0xFF0670. Actual movement paths
	// return native D6's directional position; the same-coordinate path's incidental incoming D6 is replaced by
	// the current typed companion position rather than introducing register-state emulation.
	MovementResult moveScoobyToRoomPosition(int16 targetX, int16 targetY,
											int16 movementScale,
											int16 animationBias);

	// Starts one companion movement segment and waits for its initial graphics publication.
	// targetX/targetY: signed target integer coordinates.
	// movementScale: signed scale forwarded to the companion velocity calculation.
	// animationBias: zero selects the directional matrix; nonzero offsets the new position.
	// resetMovementPointIndex: selects full entry 0x5AD8; otherwise enters at 0x5AE0.
	// Returns the host-completion state and semantic directional position selected for the segment.
	//
	// Ghidra: moveCompanionActorDirectly (0x00005AD8). The complete body is 0x00005AD8-0x00005B55. Seven
	// route-selection xrefs and one room-script xref use the full or routed entry. ComputeCompanionMovementVelocity
	// remains a separate original function boundary; UpdateActorAnimationFrames is implemented original
	// behavior. The native graphics wait advances animation at least once; every continuing managed back edge
	// services the installed room callback through the sole 60 Hz frame owner. Host closure is the managed
	// non-returning handover.
	MovementResult moveScoobyActorDirectly(int16 targetX, int16 targetY,
											  int16 movementScale,
											  int16 animationBias, bool resetMovementPointIndex);

private:
	static const int kRouteWaypointDataOffset = 0x20;

	struct VelocityResult {
		int16 horizontalStep;
		int16 verticalStep;
		int16 positionIndex;

		VelocityResult() : horizontalStep(0), verticalStep(0), positionIndex(0) {
		}

		VelocityResult(int16 newHorizontalStep, int16 newVerticalStep, int16 newPositionIndex)
			: horizontalStep(newHorizontalStep), verticalStep(newVerticalStep), positionIndex(newPositionIndex) {
		}
	};

	void buildShaggyRouteFromFirstWaypoint(int16 targetX, int16 targetY,
										 int16 movementScale);
	void buildShaggyRouteFromSecondWaypoint(int16 targetX, int16 targetY,
										  int16 movementScale);
	void appendShaggyRouteFromFirstWaypoint(int16 targetX, int16 targetY,
										  int16 movementScale);
	void appendShaggyRouteFromSecondWaypoint(int16 targetX, int16 targetY,
										   int16 movementScale);

	VelocityResult computeMovementVelocityFromStagedOrigin(int originXFixed, int originYFixed,
														   int16 targetX,
														   int16 targetY, int16 movementScale);

	static uint16 divideUnsignedWordPreservingOverflow(uint32 dividend, uint16 divisor);

	bool tracePathFromFirstWaypoint(int16 targetX, int16 targetY);
	bool tracePathFromSecondWaypoint(int16 targetX, int16 targetY);
	uint16 classifyShaggyWaypointVisibility();
	bool traceShaggyPathToTarget(int16 targetX, int16 targetY);
	bool tracePathFromStagedOrigin(int originXFixed, int originYFixed, int16 targetX,
								   int16 targetY);
	VelocityResult computeShaggyMovementVelocity(int16 targetX, int16 targetY,
											   int16 movementScale);
	bool moveShaggyActorDirectly(int16 targetX, int16 targetY, int16 movementScale,
							   int16 animationBias, uint16 encodedObjectId,
							   bool resetMovementPointIndex);

	void buildScoobyRouteFromFirstWaypoint(int16 targetX, int16 targetY,
											  int16 movementScale);
	void buildScoobyRouteFromSecondWaypoint(int16 targetX, int16 targetY,
											   int16 movementScale);
	void appendScoobyRouteFromFirstWaypoint(int16 targetX, int16 targetY,
											   int16 movementScale);
	void appendScoobyRouteFromSecondWaypoint(int16 targetX, int16 targetY,
												int16 movementScale);
	uint16 classifyScoobyWaypointVisibility();
	bool traceScoobyPathToTarget(int16 targetX, int16 targetY);
	VelocityResult computeScoobyMovementVelocity(int16 targetX, int16 targetY,
													int16 movementScale);
	VelocityResult computeScoobyVelocityFromStagedOrigin(int originXFixed, int originYFixed,
															int16 targetX,
															int16 targetY, int16 movementScale);

	static int replaceIntegerCoordinate(int fixedCoordinate, int16 integerCoordinate);
	static int16 getIntegerCoordinate(int fixedCoordinate);
	static int calculateNormalizedMinorStep(int16 minorMagnitude, int16 majorMagnitude);

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
	const RoomCollisionProbe &_collisionProbe;
	RoomActorAnimator &_actorAnimator;
	Common::Functor0<bool> &_waitForRoomVerticalBlank;
};
} // namespace Scooby

#endif // SCOOBY_SCRIPTED_ROOM_MOVEMENT_EXECUTOR_H
