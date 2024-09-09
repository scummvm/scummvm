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
#include "saga2/player.h"
#include "saga2/tile.h"

#include "saga2/pclass.r"

namespace Saga2 {

//  Determine if this object is just an object
bool objIsObject(GameObject *obj) {
	return isObject(obj);
}

//  Determine if this object is an actor
bool objIsActor(GameObject *obj) {
	return isActor(obj);
}

//  Determine if this object is a world
bool objIsWorld(GameObject *obj) {
	return isWorld(obj);
}

//  Determine if this object is locked
bool objIsLocked(GameObject *obj) {
	return obj->isLocked();
}

//  Determine if this object is unlocked
bool objIsUnlocked(GameObject *obj) {
	return !obj->isLocked();
}

//  Determine if this object is a key
bool objIsKey(GameObject *obj) {
	return obj->proto()->classType == protoClassKey;
}

//  Determine if this object is a player actor
bool objIsPlayerActor(GameObject *obj) {
	return      isActor(obj)
	            &&  isPlayerActor((Actor *)obj);
}

//  Determine if this object is an enemy of the protagonists
bool objIsEnemy(GameObject *obj) {
	return      isActor(obj)
	            &&  isEnemy((Actor *)obj);
}

/* ===================================================================== *
   Actor properties
 * ===================================================================== */

//  Determine if this actor is dead
bool actorIsDead(Actor *a) {
	return a->isDead();
}

//  Determine if this actor is the center actor
bool actorIsCenterActor(Actor *a) {
	return a == getCenterActor();
}

//  Determine if this actor is a player actor
bool actorIsPlayerActor(Actor *a) {
	return isPlayerActor(a);
}

//  Determine if this actor is an enemy of the protagonists
bool actorIsEnemy(Actor *a) {
	return isEnemy(a);
}

/* ===================================================================== *
   Tile properties
 * ===================================================================== */

//  Determine if this tile has water
bool tileHasWater(TileInfo *ti) {
	return (ti->combinedTerrainMask() & kTerrainWater) ? true : false;
}

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
	return (*_propertyFunc)(mt, mapNum, tp);
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
	_propertyArray = (MetaTileProperty **)malloc(arrayBytes);
#if DEBUG
	assert(_propertyArray);
#endif
	//  Copy the array
	memcpy(_propertyArray, array, arrayBytes);
	_arraySize = size;
}

CompoundMetaTileProperty::~CompoundMetaTileProperty() {
	//  Free the memory for the copy of the array
	free(_propertyArray);
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
	for (i = 0; i < _arraySize; i++)
		if ((*_propertyArray[i])(mt, mapNum, tp) == false) return false;

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
	for (i = 0; i < _arraySize; i++)
		if ((*_propertyArray[i])(mt, mapNum, tp)) return true;

	return false;
}


//  Determine if this MetaTile has water
bool metaTileHasWater(
    MetaTile *mt,
    int16 mapNum,
    const TilePoint &mCoords) {
	TilePoint   origin = mCoords << kPlatShift,
	            tCoords;

	tCoords.z = 0;
	for (int i = 0; i < kMaxPlatforms; i++) {
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
					if (ti->combinedTerrainMask() & kTerrainWater)
						return true;
				}
			}
		}
	}

	return false;
}

Properties::Properties() {
	_objPropArray.push_back(new SimpleObjectProperty(objIsObject));
	_objPropArray.push_back(new SimpleObjectProperty(objIsActor));
	_objPropArray.push_back(new SimpleObjectProperty(objIsWorld));
	_objPropArray.push_back(new SimpleObjectProperty(objIsLocked));
	_objPropArray.push_back(new SimpleObjectProperty(objIsUnlocked));
	_objPropArray.push_back(new SimpleObjectProperty(objIsKey));
	_objPropArray.push_back(new SimpleObjectProperty(objIsPlayerActor));
	_objPropArray.push_back(new SimpleObjectProperty(objIsEnemy));

	_actorPropArray.push_back(new SimpleActorProperty(actorIsDead));
	_actorPropArray.push_back(new SimpleActorProperty(actorIsCenterActor));
	_actorPropArray.push_back(new SimpleActorProperty(actorIsPlayerActor));
	_actorPropArray.push_back(new SimpleActorProperty(actorIsEnemy));

	_tilePropArray.push_back(new SimpleTileProperty(tileHasWater));

	_metaTilePropArray.push_back(new SimpleMetaTileProperty(metaTileHasWater));
}

Properties::~Properties() {
	for (uint i = 0; i < _objPropArray.size(); ++i) {
		if (_objPropArray[i])
			delete _objPropArray[i];
	}

	_objPropArray.clear();

	for (uint i = 0; i < _actorPropArray.size(); ++i) {
		if (_actorPropArray[i])
			delete _actorPropArray[i];
	}

	_actorPropArray.clear();

	for (uint i = 0; i < _tilePropArray.size(); ++i) {
		if (_tilePropArray[i])
			delete _tilePropArray[i];
	}

	_tilePropArray.clear();

	for (uint i = 0; i < _metaTilePropArray.size(); ++i) {
		if (_metaTilePropArray[i])
			delete _metaTilePropArray[i];
	}

	_metaTilePropArray.clear();
}

} // end of namespace Saga2
