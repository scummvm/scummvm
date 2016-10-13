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

#include "bladerunner/bladerunner.h"

#include "bladerunner/vector.h"

namespace BladeRunner {

class ActorCombat {
	BladeRunnerEngine *_vm;

private:
//	int _actorId;
//	int _combatOn;
//	int _field2;
//	int _field3;
//	int _otherActorId;
//	int _field5;
//	int _field6;
//	int _field7;
//	int _field8;
//	int _field9;
//	int _field10;
//	int _field11;
//	int _field12;
//	int _actorHp;
//	int _field14;
//	int _field15;
	Vector3 actorPosition;
	Vector3 otherActorPosition;
//	int _availableCoversCount;
//	int _availableFleeWaypointsCount;
//	int _field24;

public:
	ActorCombat(BladeRunnerEngine *vm);
	~ActorCombat();

	void setup();

	void hitAttempt();

	void combatOn(int actorId, int a3, int a4, int otherActorId, int a6, int a7, int a8, int a9, int a10, int a11, int a12);
	void combatOff();
};

} // End of namespace BladeRunner

#endif
