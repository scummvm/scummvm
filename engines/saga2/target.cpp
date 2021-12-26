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
#include "saga2/cmisc.h"
#include "saga2/objects.h"
#include "saga2/target.h"
#include "saga2/actor.h"
#include "saga2/tile.h"

namespace Saga2 {

const int   metaTileUVSize = kTileUVSize * kPlatformWidth;

/* ===================================================================== *
   Target management functions
 * ===================================================================== */

void deleteTarget(Target *t) {
	if (t) delete t;
}

void readTarget(void *mem, Common::InSaveFile *in) {
	int16 type = in->readSint16LE();

	switch (type) {
	case locationTarget:
		new (mem) LocationTarget(in);
		break;

	case specificTileTarget:
		new (mem) SpecificTileTarget(in);
		break;

	case tilePropertyTarget:
		new (mem) TilePropertyTarget(in);
		break;

	case specificMetaTileTarget:
		new (mem) SpecificMetaTileTarget(in);
		break;

	case metaTilePropertyTarget:
		new (mem) MetaTilePropertyTarget(in);
		break;

	case specificObjectTarget:
		new (mem)  SpecificObjectTarget(in);
		break;

	case objectPropertyTarget:
		new (mem)  ObjectPropertyTarget(in);
		break;

	case specificActorTarget:
		new (mem) SpecificActorTarget(in);
		break;

	case actorPropertyTarget:
		new (mem) ActorPropertyTarget(in);
		break;
	}
}

void writeTarget(const Target *t, Common::MemoryWriteStreamDynamic *out) {
	out->writeSint16LE(t->getType());

	t->write(out);
}

int32 targetArchiveSize(const Target *t) {
	return sizeof(int16) + t->archiveSize();
}

//  Insert a location into a TargetLocationArray, using a simple
//  insertion sort
void insertLocation(
    const TilePoint &tp,
    int16 dist,
    TargetLocationArray &tla) {
	int16   i = tla.locs;

	//  If there are no locations in the array place this location in
	//  the first element
	if (i == 0) {
		tla.locs = 1;
		tla.locArray[0] = tp;
		tla.distArray[0] = dist;
	} else {
		//  Search for a position to place the location
		if (dist < tla.distArray[i - 1]) {
			if (i < tla.size) {
				tla.locArray[i] = tla.locArray[i - 1];
				tla.distArray[i] = tla.distArray[i - 1];
			}
			i--;
		}

		while (i > 0 && dist < tla.distArray[i - 1]) {
			tla.locArray[i] = tla.locArray[i - 1];
			tla.distArray[i] = tla.distArray[i - 1];
			i--;
		}

		if (i < tla.size) {
			//  Place the location in the array
			if (tla.locs < tla.size) tla.locs++;
			tla.locArray[i] = tp;
			tla.distArray[i] = dist;
		}
	}
}

/* ===================================================================== *
   Target member functions
 * ===================================================================== */

bool Target::isObjectTarget() const {
	return false;
}
bool Target::isActorTarget() const {
	return false;
}

/* ===================================================================== *
   LocationTarget member functions
 * ===================================================================== */

LocationTarget::LocationTarget(Common::SeekableReadStream *stream) {
	debugC(5, kDebugSaveload, "...... LocationTarget");

	//  Restore the targe location
	loc.load(stream);
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 LocationTarget::archiveSize() const {
	return sizeof(loc);
}

void LocationTarget::write(Common::MemoryWriteStreamDynamic *out) const {
	//  Store the target location
	loc.write(out);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of target

int16 LocationTarget::getType() const {
	return locationTarget;
}

//----------------------------------------------------------------------
//	Virtual function returning the sizeof this target

size_t LocationTarget::size() const {
	return sizeof(*this);
}

//----------------------------------------------------------------------
//	Create a copy of this target at the specified address

void LocationTarget::clone(void *mem) const {
	new (mem) LocationTarget(*this);
}

//----------------------------------------------------------------------
//	Determine if the specified target is equivalent to this target

bool LocationTarget::operator == (const Target &t) const {
	if (t.getType() != locationTarget) return false;

	const LocationTarget *targetPtr = (const LocationTarget *)&t;

	return *this == *targetPtr;
}

TilePoint LocationTarget::where(GameWorld *, const TilePoint &) const {
	return loc;
}

int16 LocationTarget::where(
    GameWorld *,
    const TilePoint &tp,
    TargetLocationArray &tla) const {
	//  Place the target location in the first element of the
	//  array
	tla.locArray[0] = loc;
	tla.distArray[0] = (tp - loc).quickHDistance();
	tla.locs = 1;
	return 1;
}

/* ===================================================================== *
   TileTarget member functions
 * ===================================================================== */

TilePoint TileTarget::where(GameWorld *world, const TilePoint &tp) const {
	uint16              bestDist = maxuint16;
	TileInfo   *ti;
	TilePoint           tileCoords,
	                    bestTCoords = Nowhere;
	TileRegion          tileReg;
	StandingTileInfo    sti;

	//  Compute the tile region to search
	tileReg.min.u = (tp.u - maxTileDist) >> kTileUVShift;
	tileReg.max.u = (tp.u + maxTileDist - 1 + kTileUVMask)
	                >>  kTileUVShift;
	tileReg.min.v = (tp.v - maxTileDist) >> kTileUVShift;
	tileReg.max.v = (tp.v + maxTileDist - 1 + kTileUVMask)
	                >>  kTileUVShift;

	TileIterator        tIter(world->mapNum, tileReg);

	//  Get the first tile in tile region
	ti = tIter.first(&tileCoords, &sti);
	while (ti != nullptr) {
		//  If this has the tile ID we are looking for
		if (isTarget(sti)) {
			uint16  dist;

			tileCoords.u <<= kTileUVShift;
			tileCoords.v <<= kTileUVShift;

			//  Compute point on tile closest to center location
			tileCoords.u =  clamp(
			                    tileCoords.u,
			                    tp.u,
			                    tileCoords.u + kTileUVSize - 1);
			tileCoords.v =  clamp(
			                    tileCoords.v,
			                    tp.v,
			                    tileCoords.v + kTileUVSize - 1);
			tileCoords.z = sti.surfaceHeight;

			dist = (tileCoords - tp).quickHDistance();

			if (dist < bestDist) {
				bestTCoords = tileCoords;
				bestDist = dist;
			}
		}

		//  Get the next tile in tile region
		ti = tIter.next(&tileCoords, &sti);
	}

	return bestTCoords;
}

int16 TileTarget::where(
    GameWorld *world,
    const TilePoint &tp,
    TargetLocationArray &tla) const {
	TileInfo   *ti;
	TilePoint           tileCoords;
	TileRegion          tileReg;
	StandingTileInfo    sti;

	//  Compute the tile region to search
	tileReg.min.u = (tp.u - maxTileDist) >> kTileUVShift;
	tileReg.max.u = (tp.u + maxTileDist - 1 + kTileUVMask)
	                >>  kTileUVShift;
	tileReg.min.v = (tp.v - maxTileDist) >> kTileUVShift;
	tileReg.max.v = (tp.v + maxTileDist - 1 + kTileUVMask)
	                >>  kTileUVShift;

	TileIterator        tIter(world->mapNum, tileReg);

	//  Get the first tile in tile region
	ti = tIter.first(&tileCoords, &sti);
	while (ti != nullptr) {
		//  Determine if this tile has tile ID we're looking
		//  for
		if (isTarget(sti)) {
			uint16  dist;

			tileCoords.u <<= kTileUVShift;
			tileCoords.v <<= kTileUVShift;

			//  Compute point on tile closest to center location
			tileCoords.u =  clamp(
			                    tileCoords.u,
			                    tp.u,
			                    tileCoords.u + kTileUVSize - 1);
			tileCoords.v =  clamp(
			                    tileCoords.v,
			                    tp.v,
			                    tileCoords.v + kTileUVSize - 1);
			tileCoords.z = sti.surfaceHeight;

			dist = (tileCoords - tp).quickHDistance();

			//  Insert the location into the array
			insertLocation(tileCoords, dist, tla);
		}

		//  Get the next tile in tile region
		ti = tIter.next(&tileCoords, &sti);
	}

	return tla.locs;
}

/* ===================================================================== *
   SpecificTileTarget member functions
 * ===================================================================== */

SpecificTileTarget::SpecificTileTarget(Common::SeekableReadStream *stream) {
	debugC(5, kDebugSaveload, "...... SpecificTileTarget");

	//  Restore the tile ID
	tile = stream->readUint16LE();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 SpecificTileTarget::archiveSize() const {
	return sizeof(tile);
}

void SpecificTileTarget::write(Common::MemoryWriteStreamDynamic *out) const {
	//  Store the tile ID
	out->writeUint16LE(tile);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of target

int16 SpecificTileTarget::getType() const {
	return specificTileTarget;
}

//----------------------------------------------------------------------
//	Virtual function returning the sizeof this target

size_t SpecificTileTarget::size() const {
	return sizeof(*this);
}

//----------------------------------------------------------------------
//	Create a copy of this target at the specified address

void SpecificTileTarget::clone(void *mem) const {
	new (mem) SpecificTileTarget(*this);
}

//----------------------------------------------------------------------
//	Determine if the specified target is equivalent to this target

bool SpecificTileTarget::operator == (const Target &t) const {
	if (t.getType() != specificTileTarget) return false;

	const SpecificTileTarget *targetPtr = (const SpecificTileTarget *)&t;

	return tile == targetPtr->tile;
}

bool SpecificTileTarget::isTarget(StandingTileInfo &sti) const {
	return sti.surfaceRef.tile == tile;
}

/* ===================================================================== *
   TilePropertyTarget member functions
 * ===================================================================== */

TilePropertyTarget::TilePropertyTarget(Common::SeekableReadStream *stream) {
	debugC(5, kDebugSaveload, "...... TilePropertyTarget");

	//  Restore the TilePropertyID
	tileProp = stream->readSint16LE();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 TilePropertyTarget::archiveSize() const {
	return sizeof(tileProp);
}

void TilePropertyTarget::write(Common::MemoryWriteStreamDynamic *out) const {
	out->writeSint16LE(tileProp);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of target

int16 TilePropertyTarget::getType() const {
	return tilePropertyTarget;
}

//----------------------------------------------------------------------
//	Virtual function returning the sizeof this target

size_t TilePropertyTarget::size() const {
	return sizeof(*this);
}

//----------------------------------------------------------------------
//	Create a copy of this target at the specified address

void TilePropertyTarget::clone(void *mem) const {
	new (mem) TilePropertyTarget(*this);
}

//----------------------------------------------------------------------
//	Determine if the specified target is equivalent to this target

bool TilePropertyTarget::operator == (const Target &t) const {
	if (t.getType() != tilePropertyTarget) return false;

	const TilePropertyTarget  *targetPtr = (const TilePropertyTarget *)&t;

	return tileProp == targetPtr->tileProp;
}

bool TilePropertyTarget::isTarget(StandingTileInfo &sti) const {
	return sti.surfaceTile->hasProperty(*g_vm->_properties->getTileProp(tileProp));
}

/* ===================================================================== *
   MetaTileTarget member functions
 * ===================================================================== */

TilePoint MetaTileTarget::where(
    GameWorld *world,
    const TilePoint &tp) const {
	uint16              bestDist = maxuint16;
	MetaTile   *mt;
	TilePoint           metaCoords,
	                    bestMCoords = Nowhere;
	TileRegion          tileReg;

	//  Determine the tile region to search
	tileReg.min.u = (tp.u - maxMetaDist) >> kTileUVShift;
	tileReg.max.u = (tp.u + maxMetaDist + kTileUVMask)
	                >>  kTileUVShift;
	tileReg.min.v = (tp.v - maxMetaDist) >> kTileUVShift;
	tileReg.max.v = (tp.v + maxMetaDist + kTileUVMask)
	                >>  kTileUVShift;

	MetaTileIterator    mIter(world->mapNum, tileReg);

	//  get the first metatile in region
	mt = mIter.first(&metaCoords);
	while (mt != nullptr) {
		if (isTarget(mt, world->mapNum, metaCoords)) {
			uint16  dist;

			metaCoords.u <<= kTileUVShift;
			metaCoords.v <<= kTileUVShift;

			//  Determine point on metatile closest to center point
			metaCoords.u =  clamp(
			                    metaCoords.u,
			                    tp.u,
			                    metaCoords.u + metaTileUVSize - 1);
			metaCoords.v =  clamp(
			                    metaCoords.v,
			                    tp.v,
			                    metaCoords.v + metaTileUVSize - 1);

			dist = (metaCoords - tp).quickHDistance();

			if (dist < bestDist) {
				bestMCoords = metaCoords;
				bestDist = dist;
			}
		}
		//  Get the next metatile in region
		mt = mIter.next(&metaCoords);
	}

	if (bestMCoords == Nowhere) return Nowhere;

	metaCoords.z = tp.z;
	return metaCoords;
}

int16 MetaTileTarget::where(
    GameWorld *world,
    const TilePoint &tp,
    TargetLocationArray &tla) const {
	MetaTile   *mt;
	TilePoint           metaCoords;
	TileRegion          tileReg;

	//  Compute the tile region to search
	tileReg.min.u = (tp.u - maxMetaDist) >> kTileUVShift;
	tileReg.max.u = (tp.u + maxMetaDist + kTileUVMask)
	                >>  kTileUVShift;
	tileReg.min.v = (tp.v - maxMetaDist) >> kTileUVShift;
	tileReg.max.v = (tp.v + maxMetaDist + kTileUVMask)
	                >>  kTileUVShift;

	MetaTileIterator    mIter(world->mapNum, tileReg);

	//  Get the first metatile in tile region
	mt = mIter.first(&metaCoords);
	while (mt != nullptr) {
		if (isTarget(mt, world->mapNum, metaCoords)) {
			uint16  dist;

			metaCoords.u <<= kTileUVShift;
			metaCoords.v <<= kTileUVShift;

			metaCoords.z = tp.z;

			//  Compute point on metatile closest to center
			//  point
			metaCoords.u = clamp(metaCoords.u,
			                     tp.u,
			                     metaCoords.u + metaTileUVSize - 1);
			metaCoords.v = clamp(metaCoords.v,
			                     tp.v,
			                     metaCoords.v + metaTileUVSize - 1);

			dist = (metaCoords - tp).quickHDistance();

			insertLocation(metaCoords, dist, tla);
		}

		//  Get the next metatile in tile region
		mt = mIter.next(&metaCoords);
	}

	return tla.locs;
}

/* ===================================================================== *
   SpecificMetaTileTarget member functions
 * ===================================================================== */

SpecificMetaTileTarget::SpecificMetaTileTarget(Common::SeekableReadStream *stream) {
	debugC(5, kDebugSaveload, "...... SpecificMetaTileTarget");

	//  Restore the MetaTileID
	meta.map = stream->readSint16LE();
	meta.index = stream->readSint16LE();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 SpecificMetaTileTarget::archiveSize() const {
	return sizeof(MetaTileID);
}

void SpecificMetaTileTarget::write(Common::MemoryWriteStreamDynamic *out) const {
	//  Store the MetaTileID
	out->writeSint16LE(meta.map);
	out->writeSint16LE(meta.index);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of target

int16 SpecificMetaTileTarget::getType() const {
	return specificMetaTileTarget;
}

//----------------------------------------------------------------------
//	Virtual function returning the sizeof this target

size_t SpecificMetaTileTarget::size() const {
	return sizeof(*this);
}

//----------------------------------------------------------------------
//	Create a copy of this target at the specified address

void SpecificMetaTileTarget::clone(void *mem) const {
	new (mem) SpecificMetaTileTarget(*this);
}

//----------------------------------------------------------------------
//	Determine if the specified target is equivalent to this target

bool SpecificMetaTileTarget::operator == (const Target &t) const {
	if (t.getType() != specificMetaTileTarget) return false;

	const SpecificMetaTileTarget  *targetPtr = (const SpecificMetaTileTarget *)&t;

	return meta == targetPtr->meta;
}

bool SpecificMetaTileTarget::isTarget(
    MetaTile *mt,
    int16 mapNum,
    const TilePoint &) const {
	return mt->thisID(mapNum) == meta;
}

/* ===================================================================== *
   MetaTilePropertyTarget member functions
 * ===================================================================== */

MetaTilePropertyTarget::MetaTilePropertyTarget(Common::SeekableReadStream *stream) {
	debugC(5, kDebugSaveload, "...... MetaTilePropertyTarget");

	//  Restore the MetaTilePropertyID
	metaProp = stream->readSint16LE();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 MetaTilePropertyTarget::archiveSize() const {
	return sizeof(metaProp);
}

void MetaTilePropertyTarget::write(Common::MemoryWriteStreamDynamic *out) const {
	//  Store the MetaTilePropertyID
	out->writeSint16LE(metaProp);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of target

int16 MetaTilePropertyTarget::getType() const {
	return metaTilePropertyTarget;
}

//----------------------------------------------------------------------
//	Virtual function returning the sizeof this target

size_t MetaTilePropertyTarget::size() const {
	return sizeof(*this);
}

//----------------------------------------------------------------------
//	Create a copy of this target at the specified address

void MetaTilePropertyTarget::clone(void *mem) const {
	new (mem) MetaTilePropertyTarget(*this);
}

//----------------------------------------------------------------------
//	Determine if the specified target is equivalent to this target

bool MetaTilePropertyTarget::operator == (const Target &t) const {
	if (t.getType() != metaTilePropertyTarget) return false;

	const MetaTilePropertyTarget  *targetPtr = (const MetaTilePropertyTarget *)&t;

	return metaProp == targetPtr->metaProp;
}

bool MetaTilePropertyTarget::isTarget(
    MetaTile *mt,
    int16 mapNum,
    const TilePoint &tp) const {
	return mt->hasProperty(*g_vm->_properties->getMetaTileProp(metaProp), mapNum, tp);
}

/* ===================================================================== *
   ObjectTarget member functions
 * ===================================================================== */

//  These are recursive functions used to fill a target array
//  by inspecting an object and all of the objects contained in
//  that object
void ObjectTarget::searchObject(
    GameObject *objPtr,
    int16 dist,
    TargetObjectArray &toa) const {
	//  Determine if this object meets the target criterion
	if (isTarget(objPtr)) {
		int16   i = toa.objs;

		//  If there are no objects in the array place this object in
		//  the first element
		if (i == 0) {
			toa.objs = 1;
			toa.objArray[0] = objPtr;
			toa.distArray[0] = dist;
		} else {
			//  Search for a position to place the object
			if (dist < toa.distArray[i - 1]) {
				if (i < toa.size) {
					toa.objArray[i] = toa.objArray[i - 1];
					toa.distArray[i] = toa.distArray[i - 1];
				}
				i--;
			}

			while (i > 0 && dist < toa.distArray[i - 1]) {
				toa.objArray[i] = toa.objArray[i - 1];
				toa.distArray[i] = toa.distArray[i - 1];
				i--;
			}

			if (i < toa.size) {
				//  Place the object in the array
				if (toa.objs < toa.size) toa.objs++;
				toa.objArray[i] = objPtr;
				toa.distArray[i] = dist;
			}
		}
	}

	//  If this object is containing other objects search the contained
	//  objects
	if (objPtr->IDChild() != Nothing) {
		ContainerIterator   iter(objPtr);
		GameObject *childPtr;

		while (iter.next(&childPtr) != Nothing)
			searchObject(childPtr, dist, toa);
	}
}

void ObjectTarget::searchObject(
    GameObject *objPtr,
    const TilePoint &tp,
    int16 dist,
    TargetLocationArray &tla) const {
	//  Determine if this object meets the target criterion
	if (isTarget(objPtr))
		insertLocation(tp, dist, tla);

	//  If this object is containing other objects search the contained
	//  objects
	if (objPtr->IDChild() != Nothing) {
		ContainerIterator   iter(objPtr);
		GameObject *childPtr;

		while (iter.next(&childPtr) != Nothing)
			searchObject(childPtr, tp, dist, tla);
	}
}

bool ObjectTarget::isObjectTarget() const {
	return true;
}

TilePoint ObjectTarget::where(GameWorld *world, const TilePoint &tp) const {
	int16                   dist,
	                        bestDist = maxint16;
	GameObject     *objPtr = nullptr;
	TilePoint               objCoords,
	                        bestOCoords = Nowhere;
	CircularObjectIterator  iter(world, tp, maxObjDist);

	//  Iterate through each object in the vicinity
	for (iter.first(&objPtr, &dist);
	        objPtr != nullptr;
	        iter.next(&objPtr, &dist)) {
		//  Skip this object if we've already found a closer
		//  object
		if (dist > 0 && dist < bestDist) {
			objCoords = objPtr->getLocation();

			//  Determine if object has property
			if (isTarget(objPtr)) {
				bestOCoords = objCoords;
				bestDist = dist;
			}
			//  If not, determine if object has child objects
			else if (objPtr->IDChild() != Nothing) {
				RecursiveContainerIterator  cIter(objPtr);

				//  Iterate through each descendant object
				cIter.first(&objPtr);
				while (objPtr) {
					//  Determine if this object has property
					if (isTarget(objPtr)) {
						bestOCoords = objCoords;
						bestDist = dist;
						break;
					}
					cIter.next(&objPtr);
				}
			}
		}
	}

	return bestOCoords;
}

int16 ObjectTarget::where(
    GameWorld *world,
    const TilePoint &tp,
    TargetLocationArray &tla) const {
	CircularObjectIterator  objIter(world, tp, maxObjDist);

	GameObject     *objPtr;
	ObjectID                id;
	int16                   dist;

	//  Iterate through every object in the vicinity and
	//  fill the target array
	for (id = objIter.first(&objPtr, &dist);
	        id != Nothing;
	        id = objIter.next(&objPtr, &dist))
		if (dist > 0)
			searchObject(objPtr, objPtr->getLocation(), dist, tla);

	return tla.locs;
}

GameObject *ObjectTarget::object(
    GameWorld *world,
    const TilePoint &tp) const {
	CircularObjectIterator  objIter(world, tp, maxObjDist);

	GameObject     *objPtr,
	               *bestObj = nullptr;
	int16                   dist,
	                        bestDist = maxint16;

	//  Iterate through each object in the vicinity
	for (objIter.first(&objPtr, &dist);
	        objPtr != nullptr;
	        objIter.next(&objPtr, &dist)) {
		//  Skip this object if we have already found a closer object
		if (dist > 0 && dist < bestDist) {
			//  Determine if the object has the property we are
			//  looking for
			if (isTarget(objPtr)) {
				bestObj = objPtr;
				bestDist = dist;
			}
			//  If not, determine if the object has child objects
			else if (objPtr->IDChild() != Nothing) {
				RecursiveContainerIterator  cIter(objPtr);

				//  Iterate though each descendant object
				cIter.first(&objPtr);
				while (objPtr) {
					//  Determine if object has the property we are
					//  looking for
					if (isTarget(objPtr)) {
						bestObj = objPtr;
						bestDist = dist;
						break;
					}
					cIter.next(&objPtr);
				}
			}
		}
	}

	return bestObj;
}

int16 ObjectTarget::object(
    GameWorld *world,
    const TilePoint &tp,
    TargetObjectArray &toa) const {
	CircularObjectIterator  objIter(world, tp, maxObjDist);

	GameObject     *objPtr;
	ObjectID                id;
	int16                   dist;

	//  Iterate through each object in the vicinity and fill the
	//  array.
	for (id = objIter.first(&objPtr, &dist);
	        id != Nothing;
	        id = objIter.next(&objPtr, &dist))
		if (dist > 0)
			searchObject(objPtr, dist, toa);

	return toa.objs;
}

/* ===================================================================== *
   SpecificObjectTarget member functions
 * ===================================================================== */

SpecificObjectTarget::SpecificObjectTarget(Common::SeekableReadStream *stream) {
	debugC(5, kDebugSaveload, "...... SpecificObjectTarget");

	//  Restore the ObjectID
	obj = stream->readUint16LE();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 SpecificObjectTarget::archiveSize() const {
	return sizeof(obj);
}

void SpecificObjectTarget::write(Common::MemoryWriteStreamDynamic *out) const {
	//  Store the ObjectID
	out->writeUint16LE(obj);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of target

int16 SpecificObjectTarget::getType() const {
	return specificObjectTarget;
}

//----------------------------------------------------------------------
//	Virtual function returning the sizeof this target

size_t SpecificObjectTarget::size() const {
	return sizeof(*this);
}

//----------------------------------------------------------------------
//	Create a copy of this target at the specified address

void SpecificObjectTarget::clone(void *mem) const {
	new (mem) SpecificObjectTarget(*this);
}

//----------------------------------------------------------------------
//	Determine if the specified target is equivalent to this target

bool SpecificObjectTarget::operator == (const Target &t) const {
	if (t.getType() != specificObjectTarget) return false;

	const SpecificObjectTarget    *targetPtr = (const SpecificObjectTarget *)&t;

	return obj == targetPtr->obj;
}

//----------------------------------------------------------------------
//	Determine if the specified object is the specific object we're looking
//	for

bool SpecificObjectTarget::isTarget(GameObject *testObj) const {
	return testObj->thisID() == obj;
}

//----------------------------------------------------------------------
//	Return the location of the specific object if it is in the specified
//	world and within the maximum distance of the specified point

TilePoint SpecificObjectTarget::where(
    GameWorld *world,
    const TilePoint &tp) const {
	GameObject *o = GameObject::objectAddress(obj);

	if (o->world() == world) {
		TilePoint   objLoc = o->getLocation();

		if ((tp - objLoc).quickHDistance() < maxObjDist)
			return objLoc;
	}

	return Nowhere;
}

//----------------------------------------------------------------------
//	Return the location of the specific object if it is in the specified
//	world and within the maximum distance of the specified point

int16 SpecificObjectTarget::where(
    GameWorld *world,
    const TilePoint &tp,
    TargetLocationArray &tla) const {
	GameObject *o = GameObject::objectAddress(obj);

	if (tla.size > 0 && o->world() == world) {
		TilePoint   objLoc = o->getLocation();
		int16       dist = (tp - objLoc).quickHDistance();

		if (dist < maxObjDist) {
			tla.locs = 1;
			tla.locArray[0] = objLoc;
			tla.distArray[0] = dist;

			return 1;
		}
	}

	return 0;
}

//----------------------------------------------------------------------
//	Return a pointer to the specific object if it is in the specified
//	world and within the maximum distance of the specified point

GameObject *SpecificObjectTarget::object(
    GameWorld *world,
    const TilePoint &tp) const {
	GameObject *o = GameObject::objectAddress(obj);

	if (o->world() == world) {
		if ((tp - o->getLocation()).quickHDistance() < maxObjDist)
			return o;
	}

	return nullptr;
}

//----------------------------------------------------------------------
//	Return a pointer to the specific object if it is in the specified
//	world and within the maximum distance of the specified point

int16 SpecificObjectTarget::object(
    GameWorld *world,
    const TilePoint &tp,
    TargetObjectArray &toa) const {
	GameObject *o = GameObject::objectAddress(obj);

	if (toa.size > 0 && o->world() == world) {
		int16       dist = (tp - o->getLocation()).quickHDistance();

		if (dist < maxObjDist) {
			toa.objs = 1;
			toa.objArray[0] = o;
			toa.distArray[0] = dist;

			return 1;
		}
	}

	return 0;
}

/* ===================================================================== *
   ObjectPropertyTarget member functions
 * ===================================================================== */

ObjectPropertyTarget::ObjectPropertyTarget(Common::SeekableReadStream *stream) {
	debugC(5, kDebugSaveload, "...... ObjectPropertyTarget");

	//  Restore the ObjectPropertyID
	objProp = stream->readSint16LE();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 ObjectPropertyTarget::archiveSize() const {
	return sizeof(objProp);
}

void ObjectPropertyTarget::write(Common::MemoryWriteStreamDynamic *out) const {
	//  Store the ObjectPropertyID
	out->writeSint16LE(objProp);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of target

int16 ObjectPropertyTarget::getType() const {
	return objectPropertyTarget;
}

//----------------------------------------------------------------------
//	Virtual function returning the sizeof this target

size_t ObjectPropertyTarget::size() const {
	return sizeof(*this);
}

//----------------------------------------------------------------------
//	Create a copy of this target at the specified address

void ObjectPropertyTarget::clone(void *mem) const {
	new (mem) ObjectPropertyTarget(*this);
}

//----------------------------------------------------------------------
//	Determine if the specified target is equivalent to this target

bool ObjectPropertyTarget::operator == (const Target &t) const {
	if (t.getType() != objectPropertyTarget) return false;

	const ObjectPropertyTarget *targetPtr = (const ObjectPropertyTarget *)&t;

	return objProp == targetPtr->objProp;
}

bool ObjectPropertyTarget::isTarget(GameObject *testObj) const {
	return testObj->hasProperty(*g_vm->_properties->getObjProp(objProp));
}

/* ===================================================================== *
   ActorTarget member functions
 * ===================================================================== */

bool ActorTarget::isTarget(GameObject *obj) const {
	return isActor(obj) && isTarget((Actor *)obj);
}

bool ActorTarget::isActorTarget() const {
	return true;
}

Actor *ActorTarget::actor(GameWorld *world, const TilePoint &tp) const {
	return (Actor *)object(world, tp);
}

int16 ActorTarget::actor(
    GameWorld *world,
    const TilePoint &tp,
    TargetActorArray &taa) const {
	int16               result;
	TargetObjectArray   toa(
	    taa.size,
	    (GameObject **)taa.actorArray,
	    taa.distArray);

	result = object(world, tp, toa);
	taa.actors = toa.objs;

	return result;
}

/* ===================================================================== *
   SpecificActorTarget member functions
 * ===================================================================== */

SpecificActorTarget::SpecificActorTarget(Common::SeekableReadStream *stream) {
	debugC(5, kDebugSaveload, "...... SpecificActorTarget");

	ObjectID actorID;

	//  Get the actor's ID
	actorID = stream->readUint16LE();

	//  Convert the actor ID into an Actor pointer
	a = actorID != Nothing
	    ? (Actor *)GameObject::objectAddress(actorID)
	    :   nullptr;
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 SpecificActorTarget::archiveSize() const {
	return sizeof(ObjectID);
}

void SpecificActorTarget::write(Common::MemoryWriteStreamDynamic *out) const {
	//  Convert the actor pointer to an actor ID;
	ObjectID actorID = a != nullptr ? a->thisID() : Nothing;

	//  Store the actor ID
	out->writeUint16LE(actorID);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of target

int16 SpecificActorTarget::getType() const {
	return specificActorTarget;
}

//----------------------------------------------------------------------
//	Virtual function returning the sizeof this target

size_t SpecificActorTarget::size() const {
	return sizeof(*this);
}

//----------------------------------------------------------------------
//	Create a copy of this target at the specified address

void SpecificActorTarget::clone(void *mem) const {
	new (mem) SpecificActorTarget(*this);
}

//----------------------------------------------------------------------
//	Determine if the specified target is equivalent to this target

bool SpecificActorTarget::operator == (const Target &t) const {
	if (t.getType() != specificActorTarget) return false;

	const SpecificActorTarget *targetPtr = (const SpecificActorTarget *)&t;

	return a == targetPtr->a;
}

//----------------------------------------------------------------------
//	Determine if the specified actor is the specific actor we're looking
//	for

bool SpecificActorTarget::isTarget(Actor *testActor) const {
	return testActor == a;
}

//----------------------------------------------------------------------
//	Return the location of the specific actor if it is in the specified
//	world and within the maximum distance of the specified point

TilePoint SpecificActorTarget::where(
    GameWorld *world,
    const TilePoint &tp) const {
	if (a->world() == world) {
		TilePoint   actorLoc = a->getLocation();

		if ((tp - actorLoc).quickHDistance() < maxObjDist)
			return actorLoc;
	}

	return Nowhere;
}

//----------------------------------------------------------------------
//	Return the location of the specific actor if it is in the specified
//	world and within the maximum distance of the specified point

int16 SpecificActorTarget::where(
    GameWorld *world,
    const TilePoint &tp,
    TargetLocationArray &tla) const {
	if (tla.size > 0 && a->world() == world) {
		TilePoint   actorLoc = a->getLocation();
		int16       dist = (tp - actorLoc).quickHDistance();

		if (dist < maxObjDist) {
			tla.locs = 1;
			tla.locArray[0] = actorLoc;
			tla.distArray[0] = dist;

			return 1;
		}
	}

	return 0;
}

//----------------------------------------------------------------------
//	Return an object pointer to the specific actor if it is in the specified
//	world and within the maximum distance of the specified point

GameObject *SpecificActorTarget::object(
    GameWorld *world,
    const TilePoint &tp) const {
	if (a->world() == world) {
		if ((tp - a->getLocation()).quickHDistance() < maxObjDist)
			return a;
	}

	return nullptr;
}

//----------------------------------------------------------------------
//	Return an object pointer to the specific actor if it is in the specified
//	world and within the maximum distance of the specified point

int16 SpecificActorTarget::object(
    GameWorld *world,
    const TilePoint &tp,
    TargetObjectArray &toa) const {
	if (toa.size > 0 && a->world() == world) {
		int16       dist = (tp - a->getLocation()).quickHDistance();

		if (dist < maxObjDist) {
			toa.objs = 1;
			toa.objArray[0] = a;
			toa.distArray[0] = dist;

			return 1;
		}
	}

	return 0;
}

//----------------------------------------------------------------------
//	Return a pointer to the specific actor if it is in the specified
//	world and within the maximum distance of the specified point

Actor *SpecificActorTarget::actor(
    GameWorld *world,
    const TilePoint &tp) const {
	if (a->world() == world) {
		if ((tp - a->getLocation()).quickHDistance() < maxObjDist)
			return a;
	}

	return nullptr;
}

//----------------------------------------------------------------------
//	Return a pointer to the specific actor if it is in the specified
//	world and within the maximum distance of the specified point

int16 SpecificActorTarget::actor(
    GameWorld *world,
    const TilePoint &tp,
    TargetActorArray &taa) const {
	if (taa.size > 0 && a->world() == world) {
		int16       dist = (tp - a->getLocation()).quickHDistance();

		if (dist < maxObjDist) {
			taa.actors = 1;
			taa.actorArray[0] = a;
			taa.distArray[0] = dist;

			return 1;
		}
	}

	return 0;
}

/* ===================================================================== *
   ActorPropertyTarget member functions
 * ===================================================================== */

ActorPropertyTarget::ActorPropertyTarget(Common::SeekableReadStream *stream) {
	debugC(5, kDebugSaveload, "...... ActorPropertyTarget");

	//  Restore the ActorPropertyID
	actorProp = stream->readSint16LE();
}

//----------------------------------------------------------------------
//	Return the number of bytes needed to archive this object in
//	a buffer

inline int32 ActorPropertyTarget::archiveSize() const {
	return sizeof(actorProp);
}

void ActorPropertyTarget::write(Common::MemoryWriteStreamDynamic *out) const {
	//  Store the ActorPropertyID
	out->writeSint16LE(actorProp);
}

//----------------------------------------------------------------------
//	Return an integer representing the type of target

int16 ActorPropertyTarget::getType() const {
	return actorPropertyTarget;
}

//----------------------------------------------------------------------
//	Virtual function returning the sizeof this target

size_t ActorPropertyTarget::size() const {
	return sizeof(*this);
}

//----------------------------------------------------------------------
//	Create a copy of this target at the specified address

void ActorPropertyTarget::clone(void *mem) const {
	new (mem) ActorPropertyTarget(*this);
}

//----------------------------------------------------------------------
//	Determine if the specified target is equivalent to this target

bool ActorPropertyTarget::operator == (const Target &t) const {
	if (t.getType() != actorPropertyTarget) return false;

	const ActorPropertyTarget *targetPtr = (const ActorPropertyTarget *)&t;

	return actorProp == targetPtr->actorProp;
}

bool ActorPropertyTarget::isTarget(Actor *testActor) const {
	return testActor->hasProperty(*g_vm->_properties->getActorProp(actorProp));
}

} // end of namespace Saga2
