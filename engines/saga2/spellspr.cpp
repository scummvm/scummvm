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
#define PRIMARY   (effectron->parent->dProto->primarySpriteNo?\
                   (effectron->parent->dProto->primarySpriteID + g_vm->_rnd->getRandomNumber(effectron->parent->dProto->primarySpriteNo - 1)):\
                   effectron->parent->dProto->primarySpriteID)
// random sprite from secondary range
#define SECONDARY (effectron->parent->dProto->secondarySpriteNo?\
                   (effectron->parent->dProto->secondarySpriteID + g_vm->_rnd->getRandomNumber(effectron->parent->dProto->secondarySpriteNo - 1)):\
                   effectron->parent->dProto->secondarySpriteID)
// ordered sprite from primary range
#define SEQUENTIAL (effectron->parent->dProto->primarySpriteNo?\
                    (effectron->parent->dProto->primarySpriteID + effectron->stepNo%effectron->parent->dProto->primarySpriteNo):\
                    effectron->parent->dProto->primarySpriteID)
// ordered sprite from secondary range
#define SECUENTIAL (effectron->parent->dProto->secondarySpriteNo?\
                    (effectron->parent->dProto->secondarySpriteID + effectron->stepNo%effectron->parent->dProto->secondarySpriteNo):\
                    effectron->parent->dProto->secondarySpriteID)
// ordered sprite from primary range for exchange
#define SEMIQUENTIAL (effectron->parent->dProto->primarySpriteNo?\
                      (effectron->parent->dProto->primarySpriteID + (effectron->partno/2)%effectron->parent->dProto->primarySpriteNo):\
                      effectron->parent->dProto->primarySpriteID)

/* ===================================================================== *
   Color mapping selection
 * ===================================================================== */

// color map selection
int16 whichColorMap(EffectID eid, const Effectron *const effectron) {
	int16 rval = 0;
	switch (eid) {
	case eAreaInvisible:
	case eAreaAura:
	case eAreaGlow:
	case eAreaProjectile:
	case eAreaExchange:
	case eAreaMissle:
	case eAreaBeam:
	case eAreaWall:
		rval = 0;
		break;
	case eAreaSquare:
	case eAreaBall:
	case eAreaStorm:
		rval = (effectron->parent->effSeq == 0) ? 0 : 1;
		break;
	case eAreaBolt:
		rval = ((effectron->partno % 3) == 1) ? 0 : 1;
		break;
	case eAreaCone:
		rval = ((effectron->partno / 9) == 0) ? 0 : 1;
		break;
	case eAreaWave:
		rval = ((effectron->partno / 17) == 0) ? 0 : 1;
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
	if (effectron->parent->effSeq)
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
	if ((effectron->partno % 3) == 1)
		return PRIMARY;
	return SECONDARY;
}

SPELLSPRITATIONFUNCTION(coneSprites) {
	if ((effectron->partno / 9) == 0)
		return PRIMARY;
	return SECONDARY;
}

SPELLSPRITATIONFUNCTION(ballSprites) {
	if (effectron->parent->effSeq)
		return SECONDARY;
	return PRIMARY;
}

SPELLSPRITATIONFUNCTION(squareSprites) {
	if (effectron->parent->effSeq)
		return SECONDARY;
	return PRIMARY;
}

SPELLSPRITATIONFUNCTION(waveSprites) {
	if ((effectron->partno / 17) == 0)
		return PRIMARY;
	return SECONDARY;
}

SPELLSPRITATIONFUNCTION(stormSprites) {
	if (effectron->parent->effSeq)
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
