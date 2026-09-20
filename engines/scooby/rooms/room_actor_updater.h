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

#ifndef SCOOBY_ROOM_ACTOR_UPDATER_H
#define SCOOBY_ROOM_ACTOR_UPDATER_H

#include "common/scummsys.h"

#include "common/array.h"
#include "common/func.h"

#include "room_collision_probe.h"
#include "scooby/assets/rom.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/movement/actor_movement_point.h"
#include "scooby/runtime/runtime_state.h"

// Owns room actor movement, path traversal, priority, camera following, and interactive cursor state.

namespace Scooby {

class RoomActorUpdater {
public:
	// Binds the room actor state machine to its ROM tables, logical scene, and action-icon boundary.
	// rom: verified cartridge containing path streams, interface cells, and coordinate data.
	// scene: logical scene replacing direct interface name-table writes.
	// state: shared six-slot actor, input, camera, and interaction state.
	// collisionProbe: shared decoder for room collision cells and their per-pixel masks.
	// drawSelectedActionIcon: distinct original action-icon renderer invoked in mode zero.
	RoomActorUpdater(const ScoobyDooRom &rom, TileScene &scene, RuntimeState &state,
					 const RoomCollisionProbe &collisionProbe, Common::Functor1<int, void> &drawSelectedActionIcon)
		: _rom(rom), _scene(scene), _state(state), _collisionProbe(collisionProbe),
		  _drawSelectedActionIcon(drawSelectedActionIcon) {
	}

	// Services every room actor and the interactive lower interface once.
	//
	// Ghidra: updateRoomActorsAndInterface (0x00005E4A). The complete body is 0x00005E4A-0x000073D5. Actor
	// coordinates remain signed 16.16 values, path streams retain every negative control record, and all
	// active-low input branches remain independent. The two native 10-by-7 interface transfer loops become
	// logical layer writes; collision and direction decoding remain distinct original functions. Work RAM
	// scratch g_dwLeadActorScaleAdjustedStepMagnitude at 0xFF062A-0xFF062D,
	// g_dwLeadActorScaledHorizontalStepScratch at 0xFF0632-0xFF0635, g_dwLeadActorScaledVerticalStepScratch
	// at 0xFF0636-0xFF0639, and the signed 16.16 candidates g_dwLeadActorCandidateXCoordinate and
	// g_dwLeadActorCandidateYCoordinate at 0xFF0642-0xFF0649 all start at zero. This function replaces each
	// consumed value before reading it, so managed code keeps them operation-local.
	void updateRoomActorsAndInterface();

private:
	static const uint8 kActionButtonMask = 0x10;
	static const uint8 kActorAnimationHoldMask = 0x01;
	static const uint8 kActorGraphicsReadyMask = 0x01;
	static const uint8 kCompanionActorMask = 0x02;
	static const int kCompanionActorSlot = 1;
	static const uint8 kCursorDownMask = 0x02;
	static const uint8 kCursorLeftMask = 0x04;
	static const uint8 kCursorRightMask = 0x08;
	static const uint8 kCursorUpMask = 0x01;
	static const uint8 kDirectionMask = 0x0F;
	static const uint8 kDisplayInterfaceMask = 0x08;
	static const uint8 kDisplayRightInterfaceMask = 0x10;
	static const uint8 kDisplayTransitionMask = 0x80;
	static const uint8 kInterfaceTransitionButtonMask = 0x20;
	static const uint8 kInteractionMenuSuppressionMask = 0x04;
	static const uint8 kLeadActorMask = 0x01;
	static const int kLeadActorSlot = 0;
	static const int kLowerInterfaceColumnCount = 10;
	static const int kLowerInterfaceRowCount = 7;
	static const int kLowerInterfaceSourceRowByteStride = 0x80;
	static const int kLowerInterfaceStartColumn = 27;
	static const int kLowerInterfaceStartRow = 21;
	static const int kLowerInterfaceClosingCellsOffset = 0x3034C;
	static const int kLowerInterfaceOpeningCellsOffset = 0x3004C;
	static const uint8 kNewInterfaceButtonMask = 0x40;
	static const uint8 kProgressInterfaceSuppressionMask = 0x02;
	static const uint8 kRoomBehaviorCameraLockMask = 0x02;
	static const uint8 kRoomBehaviorDuelMask = 0x04;
	static const uint8 kRoomBehaviorTransitionScaleMask = 0x08;
	static const int kRoomObjectFirstInteraction = 3;
	static const uint8 kTransitionCenteredCursorMask = 0x40;
	static const uint8 kTransitionCompanionCopyMask = 0x08;
	static const uint8 kTransitionForegroundPolarityMask = 0x01;
	static const uint8 kTransitionHideLeadMask = 0x10;
	static const uint8 kTransitionSkipCompactScaleMask = 0x02;
	static const uint8 kVideoCompanionCopyMask = 0x40;
	static const uint8 kVideoCompanionUpdateBlockMask = 0x04;

	// Replaces the C# (short X, short Y) tuple return of ReadAndAdvancePathRecord.
	struct PathRecord {
		int16 X;
		int16 Y;
	};

	void updateLeadActorPath();
	bool updateInterfaceTransitionAndCursor();
	void advanceRoomInterfaceTransition();
	void publishLowerInterfaceCells(int sourceOffset);
	void updateInteractionCursor();
	void updateMenuCommandFromCursor();
	void closeInteractionInterface();
	void updateLeadActorAndCamera();
	bool updateLeadControlledCandidate(int &candidateX, int &candidateY, uint32 stepMagnitude,
									   int16 maximumX, int16 maximumY);
	void moveLeadLeft(int &candidateX, uint32 stepMagnitude);
	void moveLeadRight(int &candidateX, uint32 stepMagnitude, int16 maximumX);
	bool moveLeadUp(int &candidateY, uint32 movementMagnitude, int16 maximumY,
					bool horizontalDirectionHeld);
	void moveLeadDown(int &candidateY, uint32 movementMagnitude, int16 maximumY,
					  bool horizontalDirectionHeld);
	void updateLeadIdleAnimation();
	void adjustLeadMovementAroundCollision(int &candidateX, int &candidateY, int16 maximumX,
										   int16 maximumY, int16 retainedDataRegister);
	void beginLeadVerticalCollisionAdjustment(int candidateX, int candidateY, int16 movementStep);
	void beginLeadHorizontalCollisionAdjustment(int candidateX, int candidateY, int16 movementStep);
	void advanceLeadScriptedMovement();
	void beginNextLeadMovementPoint(const ActorMovementPoint &point);
	void updateCameraAndInteractionEntry(int16 maximumX, int16 maximumY);
	void updateCameraX(int16 maximumX);
	void updateCameraY(int16 maximumY);
	void tryOpenInteractionInterface();
	void setCenteredCursor();
	void updateCompanionActor();
	void advanceCompanionScriptedMovement();
	void beginNextCompanionMovementPoint(const ActorMovementPoint &point);
	void updateActorPrioritiesAndScales();
	void updateCompactActorScale(int actor, int16 &retainedPreviousScaleRow);
	void updateDynamicActors();
	void advanceDynamicActorMovement(int actor);
	void updateStandardActorPath(int actor);
	void endActorPathAtTerminal(int actor);
	PathRecord readAndAdvancePathRecord(int actor);
	void clearActorPath(int actor);
	void updateActorPriority(int actor);
	void requestActorAnimation(int actor, int16 animationOffset);
	void setActorIntegerCoordinates(int actor, int16 x, int16 y);
	bool isNewPress(uint8 buttonMask) const;
	static int32 scaleMovementStep(uint16 scale, uint16 multiplier,
								   int16 movementStep);
	static uint8 actorMask(int actor);
	static int32 composePackedTargetCoordinate(const Common::Array<int16> &targets, int actor,
											   uint16 followingWord);
	static bool hasFlag(uint8 flags, int actor);
	static int16 getIntegerCoordinate(int32 fixedCoordinate);
	static int32 replaceIntegerCoordinate(int32 fixedCoordinate, int16 integerCoordinate);

	// Classifies active-low directional input after a blocked lead-actor move.
	// retainedDataRegister: incoming D0 word returned unchanged when no direction is pressed.
	// Returns the recovered direction index, or retainedDataRegister with no direction.
	//
	// Ghidra: getControllerDirectionIndex (0x0000750A). The complete body is 0x0000750A-0x00007589. The
	// function reads active-low g_bControllerOneInput at 0xFF09E0. Left has priority over Right, and Up has
	// priority over Down within each horizontal branch; the no-horizontal path retains distinct Up, Down, and
	// no-direction outcomes.
	int16 getControllerDirectionIndex(int16 retainedDataRegister);

	const ScoobyDooRom &_rom;
	TileScene &_scene;
	RuntimeState &_state;
	const RoomCollisionProbe &_collisionProbe;
	Common::Functor1<int, void> &_drawSelectedActionIcon;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_ACTOR_UPDATER_H
