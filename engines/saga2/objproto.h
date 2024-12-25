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

#ifndef SAGA2_OBJPROTO_H
#define SAGA2_OBJPROTO_H

#include "saga2/idtypes.h"
#include "saga2/sprite.h"
#include "saga2/spells.h"
#include "saga2/effects.h"
#include "saga2/tcoords.h"

namespace Saga2 {

class Actor;
class gameObject;

/* ===================================================================== *
   Exports
 * ===================================================================== */

extern const int16 objectCount;  //  Number of elements in the object list
extern int16 worldCount;         //  Number of elements in the world list

#define Permanent ((uint8)255)

/* ===================================================================== *
   Object ID's
 * ===================================================================== */

//  Various inline tests for object type

inline bool isObject(ObjectID id) {
	return (id < objectCount);
}

inline bool isActor(ObjectID id) {
	return (id >= ActorBaseID && id < ActorBaseID + kActorCount);
}

inline bool isWorld(ObjectID id) {
	return (id >= WorldBaseID && id < WorldBaseID + worldCount);
}

inline bool isActorOrWorld(ObjectID id) {
	return (id >= ActorBaseID);
}

//  Same as above but use object addresses instead of ID's

class GameObject;
class IntangibleContainerWindow;
class ContainerWindow;
class ActiveItem;
class ObjectTarget;
class Sector;
class TimerList;
class Timer;
struct GameEvent;
struct SenseInfo;
class SensorList;
class Sensor;
struct ObjectSoundFXs;

bool isObject(GameObject *);
bool isActor(GameObject *);
bool isWorld(GameObject *);

/* ===================================================================== *
   Location: Describes location of object within world or container
 * ===================================================================== */

struct StaticLocation {
	StaticTilePoint tile;
	ObjectID context;

	void set(TilePoint t, ObjectID con) {
		tile.set(t.u, t.v, t.z);
		context = con;
	}
};

class Location : public TilePoint {
public:
	//  context = the ObjectID of containing context
	//  (either a container or a world).
	ObjectID        _context;

	/*
	        //  Member functions to translate world coords into
	        //  screen coords.
	    void screenPos( Point16 &screenCoords );
	    int16 screenDepth( void );

	        //  returns true if it is within the view area of the current
	        //  map.
	    bool visible( void );
	*/
	// assign a position to a location without changing context
	Location &operator=(TilePoint pos) {
		u = pos.u;
		v = pos.v;
		z = pos.z;
		return *this;
	}

	Location() : _context(0) {}

	Location(int16 nu, int16 nv, int16 nz, ObjectID con) {
		u = nu;
		v = nv;
		z = nz;
		_context = con;
	}

	Location(TilePoint p, ObjectID con = 0) {
		u = p.u;
		v = p.v;
		z = p.z;
		_context = con;
	}

	Location(StaticLocation l) {
		u = l.tile.u;
		v = l.tile.v;
		z = l.tile.z;
		_context = l.context;
	}

};

/* ===================================================================== *
   8-way facing directions
 * ===================================================================== */

inline bool isFlipped(Direction d) {
	return (d > kDirDown);
}

/* ===================================================================== *
   ProtoObj: Base class for all object prototypes.
 * ===================================================================== */

struct scriptCallFrame;

//  Since we want to be able to load prototypes from disk, but
//  still use virtual functions, we are going to divide the
//  prototype structure into two parts, one which has the data
//  and then a subclass which has all the functions.

struct ResourceObjectPrototype {

	//  General behavior properties

	int16           classType;              // which C++ class to use.
	uint16          script;                 // script to handle all objs of this type

	//  Appearance propertie

	int16           nameIndex;              // type name of object
	uint16          iconSprite,             // sprite # in inventory
	                groundSprite;           // sprite # when on ground

	uint8           colorMap[4];          // indirect color map

	//  Physical properties

	uint8           mass,                   // how heavy it is
	                bulk,                   // how bulky it is
	                crossSection,           // cross section width,
	                height,                 // height it extends above terrain
	                toughness,              // how easily broken,
	                breakType;              // what it breaks into

	//  Container properties

	uint16          maxCapacity;            // total space available
	uint8           lockType;               // type of key that opens
	uint8           acceptableItems;        // type of items that fit within

	//  Combat Properties (offensive)

	uint8           weaponDamage,
	                weaponFireRate,
	                maximumRange,
	                missileType;

	//  Combat Properties (defensive)

	uint8           whereWearable;
	int8            damageAbsorbtion,
	                damageDivider,
	                defenseBonus;

	//  Magical / Energy properties

	uint8           maxCharges,             // max number of charges, or 0=infinity
	                chargeType;             // charge type that can be used

	//  Packed flags

	int16           flags;

	//  Flag definitions

	enum protoFlags {
		kObjPropMergeable    = (1 << 0),     // merge with similar objects
		kObjPropRound        = (1 << 1),     // rolls easily down stairs
		kObjPropFlammable    = (1 << 2),     // object can burn
		kObjPropWeapon       = (1 << 3),     // can be wielded as weapon
		kObjPropThrownWpn    = (1 << 4),     // it's a throwable weapon
		kObjPropMissileWpn   = (1 << 5),     // it's a missile weapon
		kObjPropCharges      = (1 << 6),     // it's a missile weapon
		kObjPropEdible       = (1 << 7),     // can be eaten
		kObjPropFlipped      = (1 << 8),     // flipped left/right on ground
		kObjPropVisOpen      = (1 << 9),     // Object has separate "visible" sprite
		kObjPropHidden       = (1 << 10),    // "How not to be seen".
		kObjPropGhosted      = (1 << 11),    // Object permanently ghosted
		kObjPropHardSurface  = (1 << 12),    // Object makes hard sound when struck
		kObjPropNoSurface    = (1 << 13)     // Object makes no sound when struck (may grunt however)
	};

	int16           price;                  // object's price

	union {
		int16       heldSpriteBase;         // sprite # when in hand
		int16       appearanceType;         // container appearance type
		int16       ideaType;               // idea stimulus type
	};

	int16           resistance;             // resistance bits (see EFFECTS.H)
	int16           immunity;               // immunity bits   (see EFFECTS.H)

	uint8           soundFXClass;           // index into sound effects table

	uint8           reserved[7];

	ResourceObjectPrototype() {
		classType = 0;
		script = 0;
		nameIndex = 0;
		iconSprite = 0;
		groundSprite = 0;

		for (int i = 0; i < 4; ++i)
			colorMap[i] = 0;

		mass = bulk = crossSection = height = toughness = breakType = 0;
		maxCapacity = 0;
		lockType = 0;
		acceptableItems = 0;
		weaponDamage = weaponFireRate = maximumRange = missileType = 0;
		whereWearable = 0;
		damageAbsorbtion = damageDivider = defenseBonus = 0;
		maxCharges = chargeType = 0;
		flags = 0;
		price = 0;
		heldSpriteBase = 0;
		resistance = 0;
		immunity = 0;
		soundFXClass = 0;

		for (int i = 0; i < 7; ++i)
			reserved[i] = 0;
	}

	//  Copy constructor
	ResourceObjectPrototype(ResourceObjectPrototype &proto) {
		classType = proto.classType;
		script = proto.script;
		nameIndex = proto.nameIndex;
		iconSprite = proto.iconSprite;
		groundSprite = proto.groundSprite;

		for (int i = 0; i < 4; ++i)
			colorMap[i] = proto.colorMap[i];

		mass = proto.mass;
		bulk = proto.bulk;
		crossSection = proto.crossSection;
		height = proto.height;
		toughness = proto.toughness;
		breakType = proto.breakType;
		maxCapacity = proto.maxCapacity;
		lockType = proto.lockType;
		acceptableItems = proto.acceptableItems;
		weaponDamage = proto.weaponDamage;
		weaponFireRate = proto.weaponFireRate;
		maximumRange = proto.maximumRange;
		missileType = proto.missileType;
		whereWearable = proto.whereWearable;
		damageAbsorbtion = proto.damageAbsorbtion;
		damageDivider = proto.damageDivider;
		defenseBonus = proto.defenseBonus;
		maxCharges = proto.maxCharges;
		chargeType = proto.chargeType;
		flags = proto.flags;
		price = proto.price;
		heldSpriteBase = proto.heldSpriteBase;
		resistance = proto.resistance;
		immunity = proto.immunity;
		soundFXClass = proto.soundFXClass;

		for (int i = 0; i < 7; ++i)
			reserved[i] = proto.reserved[i];
	}

	void load(Common::SeekableReadStream *stream) {
		classType = stream->readSint16LE();
		script = stream->readUint16LE();
		nameIndex = stream->readSint16LE();
		iconSprite = stream->readUint16LE();
		groundSprite = stream->readUint16LE();

		for (int i = 0; i < 4; ++i)
			colorMap[i] = stream->readByte();

		mass = stream->readByte();
		bulk = stream->readByte();
		crossSection = stream->readByte();
		height = stream->readByte();
		toughness = stream->readByte();
		breakType = stream->readByte();
		maxCapacity = stream->readUint16LE();
		lockType = stream->readByte();
		acceptableItems = stream->readByte();
		weaponDamage = stream->readByte();
		weaponFireRate = stream->readByte();
		maximumRange = stream->readByte();
		missileType = stream->readByte();
		whereWearable = stream->readByte();
		damageAbsorbtion = stream->readSByte();
		damageDivider = stream->readSByte();
		defenseBonus = stream->readSByte();
		maxCharges = stream->readByte();
		chargeType = stream->readByte();
		flags = stream->readSint16LE();
		price = stream->readSint16LE();
		heldSpriteBase = stream->readSint16LE(); // union
		resistance = stream->readSint16LE();
		immunity = stream->readSint16LE();
		soundFXClass = stream->readByte();

		for (int i = 0; i < 7; ++i)
			reserved[i] = stream->readByte();
	}
};

class ProtoObj : public ResourceObjectPrototype {

	static uint8    *_nextAvailObj;


	// container defines
	// getable through virtual functions
	// at appropriate subclasses
private:
	enum {
		kViewableRows    = 6,
		kViewableCols    = 4,
		kMaxRows         = 8,
		kMaxCols         = 4
	};

public:

	enum containmentType {
		kIsTangible    = (1 << 0),
		kIsContainer   = (1 << 1),
		kIsBottle      = (1 << 2),
		kIsFood        = (1 << 3),
		kIsWearable    = (1 << 4),
		kIsWeapon      = (1 << 5),
		kIsArmor       = (1 << 6),
		kIsDocument    = (1 << 7),
		kIsIntangible  = (1 << 8),
		kIsConcept     = (1 << 9),
		kIsPsych       = (1 << 10),
		kIsSpell       = (1 << 11),
		kIsSkill       = (1 << 12),
		kIsEnchantment = (1 << 13),
		kIsTargetable  = (1 << 14)
	};

//	kludge: define earlier, incorrectly spelled names to correct spelling
//	REM: Later, do a global search and replace...
#define isTangable      kIsTangible
#define isIntangable    kIsIntangible

	enum spriteTypes {
		kObjOnGround = 0,
		kObjInContainerView,
		kObjAsMousePtr
	};

	//  Member functions

	//  A constructor which takes the data loaded from the file
	//  and loads it into the various fields...
	ProtoObj(ResourceObjectPrototype &proto) : ResourceObjectPrototype(proto) {}
	virtual ~ProtoObj() {}

	// returns the containment type flags for this object
	virtual uint16 containmentSet();

	//  returns true if this object can contain another object
	virtual bool canContain(ObjectID dObj, ObjectID item);

	//  Determine if this object can contain another object at a specified
	//  slot
	virtual bool canContainAt(
	    ObjectID dObj,
	    ObjectID item,
	    const TilePoint &where);

	//  Determine if this type of object is two handed
	virtual bool isTwoHanded(ObjectID actor);

	//  Determine if this type of object is a missile
	virtual bool isMissile();

	virtual ObjectID placeObject();

	//  call the object's script
	bool invokeScript(scriptCallFrame &);

	//  Handle object script in a standard fashion
	int16 stdActionScript(int               method,
	                      ObjectID        dObj,
	                      ObjectID        enactor,
	                      ObjectID        indirectObj);

	int16 stdActionScript(int               method,
	                      ObjectID        dObj,
	                      ObjectID        enactor,
	                      ObjectID        indirectObj,
	                      int16           value);

	//  generic actions
	//  Use this object
	bool use(ObjectID dObj, ObjectID enactor);
	virtual bool setUseCursor(ObjectID dObj);
	virtual bool useAction(ObjectID dObj, ObjectID enactor);

	//  Use this object on another object
	bool useOn(ObjectID dObj, ObjectID enactor, ObjectID item);
	virtual bool useOnAction(ObjectID dObj, ObjectID enactor, ObjectID item);

	//  Use this object on a tile activity instance
	bool useOn(ObjectID dObj, ObjectID enactor, ActiveItem *item);
	virtual bool useOnAction(ObjectID dObj, ObjectID enactor, ActiveItem *item);

	//  Use the object on a location
	bool useOn(ObjectID dObj, ObjectID enactor, const Location &loc);
	virtual bool useOnAction(ObjectID dObj, ObjectID enactor, const Location &loc);

	//  open this object
	bool open(ObjectID dObj, ObjectID enactor);
	virtual bool canOpen(ObjectID dObj, ObjectID enactor);
	virtual bool openAction(ObjectID dObj, ObjectID enactor);

	//  close this object
	bool close(ObjectID dObj, ObjectID enactor);
	virtual bool closeAction(ObjectID dObj, ObjectID enactor);

	//  take this object
	bool take(ObjectID dObj, ObjectID enactor, int16 num = 1);
	virtual bool takeAction(ObjectID dObj, ObjectID enactor);
	virtual bool takeAction(ObjectID dObj, ObjectID enactor, int16 num = 1);

	//  drop this object
	bool drop(ObjectID dObj, ObjectID enactor, const Location &loc, int16 num = 1);
	virtual bool canDropAt(
	    ObjectID        dObj,
	    ObjectID        enactor,
	    const Location  &loc);
	virtual bool dropAction(
	    ObjectID        dObj,
	    ObjectID        enactor,
	    const Location  &loc,
	    int16           num = 1);

	//  drop this object onto another object and handle the result.
	bool dropOn(ObjectID dObj, ObjectID enactor, ObjectID target, int16 num = 1);
	virtual bool dropOnAction(
	    ObjectID dObj,
	    ObjectID enactor,
	    ObjectID target,
	    int count);

	//  drop this object onto a TAG
	bool dropOn(
	    ObjectID        dObj,
	    ObjectID        enactor,
	    ActiveItem      *target,
	    const Location  &loc,
	    int16           num = 1);
	virtual bool dropOnAction(
	    ObjectID        dObj,
	    ObjectID        enactor,
	    ActiveItem      *target,
	    const Location  &loc,
	    int16           num = 1);

	//  Strike another object with this object
	bool strike(ObjectID dObj, ObjectID enactor, ObjectID item);
	virtual bool strikeAction(
	    ObjectID dObj,
	    ObjectID enactor,
	    ObjectID item);

	//  Damage another object with this object
	bool damage(ObjectID dObj, ObjectID enactor, ObjectID target);
	virtual bool damageAction(
	    ObjectID dObj,
	    ObjectID enactor,
	    ObjectID target);

	//  Eat this object
	bool eat(ObjectID dObj, ObjectID enactor);
	virtual bool eatAction(ObjectID dObj, ObjectID enactor);

	//  Insert this object into another object
	bool insert(ObjectID dObj, ObjectID enactor, ObjectID item);
	virtual bool insertAction(ObjectID dObj, ObjectID enactor, ObjectID item);

	//  Remove this object from another object
	bool remove(ObjectID dObj, ObjectID enactor);
	virtual bool removeAction(ObjectID dObj, ObjectID enactor);

	//  Drop another object onto this one.
	bool acceptDrop(ObjectID dObj, ObjectID enactor, ObjectID droppedObj, int count);
	virtual bool acceptDropAction(
	    ObjectID dObj,
	    ObjectID enactor,
	    ObjectID droppedObj,
	    int count);

	//  Cause damage to this object directly
	bool acceptDamage(
	    ObjectID            dObj,
	    ObjectID            enactor,
	    int8                absDamage,
	    effectDamageTypes   dType = kDamageOther,
	    int8                dice = 0,
	    uint8               sides = 1,
	    int8                perDieMod = 0);
	virtual bool acceptDamageAction(
	    ObjectID            dObj,
	    ObjectID            enactor,
	    int8                absDamage,
	    effectDamageTypes   dType,
	    int8                dice,
	    uint8               sides,
	    int8                perDieMod);

	bool acceptHealing(
	    ObjectID    dObj,
	    ObjectID    enactor,
	    int8        absHealing,
	    int8        dice = 0,
	    uint8       sides = 1,
	    int8        perDieMod = 0);
	virtual bool acceptHealingAction(
	    ObjectID    dObj,
	    ObjectID    enactor,
	    int8        healing);

	//  Accept strike from another object (allows this object to cause
	//  damage to the striking object).
	bool acceptStrike(
	    ObjectID            dObj,
	    ObjectID            enactor,
	    ObjectID            strikingObj,
	    uint8               skillIndex);
	virtual bool acceptStrikeAction(
	    ObjectID            dObj,
	    ObjectID            enactor,
	    ObjectID            strikingObj,
	    uint8               skillIndex);

	//  Unlock or lock this object with a key.
	bool acceptLockToggle(ObjectID dObj, ObjectID enactor, uint8 keyCode);
	virtual bool canToggleLock(
	    ObjectID dObj,
	    ObjectID enactor,
	    uint8 keyCode);
	virtual bool acceptLockToggleAction(ObjectID dObj, ObjectID enactor, uint8 keyCode);

	//  Mix this object with another.
	bool acceptMix(ObjectID dObj, ObjectID enactor, ObjectID mixObj);
	virtual bool acceptMixAction(
	    ObjectID dObj,
	    ObjectID enactor,
	    ObjectID mixObj);

	//  Insert another object into this object.
	bool acceptInsertion(
	    ObjectID dObj,
	    ObjectID enactor,
	    ObjectID item,
	    int16 count);
	virtual bool acceptInsertionAction(
	    ObjectID dObj,
	    ObjectID enactor,
	    ObjectID item,
	    int16 count);

	//  Insert another object into this object at a specified slot
	bool acceptInsertionAt(
	    ObjectID        dObj,
	    ObjectID        enactor,
	    ObjectID        item,
	    const TilePoint &where,
	    int16           num = 1);

	virtual bool acceptInsertionAtAction(
	    ObjectID        dObj,
	    ObjectID        enactor,
	    ObjectID        item,
	    const TilePoint &where,
	    int16           num = 1);

	//  Creates a color translation table for this object
	virtual void getColorTranslation(ColorTable map);

	//  return the sprite data of amount 'count'
	virtual ObjectSpriteInfo getSprite(GameObject *obj, spriteTypes spr, int16 count = -1);

	//  return the address of the sprite when held in hand
	virtual Sprite *getOrientedSprite(GameObject *obj, int16 offset);

	//  Initiate an attack using this type of object
	virtual void initiateAttack(ObjectID attacker, ObjectID target);

	//  Initiate a defense using this type of object
	virtual void initiateDefense(
	    ObjectID defensiveObj,
	    ObjectID defender,
	    ObjectID attacker);

	//  Get a projectile from the missile weapon
	virtual GameObject *getProjectile(ObjectID weapon, ObjectID enactor);

	//  Get a spell from a magic weapon
	virtual GameObject *getSpell(ObjectID obj);

	//  Determine if this type of object can block an attack
	virtual bool canBlock();

	//  Return a mask of bits indicating the directions relative to the
	//  wielders facing in which this object can defend
	virtual uint8 defenseDirMask();

	//  Compute how much damage this defensive object will absorb
	virtual uint8 adjustDamage(uint8 damage);

	//  Return the fight stance approriate to this weapon
	virtual int16 fightStanceAction(ObjectID actor);

	//  Given an object sound effect record, which sound should be made
	//  when this object is damaged
	virtual uint8 getDamageSound(const ObjectSoundFXs &soundFXs);

	//  Do the background processing, if needed, for this object.
	//  This will be called approximately once every 10 seconds
	//  (or whatever the background refresh period is).
	virtual void doBackgroundUpdate(GameObject *obj);

	virtual bool resists(effectResistTypes r) {
		return resistance & (1 << r);
	}
	virtual bool isImmuneTo(effectImmuneTypes r) {
		return immunity & (1 << r);
	}

	virtual bool makeSavingThrow() {
		return false;
	}

	//  Returns true if object in continuous use.
	virtual bool isObjectBeingUsed(GameObject *obj);

	//  Determine if the specified object's 'use' slot is available within
	//  the specified actor
	virtual bool useSlotAvailable(GameObject *obj, Actor *a);

	//  Get the value of the user's skill which applies to this
	//  object
	virtual uint8 getSkillValue(ObjectID enactor);

	//  Cause the user's associated skill to grow
	virtual void applySkillGrowth(ObjectID enactor, uint8 points = 1);

	// this is to determine size of containers
public:
	virtual uint16 getViewableRows() {
		return kViewableRows;
	}
	virtual uint16 getViewableCols() {
		return kViewableCols;
	}
	virtual uint16 getMaxRows() {
		return kMaxRows;
	}
	virtual uint16 getMaxCols() {
		return kMaxCols;
	}

	// this returns the type of charge an item can have
	int16 getChargeType() {
		return chargeType;
	}

	virtual bool canFitBulkwise(GameObject *container, GameObject *obj);
	virtual bool canFitMasswise(GameObject *container, GameObject *obj);

	virtual uint16 massCapacity(GameObject *container);
	virtual uint16 bulkCapacity(GameObject *container);
};

/* ======================================================================== *
   InventoryProto:  base class for all tangible object prototypes
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, InventoryProto

class InventoryProto : public ProtoObj {
public:
	InventoryProto(ResourceObjectPrototype &proto) : ProtoObj(proto) {}
	virtual ~InventoryProto() {}

	virtual uint16 containmentSet();

	virtual bool takeAction(ObjectID dObj, ObjectID enactor, int16 num = 1);

	virtual bool canDropAt(
	    ObjectID        dObj,
	    ObjectID        enactor,
	    const Location  &loc);
	virtual bool dropAction(
	    ObjectID        dObj,
	    ObjectID        enactor,
	    const Location  &loc,
	    int16           num = 1);

	virtual bool dropOnAction(
	    ObjectID        dObj,
	    ObjectID        enactor,
	    ActiveItem      *target,
	    const Location  &loc,
	    int16           num = 1);

	virtual bool acceptDropAction(
	    ObjectID dObj,
	    ObjectID enactor,
	    ObjectID droppedObj,
	    int count);

	virtual bool acceptStrikeAction(
	    ObjectID            dObj,
	    ObjectID            enactor,
	    ObjectID            strikingObj,
	    uint8               skillIndex);
};

/* ======================================================================== *
   PhysicalContainerProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, InventoryProto, PhysicalContainerProto

//  Prototype class for physical object container
class PhysicalContainerProto : public InventoryProto {
private:
	enum {
		kViewableRows    = 4,
		kViewableCols    = 4,
		kMaxRows         = 8,
		kMaxCols         = 4
	};

public:
	PhysicalContainerProto(ResourceObjectPrototype &proto) : InventoryProto(proto) {}
	virtual ~PhysicalContainerProto() {}

	virtual uint16 containmentSet();

	virtual bool canContain(ObjectID dObj, ObjectID item);
	virtual bool canContainAt(
	    ObjectID        dObj,
	    ObjectID        item,
	    const TilePoint &where);

	//  Call open() if closed, or call close() if open.
	virtual bool useAction(ObjectID dObj, ObjectID enactor);

	//  Open this object
	virtual bool canOpen(ObjectID dObj, ObjectID enactor);
	virtual bool openAction(ObjectID dObj, ObjectID enactor);

	//  Close this object
	virtual bool closeAction(ObjectID dObj, ObjectID enactor);

	//  Unlock or lock object if keyCode == lockType
	virtual bool canToggleLock(
	    ObjectID dObj,
	    ObjectID enactor,
	    uint8 keyCode);
	virtual bool acceptLockToggleAction(ObjectID dObj, ObjectID enactor, uint8 keyCode);

	//  Insert another object into this object
	bool acceptInsertionAction(
	    ObjectID dObj,
	    ObjectID enactor,
	    ObjectID item,
	    int16 num);

	//  Insert another object into this object at the specified slot
	virtual bool acceptInsertionAtAction(
	    ObjectID        dObj,
	    ObjectID        enactor,
	    ObjectID        item,
	    const TilePoint &where,
	    int16           num = 1);

public:
	virtual uint16 getViewableRows() {
		return kViewableRows;
	}
	virtual uint16 getViewableCols() {
		return kViewableCols;
	}
	virtual uint16 getMaxRows() {
		return kMaxRows;
	}
	virtual uint16 getMaxCols() {
		return kMaxCols;
	}

	virtual bool canFitBulkwise(GameObject *container, GameObject *obj);
	virtual bool canFitMasswise(GameObject *container, GameObject *obj);

	virtual uint16 massCapacity(GameObject *container);
	virtual uint16 bulkCapacity(GameObject *container);
};

/* ======================================================================== *
   KeyProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, InventoryProto, KeyProto

//  Prototype class for key objects
class KeyProto : public InventoryProto {
public:
	KeyProto(ResourceObjectPrototype &proto) : InventoryProto(proto) {}
	virtual ~KeyProto() {}

	//  Set up targeting cursor
	virtual bool setUseCursor(ObjectID dObj);

	//  Use key on lockable container
	virtual bool useOnAction(ObjectID dObj, ObjectID enactor, ObjectID withObj);

	//  Use key on active terrain
	virtual bool useOnAction(ObjectID dObj, ObjectID enactor, ActiveItem *withTAI);
};

/* ======================================================================== *
   BottleProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, InventoryProto, BottleProto

class BottleProto : public InventoryProto {
public:
	BottleProto(ResourceObjectPrototype &proto) : InventoryProto(proto) {}
	virtual ~BottleProto() {}

	virtual uint16 containmentSet();

	// Drink From Bottle
	virtual bool useAction(ObjectID dObj, ObjectID enactor);

};

/* ======================================================================== *
   FoodProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, InventoryProto, FoodProto

class FoodProto : public InventoryProto {
public:
	FoodProto(ResourceObjectPrototype &proto) : InventoryProto(proto) {}
	virtual ~FoodProto() {}

	virtual uint16 containmentSet();

	// Eat it
	virtual bool useAction(ObjectID dObj, ObjectID enactor);

};

/* ======================================================================== *
   WearableProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, InventoryProto, WearbleProto

class WearableProto : public InventoryProto {
public:
	WearableProto(ResourceObjectPrototype &proto) : InventoryProto(proto) {}
	virtual ~WearableProto() {}

	virtual uint16 containmentSet();
};

/* ======================================================================== *
   WeaponProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, InventoryProto, WeaponProto

class WeaponProto : public InventoryProto {

protected:
	enum {
		kInRangeRatingBonus = 4
	};

public:
	WeaponProto(ResourceObjectPrototype &proto) : InventoryProto(proto) {}
	virtual ~WeaponProto() {}

	virtual uint16 containmentSet();

	//  return the address of the sprite when held in hand
	virtual Sprite *getOrientedSprite(GameObject *obj, int16 offset);
	weaponID getWeaponID();

	//  Returns true if object in continuous use.
	bool isObjectBeingUsed(GameObject *obj);

	//  Rate this weapon's goodness for a specified attack situation
	virtual uint8 weaponRating(
	    ObjectID weaponID,
	    ObjectID wielderID,
	    ObjectID targetID) = 0;
};

/* ======================================================================== *
   MeleeWeaponProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, InventoryProto, WeaponProto, MeleeWeaponProto

class MeleeWeaponProto : public WeaponProto {
public:
	MeleeWeaponProto(ResourceObjectPrototype &proto) : WeaponProto(proto) {}
	virtual ~MeleeWeaponProto() {}

	virtual bool useAction(ObjectID dObj, ObjectID enactor);
	virtual bool useOnAction(ObjectID dObj, ObjectID enactor, ObjectID item);
	virtual bool strikeAction(
	    ObjectID dObj,
	    ObjectID enactor,
	    ObjectID item);
	virtual bool damageAction(
	    ObjectID dObj,
	    ObjectID enactor,
	    ObjectID target);
	virtual bool acceptDamageAction(
	    ObjectID            dObj,
	    ObjectID            enactor,
	    int8                absDamage,
	    effectDamageTypes   dType,
	    int8                dice,
	    uint8               sides,
	    int8                perDieMod);

	//  Determine if this type of weapon must be wielded with two hands
	//  for the specified actor
	virtual bool isTwoHanded(ObjectID actor);

	//  Initiate a melee weapon attack motion
	virtual void initiateAttack(ObjectID attacker, ObjectID target);
	//  Initiate a melee weapon parry motion
	virtual void initiateDefense(
	    ObjectID defensiveObj,
	    ObjectID defender,
	    ObjectID attacker);
	//  Melee weapons can block attacks
	virtual bool canBlock();
	//  Return a mask of bits indicating the directions relative to the
	//  wielders facing in which this object can defend
	virtual uint8 defenseDirMask();

	//  Determine if the specified object's 'use' slot is available within
	//  the specified actor
	virtual bool useSlotAvailable(GameObject *obj, Actor *a);

	//  Rate this weapon's goodness for a specified attack situation
	virtual uint8 weaponRating(
	    ObjectID weaponID,
	    ObjectID wielderID,
	    ObjectID targetID);

	//  Return the fight stance approriate to this weapon
	virtual int16 fightStanceAction(ObjectID actor);

	//  Given an object sound effect record, which sound should be made
	//  when this object is damaged
	virtual uint8 getDamageSound(const ObjectSoundFXs &soundFXs);
};

/* ======================================================================== *
   BludgeoningWeaponProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, InventoryProto, WeaponProto, MeleeWeaponProto,
//		BludgeoningWeaponProto

class BludgeoningWeaponProto : public MeleeWeaponProto {
public:
	BludgeoningWeaponProto(ResourceObjectPrototype &proto) : MeleeWeaponProto(proto) {}
	virtual ~BludgeoningWeaponProto() {}

	//  Get the value of the wielder's skill which applies to this
	//  weapon
	virtual uint8 getSkillValue(ObjectID enactor);

	//  Cause the user's associated skill to grow
	virtual void applySkillGrowth(ObjectID enactor, uint8 points = 1);
};

/* ======================================================================== *
   SlashingWeaponProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, InventoryProto, WeaponProto, MeleeWeaponProto,
//		SlashingWeaponProto

class SlashingWeaponProto : public MeleeWeaponProto {
public:
	SlashingWeaponProto(ResourceObjectPrototype &proto) : MeleeWeaponProto(proto) {}
	virtual ~SlashingWeaponProto() {}

	//  Get the value of the wielder's skill which applies to this
	//  weapon
	virtual uint8 getSkillValue(ObjectID enactor);

	//  Cause the user's associated skill to grow
	virtual void applySkillGrowth(ObjectID enactor, uint8 points = 1);
};

/* ======================================================================== *
   BowProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, InventoryProto, WeaponProto, BowProto

class BowProto : public WeaponProto {
public:
	BowProto(ResourceObjectPrototype &proto) : WeaponProto(proto) {}
	virtual ~BowProto() {}

	virtual bool useAction(ObjectID dObj, ObjectID enactor);

	//  Bows are two handed
	virtual bool isTwoHanded(ObjectID actor);

	//  Initiate a bow firing motion
	virtual void initiateAttack(ObjectID attacker, ObjectID target);

	//  Grab and arrow from the actor's inventory
	virtual GameObject *getProjectile(ObjectID weapon, ObjectID enactor);

	//  Determine if the specified object's 'use' slot is available within
	//  the specified actor
	virtual bool useSlotAvailable(GameObject *obj, Actor *a);

	//  Rate this weapon's goodness for a specified attack situation
	virtual uint8 weaponRating(
	    ObjectID weaponID,
	    ObjectID wielderID,
	    ObjectID targetID);

	//  Return the fight stance approriate to this weapon
	virtual int16 fightStanceAction(ObjectID actor);
};

/* ======================================================================== *
   WeaponWandProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, InventoryProto, WeaponProto, WeaponWandProto

class WeaponWandProto : public WeaponProto {
public:
	WeaponWandProto(ResourceObjectPrototype &proto) : WeaponProto(proto) {}
	virtual ~WeaponWandProto() {}

	virtual bool useAction(ObjectID dObj, ObjectID enactor);

	//  Wands are two handed
	virtual bool isTwoHanded(ObjectID actor);

	//  Initiate a bow firing motion
	virtual void initiateAttack(ObjectID attacker, ObjectID target);

	//  Determine if the specified object's 'use' slot is available within
	//  the specified actor
	virtual bool useSlotAvailable(GameObject *obj, Actor *a);

	//  Rate this weapon's goodness for a specified attack situation
	virtual uint8 weaponRating(
	    ObjectID weaponID,
	    ObjectID wielderID,
	    ObjectID targetID);

	//  Return the fight stance approriate to this weapon
	virtual int16 fightStanceAction(ObjectID actor);
};

/* ======================================================================== *
   ProjectileProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, InventoryProto, WeaponProto, ProjectileProto

class ProjectileProto : public WeaponProto {
public:
	ProjectileProto(ResourceObjectPrototype &proto) : WeaponProto(proto) {}
	virtual ~ProjectileProto() {}

	//  return the address of the sprite when held in hand
	virtual Sprite *getOrientedSprite(GameObject *obj, int16 offset);

	//  Returns true if object in continuous use.
	bool isObjectBeingUsed(GameObject *obj);

	//  Projectiles are missiles
	virtual bool isMissile();

	//  Rate this weapon's goodness for a specified attack situation
	virtual uint8 weaponRating(
	    ObjectID weaponID,
	    ObjectID wielderID,
	    ObjectID targetID);
};

/* ======================================================================== *
   ArrowProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, InventoryProto, WeaponProto, ProjectileProto, ArrowProto

class ArrowProto : public ProjectileProto {
public:
	ArrowProto(ResourceObjectPrototype &proto) : ProjectileProto(proto) {}
	virtual ~ArrowProto() {}

	virtual bool useOnAction(ObjectID dObj, ObjectID enactor, ObjectID item);
	virtual bool strikeAction(
	    ObjectID dObj,
	    ObjectID enactor,
	    ObjectID item);
	virtual bool damageAction(
	    ObjectID dObj,
	    ObjectID enactor,
	    ObjectID target);

	//  Cause the user's associated skill to grow
	virtual void applySkillGrowth(ObjectID enactor, uint8 points = 1);

};

/* ======================================================================== *
   ArmorProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, InventoryProto, ArmorProto

class ArmorProto : public InventoryProto {
public:
	ArmorProto(ResourceObjectPrototype &proto) : InventoryProto(proto) {}
	virtual ~ArmorProto() {}

	virtual uint16 containmentSet();

	virtual bool useAction(ObjectID dObj, ObjectID enactor);

	//  Compute how much damage this defensive object will absorb
	virtual uint8 adjustDamage(uint8 damage);

	//  Returns true if object in continuous use.
	bool isObjectBeingUsed(GameObject *obj);

	//  Determine if the specified object's 'use' slot is available within
	//  the specified actor
	virtual bool useSlotAvailable(GameObject *obj, Actor *a);
};

/* ======================================================================== *
   ShieldProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, InventoryProto, ShieldProto

class ShieldProto : public InventoryProto {
public:
	ShieldProto(ResourceObjectPrototype &proto) : InventoryProto(proto) {}
	virtual ~ShieldProto() {}

	virtual uint16 containmentSet();

	virtual bool useAction(ObjectID dObj, ObjectID enactor);

	virtual bool acceptDamageAction(
	    ObjectID            dObj,
	    ObjectID            enactor,
	    int8                absDamage,
	    effectDamageTypes   dType,
	    int8                dice,
	    uint8               sides,
	    int8                perDieMod);

	//  return the address of the sprite when held in hand
	virtual Sprite *getOrientedSprite(GameObject *obj, int16 offset);

	virtual void initiateDefense(
	    ObjectID defensiveObj,
	    ObjectID defender,
	    ObjectID attacker);
	virtual bool canBlock();
	//  Return a mask of bits indicating the directions relative to the
	//  wielders facing in which this object can defend
	virtual uint8 defenseDirMask();

	//  Returns true if object in continuous use.
	bool isObjectBeingUsed(GameObject *obj);

	//  Determine if the specified object's 'use' slot is available within
	//  the specified actor
	virtual bool useSlotAvailable(GameObject *obj, Actor *a);

	//  Get the value of the user's skill which applies to this
	//  object
	virtual uint8 getSkillValue(ObjectID enactor);

	//  Cause the user's associated skill to grow
	virtual void applySkillGrowth(ObjectID enactor, uint8 points = 1);

	//  Given an object sound effect record, which sound should be made
	//  when this object is damaged
	virtual uint8 getDamageSound(const ObjectSoundFXs &soundFXs);
};

/* ======================================================================== *
   ToolProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, InventoryProto, ToolProto

class ToolProto : public InventoryProto {
public:
	ToolProto(ResourceObjectPrototype &proto) : InventoryProto(proto) {}
	virtual ~ToolProto() {}

	//  Set up targeting cursor
	virtual bool setUseCursor(ObjectID dObj);

	//  Use tool on object
	virtual bool useOnAction(ObjectID dObj, ObjectID enactor, ObjectID withObj);
};

/* ======================================================================== *
   DocumentProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, InventoryProto, DocumentProto

class DocumentProto : public InventoryProto {
public:
	DocumentProto(ResourceObjectPrototype &proto) : InventoryProto(proto) {}
	virtual ~DocumentProto() {}

	virtual uint16 containmentSet();

//BookDoc
//ScrollDoc

//	virtual bool use( ObjectID dObj, ObjectID enactor );
//	Close Floating Window Used For Both Book And Scroll
//	virtual bool close( ObjectID dObj, ObjectID enactor );

};

/* ======================================================================== *
   BookProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, InventoryProto, DocumentProto, BookProto

class BookProto : public DocumentProto {
public:
	BookProto(ResourceObjectPrototype &proto) : DocumentProto(proto) {}
	virtual ~BookProto() {}
	//Read It
	virtual bool useAction(ObjectID dObj, ObjectID enactor);

};

/* ======================================================================== *
   ScrollProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, InventoryProto, DocumentProto, ScrollProto

class ScrollProto : public DocumentProto {
public:
	ScrollProto(ResourceObjectPrototype &proto) : DocumentProto(proto) {}
	virtual ~ScrollProto() {}

	//Read It
	virtual bool useAction(ObjectID dObj, ObjectID enactor);

};

/* ======================================================================== *
   AutoMapProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, InventoryProto, AutoMapProto

class AutoMapProto : public InventoryProto {
public:
	AutoMapProto(ResourceObjectPrototype &proto) : InventoryProto(proto) {}
	virtual ~AutoMapProto() {}

	//Shows Auto Map Display
	virtual bool openAction(ObjectID dObj, ObjectID enactor);

};

/* ======================================================================== *
   IntagibleObjProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, IntangibleObjProto

class IntangibleObjProto : public ProtoObj {
public:
	IntangibleObjProto(ResourceObjectPrototype &proto) : ProtoObj(proto) {}
	virtual ~IntangibleObjProto() {}

	virtual bool useAction(ObjectID dObj, ObjectID enactor);

	virtual bool takeAction(ObjectID dObj, ObjectID enactor, int16);

	virtual bool canDropAt(
	    ObjectID        dObj,
	    ObjectID        enactor,
	    const Location  &loc);
	virtual bool dropAction(
	    ObjectID        dObj,
	    ObjectID        enactor,
	    const Location  &loc,
	    int16);

	virtual bool acceptDropAction(
	    ObjectID dObj,
	    ObjectID enactor,
	    ObjectID droppedObj,
	    int count);

	virtual uint16 containmentSet();
	virtual ObjectID placeObject();

	//  Creates a color translation table for this object
	virtual void getColorTranslation(ColorTable map);

	//  return the sprite data
	virtual ObjectSpriteInfo getSprite(GameObject *obj, spriteTypes spr, int16);
};

/* ======================================================================== *
   IdeaProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, IntangibleObjProto, IdeaProto

class IdeaProto : public IntangibleObjProto {
public:
	IdeaProto(ResourceObjectPrototype &proto) : IntangibleObjProto(proto) {}
	virtual ~IdeaProto() {}

	//Talk To A Person
	uint16 containmentSet();

};

/* ======================================================================== *
   MemoryProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, IntangibleObjProto, MemoryProto

class MemoryProto : public IntangibleObjProto {
public:
	MemoryProto(ResourceObjectPrototype &proto) : IntangibleObjProto(proto) {}
	virtual ~MemoryProto() {}

	//Get Info On Person Your Talking To
	uint16 containmentSet();

};

/* ======================================================================== *
   PsychProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, IntangibleObjProto, PsychProto

class PsychProto : public IntangibleObjProto {
public:
	PsychProto(ResourceObjectPrototype &proto) : IntangibleObjProto(proto) {}
	virtual ~PsychProto() {}

	//Get Explanation Of Icon
	uint16 containmentSet();

};

/* ======================================================================== *
   SkillProto
 * ======================================================================== */

//typedef uint8 SpellID;

//	hierarchy:
//		ProtoObj, IntagibleObjProto, SkillProto

class SkillProto : public IntangibleObjProto {
public:
	SkillProto(ResourceObjectPrototype &proto) : IntangibleObjProto(proto) {}
	virtual ~SkillProto() {}

	//Perform A Skill or Cast a spell
	virtual bool useAction(ObjectID dObj, ObjectID enactor);
	virtual bool useOnAction(ObjectID dObj, ObjectID enactor, ObjectID withObj);
	virtual bool useOnAction(ObjectID dObj, ObjectID enactor, ActiveItem *item);
	virtual bool useOnAction(ObjectID dObj, ObjectID enactor, const Location &loc);

	virtual bool canDropAt(
	    ObjectID        dObj,
	    ObjectID        enactor,
	    const Location  &loc);
	virtual bool dropAction(
	    ObjectID        dObj,
	    ObjectID        enactor,
	    const Location  &loc,
	    int16           num = 1);
	virtual bool dropOnAction(
	    ObjectID dObj,
	    ObjectID enactor,
	    ObjectID target,
	    int count);
	virtual bool dropOnAction(
	    ObjectID        dObj,
	    ObjectID        enactor,
	    ActiveItem      *target,
	    const Location  &loc,
	    int16           num = 1);

	virtual bool implementAction(SpellID dObj, ObjectID enactor, ObjectID withObj);
	virtual bool implementAction(SpellID dObj, ObjectID enactor, ActiveItem *item);
	virtual bool implementAction(SpellID dObj, ObjectID enactor, Location &loc);
	uint16 containmentSet();
	SpellID getSpellID() {
		return (SpellID) lockType;
	}

};

/* ======================================================================== *
   IntangibleContainerProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, IntangibleContainerProto

class IntangibleContainerProto : public ProtoObj {
public:
	IntangibleContainerProto(ResourceObjectPrototype &proto) : ProtoObj(proto) {}
	virtual ~IntangibleContainerProto() {}

	virtual bool canContain(ObjectID dObj, ObjectID item);
	virtual bool useAction(ObjectID dObj, ObjectID enactor);
	virtual bool canOpen(ObjectID dObj, ObjectID enactor);
	virtual bool openAction(ObjectID dObj, ObjectID enactor);
	virtual bool closeAction(ObjectID dObj, ObjectID enactor);
//	virtual  bool acceptLockToggle( ObjectID dObj, ObjectID enactor, uint8 keyCode );

//	virtual  ContainerWindow *makeWindow( GameObject *Obj );
	virtual uint16 containmentSet();
};

/* ======================================================================== *
   IdeaContainerProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, IntangibleContainerProto, IdeaContainerProto

class IdeaContainerProto : public IntangibleContainerProto {
public:
	IdeaContainerProto(ResourceObjectPrototype &proto) : IntangibleContainerProto(proto) {}
	virtual ~IdeaContainerProto() {}

	//Holding Idea Objects
//	bool use( ObjectID dObj, ObjectID enactor );

};

/* ======================================================================== *
   MemoryContainerProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, IntangibleContainerProto, MemoryContainerProto

class MemoryContainerProto : public IntangibleContainerProto {
public:
	MemoryContainerProto(ResourceObjectPrototype &proto) : IntangibleContainerProto(proto) {}
	virtual ~MemoryContainerProto() {}

	//Holding Memories Of People You Met
//	bool use( ObjectID dObj, ObjectID enactor );

};

/* ======================================================================== *
   PhychContainerProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, IntangibleContainerProto, PsychContainerProto

class PsychContainerProto : public IntangibleContainerProto {
public:
	PsychContainerProto(ResourceObjectPrototype &proto) : IntangibleContainerProto(proto) {}
	virtual ~PsychContainerProto() {}

	//Holding Psychological Objects
//	bool use( ObjectID dObj, ObjectID enactor );

};

/* ======================================================================== *
   SkillContainerProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, IntangibleContainerProto, SkillContainerProto

class SkillContainerProto : public IntangibleContainerProto {
public:
	SkillContainerProto(ResourceObjectPrototype &proto) : IntangibleContainerProto(proto) {}
	virtual ~SkillContainerProto() {}

	//Holding Skills And Spells
//	bool use( ObjectID dObj, ObjectID enactor );

};

/* ======================================================================== *
   MindContainerProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, IntangibleContainerProto, MindContainerProto

class MindContainerProto : public IntangibleContainerProto {
public:
	MindContainerProto(ResourceObjectPrototype &proto) : IntangibleContainerProto(proto) {}
	virtual ~MindContainerProto() {}

	//Contains Skill Psych Memory And Idea Containers
//	virtual bool use( ObjectID dObj, ObjectID enactor );

};

/* ======================================================================== *
   EnchantmentProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, EnchantmentProto

class EnchantmentProto : public ProtoObj {
public:
	EnchantmentProto(ResourceObjectPrototype &proto) : ProtoObj(proto) {}
	virtual ~EnchantmentProto() {}

	//  Do the background processing, if needed, for this object.
	void doBackgroundUpdate(GameObject *obj);

	virtual uint16 containmentSet();
};

/* ======================================================================== *
   GeneratorProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, GeneratorProto

class GeneratorProto : public ProtoObj {
public:
	GeneratorProto(ResourceObjectPrototype &proto) : ProtoObj(proto) {}
	virtual ~GeneratorProto() {}

	//Base class for monster, encounter, and mission generators

	virtual uint16 containmentSet();
};

/* ======================================================================== *
   MonsterGeneratorProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, GeneratorProto, MonsterGeneratorProto

class MonsterGeneratorProto : public GeneratorProto {
public:
	MonsterGeneratorProto(ResourceObjectPrototype &proto) : GeneratorProto(proto) {}
	virtual ~MonsterGeneratorProto() {}

	//Monster generators
	//  REM: We don't want to generate monsters as a background activity, since
	//  we may want more rapid generation that once every 10 seconds, and we only
	//  want to do it while active anyway.
};

/* ======================================================================== *
   EncounterGeneratorProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, GeneratorProto, EncounterGeneratorProto

class EncounterGeneratorProto : public GeneratorProto {
public:
	EncounterGeneratorProto(ResourceObjectPrototype &proto) : GeneratorProto(proto) {}
	virtual ~EncounterGeneratorProto() {}

	//Encounter generator

	//  Generate an encounter at approx. 10-second intervals
	void doBackgroundUpdate(GameObject *obj);
};

/* ======================================================================== *
   MissionGeneratorProto
 * ======================================================================== */

//	hierarchy:
//		ProtoObj, GeneratorProto, MissionGeneratorProto

class MissionGeneratorProto : public GeneratorProto {
public:
	MissionGeneratorProto(ResourceObjectPrototype &proto) : GeneratorProto(proto) {}
	virtual ~MissionGeneratorProto() {}

	//  Check every 10 seconds to see if we want to generate a mission.
	void doBackgroundUpdate(GameObject *obj);
};

/*  Subclasses of "ProtoObj" which haven't been defined yet

        InventoryObjectPrototype            // can be dropped on ground
            ContainerPrototype              // holds a list of items
            BottlePrototype                 // holds 1 liquid
            FoodPrototype                   // edible
            WearablePrototype               // armor and jewelry
            WeaponPrototype                 // does damage efficiently
            DocumentPrototype               // expands to document window
        IntangableObjectPrototype           // Ideas and Magic
            ConceptObjectPrototype          // Basic Ideas (Food, Friend...)
            MemoryObjectPrototype           // Memories of game events
            PsychObjectPrototype            // I am ... (Brave, Humble...)
            SpellObjectPrototype            // Spells to cast
            EnchantmentObjectPrototype      // Enchants object that holds it
        IntangableContainerPrototype        // Containers for Ideas and Magic
            ConceptContainerPrototype       // Containers for Basic Ideas (Food, Friend...)
            MemoryContainerPrototype        // Containers for Memories of game events
            PsychContainerPrototype         // Containers for I am ... (Brave, Humble...)
            SpellContainerPrototype         // Containers for Spells to cast
//          EnchantmentContainerPrototype   // Enchants object that holds it
        ProjectilePrototype                 // a missile in flight
**      ActorPrototype
*/

} // end of namespace Saga2

#endif
