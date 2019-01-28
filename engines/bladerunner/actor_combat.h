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

#ifndef BLADERUNNER_ACTOR_COMBAT_H
#define BLADERUNNER_ACTOR_COMBAT_H

#include "bladerunner/vector.h"

namespace BladeRunner {

class BladeRunnerEngine;
class SaveFileReadStream;
class SaveFileWriteStream;

class ActorCombat {
	BladeRunnerEngine *_vm;

	int _actorId;
	bool _active;
	int _state;
	bool _rangedAttack;
	int _enemyId;
	int _waypointType;
	int _damage;
	int _fleeRatio;
	int _coverRatio;
	int _attackRatio;
	int _fleeRatioConst;
	int _coverRatioConst;
	int _attackRatioConst;
	int _actorHp;
	int _range;
	bool _unstoppable;
	Vector3 _actorPosition;
	Vector3 _enemyPosition;
	int _coversWaypointCount;
	int _fleeWaypointsCount;
	int _fleeingTowards;

public:
	ActorCombat(BladeRunnerEngine *vm);
	~ActorCombat();

	void setup();

	void combatOn(int actorId, int initialState, bool rangedAttack, int enemyId, int waypointType, int fleeRatio, int coverRatio, int attackRatio, int damage, int range, bool unstoppable);
	void combatOff();

	void tick();

	void hitAttempt();

	void save(SaveFileWriteStream &f);
	void load(SaveFileReadStream &f);

private:
	void reset();

	void cover();
	void approachToCloseAttack();
	void approachToRangedAttack();
	void uncover();
	void aim();
	void rangedAttack();
	void closeAttack();
	void flee();

	void faceEnemy();

	int getCoefficientCloseAttack() const;
	int getCoefficientRangedAttack() const;

	int getDamageCloseAttack(int min, int max) const;
	int getDamageRangedAttack(int min, int max) const;

	int calculateAttackRatio() const;
	int calculateCoverRatio() const;
	int calculateFleeRatio() const;

	bool findClosestPositionToEnemy(Vector3 &output) const;
};

} // End of namespace BladeRunner

#endif
