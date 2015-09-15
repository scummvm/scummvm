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

#include "bladerunner/bladerunner.h"

#include "bladerunner/vector.h"
#include "bladerunner/movement_track.h"
#include "bladerunner/actor_clues.h"
#include "bladerunner/actor_walk.h"
#include "bladerunner/actor_combat.h"

#include "common/rect.h"

namespace BladeRunner {

class BladeRunnerEngine;
class BoundingBox;

class Actor {
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

	ActorClues* _clues;

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
	bool _isRetired;
	bool _inCombat;
	bool _isMoving;
	bool _damageAnimIfMoving;


	// Animation
	int _width;	
	int _height;
	int _animationMode;
	int _combatAnimationMode;
	int _fps;
	int _frame_ms;
	int _animationId;
	int _animationFrame;

	int _retiredWidth;
	int _retiredHeight;


	int _timersRemain[7];
	int _timersBegan[7];

	float _scale;

public:
	Actor(BladeRunnerEngine *_vm, int actorId);
	~Actor();

	void setup(int actorId);

	void set_at_xyz(Vector3 pos, int facing, bool halfOrSet, int unknown, bool retired);
	void set_at_waypoint(int waypointId, int angle, int unknown, bool retired);

	void draw();

	int getSetId();
	void setSetId(int setId);
	BoundingBox* getBoundingBox() { return _bbox; }
	Common::Rect* getScreenRectangle() { return &_screenRectangle; }
	bool isRetired() { return _isRetired; }
	bool isTargetable() { return _isTargetable; }
	bool inCombat() { return _inCombat; }
	bool isMoving() { return _isMoving; }
	void setMoving(bool value) { _isMoving = value; }
	bool isWalking();
	void stopWalking(int value);

	void changeAnimationMode(int animationMode, int force);

	void faceActor(int otherActorId, bool animate);
	void faceObject(char *objectName, bool animate);
	void faceItem(int itemId, bool animate);
	void faceWaypoint(int waypointId, bool animate);
	void faceXYZ(float x, float y, float z, bool animate);
	void faceCurrentCamera(bool animate);
	void faceHeading(int heading, bool animate);
	int getFriendlinessToOther(int otherActorId);
	void modifyFriendlinessToOther(int otherActorId, signed int change);
	void setFriendlinessToOther(int otherActorId, int friendliness);
	void setHonesty(int honesty);
	void setIntelligence(int intelligence);
	void setStability(int stability);
	void setCombatAggressiveness(int combatAggressiveness);
	int getCurrentHP();
	int getMaxHP();
	int getCombatAggressiveness();
	int getHonesty();
	int getIntelligence();
	int getStability();
	void modifyCurrentHP(signed int change);
	void modifyMaxHP(signed int change);
	void modifyCombatAggressiveness(signed int change);
	void modifyHonesty(signed int change);
	void modifyIntelligence(signed int change);
	void modifyStability(signed int change);
	void setFlagDamageAnimIfMoving(bool value);
	bool getFlagDamageAnimIfMoving();


	void retire(bool isRetired, int unknown1, int unknown2, int retiredByActorId);




private:
	void setFacing(int facing, bool halfOrSet);
	void setBoundingBox(Vector3 position, bool retired);

};

} // End of namespace BladeRunner

#endif
