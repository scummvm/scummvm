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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_OBJECTS_H
#define SAGA2_OBJECTS_H

#include "saga2/idtypes.h"
#include "saga2/tcoords.h"
#include "saga2/sprite.h"
#include "saga2/property.h"
#include "saga2/savefile.h"
#include "saga2/spells.h"
#include "saga2/effects.h"
#include "saga2/combat.h"
#include "saga2/tilevect.h"
//  Include list of prototype classes created by MicrosoftAccess script
#include "saga2/pclass.r"
#include "saga2/objproto.h"

namespace Saga2 {

#ifdef _WIN32   //  Set structure alignment packing value to 1 byte
#pragma pack( push, 1 )
#endif

/* ======================================================================= *
   GameObject: Describes an instance of an object
 * ======================================================================= */

class GameWorld;

const uint16 unlimitedCapacity = maxuint16;

enum ActorManaID {
	manaIDRed = 0,
	manaIDOrange,
	manaIDYellow,
	manaIDGreen,
	manaIDBlue,
	manaIDViolet,

	numManas,
};

//  Used to indicate if objects can be stacked or merged
enum {
	cannotStackOrMerge = 0,
	canStack,
	canMerge
};

//  The ResourceGameObject structure represents the game object data as
//  it is structured in the resource file.

struct ResourceGameObject {
	int16           protoIndex;
	TilePoint       location;
	uint16          nameIndex;
	ObjectID        parentID;
	uint16          script;
	uint16          objectFlags;
	uint8           hitPoints;
	uint16          misc;
};

//  Base class of all objects
//
//  Unlike the object prototypes, the only subclass of GameObject is
//  the actor subclass, which is kept in an entirely seperate table.
//  This allows all objects to be kept in an array (indexed by ID number)

void     initActors(void);
void     saveActors(SaveFileConstructor &);
void     loadActors(SaveFileReader &);
void     cleanupActors(void);
class GameObject {

	friend void     initWorlds(void);
	friend void     saveWorlds(SaveFileConstructor &);
	friend void     loadWorlds(SaveFileReader &);
	friend void     cleanupWorlds(void);

	friend void     initObjects(void);
	friend void     saveObjects(SaveFileConstructor &);
	friend void     loadObjects(SaveFileReader &);
	friend void     cleanupObjects(void);

	friend void     buildDisplayList(void);
	friend void     drawDisplayList(void);
	friend void     setMindContainer(int NewContainerClass, IntangibleContainerWindow &cw);
	friend class    EnchantmentContainerWindow;
	friend bool     Enchantment(ObjectID, ObjectID);
	friend class    ProtoObj;
	friend class    PhysicalContainerProto;
	friend class    IntangibleContainerProto;
	friend class    KeyProto;
	friend class    BottleProto;
	friend class    SkillProto;
	friend class    ActorProto;
	friend class    MotionTask;
	friend class    MotionTaskList;
	friend class    ObjectIterator;
	friend class    ContainerIterator;
	friend class    RecursiveContainerIterator;
	friend class    ShopMode;



private:

	// container info
	enum {
		maxRow      = 20,
		maxCol      = 4,
	};

public:

	ObjectID thisID(void);               // calculate our own ID value

	static char *nameText(uint16 index);

protected:
	//  get address of head-of-chain id.
	static ObjectID *getHeadPtr(ObjectID parentID, TilePoint &l);

	//  Object list management functions
	void remove(void);                   // removes from old list
	void append(ObjectID newParent);         // adds to new list (no remove)
	void insert(ObjectID newPrev);           // inserts after this item (no remove)
	void protoAddressToOffset(void);         // converts proto address to offset in resource file

	ProtoObj        *prototype;             // object that defines our behavior
	TilePoint       location;               // where object is located.
	uint16          nameIndex;              // object's proper name, if any
	ObjectID        parentID,               // ID of parent object
	                siblingID,              // ID of next in chain
	                childID;                // ID of 1st child
	uint16          script;                 // script attached to this object
	uint16          objectFlags;            // various flags
	uint8           hitPoints;              // object hit points
	uint8           bParam;                 // number of spell charges an object has
	// (also generator radius in metatiles)
	union {
		uint16      massCount;              // for mergeables, object count
		uint16      textStringID;           // inscription for documents
		uint16      enchantmentType;        // for enchantments
		uint16      generatorFrequency;     // for encounter and mission generators
	};

	uint8           missileFacing;

public:
	ActiveItemID    currentTAG;             // ActiveItem object is on
	uint8           sightCtr;               // Line of sight counter

	uint8           reserved[ 2 ];

	//  Default constructor
	GameObject(void);

	//  Constructor -- initial construction
	GameObject(const ResourceGameObject &res);

	//  Constructor -- reconstruct from archive buffer
	GameObject(void **buf);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize(void);

	//  Archive the object in a buffer
	void *archive(void *buf);

	//  returns the address of the object based on the ID, and this
	//  includes accounting for actors.
	static GameObject *objectAddress(ObjectID id);

	//  Converts object ID into prototype address...
	static ProtoObj *protoAddress(ObjectID id);

	//  object creation and deletion
	static GameObject *newObject(void);      // get a newly created object
	void deleteObject(void);                 // delete this object and remove
	void deleteObjectRecursive(void);        // delete this object and every
	// object it contains

	//  Return pointer to parent/child/next sibling object, if any
	GameObject *parent(void) {
		return parentID == Nothing ? NULL : objectAddress(parentID);
	}
	GameObject *next(void) {
		return siblingID == Nothing ? NULL : objectAddress(siblingID);
	}
	GameObject *child(void) {
		return childID == Nothing ? NULL : objectAddress(childID);
	}

	//  Return ID of parent/child/next sibling object, if any
	ObjectID IDParent(void) {
		return parentID ;
	}
	ObjectID IDNext(void) {
		return siblingID;
	}
	ObjectID IDChild(void) {
		return childID  ;
	}

	//  Return a pointer to the world on which this object resides
	GameWorld *world(void);

	//  Return the number of the map of the world on which this object
	//  resides
	int16 getMapNum(void);

	//  graphics functions
	int16 sprNum(int16 state);               // returns current sprite number

	// gets the offset to the spr image according to masscount for mergeables
	int32 getSprOffset(int16 num = -1);

	// completely restore the magical energy of a magical object
	void recharge(void);

	// returns the type of charge an object has
	// be it none, red, violet, etc...
	int16 getChargeType(void);

	// use charge of this object
	bool deductCharge(ActorManaID manaID, uint16 manaCost);

	// check charge of this object
	bool hasCharge(ActorManaID manaID, uint16 manaCost);

	//  context-changing functions
	void setLocation(const Location &loc);  // move to new location in world
	void setLocation(const TilePoint &tp);  // move to new location in world
	void move(const Location &loc);          // move to new location in world
	void move(const Location &loc, int16 num);       // move to new location in world
	void move(const TilePoint &tp);          // move to new location in world
	void updateImage(ObjectID);              // move to new location in world,
	// (assumes setLocation has been called)

	//  Remove an object from a stack of objects. Returns TRUE if it was in a stack.
	bool unstack(void);

	// this correctly moves merged or stacked objects
	bool moveMerged(const Location &loc);
	bool moveMerged(const Location &loc, int16 num = 1);
	bool moveMerged(const TilePoint &tp);

	//  Extract a merged object with specified merge number from another
	//  merged object and return its ID
	ObjectID extractMerged(const Location &loc, int16 num);
	GameObject *extractMerged(int16 num);

	void moveRandom(const TilePoint &minLoc, const TilePoint &maxLoc);  //move to random location Between Two Points

	ObjectID copy(const Location &loc);      // copy item to new context
	ObjectID copy(const Location &loc, int16 num);   // copy item to new context

	//  Create an alias of this object
	ObjectID makeAlias(const Location &loc);

	//  move the item with a given context
	void move(int16 slot);                   // move to new slot in container

	//  Activate the object
	void activate(void);

	//  Deactivate this object
	void deactivate(void);

	//  Determine if this object is an alias for another object
	bool isAlias() {
		return (objectFlags & objectAlias) != 0;
	}

	//  check to see if item can be contained by this object
	bool canContain(ObjectID item) {
		return prototype->canContain(thisID(), item);
	}

	//  check to see if item is contained by the object
	bool isContaining(GameObject *item);

	//  check if an instance of the specified target is contained in
	//  this object
	bool isContaining(ObjectTarget *objTarget);

	//  determine wether this object has a specified property
	bool hasProperty(const ObjectProperty &objProp) {
		return objProp.operator()(this);
	}

	//  Return the location of the first empty slot
	TilePoint getFirstEmptySlot(GameObject *obj);

	//  Return the location of the first available slot within this object
	//  in which to place the specified object
	bool getAvailableSlot(
	    GameObject  *obj,
	    TilePoint   *slot,
	    bool        canMerge = FALSE,
	    GameObject  **mergeObj = NULL);

	//  Find a slot to place the specified object within this object and
	//  drop it in that slot
	//  If merge count == 0, then no auto-merging allowed
	bool placeObject(
	    ObjectID    enactor,
	    ObjectID    objID,
	    bool        canMerge = FALSE,
	    int16       num = 1);

	//  Drop the specified object on the ground in a semi-random location
	void dropInventoryObject(GameObject *obj, int16 count = 1);

	//Get Specific Container From Object
	GameObject *getIntangibleContainer(int containerType);

	//  generic actions
	bool use(ObjectID enactor) {
		return prototype->use(thisID(), enactor);
	}
	bool useOn(ObjectID enactor, ObjectID item) {
		return prototype->useOn(thisID(), enactor, item);
	}

	bool useOn(ObjectID enactor, ActiveItem *item) {
		return prototype->useOn(thisID(), enactor, item);
	}

	bool useOn(ObjectID enactor, Location &loc) {
		return prototype->useOn(thisID(), enactor, loc);
	}

	//  various verb actions that can take place
	bool take(ObjectID enactor, int16 num = 1) {
		return prototype->take(thisID(), enactor, num);
	}
	bool drop(ObjectID enactor, const Location &l, int16 num = 1) {
		return prototype->drop(thisID(), enactor, l, num);
	}
	//  drop an object onto another object and handle the result.
	bool dropOn(ObjectID enactor, ObjectID target, int16 num = 1) {
		return prototype->dropOn(thisID(), enactor, target, num);
	}
	//  drop this object on a TAG
	bool dropOn(ObjectID enactor, ActiveItem *target, const Location &loc, int16 num = 1) {
		return prototype->dropOn(thisID(), enactor, target, loc, num);
	}
	bool open(ObjectID enactor) {
		return prototype->open(thisID(), enactor);
	}
	bool close(ObjectID enactor) {
		return prototype->close(thisID(), enactor);
	}
	bool strike(ObjectID enactor, ObjectID item) {
		return prototype->strike(thisID(), enactor, item);
	}
	bool damage(ObjectID enactor, ObjectID target) {
		return prototype->damage(thisID(), enactor, target);
	}
	bool eat(ObjectID enactor) {
		return prototype->eat(thisID(), enactor);
	}
	bool insert(ObjectID enactor, ObjectID item) {
		return prototype->insert(thisID(), enactor, item);
	}
	bool remove(ObjectID enactor) {
		return prototype->remove(thisID(), enactor);
	}
	bool acceptDrop(ObjectID enactor, ObjectID droppedObj, int count) {
		return prototype->acceptDrop(thisID(), enactor, droppedObj, count);
	}
	bool acceptDamage(
	    ObjectID            enactor,
	    int8                absDamage,
	    effectDamageTypes   dType = damageOther,
	    int8                dice = 0,
	    uint8               sides = 1,
	    int8                perDieMod = 0) {
		return  prototype->acceptDamage(
		            thisID(),
		            enactor,
		            absDamage,
		            dType,
		            dice,
		            sides,
		            perDieMod);
	}
	bool acceptHealing(ObjectID enactor, int8 absDamage, int8 dice = 0, uint8 sides = 1, int8 perDieMod = 0) {
		return prototype->acceptHealing(thisID(), enactor, absDamage, dice, sides, perDieMod);
	}
	bool acceptStrike(
	    ObjectID            enactor,
	    ObjectID            strikingObj,
	    uint8               skillIndex) {
		return  prototype->acceptStrike(
		            thisID(),
		            enactor,
		            strikingObj,
		            skillIndex);
	}
	bool acceptLockToggle(ObjectID enactor, uint8 keyCode) {
		return prototype->acceptLockToggle(thisID(), enactor, keyCode);
	}
	bool acceptMix(ObjectID enactor, ObjectID mixObj) {
		return prototype->acceptMix(thisID(), enactor, mixObj);
	}
	bool acceptInsertion(ObjectID enactor, ObjectID item, int16 count) {
		return prototype->acceptInsertion(thisID(), enactor, item, count);
	}
	bool acceptInsertionAt(ObjectID enactor, ObjectID item, const TilePoint &where, int16 num = 1) {
		return prototype->acceptInsertionAt(thisID(), enactor, item, where, num);
	}

	//  query functions:
	ObjectID possessor(void);                // return actor posessing this object

	//  Access functions
	ProtoObj *proto(void) {
		return prototype;
	}
	TilePoint getLocation(void) {
		return location;
	}
	TilePoint getWorldLocation(void);
	bool getWorldLocation(Location &loc);
	Location notGetLocation(void);
	Location notGetWorldLocation(void);

	//  Return the name of this object (proper noun if it has one)
	char *objName(void) {
		return nameText((int16)(
		                    nameIndex > 0 ? nameIndex : prototype->nameIndex));
	}

	// return name of object, and it's quantity if merged
	void objCursorText(char nameBuf[], const int8 size, int16 count = -1);

	// find out if this is a trueskill
	bool isTrueSkill(void);

	//  Access functions for name index
	uint16 getNameIndex(void) {
		return nameIndex;
	}
	void setNameIndex(uint16 n) {
		nameIndex = n;
	}

	//  Return the name of this type of object
	char *protoName(void) {
		return nameText(prototype->nameIndex);
	}

	//  Update the state of this object.  This function is called every
	//  frame for every active object.
	void updateState(void);

	//  Flag test functions
	bool isOpen(void) {
		return (int16)(objectFlags & objectOpen);
	}
	bool isLocked(void) {
		return (int16)(objectFlags & objectLocked);
	}
	bool isImportant(void) {
		return (int16)(objectFlags & objectImportant);
	}
	bool isGhosted(void) {
		return (objectFlags & objectGhosted)
		       || (prototype->flags & ResourceObjectPrototype::objPropGhosted);
	}
	bool isInvisible(void) {
		return (objectFlags & objectInvisible)
		       || (prototype->flags & ResourceObjectPrototype::objPropHidden);
	}
	bool isMoving(void) {
		return (int16)(objectFlags & objectMoving);
	}
	bool isActivated(void) {
		return (int16)(objectFlags & objectActivated);
	}

	void setScavengable(bool val) {
		if (val)
			objectFlags |= objectScavengable;
		else
			objectFlags &= ~objectScavengable;
	}
	bool isScavengable(void) {
		return (objectFlags & objectScavengable) != 0;
	}

	void setObscured(bool val) {
		if (val)
			objectFlags |= objectObscured;
		else
			objectFlags &= ~objectObscured;
	}
	bool isObscured(void) {
		return (objectFlags & objectObscured) != 0;
	}

	void setTriggeringTAG(bool val) {
		if (val)
			objectFlags |= objectTriggeringTAG;
		else
			objectFlags &= ~objectTriggeringTAG;
	}
	bool isTriggeringTAG(void) {
		return (objectFlags & objectTriggeringTAG) != 0;
	}

	void setOnScreen(bool val) {
		if (val)
			objectFlags |= objectOnScreen;
		else
			objectFlags &= ~objectOnScreen;
	}
	bool isOnScreen(void) {
		return (objectFlags & objectOnScreen) != 0;
	}

	void setSightedByCenter(bool val) {
		if (val)
			objectFlags |= objectSightedByCenter;
		else
			objectFlags &= ~objectSightedByCenter;
	}
	bool isSightedByCenter(void) {
		return (objectFlags & objectSightedByCenter) != 0;
	}

	bool isMissile(void) {
		return prototype->isMissile();
	}

	// image data
	Sprite *getIconSprite(void);    // sprite when in inventory + cursor
	Sprite *getGroundSprite(void);  // sprite when on ground

	// world interaction type flags
	uint16 containmentSet(void);

	uint16 scriptClass(void) {
		if (script)
			return script;
		if (prototype)
			return prototype->script;
		return 0;
	}

	//  General access functions

	//  Script access functions
	uint16 getScript(void) {
		return script;
	}
	void setScript(uint16 scr) {
		script = scr;
	}

	//  access function to set object flags
	void setFlags(uint8 newval, uint8 changeMask) {
		//  Only change the flags spec'd by changeFlags
		objectFlags = (newval & changeMask)
		              | (objectFlags & ~changeMask);
	}

	//  Access functions for hit points
	uint8 getHitPoints(void) {
		return hitPoints;
	}
	void setHitPoints(uint8 hp) {
		hitPoints = hp;
	}

	//  Builds the color remapping for this object based on the
	//  prototype's color map
	void getColorTranslation(ColorTable map) {
		prototype->getColorTranslation(map);
	}

	//  Functions to get and set prototype (used by scripts)
	int32 getProtoNum(void);
	void setProtoNum(int32 nProto);

	//  Acess functions for extra data
	uint16 getExtra(void) {
		return massCount;
	}
	void setExtra(uint16 x) {
		massCount = x;
	}

	//  Function to evaluate the effects of all enchantments
	void evalEnchantments(void);

	bool makeSavingThrow(void) {
		return prototype->makeSavingThrow();
	}

	//  Generic range checking function
	bool inRange(const TilePoint &tp, uint16 range);

	//  Generic function to test if object can be picked up
	bool isCarryable(void) {
		return prototype->mass <= 200 && prototype->bulk <= 200;
	}

	bool isMergeable(void) {
		return (prototype->flags & ResourceObjectPrototype::objPropMergeable) != 0;
	}

	//  A timer for this object has ticked
	void timerTick(TimerID timer);

	//  A sensor for this object has sensed an object
	void senseObject(SensorID sensor, ObjectID sensedObj);

	//  A sensor for this object has sensed an event
	void senseEvent(
	    SensorID        sensor,
	    int16           type,
	    ObjectID        directObject,
	    ObjectID        indirectObject);

	//  Timer related member functions
	bool addTimer(TimerID id);
	bool addTimer(TimerID id, int16 frameInterval);
	void removeTimer(TimerID id);
	void removeAllTimers(void);

	//  Sensor related member functions
private:
	bool addSensor(Sensor *newSensor);
public:
	bool addProtaganistSensor(SensorID id, int16 range);
	bool addSpecificActorSensor(SensorID id, int16 range, Actor *a);
	bool addSpecificObjectSensor(SensorID id, int16 range, ObjectID obj);
	bool addActorPropertySensor(
	    SensorID            id,
	    int16               range,
	    ActorPropertyID     prop);
	bool addObjectPropertySensor(
	    SensorID            id,
	    int16               range,
	    ObjectPropertyID    prop);
	bool addEventSensor(SensorID id, int16 range, int16 eventType);
	void removeSensor(SensorID id);
	void removeAllSensors(void);

	bool canSenseProtaganist(SenseInfo &info, int16 range);
	bool canSenseSpecificActor(SenseInfo &info, int16 range, Actor *a);
	bool canSenseSpecificObject(SenseInfo &info, int16 range, ObjectID obj);
	bool canSenseActorProperty(
	    SenseInfo           &info,
	    int16               range,
	    ActorPropertyID     prop);
	bool canSenseObjectProperty(
	    SenseInfo           &info,
	    int16               range,
	    ObjectPropertyID    prop);

	static int32 canStackOrMerge(GameObject *dropObj, GameObject *target);
	static void mergeWith(GameObject *dropObj, GameObject *target, int16 count);

	bool merge(ObjectID enactor, ObjectID objToMergeID, int16 count);
	bool stack(ObjectID enactor, ObjectID objToStackID);

	bool canFitBulkwise(GameObject *obj) {
		return prototype->canFitBulkwise(this, obj);
	}
	bool canFitMasswise(GameObject *obj) {
		return prototype->canFitMasswise(this, obj);
	}

	uint16 totalContainedMass(void);
	uint16 totalContainedBulk(void);

	uint16 totalMass(void) {
		return      prototype->mass * (isMergeable() ? getExtra() : 1)
		            +   totalContainedMass();
	}
	uint16 totalBulk(void) {
		return prototype->bulk * (isMergeable() ? getExtra() : 1);
	}

	uint16 massCapacity(void) {
		return prototype->massCapacity(this);
	}
	uint16 bulkCapacity(void) {
		return prototype->bulkCapacity(this);
	}
};

/* ===================================================================== *
   Sector struct
 * ===================================================================== */

class Sector {
public:
	uint16          activationCount;
	ObjectID        childID;

	Sector(void) :
		activationCount(0),
		childID(Nothing) {
	}

	bool isActivated(void) {
		return activationCount != 0;
	}

	void activate(void);
	void deactivate(void);
};

/* ======================================================================= *
   GameWorld: Describes a world within the game
 * ======================================================================= */

//  Terminology note: A "sector" is a small portion of a map.
//  All objects within a sector are stored on a single list.

//  The size of a sector (length of side) in UV coodinates.
//  A sector is an area of about 4x4 metatiles.

class GameWorld : public GameObject {

	friend void     initWorlds(void);
	friend void     cleanupWorlds(void);
	friend void     buildDisplayList(void);

	friend class    ProtoObj;
	friend class    GameObject;
	friend class    ObjectIterator;

	TilePoint       size;                   // size of world in U/V coords
	int16           sectorArraySize;        // size of sector array
	Sector          **sectorArray;          // array of sectors
public:
	int16           mapNum;                 // map number for this world.

	//  Default constructor
	GameWorld(void) : sectorArray(NULL) {}

	//  Initial constructor
	GameWorld(int16 map);

	//  Constructor -- reconstruct from archive buffer
	GameWorld(void **buf);

	int32 archiveSize(void);
	void *archive(void *buf);

	void cleanup(void);

	Sector *getSector(int16 u, int16 v) {
		return &(*sectorArray)[ v * sectorArraySize + u ];
	}

	TilePoint sectorSize(void) {         // size of map in sectors
		return TilePoint(sectorArraySize, sectorArraySize, 0);
	}

	static uint32 IDtoMapNum(ObjectID id) {
		assert(isWorld(id));
		return ((GameWorld *)GameObject::objectAddress(id))->mapNum;
	}
};

void setCurrentWorld(ObjectID worldID);

extern GameWorld    *currentWorld;

/* ======================================================================= *
   GameObject inline member function
 * ======================================================================= */

//------------------------------------------------------------------------
//	Return the number of the map of the world on which this object resides.

inline int16 GameObject::getMapNum(void) {
	return world()->mapNum;
}

/* ======================================================================= *
   Enchantment Class
 * ======================================================================= */
//class Enchantment {
//
//public:
//	Enchantment(ObjectID Skill,ObjectID Obj);
//	~Enchantment();
//
//};

/* ===================================================================== *
   ActiveRegion class
 * ===================================================================== */

class ActiveRegion {

	friend void initActiveRegions(void);
	friend void saveActiveRegions(SaveFileConstructor &saveGame);
	friend void loadActiveRegions(SaveFileReader &saveGame);
	friend void cleanupActiveRegions(void);

	friend class ActiveRegionObjectIterator;

	ObjectID        anchor;     //  ID of object this region is attached to
	TilePoint       anchorLoc;  //  Location of anchor
	ObjectID        worldID;
	TileRegion      region;     //  Region coords ( in sectors )

public:

	void update(void);

	//  Return the current region in tile point coords
	TileRegion getRegion(void) {
		TileRegion      tReg;

		tReg.min.u = region.min.u << sectorShift;
		tReg.min.v = region.min.v << sectorShift;
		tReg.max.u = region.max.u << sectorShift;
		tReg.max.v = region.max.v << sectorShift;
		tReg.min.z = tReg.max.z = 0;

		return tReg;
	}

	//  Return the region world
	GameWorld *getWorld(void) {
		return (GameWorld *)GameObject::objectAddress(worldID);
	}
};

void updateActiveRegions(void);

//  Return a pointer to an active region given its PlayerActor's ID
ActiveRegion *getActiveRegion(PlayerActorID id);

void initActiveRegions(void);
void saveActiveRegions(SaveFileConstructor &saveGame);
void loadActiveRegions(SaveFileReader &saveGame);
inline void cleanupActiveRegions(void) {}

/* ======================================================================= *
   ObjectIterator Class
 * ======================================================================= */

//  This class simply defines a standard interface for all derived
//  object iterator classes.

class ObjectIterator {
public:
	//  Virtual destructor
	virtual ~ObjectIterator(void) {}

	//  Iteration functions
	virtual ObjectID first(GameObject **obj) = 0;
	virtual ObjectID next(GameObject **obj) = 0;
};

/* ======================================================================= *
   SectorRegionObjectIterator Class
 * ======================================================================= */

//  This class iterates through every object within a given region of
//  sectors.

class SectorRegionObjectIterator : public ObjectIterator {

	TilePoint       minSector,
	                maxSector,
	                sectorCoords;
	GameWorld       *searchWorld;
	GameObject      *currentObject;

public:
	//  Constructor
	SectorRegionObjectIterator(GameWorld *world);

	//  Constructor
	SectorRegionObjectIterator(
	    GameWorld           *world,
	    const TileRegion    &sectorRegion) :
		searchWorld(world),
		minSector(sectorRegion.min),
		maxSector(sectorRegion.max) {
		assert(searchWorld != NULL);
		assert(isWorld(searchWorld));
	}

protected:
	GameWorld *getSearchWorld(void) {
		return searchWorld;
	}

public:
	//  Iteration functions
	ObjectID first(GameObject **obj);
	ObjectID next(GameObject **obj);
};

/* ======================================================================= *
   RadialObjectIterator Class
 * ======================================================================= */

//	This class will iterate through all objects within a given radius of
//	a given center point.

class RadialObjectIterator : public SectorRegionObjectIterator {
private:

	TilePoint       center;
	int16           radius;

	//  Compute the region of sectors to pass to the ObjectIterator
	//  constructor
	static TileRegion computeSectorRegion(
	    const TilePoint &sectors,
	    const TilePoint &center,
	    int16           radius);

	//  Compute the distance to the specified point from the search
	//  center
	virtual int16 computeDist(const TilePoint &tp) = 0;

protected:

	//  Simply return the center coordinates
	TilePoint getCenter(void) {
		return center;
	}

public:

	//  Constructor
	RadialObjectIterator(
	    GameWorld       *world,
	    const TilePoint &searchCenter,
	    int16           distance) :
		SectorRegionObjectIterator(
		    world,
		    computeSectorRegion(
		        world->sectorSize(),
		        searchCenter,
		        distance)),
		center(searchCenter),
		radius(distance) {
	}

	//  Return the first object found
	ObjectID first(GameObject **obj, int16 *dist);
	//  Return the next object found
	ObjectID next(GameObject **obj, int16 *dist);

	//  Return the first object found
	ObjectID first(GameObject **obj) {
		return first(obj, NULL);
	}
	//  Return the next object found
	ObjectID next(GameObject **obj) {
		return next(obj, NULL);
	}
};

/* ======================================================================= *
   CircularObjectIterator Class
 * ======================================================================= */

//	Iterate through all objects within a circular region

class CircularObjectIterator : public RadialObjectIterator {
protected:

	//  Compute the distance to the specified point from the center
	int16 computeDist(const TilePoint &tp);

public:
	//  Constructor
	CircularObjectIterator(
	    GameWorld       *world,
	    const TilePoint &searchCenter,
	    int16           distance) :
		RadialObjectIterator(world, searchCenter, distance) {
	}
};

/* ======================================================================= *
   RingObjectIterator Class
 * ======================================================================= */

//	Iterate through all objects within a circular region

class RingObjectIterator : public CircularObjectIterator {
private:

	int16 innerDist;

public:
	//  Constructor
	RingObjectIterator(
	    GameWorld       *world,
	    const TilePoint &searchCenter,
	    int16           outerDistance,
	    int16           innerDistance) :
		CircularObjectIterator(world, searchCenter, outerDistance) {
		innerDist = innerDistance;
	}

	ObjectID first(GameObject **obj);
	ObjectID next(GameObject **obj);
};

/* ======================================================================= *
   DispRegionObjectIterator Class
 * ======================================================================= */

//	Iterate through all objects within a region parallel to the display
//	area

class DispRegionObjectIterator : public RadialObjectIterator {
private:

	//  Compute the distance to the specified point from the center
	int16 computeDist(const TilePoint &tp);

public:
	//  Constructor
	DispRegionObjectIterator(
	    GameWorld       *world,
	    const TilePoint &searchCenter,
	    int16           distance) :
		RadialObjectIterator(world, searchCenter, distance) {
	}
};

/* ======================================================================= *
   RegionalObjectIterator Class
 * ======================================================================= */

//	Iterate through all objects within a rectangular region

class RegionalObjectIterator : public SectorRegionObjectIterator {

	TilePoint       minCoords,
	                maxCoords;

	//  Calculate the sector region to pass to the ObjectIterator
	//  constructor
	static TileRegion computeSectorRegion(
	    const TilePoint &sectors,
	    const TilePoint &min,
	    const TilePoint &max);

	//  Test to see if the specified point is within the region
	bool inRegion(const TilePoint &tp);

public:
	//  Constructor
	RegionalObjectIterator(
	    GameWorld *world,
	    const TilePoint &min,
	    const TilePoint &max) :
		SectorRegionObjectIterator(
		    world,
		    computeSectorRegion(world->sectorSize(), min, max)),
		minCoords(min),
		maxCoords(max) {
	}

	//  Iteration functions
	virtual ObjectID first(GameObject **obj);
	virtual ObjectID next(GameObject **obj);
};

/* ======================================================================= *
   RectangularObjectIterator Class
 * ======================================================================= */

//	Iterate through all objects within a rectangular region

class RectangularObjectIterator : public RegionalObjectIterator {

	TilePoint center,
	          coords1,
	          coords2,
	          coords3,
	          coords4;

	//  Calculate the sector region to pass to the ObjectIterator
	//  constructor
	static TilePoint computeMinPoint(
	    const TilePoint &c1,
	    const TilePoint &c2,
	    const TilePoint &c3,
	    const TilePoint &c4) {
		return MinTilePoint(c1, c2, c3, c4);
	}

	static TilePoint computeMaxPoint(
	    const TilePoint &c1,
	    const TilePoint &c2,
	    const TilePoint &c3,
	    const TilePoint &c4) {
		return MaxTilePoint(c1, c2, c3, c4);
	}

	//  Test to see if the specified point is within the region
	bool inRegion(const TilePoint &tp);

public:
	//  Constructor
	RectangularObjectIterator(
	    GameWorld *world,
	    const TilePoint &c,
	    const TilePoint &cdelta1,
	    const TilePoint &cdelta2) :
		RegionalObjectIterator(
		    world,
		    computeMinPoint(c, c + cdelta1, c + cdelta2, c + cdelta1 + cdelta2),
		    computeMaxPoint(c, c + cdelta1, c + cdelta2, c + cdelta1 + cdelta2)),
		coords1(c),
		coords2(c + cdelta1),
		coords3(c + cdelta1 + cdelta2),
		coords4(c + cdelta2),
		center((c + (cdelta1 + cdelta2) / 2)) {
	}

	virtual ObjectID first(GameObject **obj);
	virtual ObjectID next(GameObject **obj);

};

/* ======================================================================= *
   TriangularObjectIterator Class
 * ======================================================================= */

//	Iterate through all objects within a rectangular region

class TriangularObjectIterator : public RegionalObjectIterator {

	TilePoint       coords1,
	                coords2,
	                coords3;

	//  Calculate the sector region to pass to the ObjectIterator
	//  constructor
	static TilePoint computeMinPoint(
	    const TilePoint &c1,
	    const TilePoint &c2,
	    const TilePoint &c3) {
		return MinTilePoint(c1, c2, c3);
	}

	static TilePoint computeMaxPoint(
	    const TilePoint &c1,
	    const TilePoint &c2,
	    const TilePoint &c3) {
		return MaxTilePoint(c1, c2, c3);
	}

	//  Test to see if the specified point is within the region
	bool inRegion(const TilePoint &tp);

public:
	//  Constructor
	TriangularObjectIterator(
	    GameWorld *world,
	    const TilePoint &c1,
	    const TilePoint &c2,
	    const TilePoint &c3) :
		RegionalObjectIterator(
		    world,
		    computeMinPoint(c1, c2, c3),
		    computeMaxPoint(c1, c2, c3)),
		coords1(c1),
		coords2(c2),
		coords3(c3) {
	}

	//  Iteration functions
	ObjectID first(GameObject **obj);
	ObjectID next(GameObject **obj);
};

/* ======================================================================= *
   CenterRegionObjectIterator Class
 * ======================================================================= */

class CenterRegionObjectIterator : public RegionalObjectIterator {

	static GameWorld *CenterWorld(void);
	static TilePoint MinCenterRegion(void);
	static TilePoint MaxCenterRegion(void);

public:
	//  Constructor
	CenterRegionObjectIterator(void) :
		RegionalObjectIterator(CenterWorld(),
		                       MinCenterRegion(),
		                       MaxCenterRegion()) {}

};

/* ======================================================================= *
   ActiveRegionObjectIterator Class
 * ======================================================================= */

class ActiveRegionObjectIterator : public ObjectIterator {

	int16           activeRegionIndex;
	TilePoint       baseSectorCoords,
	                size,
	                sectorCoords;
	uint8           sectorBitMask;
	GameWorld       *currentWorld;
	GameObject      *currentObject;

	bool firstActiveRegion(void);
	bool nextActiveRegion(void);
	bool firstSector(void);
	bool nextSector(void);

public:
	//  Constructor
	ActiveRegionObjectIterator(void) : activeRegionIndex(-1) {}

	//  Iteration functions
	ObjectID first(GameObject **obj);
	ObjectID next(GameObject **obj);
};

/* ============================================================================ *
   Container Iterator Class
 * ============================================================================ */

//  This class iterates through every object within a container

class ContainerIterator {
	ObjectID        *nextID;

public:
	GameObject      *object;

	//  Constructor
	ContainerIterator(GameObject *container);

	//  Iteration function
	ObjectID        next(GameObject **obj);
};

/* ============================================================================ *
   Recursive Container iterator Class
 * ============================================================================ */

//  This class iterates through every object within a container and
//  all of the containers within the container

#if 0
class RecursiveContainerIterator {
	ObjectID                    id;
	RecursiveContainerIterator  *subIter;

public:
	//  Constructor
	RecursiveContainerIterator(GameObject *container) :
		id(container->IDChild()),
		subIter(NULL) {
	}
	~RecursiveContainerIterator(void);

	//  Iteration functions
	ObjectID first(GameObject **obj);
	ObjectID next(GameObject **obj);
};
#else

class RecursiveContainerIterator {
	ObjectID                    id,
	                            root;

public:
	//  Constructor
	RecursiveContainerIterator(GameObject *container) :
		root(container->thisID()) {
	}

	//  Iteration functions
	ObjectID first(GameObject **obj);
	ObjectID next(GameObject **obj);
};

#endif

/* ============================================================================ *
   Object sound effect struct
 * ============================================================================ */

struct ObjectSoundFXs {
	uint8           soundFXHitFlesh,
	                soundFXHitHard,
	                soundFXParried,
	                soundFXMissed;
};

/* ======================================================================= *
   Misc Prototypes
 * ======================================================================= */


//  Defines values for sixteen missile facings, plus a value for no
//  missile facing.
enum MissileFacings {
	missileUp,
	missileUpUpLf,
	missileUpLf,
	missileUpLfLf,
	missileLf,
	missileDnLfLf,
	missileDnLf,
	missileDnDnLf,
	missileDn,
	missileDnDnRt,
	missileDnRt,
	missileDnRtRt,
	missileRt,
	missileUpRtRt,
	missileUpRt,
	missileUpUpRt,
	missileNoFacing,
};

enum blockageType {
	blockageNone = 0,
	blockageTerrain,
	blockageObject
};

uint32 objectTerrain(GameObject *obj);

//  Return an object which is in collision with another object.
GameObject *objectCollision(GameObject *obj, GameWorld *world, const TilePoint &loc);

//  Test for line of sight between two objects
bool lineOfSight(GameObject *obj1, GameObject *obj2, uint32 terrainMask);
bool lineOfSight(GameObject *obj, const TilePoint &loc, uint32 terrainMask);
bool lineOfSight(
    GameWorld       *world,
    const TilePoint &loc1,
    const TilePoint &loc2,
    uint32          terrainMask);

//  Test if object is obscured by terrain
bool objObscured(GameObject *testObj);

//  Determine which object mouse pointer is picking
ObjectID pickObject(const Point16 &mouse, TilePoint &objPos);

//  Create enchantment attach it to object
ObjectID EnchantObject(
    ObjectID        target,
    int             enchantmentType,
    int             duration);

//  Find an enchantment of a particular type
ObjectID FindObjectEnchantment(
    ObjectID        target,
    int             enchantmentType);

//  Remove an enchantment of a particular type
bool DispelObjectEnchantment(
    ObjectID        target,
    int             enchantmentType);

//  Function to eval the enchantments on an actor
void evalActorEnchantments(Actor *a);

//  Function to eval the enchantments on an actor
void evalObjectEnchantments(GameObject *obj);

//  Load prototypes from resource file
void initPrototypes(void);

//  Cleanup the prototype lists
void cleanupPrototypes(void);

//  Load the sound effects table
void initObjectSoundFXTable(void);

//  Cleanup the sound effects table
void cleanupObjectSoundFXTable(void);

//  Allocate array to hold the counts of the temp actors
void initTempActorCount(void);

//  Save the array of temp actor counts
void saveTempActorCount(SaveFileConstructor &saveGame);

//  Load the array of temp actor counts
void loadTempActorCount(SaveFileReader &saveGame);

//  Cleanup the array to temp actor counts
void cleanupTempActorCount(void);

//  Increment the temporary actor count for the specified prototype
void incTempActorCount(uint16 protoNum);

//  Decrement the temporary actor count for the specified prototype
void decTempActorCount(uint16 protoNum);

//  Return the number of temporary actors for the specified prototype
uint16 getTempActorCount(uint16 protoNum);

//  Init game worlds
void initWorlds(void);

//  Save worlds to the save file
void saveWorlds(SaveFileConstructor &saveGame);

//  Load worlds from the save file
void loadWorlds(SaveFileReader &saveGame);

//  Cleanup game worlds
void cleanupWorlds(void);

//  Initialize object list
void initObjects(void);

//  Save the objects to the save file
void saveObjects(SaveFileConstructor &saveGame);

//  Load the objects from the save file
void loadObjects(SaveFileReader &saveGame);

//  Cleanup object list
void cleanupObjects(void);

//  Do background processing for objects
void doBackgroundSimulation(void);

void pauseBackgroundSimulation(void);
void resumeBackgroundSimulation(void);

// cleanup the ready container stuff
void cleanupReadyContainers(void);

//  This function simply calls the GameObject::updateState() method
//  for all active objects directly within a world.
void updateObjectStates(void);

void pauseObjectStates(void);
void resumeObjectStates(void);

void readyContainerSetup(void);
void cleanupReadyContainers(void);

#ifdef _WIN32   //  Reset old structure alignment
#pragma pack( pop )
#endif

} // end of namespace Saga2

#endif
