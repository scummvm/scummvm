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

#ifndef BLADERUNNER_ACTOR_WALK_H
#define BLADERUNNER_ACTOR_WALK_H

#include "bladerunner/vector.h"

namespace BladeRunner {

class BladeRunnerEngine;

struct ActorWalkEntry
{
	int _actorId;
	int _present;
};

class ActorWalk
{
	BladeRunnerEngine *_vm;

private:
	int     _walking;
	int     _running;
	Vector3 _destination;
	Vector3 _unknown;
	Vector3 _current;
	Vector3 _next;
	int     _facing;
	ActorWalkEntry _actors[20];
	int     _actorsCount;
	int     _field15;
	int     _status;

public:
	ActorWalk(BladeRunnerEngine *vm);
	~ActorWalk();

	int setup(int actorId, int run, Vector3 from, Vector3 to, int unk1, int *unk2);
	void getCurrentPosition(int actorId, Vector3 *pos, int *facing);
	bool tick(int actorId, float stepDistance, bool flag);

	bool isWalking() { return _walking; }
	bool isRunning() { return _running; }
	void setRunning();

	void stop(int actorId, bool unknown, int animationMode = 4, int notused = 0);
	// void setWalkingMode(int actorId, int active, int unk2 = 4, int unk3 = 0);

	int nextOnPath(int actorId, Vector3 from, Vector3 to, Vector3 *next);
};

} // End of namespace BladeRunner

#endif
