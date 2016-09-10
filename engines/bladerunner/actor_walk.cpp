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
#include "bladerunner/scene_objects.h"

namespace BladeRunner {

static int angle_1024(float x1, float z1, float x2, float z2);
static int angle_1024(Vector3 &v1, Vector3 &v2);
float distance(float x1, float x2, float z1, float z2);
float distance(Vector3 &v1, Vector3 &v2);

ActorWalk::ActorWalk(BladeRunnerEngine *vm) {
	_vm = vm;

	_walking =  0;
	_running =  0;
	_facing  = -1;
	_status  =  0;
}

ActorWalk::~ActorWalk() {
}

int ActorWalk::setup(int actorId, int run, Vector3 from, Vector3 to, int unk1, int *unk2)
{
	Vector3 next;

	*unk2 = 0;
	int r = nextOnPath(actorId, from, to, &next);

	if (r == 0) {
		if (actorId != 0)
		{
			_current = from;
			_destination = to;
			stop(actorId, false);
		}
		else
		{
			stop(actorId, true);
		}
		return 0;
	}
	if (r == -1)
	{
		stop(actorId, true);
		*unk2 = 1;
		return 0;
	}

	// TODO: Init array
	// TODO: Update screen index
	// Set actor field e8

	if (_running)
	{
		run = true;
	}

	int animationMode;
	if (_vm->_actors[actorId]->inCombat())
	{
		animationMode = run ? 8 : 7;
	}
	else
	{
		animationMode = run ? 2 : 1;
	}

	_vm->_actors[actorId]->changeAnimationMode(animationMode);

	_destination = to;
	_current = from;
	_next = next;

	if (next.x != _current.x || next.z != _current.z)
	{
		_facing = angle_1024(_current, next);
		_walking = true;
		_running = run;
		_status = 2;

		return 1;
	}

	stop(actorId, true);
	return 0;
}

bool ActorWalk::tick(int actorId, float stepDistance, bool flag)
{
	if (_status == 5)
	{
		if (flag)
		{
			stop(actorId, true);
			return true;
		}

		if (actorId != 0 && _vm->_rnd.getRandomNumberRng(1, 15) != 1)
		{
			return false;
		}
		_status = 3;
	}
	// TODO: Handle collisions?

	if (stepDistance > distance(_current, _destination))
	{
		stop(actorId, true);
		_current = _destination;
		// TODO: Update y from walkbox
		return true;
	}

	float angle_rad = _facing / 512.0 * M_PI;

	_current = Vector3(
		_current.x + stepDistance * sinf(angle_rad),
		_current.y,                    // TODO: Update from walkbox
		_current.z - stepDistance * cosf(angle_rad)
	);

	return false;
}

void ActorWalk::getCurrentPosition(int actorId, Vector3 *pos, int *facing)
{
	*pos    = _current;
	*facing = _facing;
}

void ActorWalk::setRunning()
{
	_running = true;
	// TODO: Set animation mode
}

void ActorWalk::stop(int actorId, bool unknown, int animationMode, int notused) {
	_vm->_sceneObjects->setMoving(actorId, 0);
	_vm->_actors[actorId]->setMoving(0);

	if (_vm->_actors[actorId]->inCombat()) {
		_vm->_actors[actorId]->changeAnimationMode(animationMode, 0);
	} else {
		_vm->_actors[actorId]->changeAnimationMode(notused, 0);
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

int ActorWalk::nextOnPath(int actorId, Vector3 from, Vector3 to, Vector3 *next)
{
	if (distance(from, to) < 6.0)
	{
		return -1;
	}

	// if (_vm->_actors[actorId]->getImmunityToObstacles()) {
		*next = to;
		return 1;
	// }

	error("TODO!");
}

static
int angle_1024(float x1, float z1, float x2, float z2)
{
	float angle_rad = atan2(x2 - x1, z1 - z2);
	int a = int(512.0 * angle_rad / M_PI);
	return (a + 1024) % 1024;
}

static
int angle_1024(Vector3 &v1, Vector3 &v2)
{
	return angle_1024(v1.x, v1.z, v2.x, v2.z);
}

float distance(float x1, float z1, float x2, float z2)
{
	float dx = x1 - x2;
	float dz = z1 - z2;
	float d = sqrt(dx*dx + dz*dz);

	float int_part = (int)d;
	float frac_part = d - int_part;

	if (frac_part < 0.001)
		frac_part = 0.0;

	return int_part + frac_part;
}

float distance(Vector3 &v1, Vector3 &v2)
{
	return distance(v1.x, v1.z, v2.x, v2.z);
}

} // End of namespace BladeRunner
