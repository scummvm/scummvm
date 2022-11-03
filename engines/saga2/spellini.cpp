/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/spelshow.h"
#include "saga2/spelvals.h"
#include "saga2/tilevect.h"

namespace Saga2 {

// ------------------------------------------------------------------
// Spell Display Initialization handlers
//
//   What is primarily done in these routines is to set up
//   the positions, velocities & accelerations of the individual
//   effectrons
//   In most cases, once the effectrons are given their initial
//   push they can be updated by just adding the velocity to the
//   position.
//   Sprite directionality is also set up here
//

/* ===================================================================== *
   Inlines
 * ===================================================================== */

#define randOff (randomVector(TilePoint(-1,-1,0),TilePoint(1,1,0)))

/* ===================================================================== *
   Effect specific code
 * ===================================================================== */

// ------------------------------------------------------------------
// null effect

SPELLINITFUNCTION(invisibleSpellInit) {
	effectron->_velocity = TilePoint(0, 0, 0);
	effectron->_acceleration = TilePoint(0, 0, 0);
}

// ------------------------------------------------------------------
// aura that tracks target

SPELLINITFUNCTION(auraSpellInit) {
	if (effectron->_parent->_maxAge)
		effectron->_totalSteps = effectron->_parent->_maxAge;
	else
		effectron->_totalSteps = 20;
	effectron->_current = effectron->_parent->_target->getPoint();
	effectron->_velocity = TilePoint(0, 0, 0);
	effectron->_acceleration = TilePoint(0, 0, 0);
}

// ------------------------------------------------------------------
// aura that tracks target (in front)

SPELLINITFUNCTION(glowSpellInit) {
	if (effectron->_parent->_maxAge)
		effectron->_totalSteps = effectron->_parent->_maxAge;
	else
		effectron->_totalSteps = 20;
	effectron->_current = effectron->_parent->_target->getPoint() - TilePoint(1, 1, 0);
	effectron->_finish = effectron->_current;
	effectron->_velocity = TilePoint(0, 0, 0);
	effectron->_acceleration = TilePoint(0, 0, 0);
}

// ------------------------------------------------------------------
// sprites that surround target

SPELLINITFUNCTION(wallSpellInit) {
	if (effectron->_parent->_maxAge)
		effectron->_totalSteps = effectron->_parent->_maxAge;
	else
		effectron->_totalSteps = 20;
	effectron->_current = effectron->_parent->_target->getPoint();
	effectron->_velocity = WallVectors[effectron->_partno] * kWallSpellRadius / 3;
	effectron->_current = effectron->_parent->_target->getPoint() + effectron->_velocity;
	effectron->_acceleration = TilePoint(0, 0, 0);
}

// ------------------------------------------------------------------
// projectile from caster to target

SPELLINITFUNCTION(projectileSpellInit) {
	effectron->_start = effectron->_current;
	effectron->_finish = effectron->_parent->_target->getPoint();
	TilePoint tp = (effectron->_finish - effectron->_start);
	effectron->_totalSteps = 1 + (tp.magnitude() / (2 * kSpellJumpiness));
	effectron->_velocity = tp / effectron->_totalSteps;
	effectron->_acceleration = TilePoint(0, 0, 0);
}

// ------------------------------------------------------------------
// bi-directional beams of energy

SPELLINITFUNCTION(exchangeSpellInit) {
	if (effectron->_partno % 2) {
		effectron->_finish = effectron->_current;
		effectron->_start = effectron->_parent->_target->getPoint();
	} else {
		effectron->_start = effectron->_current;
		effectron->_finish = effectron->_parent->_target->getPoint();
	}
	TilePoint tp = (effectron->_finish - effectron->_start);
	effectron->_totalSteps = 1 + (tp.magnitude() / (kSpellJumpiness));
	effectron->_velocity = tp / effectron->_totalSteps;
	effectron->_totalSteps += (effectron->_partno / 2);
	effectron->_acceleration = TilePoint(0, 0, 0);
	effectron->_current = effectron->_start;
}

// ------------------------------------------------------------------
// lightning bolt shaped spell

SPELLINITFUNCTION(boltSpellInit) {
	effectron->_stepNo = 0;
	if (effectron->_parent->_maxAge)
		effectron->_totalSteps = effectron->_parent->_maxAge;
	else
		effectron->_totalSteps = 1 + (kBoltSpellLength / (kSpellJumpiness * 3));

	effectron->_start = effectron->_current;
	effectron->_finish = effectron->_parent->_target->getPoint();

	TilePoint tVect = effectron->_finish - effectron->_start ;
	setMagnitude(tVect, kBoltSpellLength);
	TilePoint orth = rightVector(tVect, 0);
	setMagnitude(orth, kBoltSpellWidth * (effectron->_partno % 3 - 1) / 6);
	TilePoint offVect = tVect * ((effectron->_partno / 3) % 3) / (kSpellJumpiness * 3);

	effectron->_start += orth;
	effectron->_finish += orth + offVect;

	effectron->_velocity = tVect / effectron->_totalSteps + randOff;
	effectron->_acceleration = TilePoint(0, 0, 0);

	effectron->_current = effectron->_start;
}

// ------------------------------------------------------------------
// narrow bolt

SPELLINITFUNCTION(beamSpellInit) {
	effectron->_stepNo = 0;
	if (effectron->_parent->_maxAge)
		effectron->_totalSteps = effectron->_parent->_maxAge;
	else
		effectron->_totalSteps = 1 + (kBeamSpellLength / (kSpellJumpiness));

	effectron->_start = effectron->_current;
	effectron->_finish = effectron->_parent->_target->getPoint();

	TilePoint tVect = effectron->_finish - effectron->_start ;
	setMagnitude(tVect, kBeamSpellLength);
	TilePoint orth = rightVector(tVect, 0);
	setMagnitude(orth, kBeamSpellWidth / 2);

	effectron->_start += (tVect * effectron->_partno) / effectron->_totalSteps;
	effectron->_finish = effectron->_start;

	effectron->_velocity = orth;
	effectron->_acceleration = TilePoint(0, 0, 0);

	effectron->_current = effectron->_start;
}

// ------------------------------------------------------------------
// narrow cone shaped spell

SPELLINITFUNCTION(coneSpellInit) {
	effectron->_stepNo = 0;
	effectron->_totalSteps = 1 + (kConeSpellLength / (kSpellJumpiness * 3));

	effectron->_start = effectron->_current;
	effectron->_finish = effectron->_parent->_target->getPoint();

	TilePoint tVect = effectron->_finish - effectron->_start ;
	setMagnitude(tVect, kConeSpellLength);
	TilePoint orth = rightVector(tVect, 0);
	setMagnitude(orth, kConeSpellWidth * (effectron->_partno % 9 - 4) / 8);

	effectron->_finish = effectron->_start + tVect + orth;

	effectron->_velocity = (effectron->_finish - effectron->_start) / effectron->_totalSteps + randOff;
	effectron->_acceleration = TilePoint(0, 0, 0);
}

// ------------------------------------------------------------------
// wide cone shaped spell

SPELLINITFUNCTION(waveSpellInit) {
	effectron->_stepNo = 0;
	effectron->_totalSteps = 1 + (kConeSpellLength / (kSpellJumpiness * 2));

	effectron->_start = effectron->_current;
	effectron->_finish = effectron->_parent->_target->getPoint();

	TilePoint tVect = effectron->_finish - effectron->_start ;
	setMagnitude(tVect, kWaveSpellLength);
	TilePoint orth = rightVector(tVect, 0);
	setMagnitude(orth, kWaveSpellWidth * (effectron->_partno % 17 - 8) / 8);

	effectron->_finish = effectron->_start + tVect + orth;

	effectron->_velocity = (effectron->_finish - effectron->_start) / effectron->_totalSteps + randOff;
	effectron->_acceleration = TilePoint(0, 0, 0);
}

// ------------------------------------------------------------------
// small exploding ball

SPELLINITFUNCTION(ballSpellInit) {
	effectron->_stepNo = 0;
	effectron->_start = effectron->_current;
	effectron->_finish = FireballVectors[effectron->_partno];
	setMagnitude(effectron->_finish, kBallSpellRadius);
	effectron->_finish = effectron->_finish + effectron->_start;
	effectron->_totalSteps = 1 + (kBallSpellRadius / kSpellJumpiness);
	effectron->_acceleration = TilePoint(0, 0, 0);

	TilePoint tp = (effectron->_finish - effectron->_start);
	effectron->_totalSteps = 1 + (tp.magnitude() / kSpellJumpiness);
	effectron->_velocity = tp / effectron->_totalSteps;
	effectron->_velocity.z = 0;
	effectron->_acceleration = TilePoint(0, 0, 0);
}

// ------------------------------------------------------------------
// square exploding ball

SPELLINITFUNCTION(squareSpellInit) {
	effectron->_stepNo = 0;
	effectron->_start = effectron->_current;
	effectron->_finish = SquareSpellVectors[effectron->_partno];
	setMagnitude(effectron->_finish, effectron->_finish.magnitude()*kSquareSpellSize / 4);
	effectron->_finish = effectron->_finish + effectron->_start;
	effectron->_totalSteps = 1 + (kSquareSpellSize / kSpellJumpiness);
	effectron->_acceleration = TilePoint(0, 0, 0);

	TilePoint tp = (effectron->_finish - effectron->_start);
	effectron->_totalSteps = 1 + (tp.magnitude() / kSpellJumpiness);
	effectron->_velocity = tp / effectron->_totalSteps;
	effectron->_velocity.z = 0;
	effectron->_acceleration = TilePoint(0, 0, 0);
}

// ------------------------------------------------------------------
// large exploding ball

SPELLINITFUNCTION(stormSpellInit) {
	effectron->_stepNo = 0;
	effectron->_start = effectron->_current;
	effectron->_finish = FireballVectors[effectron->_partno];
	setMagnitude(effectron->_finish, kBallSpellRadius);
	effectron->_finish = effectron->_finish + effectron->_start;
	effectron->_totalSteps = 1 + (kBallSpellRadius / kSpellJumpiness);
	effectron->_acceleration = TilePoint(0, 0, 0);

	TilePoint tp = (effectron->_finish - effectron->_start);
	effectron->_totalSteps = 1 + (2 * tp.magnitude() / kSpellJumpiness);
	effectron->_velocity = tp / effectron->_totalSteps;
	effectron->_velocity.z = 0;
	effectron->_acceleration = TilePoint(0, 0, 0);
}

} // end of namespace Saga2
