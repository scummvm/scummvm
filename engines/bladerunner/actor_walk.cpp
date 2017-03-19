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
#include "bladerunner/obstacles.h"
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/set.h"


namespace BladeRunner {

ActorWalk::ActorWalk(BladeRunnerEngine *vm) {
	_vm = vm;

	_walking =  0;
	_running =  0;
	_facing  = -1;
	_status  =  0;

	_entries.clear();
}

ActorWalk::~ActorWalk() {
}

bool ActorWalk::setup(int actorId, bool run, const Vector3 &from, const Vector3 &to, bool unk1, bool *stopped) {
	Vector3 next;

	*stopped = false;
	int r = nextOnPath(actorId, from, to, &next);

	if (r == 0) {
		if (actorId != 0) {
			_current = from;
			_destination = to;
			stop(actorId, false, 4, 0);
		} else {
			stop(actorId, true, 4, 0);
		}
		return false;
	}
	if (r == -1) {
		stop(actorId, true, 4, 0);
		*stopped = true;
		return false;
	}

	resetList();
	_vm->_sceneObjects->setMoving(actorId + SCENE_OBJECTS_ACTORS_OFFSET, true);
	_vm->_actors[actorId]->setMoving(true);

	if (_running) {
		run = true;
	}

	int animationMode;
	if (_vm->_actors[actorId]->inCombat()) {
		animationMode = run ? 8 : 7;
	} else {
		animationMode = run ? 2 : 1;
	}

	_vm->_actors[actorId]->changeAnimationMode(animationMode);

	_destination = to;
	_current = from;
	_next = next;

	if (next.x != _current.x || next.z != _current.z) {
		_facing = angle_1024(_current, next);
		_walking = true;
		_running = run;
		_status = 2;

		return true;
	}

	stop(actorId, true, 4, 0);
	return false;
}

bool ActorWalk::tick(int actorId, float stepDistance, bool flag) {
	if (_status == 5) {
		if (flag) {
			stop(actorId, true, 4, 0);
			return true;
		}

		if (actorId != 0 && _vm->_rnd.getRandomNumberRng(1, 15) != 1) {
			return false;
		}
		_status = 3;
	}
	// TODO: Handle collisions?

	if (stepDistance > distance(_current, _destination)) {
		stop(actorId, true, 4, 0);
		_current = _destination;
		// TODO: Update y from walkbox
		return true;
	}

	float angle_rad = _facing / 512.0 * M_PI;

	_current = Vector3(
		_current.x + stepDistance * sinf(angle_rad),
		_current.y, // TODO: Update from walkbox
		_current.z - stepDistance * cosf(angle_rad)
	);

	return false;
}

void ActorWalk::getCurrentPosition(int actorId, Vector3 *pos, int *facing) {
	*pos = _current;
	*facing = _facing;
}

void ActorWalk::setRunning() {
	_running = true;
	// TODO: Set animation mode
}

void ActorWalk::stop(int actorId, bool unknown, int combatAnimationMode, int animationMode) {
	_vm->_sceneObjects->setMoving(actorId + SCENE_OBJECTS_ACTORS_OFFSET, false);
	_vm->_actors[actorId]->setMoving(false);

	if (_vm->_actors[actorId]->inCombat()) {
		_vm->_actors[actorId]->changeAnimationMode(combatAnimationMode, false);
	} else {
		_vm->_actors[actorId]->changeAnimationMode(animationMode, false);
	}

	if (unknown) {
		_walking = false;
		_running = false;
		_status = 0;
	} else {
		_walking = true;
		_running = false;
		_status = 5;
	}
}

bool ActorWalk::isXYZEmpty(float x, float y, float z, int actorId) {
	if (_vm->_scene->_set->findWalkbox(x, z) == -1) {
		return true;
	}
	if (_vm->_actors[actorId]->isImmuneToObstacles()) {
		return false;
	}
	return _vm->_sceneObjects->existsOnXZ(actorId + SCENE_OBJECTS_ACTORS_OFFSET, x, z, false, false);
}

int ActorWalk::findU1(int actorId, const Vector3 &to, int dist, Vector3 *out) {
	bool inWalkbox;

	int facingFound = -1;
	float distFound = -1.0f;
	float x = 0.0f;
	float z = 0.0f;

	out->x = 0.0f;
	out->y = 0.0f;
	out->z = 0.0f;

	for (int facing = 0; facing < 1024; facing += 128) {
		x = to.x + sin_1024(facing) * dist;
		z = to.z + cos_1024(facing) * dist;
		float dist2 = distance(x, z, _vm->_actors[actorId]->getX(), _vm->_actors[actorId]->getZ());

		if (distFound == -1.0f || distFound > dist2) {
			distFound = dist2;
			facingFound = facing;
		}
	}

	int v23 = facingFound;
	int v24 = facingFound;
	int v25 = -1024;
	while (v25 < 0) {
		x = to.x + sin_1024(v24) * dist;
		z = to.z + cos_1024(v24) * dist;

		if (!_vm->_sceneObjects->existsOnXZ(actorId + SCENE_OBJECTS_ACTORS_OFFSET, x, z, true, true) && _vm->_scene->_set->findWalkbox(x, z) >= 0) {
			break;
		}

		x = to.x + sin_1024(v23) * dist;
		z = to.z + cos_1024(v23) * dist;

		if (!_vm->_sceneObjects->existsOnXZ(actorId + SCENE_OBJECTS_ACTORS_OFFSET, x, z, true, true) && _vm->_scene->_set->findWalkbox(x, z) >= 0) {
			break;
		}

		v24 -= 64;
		if (v24 < 0) {
			v24 += 1024;
		}
		v23 += 64;
		if (v23 >= 1024) {
			v23 -= 1024;
		}
		v25 += 64;
	}

	float y = _vm->_scene->_set->getAltitudeAtXZ(x, z, &inWalkbox);
	if (inWalkbox) {
		out->x = x;
		out->y = y;
		out->z = z;
		return true;
	}
	return false;
}

int ActorWalk::nextOnPath(int actorId, const Vector3 &from, const Vector3 &to, Vector3 *next) {
	*next = from;

	if (distance(from, to) < 6.0) {
		return -1;
	}

	if (_vm->_actors[actorId]->isImmuneToObstacles()) {
		*next = to;
		return 1;
	}
	if (_vm->_scene->_set->findWalkbox(to.x, to.z) == -1) {
		return 0;
	}
	if (_vm->_sceneObjects->existsOnXZ(actorId + SCENE_OBJECTS_ACTORS_OFFSET, to.x, to.z, false, false)) {
		return 0;
	}
	Vector3 next1;
	if (_vm->_obstacles->find(from, to, &next1)) {
		*next = next1;
		return 1;
	}
	return 0;
}

void ActorWalk::resetList() {
	_entries.clear();
}
} // End of namespace BladeRunner
