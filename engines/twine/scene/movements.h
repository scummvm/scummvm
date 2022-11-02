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

#ifndef TWINE_SCENE_MOVEMENTS_H
#define TWINE_SCENE_MOVEMENTS_H

#include "common/scummsys.h"
#include "twine/shared.h"

namespace TwinE {

class TwinEEngine;
class ActorStruct;
struct ActorMoveStruct;

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
	int16 _heroActionKey = 0;
	int32 _previousLoopActionKey = 0;
	// cursor keys
	ChangedCursorKeys _changedCursorKeys;
	ChangedCursorKeys _previousChangedCursorKeys;

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
	void processManualMovementExecution(int actorIdx);
	void processManualRotationExecution(int actorIdx);

	bool _heroAction = false;

public:
	Movements(TwinEEngine *engine);

	void update();

	/**
	 * Hero executes the current action of the trigger zone
	 */
	bool shouldTriggerZoneAction() const;

	bool _lastJoyFlag = false;

	int32 _targetActorDistance = 0;

	/**
	 * Get shadow position
	 * @param pos Shadow coordinates
	 */
	IVec3 getShadowPosition(const IVec3 &pos);

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

	inline int32 getAngleAndSetTargetActorDistance(const IVec3& v1, const IVec3 &v2) {
		return getAngleAndSetTargetActorDistance(v1.x, v1.z, v2.x, v2.z);
	}

	/**
	 * Rotate actor with a given angle
	 * @param x Actor current X coordinate
	 * @param z Actor current Z coordinate
	 * @param angle Actor angle to rotate
	 */
	IVec3 rotateActor(int32 x, int32 z, int32 angle);

	/**
	 * Move actor around the scene
	 * @param start Current actor angle
	 * @param end Angle to rotate
	 * @param duration Rotate speed
	 * @param movePtr Pointer to process movements
	 */
	void initRealAngleConst(int32 start, int32 end, int32 duration, ActorMoveStruct *movePtr) const;

	void processActorMovements(int32 actorIdx);
};

inline bool Movements::shouldTriggerZoneAction() const {
	return _heroAction;
}

} // namespace TwinE

#endif
