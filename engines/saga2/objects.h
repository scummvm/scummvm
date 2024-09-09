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

#ifndef SAGA2_OBJECTS_H
#define SAGA2_OBJECTS_H

#include "saga2/objproto.h"
#include "saga2/property.h"

namespace Saga2 {

/* ======================================================================= *
   GameObject: Describes an instance of an object
 * ======================================================================= */

class GameWorld;

const uint16 unlimitedCapacity = maxuint16;

enum ActorManaID {
	kManaIDRed = 0,
	kManaIDOrange,
	kManaIDYellow,
	kManaIDGreen,
	kManaIDBlue,
	kManaIDViolet,

	kNumManas
};

//  Used to indicate if objects can be stacked or merged
enum {
	kCannotStackOrMerge = 0,
	kCanStack,
	kCanMerge
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

	ResourceGameObject(Common::SeekableReadStream *stream);
};

//  Base class of all objects
//
//  Unlike the object prototypes, the only subclass of GameObject is
//  the actor subclass, which is kept in an entirely separate table.
//  This allows all objects to be kept in an array (indexed by ID number)

#include "common/pack-start.h"

struct ObjectData {
	uint32 projectDummy;
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
	ActiveItemID    currentTAG;             // ActiveItem object is on
	uint8           sightCtr;               // Line of sight counter

	uint8           reserved[2];

	GameObject *obj;
} PACKED_STRUCT;

#include "common/pack-end.h"

void     initActors();
void     saveActors(Common::OutSaveFile *outS);
void     loadActors(Common::InSaveFile *in);
void     cleanupActors();
class GameObject {

	friend void     initWorlds();
	friend void     cleanupWorlds();

	friend void     initObjects();
	friend void     saveObjects(Common::OutSaveFile *out);
	friend void     loadObjects(Common::InSaveFile *in);
	friend void     cleanupObjects();

	friend void     buildDisplayList();
	friend void     drawDisplayList();
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
		kMaxRow      = 20,
		kMaxCol      = 4
	};

public:

	ObjectID thisID();               // calculate our own ID value

	static const char *nameText(uint16 index);

protected:
	//  get address of head-of-chain id.
	static ObjectID *getHeadPtr(ObjectID parentID, TilePoint &l);

	//  Object list management functions
	void remove();                   // removes from old list
	void append(ObjectID newParent);         // adds to new list (no remove)
	void insert(ObjectID newPrev);           // inserts after this item (no remove)

	ProtoObj        *_prototype;             // object that defines our behavior
public:
	ObjectData _data;
	uint _index;
	bool _godmode;
	//  Default constructor
	GameObject();

	//  Constructor -- initial construction
	GameObject(const ResourceGameObject &res);

	GameObject(Common::InSaveFile *in);

	void read(Common::InSaveFile *in, bool expandProto);

	//  Return the number of bytes needed to archive this object in
	//  a buffer
	int32 archiveSize();

	void write(Common::MemoryWriteStreamDynamic *out, bool expandProto);

	//  returns the address of the object based on the ID, and this
	//  includes accounting for actors.
	static GameObject *objectAddress(ObjectID id);

	//  Converts object ID into prototype address...
	static ProtoObj *protoAddress(ObjectID id);

	// Returns first object id found associate with the given name index
	static int32 nameIndexToID(uint16 ind);

	// Returns first object id found assciated with the given name
	static Common::Array<ObjectID> nameToID(Common::String name);

	//  object creation and deletion
	static GameObject *newObject();      // get a newly created object
	void deleteObject();                 // delete this object and remove
	void deleteObjectRecursive();        // delete this object and every
	// object it contains

	//  Return pointer to parent/child/next sibling object, if any
	GameObject *parent() {
		return _data.parentID == Nothing ? NULL : objectAddress(_data.parentID);
	}
	GameObject *next() {
		return _data.siblingID == Nothing ? NULL : objectAddress(_data.siblingID);
	}
	GameObject *child() {
		return _data.childID == Nothing ? NULL : objectAddress(_data.childID);
	}

	//  Return ID of parent/child/next sibling object, if any
	ObjectID IDParent() {
		return _data.parentID ;
	}
	ObjectID IDNext() {
		return _data.siblingID;
	}
	ObjectID IDChild() {
		return _data.childID  ;
	}

	//  Return a pointer to the world on which this object resides
	GameWorld *world();

	//  Return the number of the map of the world on which this object
	//  resides
	int16 getMapNum();

	//  graphics functions
	int16 sprNum(int16 state);               // returns current sprite number

	// gets the offset to the spr image according to masscount for mergeables
	int32 getSprOffset(int16 num = -1);

	// completely restore the magical energy of a magical object
	void recharge();

	// returns the type of charge an object has
	// be it none, red, violet, etc...
	int16 getChargeType();

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

	//  Remove an object from a stack of objects. Returns true if it was in a stack.
	bool unstack();

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
	void activate();

	//  Deactivate this object
	void deactivate();

	//  Determine if this object is an alias for another object
	bool isAlias() {
		return (_data.objectFlags & kObjectAlias) != 0;
	}

	//  check to see if item can be contained by this object
	bool canContain(ObjectID item) {
		return _prototype->canContain(thisID(), item);
	}

	//  check to see if item is contained by the object
	bool isContaining(GameObject *item);

	//  check if an instance of the specified target is contained in
	//  this object
	bool isContaining(ObjectTarget *objTarget);

	//  determine whether this object has a specified property
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
	    bool        canMerge = false,
	    GameObject  **mergeObj = NULL);

	//  Find a slot to place the specified object within this object and
	//  drop it in that slot
	//  If merge count == 0, then no auto-merging allowed
	bool placeObject(
	    ObjectID    enactor,
	    ObjectID    objID,
	    bool        canMerge = false,
	    int16       num = 1);

	//  Drop the specified object on the ground in a semi-random location
	void dropInventoryObject(GameObject *obj, int16 count = 1);

	//Get Specific Container From Object
	GameObject *getIntangibleContainer(int containerType);

	//  generic actions
	bool use(ObjectID enactor) {
		return _prototype->use(thisID(), enactor);
	}
	bool useOn(ObjectID enactor, ObjectID item) {
		return _prototype->useOn(thisID(), enactor, item);
	}

	bool useOn(ObjectID enactor, ActiveItem *item) {
		return _prototype->useOn(thisID(), enactor, item);
	}

	bool useOn(ObjectID enactor, Location &loc) {
		return _prototype->useOn(thisID(), enactor, loc);
	}

	//  various verb actions that can take place
	bool take(ObjectID enactor, int16 num = 1) {
		return _prototype->take(thisID(), enactor, num);
	}
	bool drop(ObjectID enactor, const Location &l, int16 num = 1) {
		return _prototype->drop(thisID(), enactor, l, num);
	}
	//  drop an object onto another object and handle the result.
	bool dropOn(ObjectID enactor, ObjectID target, int16 num = 1) {
		return _prototype->dropOn(thisID(), enactor, target, num);
	}
	//  drop this object on a TAG
	bool dropOn(ObjectID enactor, ActiveItem *target, const Location &loc, int16 num = 1) {
		return _prototype->dropOn(thisID(), enactor, target, loc, num);
	}
	bool open(ObjectID enactor) {
		return _prototype->open(thisID(), enactor);
	}
	bool close(ObjectID enactor) {
		return _prototype->close(thisID(), enactor);
	}
	bool strike(ObjectID enactor, ObjectID item) {
		return _prototype->strike(thisID(), enactor, item);
	}
	bool damage(ObjectID enactor, ObjectID target) {
		return _prototype->damage(thisID(), enactor, target);
	}
	bool eat(ObjectID enactor) {
		return _prototype->eat(thisID(), enactor);
	}
	bool insert(ObjectID enactor, ObjectID item) {
		return _prototype->insert(thisID(), enactor, item);
	}
	bool remove(ObjectID enactor) {
		return _prototype->remove(thisID(), enactor);
	}
	bool acceptDrop(ObjectID enactor, ObjectID droppedObj, int count) {
		return _prototype->acceptDrop(thisID(), enactor, droppedObj, count);
	}
	bool acceptDamage(
	    ObjectID            enactor,
	    int8                absDamage,
	    effectDamageTypes   dType = kDamageOther,
	    int8                dice = 0,
	    uint8               sides = 1,
	    int8                perDieMod = 0) {
		if (_godmode)
			return false;

		return  _prototype->acceptDamage(
		            thisID(),
		            enactor,
		            absDamage,
		            dType,
		            dice,
		            sides,
		            perDieMod);
	}
	bool acceptHealing(ObjectID enactor, int8 absDamage, int8 dice = 0, uint8 sides = 1, int8 perDieMod = 0) {
		return _prototype->acceptHealing(thisID(), enactor, absDamage, dice, sides, perDieMod);
	}
	bool acceptStrike(
	    ObjectID            enactor,
	    ObjectID            strikingObj,
	    uint8               skillIndex) {
		return  _prototype->acceptStrike(
		            thisID(),
		            enactor,
		            strikingObj,
		            skillIndex);
	}
	bool acceptLockToggle(ObjectID enactor, uint8 keyCode) {
		return _prototype->acceptLockToggle(thisID(), enactor, keyCode);
	}
	bool acceptMix(ObjectID enactor, ObjectID mixObj) {
		return _prototype->acceptMix(thisID(), enactor, mixObj);
	}
	bool acceptInsertion(ObjectID enactor, ObjectID item, int16 count) {
		return _prototype->acceptInsertion(thisID(), enactor, item, count);
	}
	bool acceptInsertionAt(ObjectID enactor, ObjectID item, const TilePoint &where, int16 num = 1) {
		return _prototype->acceptInsertionAt(thisID(), enactor, item, where, num);
	}

	//  query functions:
	ObjectID possessor();                // return actor possessing this object

	//  Access functions
	ProtoObj *proto() {
		return _prototype;
	}
	TilePoint getLocation() const {
		return _data.location;
	}
	TilePoint getWorldLocation();
	bool getWorldLocation(Location &loc);
	Location notGetLocation();
	Location notGetWorldLocation();

	//  Return the name of this object (proper noun if it has one)
	const char *objName() {
		if (_data.nameIndex > 0)
			return nameText((int16)_data.nameIndex);
		else if (_prototype)
			return nameText((int16)_prototype->nameIndex);

		return nameText(0);
	}

	// return name of object, and it's quantity if merged
	void objCursorText(char nameBuf[], const int8 size, int16 count = -1);

	// find out if this is a trueskill
	bool isTrueSkill();

	//  Access functions for name index
	uint16 getNameIndex() {
		return _data.nameIndex;
	}
	void setNameIndex(uint16 n) {
		_data.nameIndex = n;
	}

	//  Return the name of this type of object
	const char *protoName() {
		return nameText(_prototype->nameIndex);
	}

	//  Update the state of this object.  This function is called every
	//  frame for every active object.
	void updateState();

	//  Flag test functions
	bool isOpen() {
		return (int16)(_data.objectFlags & kObjectOpen);
	}
	bool isLocked() {
		return (int16)(_data.objectFlags & kObjectLocked);
	}
	bool isImportant() {
		return (int16)(_data.objectFlags & kObjectImportant);
	}
	bool isGhosted() {
		return (_data.objectFlags & kObjectGhosted)
		       || (_prototype->flags & ResourceObjectPrototype::kObjPropGhosted);
	}
	bool isInvisible() {
		return (_data.objectFlags & kObjectInvisible)
		       || (_prototype->flags & ResourceObjectPrototype::kObjPropHidden);
	}
	bool isMoving() {
		return (int16)(_data.objectFlags & kObjectMoving);
	}
	bool isActivated() {
		return (int16)(_data.objectFlags & kObjectActivated);
	}

	void setScavengable(bool val) {
		if (val)
			_data.objectFlags |= kObjectScavengable;
		else
			_data.objectFlags &= ~kObjectScavengable;
	}
	bool isScavengable() {
		return (_data.objectFlags & kObjectScavengable) != 0;
	}

	void setObscured(bool val) {
		if (val)
			_data.objectFlags |= kObjectObscured;
		else
			_data.objectFlags &= ~kObjectObscured;
	}
	bool isObscured() {
		return (_data.objectFlags & kObjectObscured) != 0;
	}

	void setTriggeringTAG(bool val) {
		if (val)
			_data.objectFlags |= kObjectTriggeringTAG;
		else
			_data.objectFlags &= ~kObjectTriggeringTAG;
	}
	bool isTriggeringTAG() {
		return (_data.objectFlags & kObjectTriggeringTAG) != 0;
	}

	void setOnScreen(bool val) {
		if (val)
			_data.objectFlags |= kObjectOnScreen;
		else
			_data.objectFlags &= ~kObjectOnScreen;
	}
	bool isOnScreen() {
		return (_data.objectFlags & kObjectOnScreen) != 0;
	}

	void setSightedByCenter(bool val) {
		if (val)
			_data.objectFlags |= kObjectSightedByCenter;
		else
			_data.objectFlags &= ~kObjectSightedByCenter;
	}
	bool isSightedByCenter() {
		return (_data.objectFlags & kObjectSightedByCenter) != 0;
	}

	bool isMissile() {
		return _prototype->isMissile();
	}

	// image data
	Sprite *getIconSprite();    // sprite when in inventory + cursor
	Sprite *getGroundSprite();  // sprite when on ground

	// world interaction type flags
	uint16 containmentSet();

	uint16 scriptClass() {
		if (_data.script)
			return _data.script;
		if (_prototype)
			return _prototype->script;
		return 0;
	}

	//  General access functions

	//  Script access functions
	uint16 getScript() {
		return _data.script;
	}
	void setScript(uint16 scr) {
		_data.script = scr;
	}

	//  access function to set object flags
	void setFlags(uint8 newval, uint8 changeMask) {
		//  Only change the flags spec'd by changeFlags
		_data.objectFlags = (newval & changeMask)
		              | (_data.objectFlags & ~changeMask);
	}

	//  Access functions for hit points
	uint8 getHitPoints() {
		return _data.hitPoints;
	}
	void setHitPoints(uint8 hp) {
		_data.hitPoints = hp;
	}

	//  Builds the color remapping for this object based on the
	//  prototype's color map
	void getColorTranslation(ColorTable map) {
		_prototype->getColorTranslation(map);
	}

	//  Functions to get and set prototype (used by scripts)
	int32 getProtoNum();
	void setProtoNum(int32 nProto);

	//  Acess functions for extra data
	uint16 getExtra() {
		return _data.massCount;
	}
	void setExtra(uint16 x) {
		_data.massCount = x;
	}

	//  Function to evaluate the effects of all enchantments
	void evalEnchantments();

	bool makeSavingThrow() {
		return _prototype->makeSavingThrow();
	}

	//  Generic range checking function
	bool inRange(const TilePoint &tp, uint16 range);

	//  Generic function to test if object can be picked up
	bool isCarryable() {
		return _prototype->mass <= 200 && _prototype->bulk <= 200;
	}

	bool isMergeable() {
		return (_prototype->flags & ResourceObjectPrototype::kObjPropMergeable) != 0;
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
	void removeAllTimers();

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
	void removeAllSensors();

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
		return _prototype->canFitBulkwise(this, obj);
	}
	bool canFitMasswise(GameObject *obj) {
		return _prototype->canFitMasswise(this, obj);
	}

	uint16 totalContainedMass();
	uint16 totalContainedBulk();

	uint16 totalMass() {
		return      _prototype->mass * (isMergeable() ? getExtra() : 1)
		            +   totalContainedMass();
	}
	uint16 totalBulk() {
		return _prototype->bulk * (isMergeable() ? getExtra() : 1);
	}

	uint16 massCapacity() {
		return _prototype->massCapacity(this);
	}
	uint16 bulkCapacity() {
		return _prototype->bulkCapacity(this);
	}
};

/* ===================================================================== *
   Sector struct
 * ===================================================================== */

class Sector {
public:
	uint16          _activationCount;
	ObjectID        _childID;

	Sector() :
		_activationCount(0),
		_childID(Nothing) {
	}

	bool isActivated() {
		return _activationCount != 0;
	}

	void activate();
	void deactivate();

	void write(Common::MemoryWriteStreamDynamic *out);
	void read(Common::InSaveFile *in);
};

/* ======================================================================= *
   GameWorld: Describes a world within the game
 * ======================================================================= */

//  Terminology note: A "sector" is a small portion of a map.
//  All objects within a sector are stored on a single list.

//  The size of a sector (length of side) in UV coodinates.
//  A sector is an area of about 4x4 metatiles.

class GameWorld : public GameObject {

	friend void     initWorlds();
	friend void     cleanupWorlds();
	friend void     buildDisplayList();

	friend class    ProtoObj;
	friend class    GameObject;
	friend class    ObjectIterator;

public:
	TilePoint       _size;                   // size of world in U/V coords
	int16           _sectorArraySize;        // size of sector array
	Sector          *_sectorArray;          // array of sectors
	int16           _mapNum;                 // map number for this world.

	//  Default constructor
	GameWorld() : _sectorArraySize(0), _sectorArray(nullptr), _mapNum(0) {}

	//  Initial constructor
	GameWorld(int16 map);

	GameWorld(Common::SeekableReadStream *stream);

	~GameWorld();

	int32 archiveSize();

	void cleanup();

	Sector *getSector(int16 u, int16 v) {
		if (u == -1 && v == -1)
			return nullptr;

		if (v * _sectorArraySize + u >= _sectorArraySize * _sectorArraySize ||
		    v * _sectorArraySize + u < 0) {
			warning("Sector::getSector: Invalid sector: (%d, %d) (sectorArraySize = %d)", u, v, _sectorArraySize);
			return nullptr;
		}

		return &(_sectorArray)[v * _sectorArraySize + u];
	}

	TilePoint sectorSize() {         // size of map in sectors
		return TilePoint(_sectorArraySize, _sectorArraySize, 0);
	}

	static uint32 IDtoMapNum(ObjectID id) {
		assert(isWorld(id));
		return ((GameWorld *)GameObject::objectAddress(id))->_mapNum;
	}
};

void setCurrentWorld(ObjectID worldID);

extern GameWorld    *currentWorld;

/* ======================================================================= *
   GameObject inline member function
 * ======================================================================= */

//------------------------------------------------------------------------
//	Return the number of the map of the world on which this object resides.

inline int16 GameObject::getMapNum() {
	if (world())
		return world()->_mapNum;
	else if (_data.siblingID) {
		GameObject *sibling = GameObject::objectAddress(_data.siblingID);
		return sibling->getMapNum();
	} else
		return currentWorld->_mapNum;
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

	friend void initActiveRegions();
	friend void cleanupActiveRegions();

	friend class ActiveRegionObjectIterator;

	ObjectID        _anchor;     //  ID of object this region is attached to
	TilePoint       _anchorLoc;  //  Location of anchor
	ObjectID        _worldID;
	TileRegion      _region;     //  Region coords ( in sectors )

public:

	enum {
		kActiveRegionSize = 22
	};

	ActiveRegion() : _anchor(0), _worldID(0) {}
	void update();

	void read(Common::InSaveFile *in);
	void write(Common::MemoryWriteStreamDynamic *out);

	//  Return the current region in tile point coords
	TileRegion getRegion() {
		TileRegion      tReg;

		tReg.min.u = _region.min.u << kSectorShift;
		tReg.min.v = _region.min.v << kSectorShift;
		tReg.max.u = _region.max.u << kSectorShift;
		tReg.max.v = _region.max.v << kSectorShift;
		tReg.min.z = tReg.max.z = 0;

		return tReg;
	}

	//  Return the region world
	GameWorld *getWorld() {
		return (GameWorld *)GameObject::objectAddress(_worldID);
	}
};

void updateActiveRegions();

//  Return a pointer to an active region given its PlayerActor's ID
ActiveRegion *getActiveRegion(PlayerActorID id);

void initActiveRegions();
void saveActiveRegions(Common::OutSaveFile *outS);
void loadActiveRegions(Common::InSaveFile *in);
inline void cleanupActiveRegions() {}

/* ======================================================================= *
   ObjectIterator Class
 * ======================================================================= */

//  This class simply defines a standard interface for all derived
//  object iterator classes.

class ObjectIterator {
public:
	//  Virtual destructor
	virtual ~ObjectIterator() {}

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

	TilePoint       _minSector,
	                _maxSector,
	                _sectorCoords;
	GameWorld       *_searchWorld;
	GameObject      *_currentObject;

public:
	//  Constructor
	SectorRegionObjectIterator(GameWorld *world);

	//  Constructor
	SectorRegionObjectIterator(
	    GameWorld           *world,
	    const TileRegion    &sectorRegion) :
		_searchWorld(world),
		_minSector(sectorRegion.min),
		_maxSector(sectorRegion.max),
		_currentObject(nullptr) {
		assert(_searchWorld != NULL);
		assert(isWorld(_searchWorld));
	}

protected:
	GameWorld *getSearchWorld() {
		return _searchWorld;
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

	TilePoint       _center;
	int16           _radius;

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
	TilePoint getCenter() {
		return _center;
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
		_center(searchCenter),
		_radius(distance) {
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

	int16 _innerDist;

public:
	//  Constructor
	RingObjectIterator(
	    GameWorld       *world,
	    const TilePoint &searchCenter,
	    int16           outerDistance,
	    int16           innerDistance) :
		CircularObjectIterator(world, searchCenter, outerDistance) {
		_innerDist = innerDistance;
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

	//  Test to see if the specified point is within the region
	bool inRegion(const TilePoint &tp);

public:
	//  Constructor
	RectangularObjectIterator(
	    GameWorld *world,
	    const TilePoint &c,
	    const TilePoint &cdelta1,
	    const TilePoint &cdelta2);

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

	//  Test to see if the specified point is within the region
	bool inRegion(const TilePoint &tp);

public:
	//  Constructor
	TriangularObjectIterator(
	    GameWorld *world,
	    const TilePoint &c1,
	    const TilePoint &c2,
	    const TilePoint &c3);

	//  Iteration functions
	ObjectID first(GameObject **obj);
	ObjectID next(GameObject **obj);
};

/* ======================================================================= *
   CenterRegionObjectIterator Class
 * ======================================================================= */

class CenterRegionObjectIterator : public RegionalObjectIterator {

	static GameWorld *CenterWorld();
	static TilePoint MinCenterRegion();
	static TilePoint MaxCenterRegion();

public:
	//  Constructor
	CenterRegionObjectIterator() :
		RegionalObjectIterator(CenterWorld(),
		                       MinCenterRegion(),
		                       MaxCenterRegion()) {}

};

/* ======================================================================= *
   ActiveRegionObjectIterator Class
 * ======================================================================= */

class ActiveRegionObjectIterator : public ObjectIterator {

	int16           _activeRegionIndex;
	TilePoint       _baseSectorCoords,
	                _size,
	                _sectorCoords;
	uint8           _sectorBitMask;
	GameWorld       *_currentWorld;
	GameObject      *_currentObject;

	bool firstActiveRegion();
	bool nextActiveRegion();
	bool firstSector();
	bool nextSector();

public:
	//  Constructor
	ActiveRegionObjectIterator() : _activeRegionIndex(-1), _sectorBitMask(0), _currentWorld(nullptr), _currentObject(nullptr) {}

	//  Iteration functions
	ObjectID first(GameObject **obj);
	ObjectID next(GameObject **obj);
};

/* ============================================================================ *
   Container Iterator Class
 * ============================================================================ */

//  This class iterates through every object within a container

class ContainerIterator {
	ObjectID         _nextID;

public:
	GameObject      *_object;

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

class RecursiveContainerIterator {
	ObjectID                    _id,
	                            _root;

public:
	//  Constructor
	RecursiveContainerIterator(GameObject *container) :
		_root(container->thisID()), _id(0) {}

	//  Iteration functions
	ObjectID first(GameObject **obj);
	ObjectID next(GameObject **obj);
};

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
	kMissileUp,
	kMissileUpUpLf,
	kMissileUpLf,
	kMissileUpLfLf,
	kMissileLf,
	kMissileDnLfLf,
	kMissileDnLf,
	kMissileDnDnLf,
	kMissileDn,
	kMissileDnDnRt,
	kMissileDnRt,
	kMissileDnRtRt,
	kMissileRt,
	kMissileUpRtRt,
	kMissileUpRt,
	kMissileUpUpRt,
	kMissileNoFacing
};

enum blockageType {
	kBlockageNone = 0,
	kBlockageTerrain,
	kBlockageObject
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
ObjectID pickObject(const StaticPoint32 &mouse, StaticTilePoint &objPos);

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
void initPrototypes();

//  Cleanup the prototype lists
void cleanupPrototypes();

//  Load the sound effects table
void initObjectSoundFXTable();

//  Cleanup the sound effects table
void cleanupObjectSoundFXTable();

//  Allocate array to hold the counts of the temp actors
void initTempActorCount();

//  Save the array of temp actor counts
void saveTempActorCount(Common::OutSaveFile *outS);

//  Load the array of temp actor counts
void loadTempActorCount(Common::InSaveFile *in, int32 chunkSize);

//  Cleanup the array to temp actor counts
void cleanupTempActorCount();

//  Increment the temporary actor count for the specified prototype
void incTempActorCount(uint16 protoNum);

//  Decrement the temporary actor count for the specified prototype
void decTempActorCount(uint16 protoNum);

//  Return the number of temporary actors for the specified prototype
uint16 getTempActorCount(uint16 protoNum);

//  Init game worlds
void initWorlds();

//  Save worlds to the save file
void saveWorlds(Common::OutSaveFile *outS);

//  Load worlds from the save file
void loadWorlds(Common::InSaveFile *in);

//  Cleanup game worlds
void cleanupWorlds();

//  Initialize object list
void initObjects();

//  Save the objects to the save file
void saveObjects(Common::OutSaveFile *outS);

//  Load the objects from the save file
void loadObjects(Common::InSaveFile *in);

//  Cleanup object list
void cleanupObjects();

//  Do background processing for objects
void doBackgroundSimulation();

void pauseBackgroundSimulation();
void resumeBackgroundSimulation();

//  This function simply calls the GameObject::updateState() method
//  for all active objects directly within a world.
void updateObjectStates();

void pauseObjectStates();
void resumeObjectStates();

void readyContainerSetup();
void cleanupReadyContainers();

} // end of namespace Saga2

#endif
