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
	effectron->velocity = TilePoint(0, 0, 0);
	effectron->acceleration = TilePoint(0, 0, 0);
}

// ------------------------------------------------------------------
// aura that tracks target

SPELLINITFUNCTION(auraSpellInit) {
	if (effectron->parent->maxAge)
		effectron->totalSteps = effectron->parent->maxAge;
	else
		effectron->totalSteps = 20;
	effectron->current = effectron->parent->target->getPoint();
	effectron->velocity = TilePoint(0, 0, 0);
	effectron->acceleration = TilePoint(0, 0, 0);
}

// ------------------------------------------------------------------
// aura that tracks target (in front)

SPELLINITFUNCTION(glowSpellInit) {
	if (effectron->parent->maxAge)
		effectron->totalSteps = effectron->parent->maxAge;
	else
		effectron->totalSteps = 20;
	effectron->current = effectron->parent->target->getPoint() - TilePoint(1, 1, 0);
	effectron->finish = effectron->current;
	effectron->velocity = TilePoint(0, 0, 0);
	effectron->acceleration = TilePoint(0, 0, 0);
}

// ------------------------------------------------------------------
// sprites that surround target

SPELLINITFUNCTION(wallSpellInit) {
	if (effectron->parent->maxAge)
		effectron->totalSteps = effectron->parent->maxAge;
	else
		effectron->totalSteps = 20;
	effectron->current = effectron->parent->target->getPoint();
	effectron->velocity = WallVectors[effectron->partno] * wallSpellRadius / 3;
	effectron->current = effectron->parent->target->getPoint() + effectron->velocity;
	effectron->acceleration = TilePoint(0, 0, 0);
}

// ------------------------------------------------------------------
// projectile from caster to target

SPELLINITFUNCTION(projectileSpellInit) {
	effectron->start = effectron->current;
	effectron->finish = effectron->parent->target->getPoint();
	TilePoint tp = (effectron->finish - effectron->start);
	effectron->totalSteps = 1 + (tp.magnitude() / (2 * SpellJumpiness));
	effectron->velocity = tp / effectron->totalSteps;
	effectron->acceleration = TilePoint(0, 0, 0);
}

// ------------------------------------------------------------------
// bi-directional beams of energy

SPELLINITFUNCTION(exchangeSpellInit) {
	if (effectron->partno % 2) {
		effectron->finish = effectron->current;
		effectron->start = effectron->parent->target->getPoint();
	} else {
		effectron->start = effectron->current;
		effectron->finish = effectron->parent->target->getPoint();
	}
	TilePoint tp = (effectron->finish - effectron->start);
	effectron->totalSteps = 1 + (tp.magnitude() / (SpellJumpiness));
	effectron->velocity = tp / effectron->totalSteps;
	effectron->totalSteps += (effectron->partno / 2);
	effectron->acceleration = TilePoint(0, 0, 0);
	effectron->current = effectron->start;
}

// ------------------------------------------------------------------
// lightning bolt shaped spell

SPELLINITFUNCTION(boltSpellInit) {
	effectron->stepNo = 0;
	if (effectron->parent->maxAge)
		effectron->totalSteps = effectron->parent->maxAge;
	else
		effectron->totalSteps = 1 + (boltSpellLength / (SpellJumpiness * 3));

	effectron->start = effectron->current;
	effectron->finish = effectron->parent->target->getPoint();

	TilePoint tVect = effectron->finish - effectron->start ;
	setMagnitude(tVect, boltSpellLength);
	TilePoint orth = rightVector(tVect, 0);
	setMagnitude(orth, boltSpellWidth * (effectron->partno % 3 - 1) / 6);
	TilePoint offVect = tVect * ((effectron->partno / 3) % 3) / (SpellJumpiness * 3);

	effectron->start += orth;
	effectron->finish += orth + offVect;

	effectron->velocity = tVect / effectron->totalSteps + randOff;
	effectron->acceleration = TilePoint(0, 0, 0);

	effectron->current = effectron->start;
}

// ------------------------------------------------------------------
// narrow bolt

SPELLINITFUNCTION(beamSpellInit) {
	effectron->stepNo = 0;
	if (effectron->parent->maxAge)
		effectron->totalSteps = effectron->parent->maxAge;
	else
		effectron->totalSteps = 1 + (beamSpellLength / (SpellJumpiness));

	effectron->start = effectron->current;
	effectron->finish = effectron->parent->target->getPoint();

	TilePoint tVect = effectron->finish - effectron->start ;
	setMagnitude(tVect, beamSpellLength);
	TilePoint orth = rightVector(tVect, 0);
	setMagnitude(orth, beamSpellWidth / 2);

	effectron->start += (tVect * effectron->partno) / effectron->totalSteps;
	effectron->finish = effectron->start;

	effectron->velocity = orth;
	effectron->acceleration = TilePoint(0, 0, 0);

	effectron->current = effectron->start;
}

// ------------------------------------------------------------------
// narrow cone shaped spell

SPELLINITFUNCTION(coneSpellInit) {
	effectron->stepNo = 0;
	effectron->totalSteps = 1 + (coneSpellLength / (SpellJumpiness * 3));

	effectron->start = effectron->current;
	effectron->finish = effectron->parent->target->getPoint();

	TilePoint tVect = effectron->finish - effectron->start ;
	setMagnitude(tVect, coneSpellLength);
	TilePoint orth = rightVector(tVect, 0);
	setMagnitude(orth, coneSpellWidth * (effectron->partno % 9 - 4) / 8);

	effectron->finish = effectron->start + tVect + orth;

	effectron->velocity = (effectron->finish - effectron->start) / effectron->totalSteps + randOff;
	effectron->acceleration = TilePoint(0, 0, 0);
}

// ------------------------------------------------------------------
// wide cone shaped spell

SPELLINITFUNCTION(waveSpellInit) {
	effectron->stepNo = 0;
	effectron->totalSteps = 1 + (coneSpellLength / (SpellJumpiness * 2));

	effectron->start = effectron->current;
	effectron->finish = effectron->parent->target->getPoint();

	TilePoint tVect = effectron->finish - effectron->start ;
	setMagnitude(tVect, waveSpellLength);
	TilePoint orth = rightVector(tVect, 0);
	setMagnitude(orth, waveSpellWidth * (effectron->partno % 17 - 8) / 8);

	effectron->finish = effectron->start + tVect + orth;

	effectron->velocity = (effectron->finish - effectron->start) / effectron->totalSteps + randOff;
	effectron->acceleration = TilePoint(0, 0, 0);
}

// ------------------------------------------------------------------
// small exploding ball

SPELLINITFUNCTION(ballSpellInit) {
	effectron->stepNo = 0;
	effectron->start = effectron->current;
	effectron->finish = FireballVectors[effectron->partno];
	setMagnitude(effectron->finish, ballSpellRadius);
	effectron->finish = effectron->finish + effectron->start;
	effectron->totalSteps = 1 + (ballSpellRadius / SpellJumpiness);
	effectron->acceleration = TilePoint(0, 0, 0);

	TilePoint tp = (effectron->finish - effectron->start);
	effectron->totalSteps = 1 + (tp.magnitude() / SpellJumpiness);
	effectron->velocity = tp / effectron->totalSteps;
	effectron->velocity.z = 0;
	effectron->acceleration = TilePoint(0, 0, 0);
}

// ------------------------------------------------------------------
// square exploding ball

SPELLINITFUNCTION(squareSpellInit) {
	effectron->stepNo = 0;
	effectron->start = effectron->current;
	effectron->finish = SquareSpellVectors[effectron->partno];
	setMagnitude(effectron->finish, effectron->finish.magnitude()*squareSpellSize / 4);
	effectron->finish = effectron->finish + effectron->start;
	effectron->totalSteps = 1 + (squareSpellSize / SpellJumpiness);
	effectron->acceleration = TilePoint(0, 0, 0);

	TilePoint tp = (effectron->finish - effectron->start);
	effectron->totalSteps = 1 + (tp.magnitude() / SpellJumpiness);
	effectron->velocity = tp / effectron->totalSteps;
	effectron->velocity.z = 0;
	effectron->acceleration = TilePoint(0, 0, 0);
}

// ------------------------------------------------------------------
// large exploding ball

SPELLINITFUNCTION(stormSpellInit) {
	effectron->stepNo = 0;
	effectron->start = effectron->current;
	effectron->finish = FireballVectors[effectron->partno];
	setMagnitude(effectron->finish, ballSpellRadius);
	effectron->finish = effectron->finish + effectron->start;
	effectron->totalSteps = 1 + (ballSpellRadius / SpellJumpiness);
	effectron->acceleration = TilePoint(0, 0, 0);

	TilePoint tp = (effectron->finish - effectron->start);
	effectron->totalSteps = 1 + (2 * tp.magnitude() / SpellJumpiness);
	effectron->velocity = tp / effectron->totalSteps;
	effectron->velocity.z = 0;
	effectron->acceleration = TilePoint(0, 0, 0);
}

} // end of namespace Saga2
