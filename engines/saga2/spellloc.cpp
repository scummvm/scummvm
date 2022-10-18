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
#include "saga2/tilevect.h"

namespace Saga2 {

//-----------------------------------------------------------------------
// Spell Display positioning routines
//   For the most part these simply update the position with the
//   velocity.  Cone and bolt spells though have extra code to stagger
//   rows of effectrons
//

/* ===================================================================== *
   Effect specific code
 * ===================================================================== */

//-----------------------------------------------------------------------
// null spell

SPELLLOCATIONFUNCTION(invisibleSpellPos) {
	return effectron->_finish;
}

// ------------------------------------------------------------------
// aura that tracks target

SPELLLOCATIONFUNCTION(auraSpellPos) {
	return effectron->_finish;
}

// ------------------------------------------------------------------
// aura that tracks target (in front)

SPELLLOCATIONFUNCTION(glowSpellPos) {
	return effectron->_finish - TilePoint(8, 8, 0);
}

// ------------------------------------------------------------------
// sprites that surround target

SPELLLOCATIONFUNCTION(wallSpellPos) {
	return effectron->_parent->_target->getPoint() + effectron->_velocity;
}

// ------------------------------------------------------------------
// projectile from caster to target

SPELLLOCATIONFUNCTION(projectileSpellPos) {
	return effectron->_current + effectron->_velocity;
}

// ------------------------------------------------------------------
// bi-directional beams of energy

SPELLLOCATIONFUNCTION(exchangeSpellPos) {
	if (effectron->_stepNo < effectron->_partno / 2)
		return effectron->_current;
	return effectron->_current + effectron->_velocity;
}

// ------------------------------------------------------------------
// lightning bolt shaped spell

SPELLLOCATIONFUNCTION(boltSpellPos) {
	if ((effectron->_partno / 9) >= effectron->_stepNo)
		return effectron->_current;
	return effectron->_current +
	       effectron->_velocity;
}

// ------------------------------------------------------------------
// narrow bolt

SPELLLOCATIONFUNCTION(beamSpellPos) {
	return effectron->_start + randomVector(-effectron->_velocity, effectron->_velocity);
}

// ------------------------------------------------------------------
// narrow cone shaped spell

SPELLLOCATIONFUNCTION(coneSpellPos) {
	if (effectron->_partno / 9 >= effectron->_stepNo)
		return effectron->_current;
	return effectron->_current +
	       effectron->_velocity;
}

// ------------------------------------------------------------------
// wide cone shaped spell

SPELLLOCATIONFUNCTION(waveSpellPos) {
	if (effectron->_partno / 17 >= effectron->_stepNo)
		return effectron->_current;
	return effectron->_current +
	       effectron->_velocity;
}

// ------------------------------------------------------------------
// small exploding ball

SPELLLOCATIONFUNCTION(ballSpellPos) {
	return effectron->_current +
	       effectron->_velocity;
}

// ------------------------------------------------------------------
// square exploding ball

SPELLLOCATIONFUNCTION(squareSpellPos) {
	return effectron->_current +
	       effectron->_velocity;
}

// ------------------------------------------------------------------
// large exploding ball

SPELLLOCATIONFUNCTION(stormSpellPos) {
	return effectron->_current +
	       effectron->_velocity;
}

} // end of namespace Saga2
