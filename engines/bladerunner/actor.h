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

#include "bladerunner/vector.h"

#include "common/rect.h"

namespace BladeRunner {

class ActorClues;
class ActorCombat;
class ActorWalk;
class BladeRunnerEngine;
class BoundingBox;
class MovementTrack;
class View;

class Actor {
	friend class ScriptBase;

	BladeRunnerEngine *_vm;

private:
	BoundingBox   *_bbox;
	Common::Rect   _screenRectangle;
	MovementTrack *_movementTrack;
	ActorWalk     *_walkInfo;
	ActorCombat   *_combatInfo;

	int  _honesty;
	int  _intelligence;
	int  _stability;
	int  _combatAggressiveness;
	int  _goalNumber;
	int *_friendlinessToOther;

	int _currentHP;
	int _maxHP;

	ActorClues *_clues;

	int     _id;
	int     _setId;
	Vector3 _position;
	int     _facing; // [0, 1024)
	int     _targetFacing;
	int     _walkboxId;

	// Flags
	bool _isTargetable;
	bool _isInvisible;
	bool _isImmuneToObstacles;
	bool _inWalkLoop;
	bool _isRetired;
	bool _inCombat;
	bool _isMoving;
	bool _damageAnimIfMoving;

	// Movement
	bool _movementTrackPaused;
	int _movementTrackNextWaypointId;
	int _movementTrackNextDelay; // probably not used
	int _movementTrackNextAngle; // probably not used
	bool _movementTrackNextRunning;

	int _movementTrackWalkingToWaypointId;
	int _movementTrackDelayOnNextWaypoint;

	// Animation
	int _width;
	int _height;
	int _animationMode;
	int _animationModeCombatIdle;
	int _animationModeCombatWalk;
	int _animationModeCombatRun;
	int _fps;
	int _frame_ms;
	int _animationId;
	int _animationFrame;

	int _retiredWidth;
	int _retiredHeight;

	int _timersRemain[7];
	int _timersStart[7];

	float _scale;

	Vector3 _actorSpeed;

public:
	Actor(BladeRunnerEngine *_vm, int actorId);
	~Actor();

	void setup(int actorId);

	void setAtXYZ(const Vector3 &pos, int facing, bool setFacing = true, bool moving = false, bool retired = false);
	void setAtWaypoint(int waypointId, int angle, int unknown, bool retired);

	float getX();
	float getY();
	float getZ();
	void getXYZ(float* x, float* y, float* z);
	int getFacing();
	int getAnimationMode();

	Vector3 getPosition() { return _position; }

	void changeAnimationMode(int animationMode, bool force = false);
	void setFPS(int fps);

	void countdownTimerStart(int timerId, int interval);
	void countdownTimerReset(int timerId);
	int  countdownTimerGetRemainingTime(int timerId);
	void countdownTimersUpdate();
	void countdownTimerUpdate(int timerId);

	void movementTrackNext(bool omitAiScript);
	void movementTrackPause();
	void movementTrackUnpause();
	void movementTrackWaypointReached();

	bool loopWalkToActor(int otherActorId, int destinationOffset, int a3, bool run, bool a5, bool *flagIsRunning);
	bool loopWalkToItem(int itemId, int destinationOffset, int a3, bool run, bool a5, bool *flagIsRunning);
	bool loopWalkToSceneObject(const char *objectName, int destinationOffset, bool a3, bool run, bool a5, bool *flagIsRunning);
	bool loopWalkToWaypoint(int waypointId, int destinationOffset, int a3, bool run, bool a5, bool *flagIsRunning);
	bool loopWalkToXYZ(const Vector3 &destination, int destinationOffset, bool a3, bool run, bool a5, bool *flagIsRunning);

	bool tick(bool forceUpdate, Common::Rect *screenRect);
	bool draw(Common::Rect *screenRect);

	int getSetId();
	void setSetId(int setId);
	BoundingBox *getBoundingBox() const { return _bbox; }
	Common::Rect *getScreenRectangle() { return &_screenRectangle; }
	int getWalkbox() const { return _walkboxId; }
	bool isRetired()const { return _isRetired; }
	bool isTargetable() const { return _isTargetable; }
	void setTargetable(bool targetable);
	bool isImmuneToObstacles() const { return _isImmuneToObstacles; }
	bool inCombat() const { return _inCombat; }
	bool isMoving() const { return _isMoving; }
	void setMoving(bool value) { _isMoving = value; }
	bool inWalkLoop() const { return _inWalkLoop; }
	bool isWalking() const;
	bool isRunning() const;
	void stopWalking(bool value);

	void faceActor(int otherActorId, bool animate);
	void faceObject(const char *objectName, bool animate);
	void faceItem(int itemId, bool animate);
	void faceWaypoint(int waypointId, bool animate);
	void faceXYZ(float x, float y, float z, bool animate);
	void faceCurrentCamera(bool animate);
	void faceHeading(int heading, bool animate);
	void modifyFriendlinessToOther(int otherActorId, signed int change);
	void setFriendlinessToOther(int otherActorId, int friendliness);
	void setHonesty(int honesty);
	void setIntelligence(int intelligence);
	void setStability(int stability);
	void setCombatAggressiveness(int combatAggressiveness);
	void setInvisible(bool isInvisible);
	void setImmunityToObstacles(bool isImmune);
	void modifyCurrentHP(signed int change);
	void modifyMaxHP(signed int change);
	void modifyCombatAggressiveness(signed int change);
	void modifyHonesty(signed int change);
	void modifyIntelligence(signed int change);
	void modifyStability(signed int change);
	void setFlagDamageAnimIfMoving(bool value);
	bool getFlagDamageAnimIfMoving();
	void setHealth(int hp, int maxHp);

	void retire(bool isRetired, int width, int height, int retiredByActorId);

	void combatModeOn(int a2, int a3, int a4, int a5, int combatAnimationMode, int a7, int a8, int a9, int a10, int a11, int a12, int a13, int a14);
	void combatModeOff();

	void setGoal(int goalNumber);
	int getGoal();

	float distanceFromActor(int otherActorId);

	void speechPlay(int sentenceId, bool voiceOver);
	void speechStop();
	bool isSpeeching();

	void addClueToDatabase(int clueId, int unknown, bool clueAcquired, bool unknownFlag, int fromActorId);
	void acquireClue(int clueId, bool unknownFlag, int fromActorId);
	void loseClue(int clueId);
	bool hasClue(int clueId);
	void copyClues(int actorId);

	int soundVolume() const;
	int soundBalance() const;
private:
	void setFacing(int facing, bool halfOrSet = true);
	void setBoundingBox(const Vector3 &position, bool retired);
	float distanceFromView(View* view) const;
	
	bool loopWalk(const Vector3 &destination, int destinationOffset, bool a3, bool run, const Vector3 &start, float a6, float a7, bool a8, bool *isRunning, bool async);
	bool walkTo(bool run, const Vector3 &destination, bool a3);

	bool walkFindU1(const Vector3 &startPosition, const Vector3 &targetPosition, float a3, Vector3 *newDestination);
	bool walkFindU2(Vector3 *newDestination, float targetWidth, int destinationOffset, float targetSize, const Vector3 &startPosition, const Vector3 &targetPosition);
	bool walkToNearestPoint(const Vector3 &destination, float distance);
	//bool walkFindU3(int actorId, Vector3 from, int distance, Vector3 *out);
};

} // End of namespace BladeRunner

#endif
