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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_TARGET_H
#define SAGA2_TARGET_H

namespace Saga2 {

struct StandingTileInfo;

const   int     maxObjDist = kPlatformWidth * kTileUVSize * 8;
const   int     maxTileDist = kPlatformWidth * kTileUVSize * 2;
const   int     maxMetaDist = kPlatformWidth * kTileUVSize * 8;

enum TargetType {
	locationTarget,
	specificTileTarget,
	tilePropertyTarget,
	specificMetaTileTarget,
	metaTilePropertyTarget,
	specificObjectTarget,
	objectPropertyTarget,
	specificActorTarget,
	actorPropertyTarget
};

/* ===================================================================== *
   Misc. function prototypes
 * ===================================================================== */

class Target;

//  Deletes targets allocated on the heap using new
void deleteTarget(Target *t);

void readTarget(void *mem, Common::InSaveFile *in);
void writeTarget(const Target *t, Common::MemoryWriteStreamDynamic *out);
int32 targetArchiveSize(const Target *t);

/* ===================================================================== *
   TargetLocationArray structure
 * ===================================================================== */

//  This structure is used to query the target class for multiple target
//  locations
struct TargetLocationArray {
	const int16         size;   //  number of allocated elements in array
	int16               locs;   //  number of array elements with valid data

	//  These arrays must be allocated by calling function
	TilePoint *const   locArray;    //  pointer to location array
	int16 *const       distArray;   //  pointer to distance array

	//  Constructor
	TargetLocationArray(int16 s, TilePoint *arr, int16 *dArr) :
		size(s),
		locs(0),
		locArray(arr),
		distArray(dArr) {
	}
};

/* ===================================================================== *
   TargetObjectArray structure
 * ===================================================================== */

//  This structure is used to query the target class for multiple target
//  objects
struct TargetObjectArray {
	const int16         size;   //  number of allocated elements in array
	int16               objs;   //  number of array elements with valid data

	//  These arrays must be allocated by calling function
	GameObject **const objArray;    //  pointer to object pointer array
	int16 *const       distArray;   //  pointer to distance array

	//  Constructor
	TargetObjectArray(int16 s, GameObject **arr, int16 *dArr) :
		size(s),
		objs(0),
		objArray(arr),
		distArray(dArr) {
	}
};

/* ===================================================================== *
   TargetActorArray structure
 * ===================================================================== */

//  This structure is used to query the target class for multiple target
//  actors
struct TargetActorArray {
	const int16     size;       //  number of allocated elements in array
	int16           actors;     //  number of array elements with valid data

	//  These arrays must be allocated by calling function
	Actor **const  actorArray;  //  pointer to actor pointer array
	int16 *const   distArray;   //  pointer to distance array

	//  Constructor
	TargetActorArray(int16 s, Actor **arr, int16 *dArr) :
		size(s),
		actors(0),
		actorArray(arr),
		distArray(dArr) {
	}
};

/* ===================================================================== *
   Target class
 * ===================================================================== */

class Target {
public:
	//  virtual destructor
	virtual ~Target() {}

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	virtual int32 archiveSize() const = 0;

	virtual void write(Common::MemoryWriteStreamDynamic *out) const = 0;

	//  Return an integer representing the type of target
	virtual int16 getType() const = 0;

	//  Virtual function returning the sizeof this target
	virtual size_t size() const = 0;

	//  Create a copy of this target at the specified address
	virtual void clone(void *mem) const = 0;

	//  Determine if the specified target is equivalent to this target
	virtual bool operator == (const Target &t) const = 0;
	bool operator != (const Target &t) const {
		return !operator == (t);
	}

	//  Overloaded memory management functions for constructing and
	//  destructing Targets in place.
	void *operator new (size_t, void *p) {
		return p;
	}

	virtual bool isObjectTarget() const;
	virtual bool isActorTarget() const;

	//  Return location of closest instance of target
	virtual TilePoint where(GameWorld *world, const TilePoint &tp) const = 0;
	//  Fill array with locations of closest instances of target
	virtual int16 where(
	    GameWorld *world,
	    const TilePoint &tp,
	    TargetLocationArray &tla) const = 0;
};

/* ===================================================================== *
   LocationTarget class
 * ===================================================================== */

class LocationTarget : public Target {
	TilePoint   loc;

public:
	//  Constructor -- initial construction
	LocationTarget(const TilePoint &tp) : loc(tp) {}

	LocationTarget(Common::SeekableReadStream *stream);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of target
	int16 getType() const;

	//  Virtual function returning the sizeof this target
	size_t size() const;

	//  Create a copy of this target at the specified address
	void clone(void *mem) const;

	//  Determine if the specified target is equivalent to this target
	bool operator == (const Target &t) const;

	//  Determine if the specified location target is equivalent to this
	//  location target
	bool operator == (const LocationTarget &lt) const {
		return loc == lt.loc;
	}
	bool operator != (const LocationTarget &lt) const {
		return loc != lt.loc;
	}

	TilePoint where(GameWorld *world, const TilePoint &tp) const;
	int16 where(
	    GameWorld *world,
	    const TilePoint &tp,
	    TargetLocationArray &tla) const;
};

/* ===================================================================== *
   TileTarget class
 * ===================================================================== */

class TileTarget : public Target {
public:
	virtual bool isTarget(StandingTileInfo &sti) const = 0;

	TilePoint where(GameWorld *world, const TilePoint &tp) const;
	int16 where(
	    GameWorld *world,
	    const TilePoint &tp,
	    TargetLocationArray &tla) const;
};

/* ===================================================================== *
   SpecificTileTarget class
 * ===================================================================== */

class SpecificTileTarget : public TileTarget {
	TileID  tile;

public:
	//  Constructor -- initial construction
	SpecificTileTarget(TileID t) : tile(t) {}

	SpecificTileTarget(Common::SeekableReadStream *stream);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of target
	int16 getType() const;

	//  Virtual function returning the sizeof this target
	size_t size() const;

	//  Create a copy of this target at the specified address
	void clone(void *mem) const;

	//  Determine if the specified target is equivalent to this target
	bool operator == (const Target &t) const;

	bool isTarget(StandingTileInfo &sti) const;
};

/* ===================================================================== *
   TilePropertyTarget class
 * ===================================================================== */

class TilePropertyTarget : public TileTarget {
	TilePropertyID  tileProp;

public:
	//  Constructor -- initial construction
	TilePropertyTarget(TilePropertyID tProp) : tileProp(tProp) {}

	TilePropertyTarget(Common::SeekableReadStream *stream);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of target
	int16 getType() const;

	//  Virtual function returning the sizeof this target
	size_t size() const;

	//  Create a copy of this target at the specified address
	void clone(void *mem) const;

	//  Determine if the specified target is equivalent to this target
	bool operator == (const Target &t) const;

	bool isTarget(StandingTileInfo &sti) const;
};

/* ===================================================================== *
   MetaTileTarget class
 * ===================================================================== */

class MetaTileTarget : public Target {
public:
	virtual bool isTarget(
	    MetaTile *mt,
	    int16 mapNum,
	    const TilePoint &tp) const = 0;

	TilePoint where(GameWorld *world, const TilePoint &tp) const;
	int16 where(
	    GameWorld *world,
	    const TilePoint &tp,
	    TargetLocationArray &tla) const;
};

/* ===================================================================== *
   SpecificMetaTileTarget class
 * ===================================================================== */

class SpecificMetaTileTarget : public MetaTileTarget {
	MetaTileID  meta;

public:
	//  Constructor -- initial construction
	SpecificMetaTileTarget(MetaTileID mt) : meta(mt) {}

	SpecificMetaTileTarget(Common::SeekableReadStream *stream);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of target
	int16 getType() const;

	//  Virtual function returning the sizeof this target
	size_t size() const;

	//  Create a copy of this target at the specified address
	void clone(void *mem) const;

	//  Determine if the specified target is equivalent to this target
	bool operator == (const Target &t) const;

	bool isTarget(MetaTile *mt, int16 mapNum, const TilePoint &tp) const;
};

/* ===================================================================== *
   MetaTilePropertyTarget class
 * ===================================================================== */

class MetaTilePropertyTarget : public MetaTileTarget {
	MetaTilePropertyID  metaProp;

public:
	//  Constructor -- initial construction
	MetaTilePropertyTarget(MetaTilePropertyID mtProp) :
		metaProp(mtProp) {
	}

	MetaTilePropertyTarget(Common::SeekableReadStream *stream);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of target
	int16 getType() const;

	//  Virtual function returning the sizeof this target
	size_t size() const;

	//  Create a copy of this target at the specified address
	void clone(void *mem) const;

	//  Determine if the specified target is equivalent to this target
	bool operator == (const Target &t) const;

	bool isTarget(MetaTile *mt, int16 mapNum, const TilePoint &tp) const;
};

/* ===================================================================== *
   ObjectTarget class
 * ===================================================================== */

class ObjectTarget : public Target {

	//  These are recursive functions used to fill a target array
	//  by inspecting an object and all of the objects contained in
	//  that object
	void searchObject(
	    GameObject *obj,
	    int16 dist,
	    TargetObjectArray &toa) const;
	void searchObject(
	    GameObject *obj,
	    const TilePoint &tp,
	    int16 dist,
	    TargetLocationArray &tla) const;

public:
	bool isObjectTarget() const;

	TilePoint where(GameWorld *world, const TilePoint &tp) const;
	int16 where(
	    GameWorld *world,
	    const TilePoint &tp,
	    TargetLocationArray &tla) const;

	//  Determine if the specified object meets the target criterion
	virtual bool isTarget(GameObject *testObj) const = 0;

	//  Return closest instance of target object
	virtual GameObject *object(GameWorld *world, const TilePoint &tp) const;
	//  Fill array with closest instances of target object
	virtual int16 object(
	    GameWorld *world,
	    const TilePoint &tp,
	    TargetObjectArray &toa) const;
};

/* ===================================================================== *
   SpecificObjectTarget class
 * ===================================================================== */

class SpecificObjectTarget : public ObjectTarget {
	ObjectID    obj;

public:
	//  Constructors -- initial construction
	SpecificObjectTarget(ObjectID id) :
		obj(id) {
		assert(isObject(obj));
	}
	SpecificObjectTarget(GameObject *ptr) :
		obj((assert(isObject(ptr)), ptr->thisID())) {
	}

	SpecificObjectTarget(Common::SeekableReadStream *stream);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of target
	int16 getType() const;

	//  Virtual function returning the sizeof this target
	size_t size() const;

	//  Create a copy of this target at the specified address
	void clone(void *mem) const;

	//  Determine if the specified target is equivalent to this target
	bool operator == (const Target &t) const;

	bool isTarget(GameObject *testObj) const;

	TilePoint where(GameWorld *world, const TilePoint &tp) const;
	int16 where(
	    GameWorld *world,
	    const TilePoint &tp,
	    TargetLocationArray &tla) const;

	GameObject *object(GameWorld *world, const TilePoint &tp) const;
	int16 object(
	    GameWorld *world,
	    const TilePoint &tp,
	    TargetObjectArray &toa) const;

	//  Return a pointer to the target object, unconditionally
	GameObject *getTargetObject() const {
		return GameObject::objectAddress(obj);
	}
};

/* ===================================================================== *
   ObjectPropertTarget class
 * ===================================================================== */

class ObjectPropertyTarget : public ObjectTarget {
	ObjectPropertyID    objProp;

public:
	//  Constructor -- initial construction
	ObjectPropertyTarget(ObjectPropertyID prop) : objProp(prop) {}

	ObjectPropertyTarget(Common::SeekableReadStream *stream);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of target
	int16 getType() const;

	//  Virtual function returning the sizeof this target
	size_t size() const;

	//  Create a copy of this target at the specified address
	void clone(void *mem) const;

	//  Determine if the specified target is equivalent to this target
	bool operator == (const Target &t) const;

	bool isTarget(GameObject *testObj) const;
};

/* ===================================================================== *
   ActorTarget class
 * ===================================================================== */

class ActorTarget : public ObjectTarget {

public:
	bool isActorTarget() const;

	bool isTarget(GameObject *testObj) const;
	virtual bool isTarget(Actor *testActor) const = 0;

	virtual Actor *actor(GameWorld *world, const TilePoint &tp) const;
	virtual int16 actor(
	    GameWorld *world,
	    const TilePoint &tp,
	    TargetActorArray &taa) const;
};

/* ===================================================================== *
   SpecificActorTarget class
 * ===================================================================== */

class SpecificActorTarget : public ActorTarget {
	Actor   *a;

public:
	//  Constructor -- initial construction
	SpecificActorTarget(Actor *actor_) :
		a(actor_) {
	}

	SpecificActorTarget(Common::SeekableReadStream *stream);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of target
	int16 getType() const;

	//  Virtual function returning the sizeof this target
	size_t size() const;

	//  Create a copy of this target at the specified address
	void clone(void *mem) const;

	//  Determine if the specified target is equivalent to this target
	bool operator == (const Target &t) const;

	bool isTarget(Actor *testActor) const;

	TilePoint where(GameWorld *world, const TilePoint &tp) const;
	int16 where(
	    GameWorld *world,
	    const TilePoint &tp,
	    TargetLocationArray &tla) const;

	GameObject *object(GameWorld *world, const TilePoint &tp) const;
	int16 object(
	    GameWorld *world,
	    const TilePoint &tp,
	    TargetObjectArray &toa) const;

	Actor *actor(GameWorld *world, const TilePoint &tp) const;
	int16 actor(
	    GameWorld *world,
	    const TilePoint &tp,
	    TargetActorArray &taa) const;

	//  Return a pointer to the target actor, unconditionally
	Actor *getTargetActor() const {
		return a;
	}
};

/* ===================================================================== *
   ActorPropertyTarget class
 * ===================================================================== */

class ActorPropertyTarget : public ActorTarget {
	ActorPropertyID     actorProp;

public:
	//  Constructor -- initial construction
	ActorPropertyTarget(ActorPropertyID aProp) :
		actorProp(aProp) {
	}

	ActorPropertyTarget(Common::SeekableReadStream *stream);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize() const;

	void write(Common::MemoryWriteStreamDynamic *out) const;

	//  Return an integer representing the type of target
	int16 getType() const;

	//  Virtual function returning the sizeof this target
	size_t size() const;

	//  Create a copy of this target at the specified address
	void clone(void *mem) const;

	//  Determine if the specified target is equivalent to this target
	bool operator == (const Target &t) const;

	bool isTarget(Actor *testActor) const;
};

/* ===================================================================== *
   Constants
 * ===================================================================== */

//	This const and typedef are used to define an area of memory large enough
//	to contain any target.  The sizeof( LocationTarget ) is used because
//	the LocationTarget takes the most memory.

const size_t    targetBytes = sizeof(LocationTarget);

typedef uint8 TargetPlaceHolder[targetBytes];

} // end of namespace Saga2

#endif
