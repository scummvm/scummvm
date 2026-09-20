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
#include "scooby/rooms/room_collision_probe.h"
#include "scooby/runtime/runtime_state.h"

// Owns the original actor and room-object movement boundaries shared by room-script opcodes.

namespace Scooby {

// Replaces the C# (bool Completed, short PositionIndex) tuple return.
struct MovementResult {
	bool Completed;
	int16 PositionIndex;

	MovementResult() : Completed(false), PositionIndex(0) {
	}

	MovementResult(bool completed, int16 positionIndex) : Completed(completed),
														  PositionIndex(positionIndex) {
	}
};

class ScriptedRoomMovementExecutor {
public:
	// Binds scripted movement to shared actor state and the callback that advances native waits.
	// rom: verified cartridge containing the active room's immutable route waypoints.
	// state: shared room-object, actor-coordinate, animation, and movement state.
	// collisionProbe: shared owner of decoded room collision queries.
	// updateActorAnimationFrames: recovered actor-animation update used by synchronous waits.
	// waitForRoomVerticalBlank: callback-aware clocked frame that publishes and advances movement.
	ScriptedRoomMovementExecutor(const ScoobyDooRom &rom, RuntimeState &state,
								 const RoomCollisionProbe &collisionProbe,
								 Common::Functor0<void> &updateActorAnimationFrames,
								 Common::Functor0<bool> &waitForRoomVerticalBlank)
		: _rom(rom), _state(state), _collisionProbe(collisionProbe),
		  _updateActorAnimationFrames(updateActorAnimationFrames),
		  _waitForRoomVerticalBlank(waitForRoomVerticalBlank) {
	}

	// Moves one room object and its attached dynamic actor to a scripted target.
	// objectIdentity: one-based object identity after the caller masks encoded bit 15.
	// targetX/targetY: signed target coordinates written to the mutable room-object record.
	// movementMode: negative places directly; nonnegative scales and starts dynamic movement.
	// animationOffset: negative retains animation; nonnegative selects and restarts it.
	// encodedObjectIdentity: original command identity whose sign suppresses movement waiting.
	// Returns false when host closure interrupts a synchronous movement wait.
	//
	// Ghidra: moveRoomObjectToPosition (0x00004B24). The complete body is 0x00004B24-0x00004C1F. Mutable
	// record fields +0x12/+0x14 are replaced before the signed attached-actor guard. Native VBlank advances
	// graphics publication and movement during the two busy waits; the managed room callback and shared frame
	// clock supply those updates explicitly.
	bool moveRoomObjectToPosition(uint16 objectIdentity, int16 targetX, int16 targetY,
								  int16 movementMode, int16 animationOffset,
								  uint16 encodedObjectIdentity);

	// Chooses and starts a direct or waypoint-routed lead-actor movement.
	// targetX/targetY: signed final coordinates in room pixels.
	// movementScale: signed movement multiplier forwarded to each selected route segment.
	// animationBias: zero derives a directional animation; nonzero biases the selected position.
	// encodedObjectIdentity: original command identity whose sign suppresses completion waiting.
	// Returns false when host closure interrupts a callback-aware movement wait.
	//
	// Ghidra: moveLeadActorToRoomPosition (0x00005002). The complete body is 0x00005002-0x0000519B. The two
	// signed waypoint pairs follow the eight-entry ambient section in g_pRoomMovementData. Visibility bits
	// independently describe clear actor-to-waypoint paths; every clear/blocked waypoint-to-target combination
	// is preserved before the direct-movement tail.
	bool moveLeadActorToRoomPosition(int16 targetX, int16 targetY, int16 movementScale,
									 int16 animationBias, uint16 encodedObjectIdentity);

	// Starts a companion move and optionally waits through movement and its final animation.
	// targetX/targetY: signed final coordinates in room pixels.
	// movementMode: signed movement mode or scale forwarded to route selection.
	// destinationBias: animation or destination bias forwarded to route selection.
	// encodedObjectIdentity: a negative encoded word suppresses completion waiting.
	// Returns false when host closure interrupts one of the clocked waits.
	//
	// Ghidra: finishCompanionMovement (0x0000566A). The complete body is 0x0000566A-0x000056AF. Actual
	// movement paths retain the directional position in native D6 across the movement wait; the managed result
	// preserves that semantic value without rereading callback-mutated actor state. The callee's same-coordinate
	// return leaves incidental caller residue in D6, which is deliberately not represented as managed state.
	// Both busy loops update animations at least once and service the installed room callback on every
	// continuing back edge.
	bool finishCompanionMovement(int16 targetX, int16 targetY, int16 movementMode,
								 int16 destinationBias, uint16 encodedObjectIdentity);

	// Chooses and starts a direct or waypoint-routed companion-actor movement.
	// targetX/targetY: signed final coordinates in room pixels.
	// movementScale: signed movement multiplier forwarded to each selected route segment.
	// animationBias: zero derives a directional animation; nonzero biases the selected position.
	// Returns the host-completion state and semantic directional position selected for the first segment.
	//
	// Ghidra: movePlayerToRoomPosition (0x000056B0). The complete body is 0x000056B0-0x00005851. Both signed
	// waypoint pairs follow the eight ambient entries in g_pRoomMovementData. Route points are queued in
	// reverse consumption order under g_nCompanionActorMovementPointIndex at 0xFF0670. Actual movement paths
	// return native D6's directional position; the same-coordinate path's incidental incoming D6 is replaced by
	// the current typed companion position rather than introducing register-state emulation.
	MovementResult movePlayerToRoomPosition(int16 targetX, int16 targetY,
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
	MovementResult moveCompanionActorDirectly(int16 targetX, int16 targetY,
											  int16 movementScale,
											  int16 animationBias, bool resetMovementPointIndex);

private:
	static const uint8 kCompanionActorMask = 0x02;
	static const uint8 kLeadActorMask = 0x01;
	static const int kRouteWaypointDataOffset = 0x20;

	// Small named replacement for the C# private (short HorizontalStep, short VerticalStep, short PositionIndex)
	// tuple returned by the two staged-origin velocity calculators.
	struct VelocityResult {
		int16 HorizontalStep;
		int16 VerticalStep;
		int16 PositionIndex;

		VelocityResult() : HorizontalStep(0), VerticalStep(0), PositionIndex(0) {
		}

		VelocityResult(int16 horizontalStep, int16 verticalStep, int16 positionIndex)
			: HorizontalStep(horizontalStep), VerticalStep(verticalStep), PositionIndex(positionIndex) {
		}
	};

	void buildLeadRouteFromFirstWaypoint(int16 targetX, int16 targetY,
										 int16 movementScale);
	void buildLeadRouteFromSecondWaypoint(int16 targetX, int16 targetY,
										  int16 movementScale);
	void appendLeadRouteFromFirstWaypoint(int16 targetX, int16 targetY,
										  int16 movementScale);
	void appendLeadRouteFromSecondWaypoint(int16 targetX, int16 targetY,
										   int16 movementScale);

	VelocityResult computeMovementVelocityFromStagedOrigin(int originXFixed, int originYFixed,
														   int16 targetX,
														   int16 targetY, int16 movementScale);

	static uint16 divideUnsignedWordPreservingOverflow(uint32 dividend, uint16 divisor);

	bool tracePathFromFirstWaypoint(int16 targetX, int16 targetY);
	bool tracePathFromSecondWaypoint(int16 targetX, int16 targetY);
	uint16 classifyLeadWaypointVisibility();
	bool traceLeadPathToTarget(int16 targetX, int16 targetY);
	bool tracePathFromStagedOrigin(int originXFixed, int originYFixed, int16 targetX,
								   int16 targetY);
	VelocityResult computeLeadMovementVelocity(int16 targetX, int16 targetY,
											   int16 movementScale);
	bool moveLeadActorDirectly(int16 targetX, int16 targetY, int16 movementScale,
							   int16 animationBias, uint16 encodedObjectIdentity,
							   bool resetMovementPointIndex);

	void buildCompanionRouteFromFirstWaypoint(int16 targetX, int16 targetY,
											  int16 movementScale);
	void buildCompanionRouteFromSecondWaypoint(int16 targetX, int16 targetY,
											   int16 movementScale);
	void appendCompanionRouteFromFirstWaypoint(int16 targetX, int16 targetY,
											   int16 movementScale);
	void appendCompanionRouteFromSecondWaypoint(int16 targetX, int16 targetY,
												int16 movementScale);
	uint16 classifyCompanionWaypointVisibility();
	bool traceCompanionPathToTarget(int16 targetX, int16 targetY);
	VelocityResult computeCompanionMovementVelocity(int16 targetX, int16 targetY,
													int16 movementScale);
	VelocityResult computeCompanionVelocityFromStagedOrigin(int originXFixed, int originYFixed,
															int16 targetX,
															int16 targetY, int16 movementScale);

	static int replaceIntegerCoordinate(int fixedCoordinate, int16 integerCoordinate);
	static int16 getIntegerCoordinate(int fixedCoordinate);
	static int calculateNormalizedMinorStep(int16 minorMagnitude, int16 majorMagnitude);

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
	const RoomCollisionProbe &_collisionProbe;
	Common::Functor0<void> &_updateActorAnimationFrames;
	Common::Functor0<bool> &_waitForRoomVerticalBlank;
};
} // namespace Scooby

#endif // SCOOBY_SCRIPTED_ROOM_MOVEMENT_EXECUTOR_H
