/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TWINE_MOVEMENTS_H
#define TWINE_MOVEMENTS_H

#include "common/scummsys.h"
#include "twine/scene/actor.h"

namespace TwinE {

class TwinEEngine;

class Movements {
private:
	TwinEEngine *_engine;

	struct ChangedCursorKeys {
		uint8 forwardChange = 0;
		uint8 backwardChange = 0;
		uint8 leftChange = 0;
		uint8 rightChange = 0;
		uint8 forwardDown = 0;
		uint8 backwardDown = 0;
		uint8 leftDown = 0;
		uint8 rightDown = 0;

		void update(TwinEEngine *engine);

		inline bool operator==(const ChangedCursorKeys &rhs) const {
			return forwardChange == rhs.forwardChange && backwardChange == rhs.backwardChange && leftChange == rhs.leftChange && rightChange == rhs.rightChange;
		}

		inline operator bool() const {
			return forwardChange && backwardChange && leftChange && rightChange;
		}

		inline bool operator!=(const ChangedCursorKeys &rhs) const {
			return forwardChange != rhs.forwardChange || backwardChange != rhs.backwardChange || leftChange != rhs.leftChange || rightChange != rhs.rightChange;
		}
	};

	// enter, space, ...
	int16 heroActionKey = 0;
	int32 previousLoopActionKey = 0;
	// cursor keys
	ChangedCursorKeys changedCursorKeys;
	ChangedCursorKeys previousChangedCursorKeys;

	/**
	 * The Actor is controlled by the player. This works well only for the Hero Actor in general.
	 * To use it for other Actors they would have to have necessary animations that would be also
	 * correctly indexed. The primary purpose for this mode is to re-enable player's control over
	 * the Hero after it has been disabled for some reasons.
	 */
	void processManualAction(int actorIdx);
	/**
	 * The Actor tries to move towards the target Actor. This only means that it will always face
	 * in its direction (as fast as the Rotation delay property allows). To make it really follow
	 * anything it must be assigned a moving Animation first, and the Actor will not stop by itself
	 * after reaching the target. To make a real following, the Actor's animation must be changed
	 * for example to standing animation when the Actor is near the target, and changed back to a
	 * moving animation when it's far from it. The Follow mode handles only the facing angle.
	 */
	void processFollowAction(int actorIdx);
	/**
	 * Makes the Actor walk and turn by random angles and at random moments. In original game it is
	 * only used for Nitro-Mecha-Penguins, but it can be used for any 3-D Actor that has standing
	 * and walking animation (with virtual indexes 0 and 1 respectively). This mode requires the
	 * Randomize interval (Info1) property to be less or equal to 117, otherwise the Actor will just
	 * walk without turning. Exact meaning of the property is not known.
	 */
	void processRandomAction(int actorIdx);
	/**
	 * The Actor's Track Script is run from the first command, and when it reaches END or STOP it
	 * starts over again.
	 */
	void processTrackAction(int actorIdx);
	/**
	 * This mode is used to make an Actor follow specified Actor's X and Z (horizontal) coordinates.
	 * This is mainly used for Sprite Actors to be always above other Sprite Actors (like platforms).
	 * Unlike the Follow mode, this mode sets the Actor's position. If the Actor is a Sprite Actor,
	 * its speed is not taken into consideration in this mode.
	 */
	void processSameXZAction(int actorIdx);

	/**
	 * @return A value of @c true means that the actor should e.g. start reading a sign or checking
	 * a locker for loot or secrets
	 */
	bool processBehaviourExecution(int actorIdx);
	bool processAttackExecution(int actorIdx);
	void processMovementExecution(int actorIdx);
	void processRotationExecution(int actorIdx);

	bool heroAction = false;

	/**
	 * @brief This is a bitmask of 4 bits that is changed whenever a cursor key has changed. A set bit
	 * does not mean that the cursor is pressed - but that a change has happened in this particular frame
	 *
	 * @note This value is reset with every single call to @c readKeys()
	 */
	uint8 cursorKeyMask = 0;

public:
	Movements(TwinEEngine *engine);

	void update();

	/**
	 * Hero executes the current action of the trigger zone
	 */
	bool shouldTriggerZoneAction() const;

	/** Hero moved */
	bool heroMoved = false; // twinsenMove

	/** Process actor.x coordinate */
	int16 processActorX = 0;
	/** Process actor.y coordinate */
	int16 processActorY = 0;
	/** Process actor.z coordinate */
	int16 processActorZ = 0;

	/** Previous process actor.x coordinate */
	int16 previousActorX = 0; // processActorVar2
	/** Previous process actor.y coordinate */
	int16 previousActorY = 0; // processActorVar3
	/** Previous process actor.z coordinate */
	int16 previousActorZ = 0; // processActorVar4

	int32 targetActorDistance = 0; // DoTrackVar1

	/**
	 * Get shadow position
	 * @param X Shadow X coordinate
	 * @param Y Shadow Y coordinate
	 * @param Z Shadow Z coordinate
	 */
	void getShadowPosition(int32 X, int32 Y, int32 Z);

	/**
	 * Set actor safe angle
	 * @param startAngle start angle
	 * @param endAngle end angle
	 * @param stepAngle number of steps
	 * @param movePtr time pointer to update
	 */
	void setActorAngleSafe(int16 startAngle, int16 endAngle, int16 stepAngle, ActorMoveStruct *movePtr);

	/**
	 * Clear actors safe angle
	 * @param actorPtr actor pointer
	 */
	void clearRealAngle(ActorStruct *actorPtr);

	/**
	 * Set actor safe angle
	 * @param startAngle start angle
	 * @param endAngle end angle
	 * @param stepAngle number of steps
	 * @param movePtr time pointer to update
	 */
	void setActorAngle(int16 startAngle, int16 endAngle, int16 stepAngle, ActorMoveStruct *movePtr);

	/**
	 * Get actor angle
	 * @param x1 Actor 1 X
	 * @param z1 Actor 1 Z
	 * @param x2 Actor 2 X
	 * @param z2 Actor 2 Z
	 */
	int32 getAngleAndSetTargetActorDistance(int32 x1, int32 z1, int32 x2, int32 z2);

	/**
	 * Rotate actor with a given angle
	 * @param x Actor current X coordinate
	 * @param z Actor current Z coordinate
	 * @param angle Actor angle to rotate
	 */
	void rotateActor(int32 x, int32 z, int32 angle);

	/**
	 * Get distance value in 2D
	 * @param x1 Actor 1 X coordinate
	 * @param z1 Actor 1 Z coordinate
	 * @param x2 Actor 2 X coordinate
	 * @param z2 Actor 2 Z coordinate
	 */
	int32 getDistance2D(int32 x1, int32 z1, int32 x2, int32 z2);

	/**
	 * Get distance value in 3D
	 * @param x1 Actor 1 X coordinate
	 * @param y1 Actor 1 Y coordinate
	 * @param z1 Actor 1 Z coordinate
	 * @param x2 Actor 2 X coordinate
	 * @param y2 Actor 2 Y coordinate
	 * @param z2 Actor 2 Z coordinate
	 */
	int32 getDistance3D(int32 x1, int32 y1, int32 z1, int32 x2, int32 y2, int32 z2);

	/**
	 * Move actor around the scene
	 * @param angleFrom Current actor angle
	 * @param angleTo Angle to rotate
	 * @param speed Rotate speed
	 * @param movePtr Pointer to process movements
	 */
	void moveActor(int32 angleFrom, int32 angleTo, int32 speed, ActorMoveStruct *movePtr);

	void processActorMovements(int32 actorIdx);
};

inline bool Movements::shouldTriggerZoneAction() const {
	return heroAction;
}

} // namespace TwinE

#endif
