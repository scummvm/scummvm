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
#include "saga2/objproto.h"
#include "saga2/spellbuk.h"
#include "saga2/spelshow.h"

namespace Saga2 {

//-----------------------------------------------------------------------
// General contents
//   This file contains the effectron sprite selection functions

/* ===================================================================== *
   Inlines
 * ===================================================================== */

// random sprite from primary range
#define PRIMARY   (effectron->_parent->_dProto->_primarySpriteNo?\
                   (effectron->_parent->_dProto->_primarySpriteID + g_vm->_rnd->getRandomNumber(effectron->_parent->_dProto->_primarySpriteNo - 1)):\
                   effectron->_parent->_dProto->_primarySpriteID)
// random sprite from secondary range
#define SECONDARY (effectron->_parent->_dProto->_secondarySpriteNo?\
                   (effectron->_parent->_dProto->_secondarySpriteID + g_vm->_rnd->getRandomNumber(effectron->_parent->_dProto->_secondarySpriteNo - 1)):\
                   effectron->_parent->_dProto->_secondarySpriteID)
// ordered sprite from primary range
#define SEQUENTIAL (effectron->_parent->_dProto->_primarySpriteNo?\
                    (effectron->_parent->_dProto->_primarySpriteID + effectron->_stepNo%effectron->_parent->_dProto->_primarySpriteNo):\
                    effectron->_parent->_dProto->_primarySpriteID)
// ordered sprite from secondary range
#define SECUENTIAL (effectron->_parent->_dProto->_secondarySpriteNo?\
                    (effectron->_parent->_dProto->_secondarySpriteID + effectron->_stepNo%effectron->_parent->_dProto->_secondarySpriteNo):\
                    effectron->_parent->_dProto->_secondarySpriteID)
// ordered sprite from primary range for exchange
#define SEMIQUENTIAL (effectron->_parent->_dProto->_primarySpriteNo?\
                      (effectron->_parent->_dProto->_primarySpriteID + (effectron->_partno/2)%effectron->_parent->_dProto->_primarySpriteNo):\
                      effectron->_parent->_dProto->_primarySpriteID)

/* ===================================================================== *
   Color mapping selection
 * ===================================================================== */

// color map selection
int16 whichColorMap(EffectID eid, const Effectron *const effectron) {
	int16 rval = 0;
	switch (eid) {
	case keAreaInvisible:
	case keAreaAura:
	case keAreaGlow:
	case keAreaProjectile:
	case keAreaExchange:
	case keAreaMissle:
	case keAreaBeam:
	case keAreaWall:
		rval = 0;
		break;
	case keAreaSquare:
	case keAreaBall:
	case keAreaStorm:
		rval = (effectron->_parent->_effSeq == 0) ? 0 : 1;
		break;
	case keAreaBolt:
		rval = ((effectron->_partno % 3) == 1) ? 0 : 1;
		break;
	case keAreaCone:
		rval = ((effectron->_partno / 9) == 0) ? 0 : 1;
		break;
	case keAreaWave:
		rval = ((effectron->_partno / 17) == 0) ? 0 : 1;
		break;
	}
	return rval;
}



/* ===================================================================== *
   Effect specific code
 * ===================================================================== */

SPELLSPRITATIONFUNCTION(invisibleSprites) {
	return PRIMARY;
}

SPELLSPRITATIONFUNCTION(auraSprites) {
	if (effectron->_parent->_effSeq)
		return SECUENTIAL;
	return SEQUENTIAL;
}

SPELLSPRITATIONFUNCTION(wallSprites) {
	return SEQUENTIAL;
}

SPELLSPRITATIONFUNCTION(projectileSprites) {
	return PRIMARY;
}

SPELLSPRITATIONFUNCTION(exchangeSprites) {
	return SEMIQUENTIAL;
}

SPELLSPRITATIONFUNCTION(beamSprites) {
	return PRIMARY;
}

SPELLSPRITATIONFUNCTION(boltSprites) {
	if ((effectron->_partno % 3) == 1)
		return PRIMARY;
	return SECONDARY;
}

SPELLSPRITATIONFUNCTION(coneSprites) {
	if ((effectron->_partno / 9) == 0)
		return PRIMARY;
	return SECONDARY;
}

SPELLSPRITATIONFUNCTION(ballSprites) {
	if (effectron->_parent->_effSeq)
		return SECONDARY;
	return PRIMARY;
}

SPELLSPRITATIONFUNCTION(squareSprites) {
	if (effectron->_parent->_effSeq)
		return SECONDARY;
	return PRIMARY;
}

SPELLSPRITATIONFUNCTION(waveSprites) {
	if ((effectron->_partno / 17) == 0)
		return PRIMARY;
	return SECONDARY;
}

SPELLSPRITATIONFUNCTION(stormSprites) {
	if (effectron->_parent->_effSeq)
		return SECONDARY;
	return PRIMARY;
}


//-----------------------------------------------------------------------

SPELLSPRITATIONFUNCTION(RandomFacing) {
	return g_vm->_rnd->getRandomNumber(255);
}

SPELLSPRITATIONFUNCTION(FaceOut) {
	return g_vm->_rnd->getRandomNumber(255);
}

} // end of namespace Saga2
