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

#ifndef BLADERUNNER_ACTOR_H
#define BLADERUNNER_ACTOR_H

#include "bladerunner/boundingbox.h"
#include "bladerunner/vector.h"

#include "common/array.h"
#include "common/rect.h"

namespace BladeRunner {

class ActorClues;
class ActorCombat;
class ActorWalk;
class BladeRunnerEngine;
class BoundingBox;
class MovementTrack;
class SaveFileReadStream;
class SaveFileWriteStream;
class View;

class Actor {
	BladeRunnerEngine *_vm;

	static const int kActorTimers = 7;

public:
	BoundingBox    _bbox;
	Common::Rect   _screenRectangle;
	MovementTrack *_movementTrack;
	ActorWalk     *_walkInfo;
	ActorCombat   *_combatInfo;
	ActorClues    *_clues;

private:
	int                _honesty;
	int                _intelligence;
	int                _stability;
	int                _combatAggressiveness;
	int                _goalNumber;
	Common::Array<int> _friendlinessToOther;

	int _currentHP;
	int _maxHP;

	int     _id;
	int     _setId;
	Vector3 _position;
	int     _facing; // [0, 1024)
	int     _targetFacing;
	int     _walkboxId;

	int     _cluesLimit;
	uint32  _timer4RemainDefault;

	// Flags
	bool _isTarget;
	bool _isInvisible;
	bool _isImmuneToObstacles;
	bool _mustReachWalkDestination;
	bool _isRetired;
	bool _inCombat;
	bool _isMoving;
	bool _damageAnimIfMoving;

	// Movement
	bool   _movementTrackPaused;
	int    _movementTrackNextWaypointId;
	int32  _movementTrackNextDelay;  // probably not used
	int    _movementTrackNextAngle;  // fixed: used for AI_Movement_Track_Append_With_Facing - original: probably not used
	bool   _movementTrackNextRunning;

	int    _movementTrackWalkingToWaypointId;
	int32  _movementTrackDelayOnNextWaypoint;

	// Animation
	int _width;
	int _height;
	int _animationMode;
	int _animationModeCombatIdle;
	int _animationModeCombatWalk;
	int _animationModeCombatRun;
	int _fps;
	int _frameMs;
	int _animationId;
	int _animationFrame;

	int _retiredWidth;
	int _retiredHeight;

	uint32 _timersLeft[kActorTimers]; // this keeps time difference, and it is stored during save() (saveInt actually saves a uint32)
	uint32 _timersLast[kActorTimers]; // this keeps actual time, and is not stored during save(), so it can be a uint32

	float _scale;

	Vector3 _actorSpeed;

	int _sitcomRatio;

public:
	Actor(BladeRunnerEngine *_vm, int actorId);
	~Actor();

	void setup(int actorId);

	void setAtXYZ(const Vector3 &pos, int facing, bool setFacing = true, bool moving = false, bool retired = false);
	void setAtWaypoint(int waypointId, int angle, int unknown, bool retired);

	int  getId() const { return _id; };
	float getX() const;
	float getY() const;
	float getZ() const;
	Vector3 getXYZ() const;
	int getFacing() const;
	int getAnimationMode() const;
	int getAnimationId() const;

	Vector3 getPosition() const { return _position; }

	void changeAnimationMode(int animationMode, bool force = false);
	void setFPS(int fps);
	void increaseFPS();

	void   timerStart(int timerId, int32 interval);
	void   timerReset(int timerId);
	uint32 timerLeft(int timerId);
	void   timersUpdate();
	void   timerUpdate(int timerId);

	void movementTrackNext(bool omitAiScript);
	void movementTrackPause();
	void movementTrackUnpause();
	void movementTrackWaypointReached();

	bool loopWalk(const Vector3 &destination, int proximity, bool interruptible, bool runFlag, const Vector3 &start, float targetWidth, float targetSize, bool mustReach, bool *isRunningFlag, bool async);
	bool walkTo(bool runFlag, const Vector3 &destination, bool mustReach);
	bool loopWalkToActor(int otherActorId, int proximity, int interruptible, bool runFlag, bool mustReach, bool *isRunningFlag);
	bool loopWalkToItem(int itemId, int proximity, int interruptible, bool runFlag, bool mustReach, bool *isRunningFlag);
	bool loopWalkToSceneObject(const Common::String &objectName, int proximity, bool interruptible, bool runFlag, bool mustReach, bool *isRunningFlag);
	bool loopWalkToWaypoint(int waypointId, int proximity, int interruptible, bool runFlag, bool mustReach, bool *isRunningFlag);
	bool loopWalkToXYZ(const Vector3 &destination, int proximity, bool interruptible, bool runFlag, bool mustReach, bool *isRunningFlag);
	bool asyncWalkToWaypoint(int waypointId, int proximity, bool runFlag, bool mustReach);
	void asyncWalkToXYZ(const Vector3 &destination, int proximity, bool runFlag, bool mustReach);
	void run();

	bool tick(bool forceUpdate, Common::Rect *screenRect);
	void tickCombat();
	bool draw(Common::Rect *screenRect);

	int getSetId()  const;
	void setSetId(int setId);
	const BoundingBox &getBoundingBox() const { return _bbox; }
	const Common::Rect &getScreenRectangle() { return _screenRectangle; }
	int getWalkbox() const { return _walkboxId; }

	bool isRetired() const { return _isRetired; }
	bool isTarget() const { return _isTarget; }
	void setTarget(bool targetable);
	bool isImmuneToObstacles() const { return _isImmuneToObstacles; }
	bool inCombat() const { return _inCombat; }

	bool isMoving() const { return _isMoving; }
	void setMoving(bool value) { _isMoving = value; }

	bool mustReachWalkDestination() const { return _mustReachWalkDestination; }
	bool isWalking() const;
	bool isRunning() const;
	void stopWalking(bool value);

	void faceActor(int otherActorId, bool animate);
	void faceObject(const Common::String &objectName, bool animate);
	void faceItem(int itemId, bool animate);
	void faceWaypoint(int waypointId, bool animate);
	void faceXYZ(float x, float y, float z, bool animate);
	void faceXYZ(const Vector3 &pos, bool animate);
	void faceCurrentCamera(bool animate);
	void faceHeading(int heading, bool animate);
	void setFacing(int facing, bool halfOrSet = true);

	int getCurrentHP() const { return _currentHP; }
	int getMaxHP() const { return _maxHP; }
	void setCurrentHP(int hp);
	void setHealth(int hp, int maxHp);
	void modifyCurrentHP(signed int change);
	void modifyMaxHP(signed int change);

	int getFriendlinessToOther(int otherActorId) const { return _friendlinessToOther[otherActorId]; }
	void setFriendlinessToOther(int otherActorId, int friendliness);
	void modifyFriendlinessToOther(int otherActorId, signed int change);
	bool checkFriendlinessAndHonesty(int otherActorId);

	int getHonesty() const { return _honesty; }
	void setHonesty(int honesty);
	void modifyHonesty(signed int change);

	int getIntelligence() const { return _intelligence; }
	void setIntelligence(int intelligence);
	void modifyIntelligence(signed int change);

	int getStability() const { return _stability; }
	void setStability(int stability);
	void modifyStability(signed int change);

	int getCombatAggressiveness() const { return _combatAggressiveness; }
	void setCombatAggressiveness(int combatAggressiveness);
	void modifyCombatAggressiveness(signed int change);

	void setInvisible(bool isInvisible);
	void setImmunityToObstacles(bool isImmune);

	void setFlagDamageAnimIfMoving(bool value);
	bool getFlagDamageAnimIfMoving() const;

	int getSitcomRatio() const;

	void retire(bool isRetired, int width, int height, int retiredByActorId);

	void combatModeOn(int initialState, bool rangedAttack, int enemyId, int waypointType, int animationModeCombatIdle, int animationModeCombatWalk, int animationModeCombatRun, int fleeRatio, int coverRatio, int attackRatio, int damage, int range, bool unstoppable);
	void combatModeOff();

	void setGoal(int goalNumber);
	int getGoal() const;

	float distanceFromActor(int otherActorId);
	int angleTo(const Vector3 &target) const;

	void speechPlay(int sentenceId, bool voiceOver);
	void speechStop();
	bool isSpeeching();

	void addClueToDatabase(int clueId, int unknown, bool clueAcquired, bool unknownFlag, int fromActorId);
	bool canAcquireClue(int clueId) const;
	void acquireClue(int clueId, bool unknownFlag, int fromActorId);
	void loseClue(int clueId);
	bool hasClue(int clueId) const;
	bool copyClues(int actorId);
	void acquireCluesByRelations();

	int soundVolume() const;
	int soundPan() const;

	bool isObstacleBetween(const Vector3 &target);

	void save(SaveFileWriteStream &f);
	void load(SaveFileReadStream &f);

	static int findTargetUnderMouse(BladeRunnerEngine *vm, int mouseX, int mouseY);

private:
	void setBoundingBox(const Vector3 &position, bool retired);
	float distanceFromView(View *view) const;

	bool findEmptyPositionAround(const Vector3 &startPosition, const Vector3 &targetPosition, float size, Vector3 *emptyPosition);
	bool findNearestPosition(Vector3 *nearestPosition, float targetWidth, int proximity, float targetSize, const Vector3 &startPosition, const Vector3 &targetPosition);
	bool stepAway(const Vector3 &destination, float distance);
	//bool walkFindU3(int actorId, Vector3 from, int distance, Vector3 *out);
};

} // End of namespace BladeRunner

#endif
