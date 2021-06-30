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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/player.h"
#include "saga2/tile.h"

#include "saga2/pclass.r"

namespace Saga2 {

//  Determine if this object is just an object
static bool objIsObject(GameObject *obj) {
	return isObject(obj);
}

//  Determine if this object is an actor
static bool objIsActor(GameObject *obj) {
	return isActor(obj);
}

//  Determine if this object is a world
static bool objIsWorld(GameObject *obj) {
	return isWorld(obj);
}

//  Determine if this object is locked
static bool objIsLocked(GameObject *obj) {
	return obj->isLocked();
}

//  Determine if this object is unlocked
static bool objIsUnlocked(GameObject *obj) {
	return !obj->isLocked();
}

//  Determine if this object is a key
static bool objIsKey(GameObject *obj) {
	return obj->proto()->classType == protoClassKey;
}

//  Determine if this object is a player actor
static bool objIsPlayerActor(GameObject *obj) {
	return      isActor(obj)
	            &&  isPlayerActor((Actor *)obj);
}

//  Determine if this object is an enemy of the protaganists
static bool objIsEnemy(GameObject *obj) {
	return      isActor(obj)
	            &&  isEnemy((Actor *)obj);
}

const SimpleObjectProperty objPropObject(objIsObject);
const SimpleObjectProperty objPropActor(objIsActor);
const SimpleObjectProperty objPropWorld(objIsWorld);
const SimpleObjectProperty objPropLocked(objIsLocked);
const SimpleObjectProperty objPropUnlocked(objIsUnlocked);
const SimpleObjectProperty objPropKey(objIsKey);
const SimpleObjectProperty objPropPlayerActor(objIsPlayerActor);
const SimpleObjectProperty objPropEnemy(objIsEnemy);

const ObjectProperty *objPropArray[objPropIDCount] = {
	&objPropObject,
	&objPropActor,
	&objPropWorld,
	&objPropLocked,
	&objPropUnlocked,
	&objPropKey,
	&objPropPlayerActor,
	&objPropEnemy,
};

/* ===================================================================== *
   Actor properties
 * ===================================================================== */

#ifdef FTA
//  Determine if this actor is dead
static bool actorIsDead(Actor *a) {
	return a->isDead();
}
#endif

//  Determine if this actor is the center actor
static bool actorIsCenterActor(Actor *a) {
	return a == getCenterActor();
}

//  Determine if this actor is a player actor
static bool actorIsPlayerActor(Actor *a) {
	return isPlayerActor(a);
}

//  Determine if this actor is an enemy of the protaganists
static bool actorIsEnemy(Actor *a) {
	return isEnemy(a);
}

#ifdef FTA
const SimpleActorProperty actorPropDead(actorIsDead);
#endif
const SimpleActorProperty actorPropCenterActor(actorIsCenterActor);
const SimpleActorProperty actorPropPlayerActor(actorIsPlayerActor);
const SimpleActorProperty actorPropEnemy(actorIsEnemy);

const ActorProperty *actorPropArray[actorPropIDCount] = {
#ifdef FTA
	&actorPropDead,
#endif
	&actorPropCenterActor,
	&actorPropPlayerActor,
	&actorPropEnemy,
};

/* ===================================================================== *
   Tile properties
 * ===================================================================== */

//  Determine if this tile has water
static bool tileHasWater(TileInfo *ti) {
	return (ti->combinedTerrainMask() & terrainWater) ? true : false;
}

const SimpleTileProperty tilePropHasWater(tileHasWater);

const TileProperty *tilePropArray[tilePropIDCount] = {
	&tilePropHasWater,
};

/* ===================================================================== *
   MetaTile properties
 * ===================================================================== */

/* ===================================================================== *
   SimpleMetaTileProperty class member functions
 * ===================================================================== */

bool SimpleMetaTileProperty::operator()(
    MetaTile *mt,
    int16 mapNum,
    const TilePoint &tp) const {
	//  Simply pass this call to the property evaluation function
	return (*propertyFunc)(mt, mapNum, tp);
}

/* ===================================================================== *
   CompoundMetaTileProperty class member functions
 * ===================================================================== */

CompoundMetaTileProperty::CompoundMetaTileProperty(
    MetaTileProperty **array,
    uint16 size) {
	//  Calculate the number of bytes required to copy the array
	uint16  arrayBytes = sizeof(MetaTileProperty *) * size;

	//  Allocate memory for a copy of the array
	propertyArray = (MetaTileProperty **)malloc(arrayBytes);
#if DEBUG
	assert(propertyArray);
#endif
	//  Copy the array
	memcpy(propertyArray, array, arrayBytes);
	arraySize = size;
}

CompoundMetaTileProperty::~CompoundMetaTileProperty(void) {
	//  Free the memory for the copy of the array
	free(propertyArray);
}

/* ===================================================================== *
   MetaTilePropertyAnd class member functions
 * ===================================================================== */

bool MetaTilePropertyAnd::operator()(
    MetaTile *mt,
    int16 mapNum,
    const TilePoint &tp) const {
	uint16  i;

	//  Iterate through each element in the array and if any evaluate to
	//  false, return false immediately.
	for (i = 0; i < arraySize; i++)
		if ((*propertyArray[i])(mt, mapNum, tp) == false) return false;

	return true;
}

/* ===================================================================== *
   MetaTilePropertyOr class member functions
 * ===================================================================== */

bool MetaTilePropertyOr::operator()(
    MetaTile *mt,
    int16 mapNum,
    const TilePoint &tp) const {
	uint16  i;

	//  Iterate through each element in the array and if any evaluate to
	//  true, return true immediately.
	for (i = 0; i < arraySize; i++)
		if ((*propertyArray[i])(mt, mapNum, tp)) return true;

	return false;
}


//  Determine if this MetaTile has water
static bool metaTileHasWater(
    MetaTile *mt,
    int16 mapNum,
    const TilePoint &mCoords) {
	TilePoint   origin = mCoords << kPlatShift,
	            tCoords;

	tCoords.z = 0;
	for (int i = 0; i < maxPlatforms; i++) {
		Platform *p = mt->fetchPlatform(mapNum, i);

		if (p) {
			for (tCoords.u = 0; tCoords.u < kPlatformWidth; tCoords.u++) {
				for (tCoords.v = 0; tCoords.v < kPlatformWidth; tCoords.v++) {
					TileInfo        *ti;
					int16           height;
					int16           trFlags;

					ti =    p->fetchTile(
					            mapNum,
					            tCoords,
					            origin,
					            height,
					            trFlags);
					if (ti->combinedTerrainMask() & terrainWater)
						return true;
				}
			}
		}
	}

	return false;
}

const SimpleMetaTileProperty metaTilePropHasWater(metaTileHasWater);

const MetaTileProperty *metaTilePropArray[metaTilePropIDCount] = {
	&metaTilePropHasWater,
};

} // end of namespace Saga2
