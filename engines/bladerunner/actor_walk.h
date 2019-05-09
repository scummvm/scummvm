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
#include "common/hashmap.h"

namespace BladeRunner {

class BladeRunnerEngine;
class SaveFileReadStream;
class SaveFileWriteStream;

class ActorWalk {
	BladeRunnerEngine *_vm;

	int                        _walking;
	int                        _running;
	Vector3                    _destination;
	Vector3                    _originalDestination;
	Vector3                    _current;
	Vector3                    _next;
	int                        _facing;
	Common::HashMap<int, bool> _nearActors;
	int                        _status;

public:
	ActorWalk(BladeRunnerEngine *vm);
	~ActorWalk();

	void reset(); // added method for bug fix (bad new game state for player actor) and better management of object

	bool setup(int actorId, bool runFlag, const Vector3 &from, const Vector3 &to, bool mustReach, bool *arrived);
	void getCurrentPosition(int actorId, Vector3 *pos, int *facing) const;
	bool tick(int actorId, float stepDistance, bool flag);

	bool isWalking() const { return _walking; }
	bool isRunning() const { return _running; }

	bool isXYZOccupied(float x, float y, float z, int actorId) const;
	bool findEmptyPositionAround(int actorId, const Vector3 &from, int distance, Vector3 &out) const;

	void stop(int actorId, bool immediately, int combatAnimationMode, int animationMode);
	void run(int actorId);

	void save(SaveFileWriteStream &f);
	void load(SaveFileReadStream &f);

private:
	int nextOnPath(int actorId, const Vector3 &from, const Vector3 &to, Vector3 &next) const;

	bool findEmptyPositionAroundToOriginalDestination(int actorId, Vector3 &out) const;

	bool addNearActors(int skipActorId);

	void obstaclesAddNearActors(int actorId) const;
	void obstaclesRestore() const;
};

} // End of namespace BladeRunner

#endif
