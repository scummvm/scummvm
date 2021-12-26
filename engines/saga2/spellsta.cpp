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

namespace Saga2 {

// ------------------------------------------------------------------
// Spell Status functions
//   These routines are checked each display cycle to see if a  given
//     effectron should still be shown
//   Return values
//     0 : show normally
//     effectronHidden: hide but don't remove effectron
//     effectronDead : permanently hide effectron. when all
//       effectrons have this status the effect ends
//

/* ===================================================================== *
   Effect specific code
 * ===================================================================== */

// null spell

SPELLSTATUSFUNCTION(invisibleSpellSta) {
	return effectronDead;
}

// semi permanent aura

SPELLSTATUSFUNCTION(auraSpellSta) {
	if (effectron->stepNo > effectron->totalSteps)
		return effectronDead;
	return 0;
}

SPELLSTATUSFUNCTION(wallSpellSta) {
	if (effectron->stepNo > effectron->totalSteps)
		return effectronDead;
	return 0;
}

// projectile spell ( also used as first half of exploding spells

SPELLSTATUSFUNCTION(projectileSpellSta) {
	if (effectron->stepNo > effectron->totalSteps)
		return effectronDead;
	return 0;
}


SPELLSTATUSFUNCTION(exchangeSpellSta) {
	if (effectron->stepNo < effectron->partno / 2)
		return effectronHidden;
	if (effectron->stepNo >= effectron->totalSteps)
		return effectronDead;
	return 0;
}

SPELLSTATUSFUNCTION(boltSpellSta) {
	if (effectron->stepNo - (effectron->partno / 9) > effectron->totalSteps)
		return effectronDead;
	if ((effectron->partno / 9) >= effectron->stepNo)
		return effectronHidden;
	return 0;
}

SPELLSTATUSFUNCTION(beamSpellSta) {
	if ((effectron->partno > effectron->totalSteps) ||
	        (effectron->stepNo > effectron->totalSteps))
		return effectronDead;
	return 0;
}

SPELLSTATUSFUNCTION(coneSpellSta) {
	if (effectron->stepNo - (effectron->partno / 9) > effectron->totalSteps)
		return effectronDead;
	if (effectron->partno / 9 >= effectron->stepNo)
		return effectronHidden;
	return 0;
}

SPELLSTATUSFUNCTION(waveSpellSta) {
	if (effectron->stepNo - (effectron->partno / 17) > effectron->totalSteps)
		return effectronDead;
	if (effectron->partno / 17 < effectron->stepNo)
		return effectronHidden;
	return 0;
}

SPELLSTATUSFUNCTION(ballSpellSta) {
	if (effectron->isBumped() ||
	        effectron->stepNo > effectron->totalSteps)
		return effectronDead;
	return 0;
}

SPELLSTATUSFUNCTION(squareSpellSta) {
	if (effectron->isBumped() ||
	        effectron->stepNo > effectron->totalSteps)
		return effectronDead;
	return 0;
}

SPELLSTATUSFUNCTION(stormSpellSta) {
	if (effectron->isBumped() ||
	        effectron->stepNo > effectron->totalSteps)
		return effectronDead;
	return 0;
}

} // end of namespace Saga2
