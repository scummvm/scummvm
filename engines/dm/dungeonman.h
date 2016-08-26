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
*/

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#ifndef DUNGEONMAN_H
#define DUNGEONMAN_H

#include "dm.h"
#include "gfx.h"


namespace DM {

/* Object info */
#define kObjectInfoIndexFirstScroll 0 // @ C000_OBJECT_INFO_INDEX_FIRST_SCROLL
#define kObjectInfoIndexFirstContainer 1 // @ C001_OBJECT_INFO_INDEX_FIRST_CONTAINER
#define kObjectInfoIndexFirstPotion 2 // @ C002_OBJECT_INFO_INDEX_FIRST_POTION
#define kObjectInfoIndexFirstWeapon 23 // @ C023_OBJECT_INFO_INDEX_FIRST_WEAPON
#define kObjectInfoIndexFirstArmour 69 // @ C069_OBJECT_INFO_INDEX_FIRST_ARMOUR
#define kObjectInfoIndexFirstJunk 127 // @ C127_OBJECT_INFO_INDEX_FIRST_JUNK

#define kMapXNotOnASquare -1 // @ CM1_MAPX_NOT_ON_A_SQUARE

enum ElementType {
	kElementTypeChampion = -2, // @ CM2_ELEMENT_CHAMPION /* Values -2 and -1 are only used as projectile impact types */
	kElementTypeCreature = -1, // @ CM1_ELEMENT_CREATURE
	kElementTypeWall = 0, // @ C00_ELEMENT_WALL /* Values 0-6 are used as square types and projectile impact types. Values 0-2 and 5-6 are also used for square aspect */
	kElementTypeCorridor = 1, // @ C01_ELEMENT_CORRIDOR
	kElementTypePit = 2, // @ C02_ELEMENT_PIT
	kElementTypeStairs = 3, // @ C03_ELEMENT_STAIRS
	kElementTypeDoor = 4, // @ C04_ELEMENT_DOOR
	kElementTypeTeleporter = 5, // @ C05_ELEMENT_TELEPORTER
	kElementTypeFakeWall = 6, // @ C06_ELEMENT_FAKEWALL
	kElementTypeDoorSide = 16, // @ C16_ELEMENT_DOOR_SIDE /* Values 16-19 are only used for square aspect */
	kElementTypeDoorFront = 17, // @ C17_ELEMENT_DOOR_FRONT
	kElementTypeStairsSide = 18, // @ C18_ELEMENT_STAIRS_SIDE
	kElementTypeStaisFront = 19  // @ C19_ELEMENT_STAIRS_FRONT	
};


enum ObjectAllowedSlot {
	kObjectAllowedSlotMouth = 0x0001, // @ MASK0x0001_MOUTH
	kObjectAllowedSlotHead = 0x0002, // @ MASK0x0002_HEAD
	kObjectAllowedSlotNeck = 0x0004, // @ MASK0x0004_NECK
	kObjectAllowedSlotTorso = 0x0008, // @ MASK0x0008_TORSO
	kObjectAllowedSlotLegs = 0x0010, // @ MASK0x0010_LEGS
	kObjectAllowedSlotFeet = 0x0020, // @ MASK0x0020_FEET
	kObjectAllowedSlotQuiverLine_1 = 0x0040, // @ MASK0x0040_QUIVER_LINE1
	kObjectAllowedSlotQuiverLine_2 = 0x0080, // @ MASK0x0080_QUIVER_LINE2
	kObjectAllowedSlotPouchPassAndThroughDoors = 0x0100, // @ MASK0x0100_POUCH_PASS_AND_THROUGH_DOORS
	kObjectAllowedSlotHands = 0x0200, // @ MASK0x0200_HANDS
	kObjectAllowedSlotContainer = 0x0400 // @ MASK0x0400_CONTAINER
};

class ObjectInfo {
public:
	int16 _type;
	uint16 _objectAspectIndex;
	uint16 _actionSetIndex;
private:
	uint16 _allowedSlots;
public:
	ObjectInfo(int16 type, uint16 objectAspectIndex, uint16 actionSetIndex, uint16 allowedSlots)
		: _type(type), _objectAspectIndex(objectAspectIndex), _actionSetIndex(actionSetIndex), _allowedSlots(allowedSlots) {}
	bool getAllowedSlot(ObjectAllowedSlot slot) { return _allowedSlots & slot; }
	uint16 getAllowedSlots() { return _allowedSlots; }
	void setAllowedSlot(ObjectAllowedSlot slot, bool val) {
		if (val) {
			_allowedSlots |= slot;
		} else {
			_allowedSlots &= ~slot;
		}
	}
}; // @ OBJECT_INFO

extern ObjectInfo gObjectInfo[180];

enum ArmourAttribute {
	kArmourAttributeIsAShield = 0x0080, // @ MASK0x0080_IS_A_SHIELD
	kArmourAttributeSharpDefense = 0x0007 // @ MASK0x0007_SHARP_DEFENSE
};

class ArmourInfo {
public:
	uint16 _weight;
	uint16 _defense;
private:
	uint16 _attributes;
public:
	ArmourInfo(uint16 weight, uint16 defense, uint16 attributes)
		:_weight(weight), _defense(defense), _attributes(attributes) {}

	uint16 getAttribute(ArmourAttribute attribute) { return _attributes & attribute; }
	void setAttribute(ArmourAttribute attribute) { _attributes |= attribute; }
}; // @ ARMOUR_INFO

extern ArmourInfo gArmourInfo[58];

/* Class 0: SWING weapons */
#define kWeaponClassSwingWeapon			0 // @ C000_CLASS_SWING_WEAPON 
/* Class 1 to 15: THROW weapons */
#define kWeaponClassDaggerAndAxes		2 // @ C002_CLASS_DAGGER_AND_AXES 
#define kWeaponClassBowAmmunition		10 // @ C010_CLASS_BOW_AMMUNITION
#define kWeaponClassSlingAmmunition		11 // @ C011_CLASS_SLING_AMMUNITION
#define kWeaponClassPoisinDart			12 // @ C012_CLASS_POISON_DART
/* Class 16 to 111: SHOOT weapons */
#define kWeaponClassFirstBow			16 // @ C016_CLASS_FIRST_BOW 
#define kWeaponClassLastBow				31 // @ C031_CLASS_LAST_BOW
#define kWeaponClassFirstSling			32 // @ C032_CLASS_FIRST_SLING
#define kWeaponClassLastSling			47 // @ C047_CLASS_LAST_SLING
/* Class 112 to 255: Magic and special weapons */
#define kWeaponClassFirstMagicWeapon	112 // @ C112_CLASS_FIRST_MAGIC_WEAPON

class WeaponInfo {

public:
	uint16 _weight;
	uint16 _class;
	uint16 _strength;
	uint16 _kineticEnergy;
private:
	uint16 _attributes; /* Bits 15-13 Unreferenced */
public:
	WeaponInfo(uint16 weight, uint16 wClass, uint16 strength, uint16 kineticEnergy, uint16 attributes)
		: _weight(weight), _class(wClass), _strength(strength), _kineticEnergy(kineticEnergy), _attributes(attributes) {}

	uint16 getShootAttack() { return _attributes & 0xFF; } // @ M65_SHOOT_ATTACK
	uint16 getProjectileAspectOrdinal() { return (_attributes >> 8) & 0x1F; } // @ M66_PROJECTILE_ASPECT_ORDINAL
}; // @ WEAPON_INFO

extern WeaponInfo gWeaponInfo[46];

enum TextType {
	/* Used for text on walls */
	kTextTypeInscription = 0, // @ C0_TEXT_TYPE_INSCRIPTION 
	/* Used for messages displayed when the party walks on a square */
	kTextTypeMessage = 1, // @ C1_TEXT_TYPE_MESSAGE 
	/* Used for text on scrolls and champion information */
	kTextTypeScroll = 2 // @ C2_TEXT_TYPE_SCROLL 
};

enum SquareAspectIndice {
	kElemAspect = 0,
	kFirstGroupOrObjectAspect = 1,
	kRightWallOrnOrdAspect = 2,
	kFrontWallOrnOrdAspect = 3,
	kLeftWallOrnOrdAspect = 4,
	kPitInvisibleAspect = 2,
	kTeleporterVisibleAspect = 2,
	kStairsUpAspect = 2,
	kDoorStateAspect = 2,
	kDoorThingIndexAspect = 3,
	kFloorOrnOrdAspect = 4,
	kFootprintsAspect = 0x8000 // @ MASK0x8000_FOOTPRINTS          
};


struct CreatureInfo {
	byte _creatureAspectIndex;
	byte _attackSoundOrdinal;
	uint16 _attributes; /* Bits 15-14 Unreferenced */
	uint16 _graphicInfo; /* Bits 11 and 6 Unreferenced */
	byte _movementTicks; /* Value 255 means the creature cannot move */
	byte _attackTicks; /* Minimum ticks between attacks */
	byte _defense;
	byte _baseHealth;
	byte _attack;
	byte _poisonAttack;
	byte _dexterity;
	uint16 _ranges; /* Bits 7-4 Unreferenced */
	uint16 _properties;
	uint16 _resistances; /* Bits 15-12 and 3-0 Unreferenced */
	uint16 _animationTicks; /* Bits 15-12 Unreferenced */
	uint16 _woundProbabilities; /* Contains 4 probabilities to wound a champion's Head (Bits 15-12), Legs (Bits 11-8), Torso (Bits 7-4) and Feet (Bits 3-0) */
	byte _attackType;
}; // @ CREATURE_INFO


extern CreatureInfo gCreatureInfo[kCreatureTypeCount];

class Door {
	Thing _nextThing;
	uint16 _attributes;
public:
	explicit Door(uint16 *rawDat) : _nextThing(rawDat[0]), _attributes(rawDat[1]) {}
	Thing getNextThing() { return _nextThing; }
	bool isMeleeDestructible() { return (_attributes >> 8) & 1; }
	bool isMagicDestructible() { return (_attributes >> 7) & 1; }
	bool hasButton() { return (_attributes >> 6) & 1; }
	bool opensVertically() { return (_attributes >> 5) & 1; }
	byte getOrnOrdinal() { return (_attributes >> 1) & 0xF; }
	byte getType() { return _attributes & 1; }
}; // @ DOOR

enum TeleporterScope {
	kTelepScopeCreatures = 1, // @ MASK0x0001_SCOPE_CREATURES
	kTelepScopeObjOrParty = 2 // @ MASK0x0002_SCOPE_OBJECTS_OR_PARTY
};


class Teleporter {
	Thing _nextThing;
	uint16 _attributes;
	uint16 _destMapIndex;
public:
	explicit Teleporter(uint16 *rawDat) : _nextThing(rawDat[0]), _attributes(rawDat[1]), _destMapIndex(rawDat[2]) {}
	Thing getNextThing() { return _nextThing; }
	bool makesSound() { return (_attributes >> 15) & 1; }
	TeleporterScope getScope() { return (TeleporterScope)((_attributes >> 13) & 1); }
	bool absRotation() { return (_attributes >> 12) & 1; }
	direction getRotationDir() { return (direction)((_attributes >> 10) & 1); }
	byte getDestY() { return (_attributes >> 5) & 0xF; }
	byte getDestX() { return _attributes & 0xF; }
	uint16 getDestMapIndex() { return _destMapIndex >> 8; }
}; // @ TELEPORTER



class TextString {
	Thing _nextThing;
	uint16 _textDataRef;
public:
	explicit TextString(uint16 *rawDat) : _nextThing(rawDat[0]), _textDataRef(rawDat[1]) {}

	Thing getNextThing() { return _nextThing; }
	uint16 getWordOffset() { return _textDataRef >> 3; }
	bool isVisible() { return _textDataRef & 1; }
}; // @ TEXTSTRING

enum SensorActionType {
	kSensorEffNone = -1, // @ CM1_EFFECT_NONE
	kSensorEffSet = 0, // @ C00_EFFECT_SET
	kSensorEffClear = 1, // @ C01_EFFECT_CLEAR
	kSensorEffToggle = 2, // @ C02_EFFECT_TOGGLE
	kSensorEffHold = 3, // @ C03_EFFECT_HOLD
	kSensorEffAddExp = 10 // @ C10_EFFECT_ADD_EXPERIENCE
};

enum SensorType {
	kSensorDisabled = 0, // @ C000_SENSOR_DISABLED    /* Never triggered, may be used for a floor or wall ornament */
	kSensorFloorTheronPartyCreatureObj = 1, // @ C001_SENSOR_FLOOR_THERON_PARTY_CREATURE_OBJECT    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	kSensorFloorTheronPartyCreature = 2, // @ C002_SENSOR_FLOOR_THERON_PARTY_CREATURE    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	kSensorFloorParty = 3, // @ C003_SENSOR_FLOOR_PARTY    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	kSensorFloorObj = 4, // @ C004_SENSOR_FLOOR_OBJECT    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	kSensorFloorPartyOnStairs = 5, // @ C005_SENSOR_FLOOR_PARTY_ON_STAIRS    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	kSensorFloorGroupGenerator = 6, // @ C006_SENSOR_FLOOR_GROUP_GENERATOR    /* Triggered by event F0245_TIMELINE_ProcessEvent5_Square_Corridor */
	kSensorFloorCreature = 7, // @ C007_SENSOR_FLOOR_CREATURE    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	kSensorFloorPartyPossession = 8, // @ C008_SENSOR_FLOOR_PARTY_POSSESSION    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	kSensorFloorVersionChecker = 9, // @ C009_SENSOR_FLOOR_VERSION_CHECKER    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	kSensorWallOrnClick = 1, // @ C001_SENSOR_WALL_ORNAMENT_CLICK    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	kSensorWallOrnClickWithAnyObj = 2, // @ C002_SENSOR_WALL_ORNAMENT_CLICK_WITH_ANY_OBJECT    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	kSensorWallOrnClickWithSpecObj = 3, // @ C003_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	kSensorWallOrnClickWithSpecObjRemoved = 4, // @ C004_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	kSensorWallAndOrGate = 5, // @ C005_SENSOR_WALL_AND_OR_GATE    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	kSensorWallCountdown = 6, // @ C006_SENSOR_WALL_COUNTDOWN    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	kSensorWallSingleProjLauncherNewObj = 7, // @ C007_SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_NEW_OBJECT    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	kSensorWallSingleProjLauncherExplosion = 8, // @ C008_SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_EXPLOSION    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	kSensorWallDoubleProjLauncherNewObj = 9, // @ C009_SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_NEW_OBJECT    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	kSensorWallDoubleProjLauncherExplosion = 10, // @ C010_SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_EXPLOSION    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	kSensorWallOrnClickWithSpecObjRemovedRotateSensors = 11, // @ C011_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED_ROTATE_SENSORS   /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	kSensorWallObjGeneratorRotateSensors = 12, // @ C012_SENSOR_WALL_OBJECT_GENERATOR_ROTATE_SENSORS    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	kSensorWallSingleObjStorageRotateSensors = 13, // @ C013_SENSOR_WALL_SINGLE_OBJECT_STORAGE_ROTATE_SENSORS    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	kSensorWallSingleProjLauncherSquareObj = 14, // @ C014_SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	kSensorWallDoubleProjLauncherSquareObj = 15, // @ C015_SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	kSensorWallObjExchanger = 16, // @ C016_SENSOR_WALL_OBJECT_EXCHANGER    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	kSensorWallOrnClickWithSpecObjRemovedSensor = 17, // @ C017_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED_REMOVE_SENSOR    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	kSensorWallEndGame = 18, // @ C018_SENSOR_WALL_END_GAME    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	kSensorWallChampionPortrait = 127 // @ C127_SENSOR_WALL_CHAMPION_PORTRAIT    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
};

class Sensor {
	Thing _nextThing;
	uint16 _datAndType;
	uint16 _attributes;
	uint16 _action;
public:
	explicit Sensor(uint16 *rawDat) : _nextThing(rawDat[0]), _datAndType(rawDat[1]), _attributes(rawDat[2]), _action(rawDat[3]) {}

	Thing getNextThing() { return _nextThing; }
	void setNextThing(Thing thing) { _nextThing = thing; }
	SensorType getType() { return (SensorType)(_datAndType & 0x7F); } // @ M39_TYPE
	uint16 getData() { return _datAndType >> 7; } // @ M40_DATA
	uint16 getDataMask1() { return (_datAndType >> 7) & 0xF; } // @ M42_MASK1
	uint16 getDataMask2() { return (_datAndType >> 11) & 0xF; } // @ M43_MASK2
	void setData(int16 dat) { _datAndType = (_datAndType & 0x7F) | (dat << 7); } // @ M41_SET_DATA
	void setTypeDisabled() { _datAndType &= 0xFF80; } // @ M44_SET_TYPE_DISABLED
	uint16 getOrnOrdinal() { return _attributes >> 12; }
	bool isLocalAction() { return (_attributes >> 11) & 1; }
	uint16 getDelay() { return (_attributes >> 7) & 0xF; }
	bool hasSound() { return (_attributes >> 6) & 1; }
	bool shouldRevert() { return (_attributes >> 5) & 1; }
	SensorActionType getActionType() { return (SensorActionType)((_attributes >> 3) & 3); }
	bool isSingleUse() { return (_attributes >> 2) & 1; }
	uint16 getRemoteMapY() { return (_action >> 11); }
	uint16 getRemoteMapX() { return (_action >> 6) & 0x1F; }
	direction getRemoteDir() { return (direction)((_action >> 4) & 3); }
	uint16 getLocalAction() { return (_action >> 4); }
	uint16 getEffectA() { return (_attributes >> 3) & 0x3; }
	bool getRevertEffectA() { return (_attributes >> 5) & 0x1; }
	bool getAudibleA() { return (_attributes >> 6) & 0x1; }


	// some macros missing, i got bored
}; // @ SENSOR


enum WeaponType {
	kWeaponTypeTorch = 2, // @ C02_WEAPON_TORCH
	kWeaponTypeDagger = 8, // @ C08_WEAPON_DAGGER
	kWeaponTypeFalchion = 9, // @ C09_WEAPON_FALCHION
	kWeaponTypeSword = 10, // @ C10_WEAPON_SWORD
	kWeaponTypeClub = 23, // @ C23_WEAPON_CLUB
	kWeaponTypeStoneClub = 24, // @ C24_WEAPON_STONE_CLUB
	kWeaponTypeArrow = 27, // @ C27_WEAPON_ARROW
	kWeaponTypeSlayer = 28, // @ C28_WEAPON_SLAYER
	kWeaponTypeRock = 30, // @ C30_WEAPON_ROCK
	kWeaponTypePoisonDart = 31, // @ C31_WEAPON_POISON_DART
	kWeaponTypeThrowingStar = 32 // @ C32_WEAPON_THROWING_STAR
};
class Weapon {
	Thing _nextThing;
	uint16 _desc;
public:
	explicit Weapon(uint16 *rawDat) : _nextThing(rawDat[0]), _desc(rawDat[1]) {}

	WeaponType getType() { return (WeaponType)(_desc & 0x7F); }
	bool isLit() { return (_desc >> 15) & 1; }
	void setLit(bool val) {
		if (val)
			_desc |= (1 << 15);
		else
			_desc &= (~(1 << 15));
	}
	uint16 getChargeCount() { return (_desc >> 10) & 0xF; }
	Thing getNextThing() { return _nextThing; }
	uint16 getCursed() { return (_desc >> 8) & 1; }
	uint16 getPoisoned() { return (_desc >> 9) & 1; }
	uint16 getBroken() { return (_desc >> 14) & 1; }
}; // @ WEAPON

enum ArmourType {
	kArmourTypeWoodenShield = 30, // @ C30_ARMOUR_WOODEN_SHIELD
	kArmourTypeArmet = 38, // @ C38_ARMOUR_ARMET
	kArmourTypeTorsoPlate = 39, // @ C39_ARMOUR_TORSO_PLATE
	kArmourTypeLegPlate = 40, // @ C40_ARMOUR_LEG_PLATE
	kArmourTypeFootPlate = 41 // @ C41_ARMOUR_FOOT_PLATE
};
class Armour {
	Thing _nextThing;
	uint16 _attributes;
public:
	explicit Armour(uint16 *rawDat) : _nextThing(rawDat[0]), _attributes(rawDat[1]) {}

	ArmourType getType() { return (ArmourType)(_attributes & 0x7F); }
	Thing getNextThing() { return _nextThing; }
	uint16 getCursed() { return (_attributes >> 8) & 1; }
	uint16 getBroken() { return (_attributes >> 13) & 1; }
}; // @ ARMOUR

class Scroll {
	Thing _nextThing;
	uint16 _attributes;
public:
	explicit Scroll(uint16 *rawDat) : _nextThing(rawDat[0]), _attributes(rawDat[1]) {}
	void set(Thing next, uint16 attribs) {
		_nextThing = next;
		_attributes = attribs;
	}
	Thing getNextThing() { return _nextThing; }
	uint16 getClosed() { return (_attributes >> 10) & 0x3F; } // ??? dunno why, the original bitfield is 6 bits long
	void setClosed(bool val) {
		if (val)
			_attributes |= (1 << 10);
		else
			_attributes &= (~(0x3F << 10));
	}
	uint16 getTextStringThingIndex() { return _attributes & 0x3FF; }
}; // @ SCROLL

enum PotionType {
	kPotionTypeVen = 3, // @ C03_POTION_VEN_POTION,
	kPotionTypeRos = 6, // @ C06_POTION_ROS_POTION,
	kPotionTypeKu = 7, // @ C07_POTION_KU_POTION,
	kPotionTypeDane = 8, // @ C08_POTION_DANE_POTION,
	kPotionTypeNeta = 9, // @ C09_POTION_NETA_POTION,
	kPotionTypeAntivenin = 10, // @ C10_POTION_ANTIVENIN,
	kPotionTypeMon = 11, // @ C11_POTION_MON_POTION,
	kPotionTypeYa = 12, // @ C12_POTION_YA_POTION,
	kPotionTypeEe = 13, // @ C13_POTION_EE_POTION,
	kPotionTypeVi = 14, // @ C14_POTION_VI_POTION,
	kPotionTypeWaterFlask = 15, // @ C15_POTION_WATER_FLASK,
	kPotionTypeFulBomb = 19, // @ C19_POTION_FUL_BOMB,
	kPotionTypeEmptyFlask = 20 // @ C20_POTION_EMPTY_FLASK,
};
class Potion {
	Thing _nextThing;
	uint16 _attributes;
public:
	explicit Potion(uint16 *rawDat) : _nextThing(rawDat[0]), _attributes(rawDat[1]) {}

	PotionType getType() { return (PotionType)((_attributes >> 8) & 0x7F); }
	void setType(PotionType val) { _attributes = (_attributes & ~(0x7F << 8)) | ((val & 0x7F) << 8); }
	Thing getNextThing() { return _nextThing; }
	uint16 getPower() { return _attributes & 0xFF; }
}; // @ POTION

class Container {
	Thing _nextThing;
	Thing _slot;
	uint16 _type;
public:
	explicit Container(uint16 *rawDat) : _nextThing(rawDat[0]), _slot(rawDat[1]), _type(rawDat[2]) {}

	uint16 getType() { return (_type >> 1) & 0x3; }
	Thing &getSlot() { return _slot; }
	Thing &getNextThing() { return _nextThing; }
}; // @ CONTAINER

enum JunkType {
	kJunkTypeWaterskin = 1, // @ C01_JUNK_WATERSKIN,
	kJunkTypeBones = 5, // @ C05_JUNK_BONES,
	kJunkTypeBoulder = 25, // @ C25_JUNK_BOULDER,
	kJunkTypeScreamerSlice = 33, // @ C33_JUNK_SCREAMER_SLICE,
	kJunkTypeWormRound = 34, // @ C34_JUNK_WORM_ROUND,
	kJunkTypeDrumstickShank = 35, // @ C35_JUNK_DRUMSTICK_SHANK,
	kJunkTypeDragonSteak = 36, // @ C36_JUNK_DRAGON_STEAK,
	kJunkTypeMagicalBoxBlue = 42, // @ C42_JUNK_MAGICAL_BOX_BLUE,
	kJunkTypeMagicalBoxGreen = 43, // @ C43_JUNK_MAGICAL_BOX_GREEN,
	kJunkTypeZokathra = 51 // @ C51_JUNK_ZOKATHRA,
};

class Junk {
	Thing _nextThing;
	uint16 _attributes;
public:
	explicit Junk(uint16 *rawDat) : _nextThing(rawDat[0]), _attributes(rawDat[1]) {}

	JunkType getType() { return (JunkType)(_attributes & 0x7F); }
	uint16 getChargeCount() { return (_attributes >> 14) & 0x3; }
	void setChargeCount(uint16 val) { _attributes = (_attributes & ~(0x3 << 14)) | ((val & 0x3) << 14); }

	Thing getNextThing() { return _nextThing; }
}; // @ JUNK

class Projectile {
	Thing _nextThing;
	Thing _object;
	byte _kineticEnergy;
	byte _damageEnergy;
	uint16 _timerIndex;
public:
	explicit Projectile(uint16 *rawDat) : _nextThing(rawDat[0]), _object(rawDat[1]), _kineticEnergy(rawDat[2]),
		_damageEnergy(rawDat[3]), _timerIndex(rawDat[4]) {}

	Thing getNextThing() { return _nextThing; }
}; // @ PROJECTILE

class Explosion {
	Thing _nextThing;
	uint16 _attributes;
public:
	explicit Explosion(uint16 *rawDat) : _nextThing(rawDat[0]), _attributes(rawDat[1]) {}

	Thing getNextThing() { return _nextThing; }
}; // @ EXPLOSION


enum SquareMask {
	kWallWestRandOrnAllowed = 0x1,
	kWallSouthRandOrnAllowed = 0x2,
	kWallEastRandOrnAllowed = 0x4,
	kWallNorthRandOrnAllowed = 0x8,
	kCorridorRandOrnAllowed = 0x8,
	kPitImaginary = 0x1,
	kPitInvisible = 0x4,
	kPitOpen = 0x8,
	kStairsUp = 0x4,
	kStairsNorthSouthOrient = 0x8,
	kDoorNorthSouthOrient = 0x8,
	kTeleporterVisible = 0x4,
	kTeleporterOpen = 0x8,
	kFakeWallImaginary = 0x1,
	kFakeWallOpen = 0x4,
	kFakeWallRandOrnOrFootPAllowed = 0x8,
	kThingListPresent = 0x10,
	kDecodeEvenIfInvisible = 0x8000
};

enum SquareType {
	kChampionElemType = -2,
	kCreatureElemType = -1,
	kWallElemType = 0,
	kCorridorElemType = 1,
	kPitElemType = 2,
	kStairsElemType = 3,
	kDoorElemType = 4,
	kTeleporterElemType = 5,
	kFakeWallElemType = 6,
	kDoorSideElemType = 16,
	kDoorFrontElemType = 17,
	kStairsSideElemType = 18,
	kStairsFrontElemType = 19
}; // @ C[-2..19]_ELEMENT_...

class Square {
	byte _data;
public:
	explicit Square(byte dat = 0) : _data(dat) {}
	explicit Square(SquareType type) { setType(type); }
	Square &set(byte dat) { this->_data = dat; return *this; }
	Square &set(SquareMask mask) { _data |= mask; return *this; }
	byte get(SquareMask mask) { return _data & mask; }
	byte getDoorState() { return _data & 0x7; } // @ M36_DOOR_STATE
	void setDoorState(byte state) { _data = ((_data & ~0x7) | state); } // @ M37_SET_DOOR_STATE
	SquareType getType() { return (SquareType)(_data >> 5); } // @ M34_SQUARE_TYPE
	void setType(SquareType type) { _data = (_data & 0x1F) | type << 5; }
	byte toByte() { return _data; } // I don't like 'em casts
};

struct DungeonFileHeader {
	uint16 _dungeonId; // @ G0526_ui_DungeonID
	// equal to dungeonId
	uint16 _ornamentRandomSeed;
	uint32 _rawMapDataSize;
	uint8 _mapCount;
	uint16 _textDataWordCount;
	direction _partyStartDir; // @ InitialPartyLocation
	uint16 _partyStartPosX, _partyStartPosY;
	uint16 _squareFirstThingCount; // @ SquareFirstThingCount
	uint16 _thingCounts[16]; // @ ThingCount[16]
}; // @ DUNGEON_HEADER

struct Map {
	uint32 _rawDunDataOffset;
	uint8 _offsetMapX, _offsetMapY;

	uint8 _level; // only used in DMII
	uint8 _width, _height; // !!! THESRE ARE INCLUSIVE BOUNDARIES
	// orn short for Ornament
	uint8 _wallOrnCount; /* May be used in a Sensor on a Wall or closed Fake Wall square */
	uint8 _randWallOrnCount; /* Used only on some Wall squares and some closed Fake Wall squares */
	uint8 _floorOrnCount; /* May be used in a Sensor on a Pit, open Fake Wall, Corridor or Teleporter square */
	uint8 _randFloorOrnCount; /* Used only on some Corridor squares and some open Fake Wall squares */

	uint8 _doorOrnCount;
	uint8 _creatureTypeCount;
	uint8 _difficulty;

	FloorSet _floorSet;
	WallSet _wallSet;
	uint8 _doorSet0, _doorSet1;
}; // @ MAP

struct DungeonData {
	// I have no idea the heck is this
	uint16 *_mapsFirstColumnIndex; // @ G0281_pui_DungeonMapsFirstColumnIndex
	uint16 _columCount; // @ G0282_ui_DungeonColumnCount

	// I have no idea the heck is this
	uint16 *_columnsCumulativeSquareThingCount; // @ G0280_pui_DungeonColumnsCumulativeSquareThingCount
	Thing *_squareFirstThings; // @ G0283_pT_SquareFirstThings
	uint16 *_textData; // @ G0260_pui_DungeonTextData

	uint16 **_thingsData[16]; // @ G0284_apuc_ThingData

	byte ***_mapData; // @ G0279_pppuc_DungeonMapData

	// TODO: ??? is this doing here
	uint16 _eventMaximumCount; // @ G0369_ui_EventMaximumCount
}; // @ AGGREGATE

struct CurrMapData {
	direction _partyDir; // @ G0308_i_PartyDirection
	int16 _partyPosX; // @ G0306_i_PartyMapX
	int16 _partyPosY; // @ G0307_i_PartyMapY
	uint8 _currPartyMapIndex; // @ G0309_i_PartyMapIndex

	uint8 _index; // @ G0272_i_CurrentMapIndex
	byte **_data; // @ G0271_ppuc_CurrentMapData
	Map *_map; // @ G0269_ps_CurrentMap
	uint16 _width; // @ G0273_i_CurrentMapWidth
	uint16 _height; // @ G0274_i_CurrentMapHeight
	uint16 *_colCumulativeSquareFirstThingCount; // @G0270_pui_CurrentMapColumnsCumulativeSquareFirstThingCount
}; // @ AGGREGATE

struct Messages {
	bool _newGame; // @ G0298_B_NewGame
	bool _restartGameRequest; // @ G0523_B_RestartGameRequested
}; // @ AGGREGATE

class DungeonMan {
	DMEngine *_vm;

	DungeonMan(const DungeonMan &other); // no implementation on purpose
	void operator=(const DungeonMan &rhs); // no implementation on purpose

	Square getSquare(int16 mapX, int16 mapY); // @ F0151_DUNGEON_GetSquare
	Square getRelSquare(direction dir, int16 stepsForward, int16 stepsRight, int16 posX, int16 posY); // @ F0152_DUNGEON_GetRelativeSquare

	void decompressDungeonFile(); // @ F0455_FLOPPY_DecompressDungeon

	int16 getSquareFirstThingIndex(int16 mapX, int16 mapY); // @ F0160_DUNGEON_GetSquareFirstThingIndex

	int16 getRandomOrnOrdinal(bool allowed, int16 count, int16 mapX, int16 mapY, int16 modulo); // @ F0170_DUNGEON_GetRandomOrnamentOrdinal
	void setSquareAspectOrnOrdinals(uint16 *aspectArray, bool leftAllowed, bool frontAllowed, bool rightAllowed, direction dir,
									int16 mapX, int16 mapY, bool isFakeWall); // @ F0171_DUNGEON_SetSquareAspectRandomWallOrnamentOrdinals

	void setCurrentMap(uint16 mapIndex); // @ F0173_DUNGEON_SetCurrentMap

public:
	explicit DungeonMan(DMEngine *dmEngine);
	~DungeonMan();

	Thing getSquareFirstThing(int16 mapX, int16 mapY); // @ F0161_DUNGEON_GetSquareFirstThing
	Thing getNextThing(Thing thing); // @ F0159_DUNGEON_GetNextThing(THING P0280_T_Thing)
	uint16 *getThingData(Thing thing); // @ unsigned char* F0156_DUNGEON_GetThingData(register THING P0276_T_Thing)
	uint16 *getSquareFirstThingData(int16 mapX, int16 mapY); // @ F0157_DUNGEON_GetSquareFirstThingData

	// TODO: this does stuff other than load the file!
	void loadDungeonFile();	// @ F0434_STARTEND_IsLoadDungeonSuccessful_CPSC
	void setCurrentMapAndPartyMap(uint16 mapIndex); // @ F0174_DUNGEON_SetCurrentMapAndPartyMap

	bool isWallOrnAnAlcove(int16 wallOrnIndex); // @ F0149_DUNGEON_IsWallOrnamentAnAlcove
	void mapCoordsAfterRelMovement(direction dir, int16 stepsForward, int16 stepsRight, int16 &posX, int16 &posY); // @ F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement
	SquareType getRelSquareType(direction dir, int16 stepsForward, int16 stepsRight, int16 posX, int16 posY) {
		return Square(getRelSquare(dir, stepsForward, stepsRight, posX, posY)).getType();
	} // @ F0153_DUNGEON_GetRelativeSquareType
	void setSquareAspect(uint16 *aspectArray, direction dir, int16 mapX, int16 mapY); // @ F0172_DUNGEON_SetSquareAspect
	void decodeText(char *destString, Thing thing, TextType type); // F0168_DUNGEON_DecodeText

	uint16 getObjectWeight(Thing thing); // @ F0140_DUNGEON_GetObjectWeight
	int16 getObjectInfoIndex(Thing thing); // @ F0141_DUNGEON_GetObjectInfoIndex
	void linkThingToList(Thing thingToLink, Thing thingInList, int16 mapX, int16 mapY); // @ F0163_DUNGEON_LinkThingToList
	WeaponInfo *getWeaponInfo(Thing thing); // @ F0158_DUNGEON_GetWeaponInfo
	int16 getProjectileAspect(Thing thing); // @ F0142_DUNGEON_GetProjectileAspect

	uint32 _rawDunFileDataSize;	 // @ probably NONE
	byte *_rawDunFileData; // @ ???
	DungeonFileHeader _fileHeader; // @ G0278_ps_DungeonHeader

	DungeonData _dunData; // @ NONE
	CurrMapData _currMap; // @ NONE
	Map *_maps; // @ G0277_ps_DungeonMaps
	// does not have to be freed
	byte *_rawMapData; // @ G0276_puc_DungeonRawMapData
	Messages _messages; // @ NONE;

	int16 _currMapInscriptionWallOrnIndex; // @ G0265_i_CurrentMapInscriptionWallOrnamentIndex
	Box _dungeonViewClickableBoxes[6]; // G0291_aauc_DungeonViewClickableBoxes
	bool _isFacingAlcove; // @ G0286_B_FacingAlcove
	bool _isFacingViAltar; // @ G0287_B_FacingViAltar
	bool _isFacingFountain; // @ G0288_B_FacingFountain
	ElementType _squareAheadElement; // @ G0285_i_SquareAheadElement 
};

}

#endif
