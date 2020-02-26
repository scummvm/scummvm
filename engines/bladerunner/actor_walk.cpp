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

#include "bladerunner/actor_walk.h"

#include "bladerunner/bladerunner.h"

#include "bladerunner/actor.h"
#include "bladerunner/game_constants.h"
#include "bladerunner/game_info.h"
#include "bladerunner/obstacles.h"
#include "bladerunner/savefile.h"
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/set.h"

namespace BladeRunner {

ActorWalk::ActorWalk(BladeRunnerEngine *vm) {
	_vm = vm;

	reset();
}

ActorWalk::~ActorWalk() {}

// added method for bug fix (bad new game state for player actor) and better management of object
void ActorWalk::reset() {
	_walking = false;
	_running = false;
	_facing = -1;
	_status = 0;

	_destination = Vector3(0.0f, 0.0f, 0.0f);
	_originalDestination = Vector3(0.0f, 0.0f, 0.0f);
	_current = Vector3(0.0f, 0.0f, 0.0f);
	_next = Vector3(0.0f, 0.0f, 0.0f);

	_nearActors.clear();
}

bool ActorWalk::setup(int actorId, bool runFlag, const Vector3 &from, const Vector3 &to, bool mustReach, bool *arrived) {
	Vector3 next;

	*arrived = false;

	int r = nextOnPath(actorId, from, to, next);

	if (r == 0) {
		if (actorId != 0) {
			_current = from;
			_destination = to;
			stop(actorId, false, kAnimationModeCombatIdle, kAnimationModeIdle);
		} else {
			stop(actorId, true, kAnimationModeCombatIdle, kAnimationModeIdle);
		}
//		debug("actor id: %d, arrived: %d - false setup 01", actorId, (*arrived)? 1:0);
		return false;
	}

	if (r == -1) {
		stop(actorId, true, kAnimationModeCombatIdle, kAnimationModeIdle);
		*arrived = true;
//		debug("actor id: %d, arrived: %d - false setup 02", actorId, (*arrived)? 1:0);
		return false;
	}

	_nearActors.clear();
	_vm->_sceneObjects->setMoving(actorId + kSceneObjectOffsetActors, true);
	_vm->_actors[actorId]->setMoving(true);

	if (_running) {
		runFlag = true;
	}

	int animationMode;
	if (_vm->_actors[actorId]->inCombat()) {
		animationMode = runFlag ? kAnimationModeCombatRun : kAnimationModeCombatWalk;
	} else {
		animationMode = runFlag ? kAnimationModeRun : kAnimationModeWalk;
	}

	_vm->_actors[actorId]->changeAnimationMode(animationMode);

	_destination = to;
	_originalDestination = to;
	_current = from;
	_next = next;

	if (next.x == _current.x && next.z == _current.z) {
		stop(actorId, true, kAnimationModeCombatIdle, kAnimationModeIdle);
		*arrived = true;
//		debug("actor id: %d, arrived: %d - false setup 03", actorId, (*arrived)? 1:0);
		return false;
	}

	_facing = angle_1024(_current, next);
	_walking = true;
	_running = runFlag;
	_status = 2;

//	debug("actor id: %d, arrived: %d - true setup 01", actorId, (*arrived)? 1:0);
	return true;
}

bool ActorWalk::tick(int actorId, float stepDistance, bool mustReachWalkDestination) {
	bool walkboxFound;

	if (_status == 5) {
		if (mustReachWalkDestination) {
			stop(actorId, true, kAnimationModeCombatIdle, kAnimationModeIdle);
			return true;
		}

		if (actorId != 0 && _vm->_rnd.getRandomNumberRng(1, 15) != 1) { // why random?
			return false;
		}
		_status = 3;
	}

	bool nearActorExists = addNearActors(actorId);
	if (_nearActors.size() > 0) {
		nearActorExists = true;
		if (_vm->_sceneObjects->existsOnXZ(actorId + kSceneObjectOffsetActors, _destination.x, _destination.z, true, true)) {
			if (actorId > 0) {
				if (_vm->_actors[actorId]->mustReachWalkDestination()) {
					stop(actorId, true, kAnimationModeCombatIdle, kAnimationModeIdle);
					_nearActors.clear();
					return true;
				} else {
					Vector3 newDestination;
					findEmptyPositionAroundToOriginalDestination(actorId, newDestination);
					_destination = newDestination;
					return false;
				}
			} else {
				if (_vm->_playerActor->mustReachWalkDestination()) {
					_destination = _current;
				}
				stop(0, true, kAnimationModeCombatIdle, kAnimationModeIdle);
				_nearActors.clear();
				return true;
			}
		}
	}
	_status = 3;

	if (stepDistance > distance(_current, _destination)) {
		stop(actorId, true, kAnimationModeCombatIdle, kAnimationModeIdle);
		_current = _destination;
		_current.y = _vm->_scene->_set->getAltitudeAtXZ(_current.x, _current.z, &walkboxFound);
		return true;
	}

	float distanceToNext = distance(_current, _next);
	if (1.0f < distanceToNext) {
		_facing = angle_1024(_current, _next);
	}

	bool nextIsCloseEnough = stepDistance > distanceToNext;

	if (nextIsCloseEnough || nearActorExists || _status == 3) {
		if (nextIsCloseEnough) {
			_current = _next;
		}
		_status = 1;
		Vector3 next;
		obstaclesAddNearActors(actorId);
		int r = nextOnPath(actorId, _current, _destination, next);
		obstaclesRestore();
		if (r == 0) {
			stop(actorId, actorId == kActorMcCoy, kAnimationModeCombatIdle, kAnimationModeIdle);
			return false;
		}
		if (r != -1) {
			_next = next;
			_facing = angle_1024(_current, _next);
			_status = 2;
			int animationMode;
			if (_vm->_actors[actorId]->inCombat()) {
				animationMode = _running ? kAnimationModeCombatRun : kAnimationModeCombatWalk;
			} else {
				animationMode = _running ? kAnimationModeRun : kAnimationModeWalk;
			}
			_vm->_actors[actorId]->changeAnimationMode(animationMode);
			if (nextIsCloseEnough) {
				return false;
			}
		} else {
			stop(actorId, true, kAnimationModeCombatIdle, kAnimationModeIdle); // too close
			return true;
		}
	}

#if !BLADERUNNER_ORIGINAL_BUGS
	// safety-guard / validator  check
	if (_facing >= 1024) {
		_facing = (_facing % 1024);
	} else if (_facing < 0) {
		_facing  = (-1) * _facing;
		_facing = (_facing % 1024);
		if (_facing > 0) {
			_facing  = 1024 - _facing; // this will always be in [1, 1023]
		}
	}
#endif
	_current.x += stepDistance * _vm->_sinTable1024->at(_facing);
	_current.z -= stepDistance * _vm->_cosTable1024->at(_facing);
	_current.y = _vm->_scene->_set->getAltitudeAtXZ(_current.x, _current.z, &walkboxFound);

	return false;
}

void ActorWalk::getCurrentPosition(int actorId, Vector3 *pos, int *facing) const {
	*pos = _current;
	*facing = _facing;
}

void ActorWalk::stop(int actorId, bool immediately, int combatAnimationMode, int animationMode) {
	_vm->_sceneObjects->setMoving(actorId + kSceneObjectOffsetActors, false);
	_vm->_actors[actorId]->setMoving(false);

	if (_vm->_actors[actorId]->inCombat()) {
		_vm->_actors[actorId]->changeAnimationMode(combatAnimationMode, false);
	} else {
		_vm->_actors[actorId]->changeAnimationMode(animationMode, false);
	}

	if (immediately) {
		_walking = false;
		_running = false;
		_status = 0;
	} else {
		_walking = true;
		_running = false;
		_status = 5;
	}
}

void ActorWalk::run(int actorId) {
	_running = true;

	int animationMode = kAnimationModeRun;
	if (_vm->_actors[actorId]->inCombat()) {
		animationMode = kAnimationModeCombatRun;
	}
	_vm->_actors[actorId]->changeAnimationMode(animationMode, false);
}

void ActorWalk::save(SaveFileWriteStream &f) {
	f.writeInt(_walking);
	f.writeInt(_running);
	f.writeVector3(_destination);
	// _originalDestination is not saved
	f.writeVector3(_current);
	f.writeVector3(_next);
	f.writeInt(_facing);

	assert(_nearActors.size() <= 20);
	for (Common::HashMap<int, bool>::const_iterator it = _nearActors.begin(); it != _nearActors.end(); ++it) {
		f.writeInt(it->_key);
		f.writeBool(it->_value);
	}
	f.padBytes(8 * (20 - _nearActors.size()));
	f.writeInt(_nearActors.size());

	f.writeInt(0); // _notUsed
	f.writeInt(_status);
}

void ActorWalk::load(SaveFileReadStream &f) {
	_walking = f.readInt();
	_running = f.readInt();
	_destination = f.readVector3();
	// _originalDestination is not saved
	_current = f.readVector3();
	_next = f.readVector3();
	_facing = f.readInt();

	int actorId[20];
	bool isNear[20];

	for (int i = 0; i < 20; ++i) {
		actorId[i] = f.readInt();
		isNear[i] = f.readBool();
	}

	int count = f.readInt();
	for (int i = 0; i < count; ++i) {
		_nearActors.setVal(actorId[i], isNear[i]);
	}

	f.skip(4); // _notUsed
	_status = f.readInt();
}

bool ActorWalk::isXYZOccupied(float x, float y, float z, int actorId) const {
	if (_vm->_scene->_set->findWalkbox(x, z) == -1) {
		return true;
	}
	if (_vm->_actors[actorId]->isImmuneToObstacles()) {
		return false;
	}
	return _vm->_sceneObjects->existsOnXZ(actorId + kSceneObjectOffsetActors, x, z, false, false);
}

bool ActorWalk::findEmptyPositionAround(int actorId, const Vector3 &destination, int dist, Vector3 &out) const {
	bool inWalkbox;

	int facingToMinDistance = -1;
	float minDistance = -1.0f;
	float x = 0.0f;
	float z = 0.0f;

	out.x = 0.0f;
	out.y = 0.0f;
	out.z = 0.0f;

	for (int facing = 0; facing < 1024; facing += 128) {
		x = destination.x + _vm->_sinTable1024->at(facing) * dist;
		z = destination.z - _vm->_cosTable1024->at(facing) * dist;
		float distanceBetweenActorAndDestination = distance(x, z, _vm->_actors[actorId]->getX(), _vm->_actors[actorId]->getZ());

		if (minDistance == -1.0f || minDistance > distanceBetweenActorAndDestination) {
			minDistance = distanceBetweenActorAndDestination;
			facingToMinDistance = facing;
		}
	}

	int facingLeft = facingToMinDistance;
	int facingRight = facingToMinDistance;
	int facing = -1024;
	while (facing < 0) {
		x = destination.x + _vm->_sinTable1024->at(facingRight) * dist;
		z = destination.z - _vm->_cosTable1024->at(facingRight) * dist;

		if (!_vm->_sceneObjects->existsOnXZ(actorId + kSceneObjectOffsetActors, x, z, true, true) && _vm->_scene->_set->findWalkbox(x, z) >= 0) {
			break;
		}

		x = destination.x + _vm->_sinTable1024->at(facingLeft) * dist;
		z = destination.z - _vm->_cosTable1024->at(facingLeft) * dist;

		if (!_vm->_sceneObjects->existsOnXZ(actorId + kSceneObjectOffsetActors, x, z, true, true) && _vm->_scene->_set->findWalkbox(x, z) >= 0) {
			break;
		}

		facingRight -= 64;
		if (facingRight < 0) {
			facingRight += 1024;
		}
		facingLeft += 64;
		if (facingLeft >= 1024) {
			facingLeft -= 1024;
		}
		facing += 64;
	}

	float y = _vm->_scene->_set->getAltitudeAtXZ(x, z, &inWalkbox);
	if (inWalkbox) {
		out.x = x;
		out.y = y;
		out.z = z;
		return true;
	}
	return false;
}

bool ActorWalk::findEmptyPositionAroundToOriginalDestination(int actorId, Vector3 &out) const {
	return findEmptyPositionAround(actorId, _originalDestination, 30, out);
}

bool ActorWalk::addNearActors(int skipActorId) {
	bool added = false;
	int setId = _vm->_scene->getSetId();
	for (int i = 0; i < (int)_vm->_gameInfo->getActorCount(); ++i) {
		assert(_vm->_actors[i] != nullptr);

		if (_vm->_actors[skipActorId] != nullptr
		 && _vm->_actors[i]->getSetId() == setId
		 && i != skipActorId
		) {
			if (_nearActors.contains(i)) {
				_nearActors.setVal(i, false);
			} else if (_vm->_actors[skipActorId]->distanceFromActor(i) <= 48.0f) {
				_nearActors.setVal(i, true);
				added = true;
			}
		}
	}
	return added;
}

void ActorWalk::obstaclesAddNearActors(int actorId) const {
	Vector3 position = _vm->_actors[actorId]->getPosition();
	for (Common::HashMap<int, bool>::const_iterator it = _nearActors.begin(); it != _nearActors.end(); ++it) {
		Actor *otherActor = _vm->_actors[it->_key];
		assert(otherActor != nullptr);

		if ( otherActor->isRetired()) {
			continue;
		}
		Vector3 otherPosition = otherActor->getPosition();
		float x0 = otherPosition.x - 12.0f;
		float z0 = otherPosition.z - 12.0f;
		float x1 = otherPosition.x + 12.0f;
		float z1 = otherPosition.z + 12.0f;
		if (position.x < (x0 - 12.0f) || position.z < (z0 - 12.0f) || position.x > (x1 + 12.0f) || position.z > (z1 + 12.0f)) {
			_vm->_obstacles->add(x0, z0, x1, z1);
		}
	}
}

void ActorWalk::obstaclesRestore() const {
	_vm->_obstacles->restore();
}

int ActorWalk::nextOnPath(int actorId, const Vector3 &from, const Vector3 &to, Vector3 &next) const {
	next = from;

	if (distance(from, to) < 6.0) {
//		debug("Id: %d Distance: %f::Result -1", actorId, distance(from, to));
		return -1;
	}

	if (_vm->_actors[actorId]->isImmuneToObstacles()) {
		next = to;
		return 1;
	}
	if (_vm->_scene->_set->findWalkbox(to.x, to.z) == -1) {
//		debug("Id: %d No walkbox::Result 0", actorId);
		return 0;
	}
	if (_vm->_sceneObjects->existsOnXZ(actorId + kSceneObjectOffsetActors, to.x, to.z, false, false)) {
//		debug("Actor Id: %d existsOnXZ::Result 0", actorId);
		return 0;
	}
	Vector3 next1;
	if (_vm->_obstacles->findNextWaypoint(from, to, &next1)) {
		next = next1;
		return 1;
	}
//	debug("Id: %d DEFAULTED::Result 0", actorId);
	return 0;
}

} // End of namespace BladeRunner
