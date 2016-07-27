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
#define k0_ObjectInfoIndexFirstScroll 0 // @ C000_OBJECT_INFO_INDEX_FIRST_SCROLL
#define k1_ObjectInfoIndexFirstContainer 1 // @ C001_OBJECT_INFO_INDEX_FIRST_CONTAINER
#define k2_ObjectInfoIndexFirstPotion 2 // @ C002_OBJECT_INFO_INDEX_FIRST_POTION
#define k23_ObjectInfoIndexFirstWeapon 23 // @ C023_OBJECT_INFO_INDEX_FIRST_WEAPON
#define k69_ObjectInfoIndexFirstArmour 69 // @ C069_OBJECT_INFO_INDEX_FIRST_ARMOUR
#define k127_ObjectInfoIndexFirstJunk 127 // @ C127_OBJECT_INFO_INDEX_FIRST_JUNK

#define kM1_MapXNotOnASquare -1 // @ CM1_MAPX_NOT_ON_A_SQUARE

enum ElementType {
	kM2_ElementTypeChampion = -2, // @ CM2_ELEMENT_CHAMPION /* Values -2 and -1 are only used as projectile impact types */
	kM1_ElementTypeCreature = -1, // @ CM1_ELEMENT_CREATURE
	k0_ElementTypeWall = 0, // @ C00_ELEMENT_WALL /* Values 0-6 are used as square types and projectile impact types. Values 0-2 and 5-6 are also used for square aspect */
	k1_ElementTypeCorridor = 1, // @ C01_ELEMENT_CORRIDOR
	k2_ElementTypePit = 2, // @ C02_ELEMENT_PIT
	k3_ElementTypeStairs = 3, // @ C03_ELEMENT_STAIRS
	// TODO: refactor direction into a class
	k4_ElementTypeDoor = 4, // @ C04_ELEMENT_DOOR
	k5_ElementTypeTeleporter = 5, // @ C05_ELEMENT_TELEPORTER
	k6_ElementTypeFakeWall = 6, // @ C06_ELEMENT_FAKEWALL
	k16_ElementTypeDoorSide = 16, // @ C16_ELEMENT_DOOR_SIDE /* Values 16-19 are only used for square aspect */
	k17_ElementTypeDoorFront = 17, // @ C17_ELEMENT_DOOR_FRONT
	k18_ElementTypeStairsSide = 18, // @ C18_ELEMENT_STAIRS_SIDE
	k19_ElementTypeStaisFront = 19  // @ C19_ELEMENT_STAIRS_FRONT	
};


enum ObjectAllowedSlot {
	k0x0001_ObjectAllowedSlotMouth = 0x0001, // @ MASK0x0001_MOUTH
	k0x0002_ObjectAllowedSlotHead = 0x0002, // @ MASK0x0002_HEAD
	k0x0004_ObjectAllowedSlotNeck = 0x0004, // @ MASK0x0004_NECK
	k0x0008_ObjectAllowedSlotTorso = 0x0008, // @ MASK0x0008_TORSO
	k0x0010_ObjectAllowedSlotLegs = 0x0010, // @ MASK0x0010_LEGS
	k0x0020_ObjectAllowedSlotFeet = 0x0020, // @ MASK0x0020_FEET
	k0x0040_ObjectAllowedSlotQuiverLine_1 = 0x0040, // @ MASK0x0040_QUIVER_LINE1
	k0x0080_ObjectAllowedSlotQuiverLine_2 = 0x0080, // @ MASK0x0080_QUIVER_LINE2
	k0x0100_ObjectAllowedSlotPouchPassAndThroughDoors = 0x0100, // @ MASK0x0100_POUCH_PASS_AND_THROUGH_DOORS
	k0x0200_ObjectAllowedSlotHands = 0x0200, // @ MASK0x0200_HANDS
	k0x0400_ObjectAllowedSlotContainer = 0x0400 // @ MASK0x0400_CONTAINER
};

class ObjectInfo {
public:
	int16 _type;
	uint16 _objectAspectIndex;
	uint16 _actionSetIndex;
	uint16 _allowedSlots;
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

extern ObjectInfo g237_ObjectInfo[180]; // @ G0237_as_Graphic559_ObjectInfo

enum ArmourAttribute {
	k0x0080_ArmourAttributeIsAShield = 0x0080, // @ MASK0x0080_IS_A_SHIELD
	k0x0007_ArmourAttributeSharpDefense = 0x0007 // @ MASK0x0007_SHARP_DEFENSE
};

class ArmourInfo {
public:
	uint16 _weight;
	uint16 _defense;
	uint16 _attributes;
	ArmourInfo(uint16 weight, uint16 defense, uint16 attributes)
		:_weight(weight), _defense(defense), _attributes(attributes) {}

	uint16 getAttribute(ArmourAttribute attribute) { return _attributes & attribute; }
	void setAttribute(ArmourAttribute attribute) { _attributes |= attribute; }
}; // @ ARMOUR_INFO

extern ArmourInfo g239_ArmourInfo[58]; // G0239_as_Graphic559_ArmourInfo

#define kM1_WeaponClassNone					-1
/* Class 0: SWING weapons */
#define k0_WeaponClassSwingWeapon			0 // @ C000_CLASS_SWING_WEAPON 
/* Class 1 to 15: THROW weapons */
#define k2_WeaponClassDaggerAndAxes		2 // @ C002_CLASS_DAGGER_AND_AXES 
#define k10_WeaponClassBowAmmunition		10 // @ C010_CLASS_BOW_AMMUNITION
#define k11_WeaponClassSlingAmmunition		11 // @ C011_CLASS_SLING_AMMUNITION
#define k12_WeaponClassPoisinDart			12 // @ C012_CLASS_POISON_DART
/* Class 16 to 111: SHOOT weapons */
#define k16_WeaponClassFirstBow			16 // @ C016_CLASS_FIRST_BOW 
#define k31_WeaponClassLastBow				31 // @ C031_CLASS_LAST_BOW
#define k32_WeaponClassFirstSling			32 // @ C032_CLASS_FIRST_SLING
#define k47_WeaponClassLastSling			47 // @ C047_CLASS_LAST_SLING
/* Class 112 to 255: Magic and special weapons */
#define k112_WeaponClassFirstMagicWeapon	112 // @ C112_CLASS_FIRST_MAGIC_WEAPON

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

extern WeaponInfo g238_WeaponInfo[46]; // @ G0238_as_Graphic559_WeaponInfo;

enum TextType {
	/* Used for text on walls */
	k0_TextTypeInscription = 0, // @ C0_TEXT_TYPE_INSCRIPTION 
	/* Used for messages displayed when the party walks on a square */
	k1_TextTypeMessage = 1, // @ C1_TEXT_TYPE_MESSAGE 
	/* Used for text on scrolls and champion information */
	k2_TextTypeScroll = 2 // @ C2_TEXT_TYPE_SCROLL 
};

enum SquareAspectIndice {
	k0_ElemAspect = 0, // @ C0_ELEMENT
	k1_FirstGroupOrObjectAspect = 1, // @ C1_FIRST_GROUP_OR_OBJECT 
	k2_RightWallOrnOrdAspect = 2, // @ C2_RIGHT_WALL_ORNAMENT_ORDINAL 
	k3_FrontWallOrnOrdAspect = 3, // @ C3_FRONT_WALL_ORNAMENT_ORDINAL 
	k4_LeftWallOrnOrdAspect = 4, // @ C4_LEFT_WALL_ORNAMENT_ORDINAL 
	k2_PitInvisibleAspect = 2, // @ C2_PIT_INVISIBLE 
	k2_TeleporterVisibleAspect = 2, // @ C2_TELEPORTER_VISIBLE 
	k2_StairsUpAspect = 2, // @ C2_STAIRS_UP 
	k2_DoorStateAspect = 2, // @ C2_DOOR_STATE 
	k3_DoorThingIndexAspect = 3, // @ C3_DOOR_THING_INDEX 
	k4_FloorOrnOrdAspect = 4, // @ C4_FLOOR_ORNAMENT_ORDINAL 
	k0x8000_FootprintsAspect = 0x8000 // @ MASK0x8000_FOOTPRINTS             
};



#define k15_immuneToFire 15 // @ C15_IMMUNE_TO_FIRE   
#define k15_immuneToPoison 15 // @ C15_IMMUNE_TO_POISON 

class CreatureInfo {
public:
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

	uint16 M57_getFearResistance() { return (_properties >> 4) & 0xF; }
	uint16 M58_getExperience() { return (_properties >> 8) & 0xF; }
	uint16 M59_getWariness() { return (_properties >> 12) & 0xF; }
	uint16 M60_getFireResistance() { return (_resistances >> 4) & 0xF; }
	uint16 M61_poisonResistance() { return (_resistances >> 8) & 0xF; }
	static uint16 M51_height(uint16 attrib) { return (attrib >> 7) & 0x3; }
	uint16 M54_getSightRange() { return (_ranges) & 0xF; }
	uint16 M55_getSmellRange() { return  (_ranges >> 8) & 0xF; }
	uint16 M56_getAttackRange() { return (_ranges >> 12) & 0xF; }
}; // @ CREATURE_INFO


extern CreatureInfo g243_CreatureInfo[k27_CreatureTypeCount]; // @ G0243_as_Graphic559_CreatureInfo

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
	k0x0001_TelepScopeCreatures = 1, // @ MASK0x0001_SCOPE_CREATURES
	k0x0002_TelepScopeObjOrParty = 2 // @ MASK0x0002_SCOPE_OBJECTS_OR_PARTY
};


class Teleporter {
	Thing _nextThing;
	uint16 _attributes;
	uint16 _destMapIndex;
public:
	explicit Teleporter(uint16 *rawDat) : _nextThing(rawDat[0]), _attributes(rawDat[1]), _destMapIndex(rawDat[2]) {}
	Thing getNextThing() { return _nextThing; }
	bool isAudible() { return (_attributes >> 15) & 1; }
	TeleporterScope getScope() { return (TeleporterScope)((_attributes >> 13) & 1); }
	bool getAbsoluteRotation() { return (_attributes >> 12) & 1; }
	Direction getRotation() { return (Direction)((_attributes >> 10) & 1); }
	byte getTargetMapY() { return (_attributes >> 5) & 0xF; }
	byte getTargetMapX() { return _attributes & 0xF; }
	uint16 getTargetMapIndex() { return _destMapIndex >> 8; }
}; // @ TELEPORTER



class TextString {
	Thing _nextThing;
	uint16 _textDataRef;
public:
	explicit TextString(uint16 *rawDat) : _nextThing(rawDat[0]), _textDataRef(rawDat[1]) {}

	Thing getNextThing() { return _nextThing; }
	uint16 getWordOffset() { return _textDataRef >> 3; }
	bool isVisible() { return _textDataRef & 1; }
	void setVisible(bool visible) { _textDataRef = (_textDataRef & ~1) | (visible ? 1 : 0); }
}; // @ TEXTSTRING

enum SensorActionType {
	kM1_SensorEffNone = -1, // @ CM1_EFFECT_NONE
	k0_SensorEffSet = 0, // @ C00_EFFECT_SET
	k1_SensorEffClear = 1, // @ C01_EFFECT_CLEAR
	k2_SensorEffToggle = 2, // @ C02_EFFECT_TOGGLE
	k3_SensorEffHold = 3, // @ C03_EFFECT_HOLD
	k10_SensorEffAddExp = 10 // @ C10_EFFECT_ADD_EXPERIENCE
};

enum SensorType {
	k0_SensorDisabled = 0, // @ C000_SENSOR_DISABLED    /* Never triggered, may be used for a floor or wall ornament */
	k1_SensorFloorTheronPartyCreatureObj = 1, // @ C001_SENSOR_FLOOR_THERON_PARTY_CREATURE_OBJECT    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	k2_SensorFloorTheronPartyCreature = 2, // @ C002_SENSOR_FLOOR_THERON_PARTY_CREATURE    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	k3_SensorFloorParty = 3, // @ C003_SENSOR_FLOOR_PARTY    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	k4_SensorFloorObj = 4, // @ C004_SENSOR_FLOOR_OBJECT    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	k5_SensorFloorPartyOnStairs = 5, // @ C005_SENSOR_FLOOR_PARTY_ON_STAIRS    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	k6_SensorFloorGroupGenerator = 6, // @ C006_SENSOR_FLOOR_GROUP_GENERATOR    /* Triggered by event F0245_TIMELINE_ProcessEvent5_Square_Corridor */
	k7_SensorFloorCreature = 7, // @ C007_SENSOR_FLOOR_CREATURE    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	k8_SensorFloorPartyPossession = 8, // @ C008_SENSOR_FLOOR_PARTY_POSSESSION    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	k9_SensorFloorVersionChecker = 9, // @ C009_SENSOR_FLOOR_VERSION_CHECKER    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	k1_SensorWallOrnClick = 1, // @ C001_SENSOR_WALL_ORNAMENT_CLICK    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	k2_SensorWallOrnClickWithAnyObj = 2, // @ C002_SENSOR_WALL_ORNAMENT_CLICK_WITH_ANY_OBJECT    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	k3_SensorWallOrnClickWithSpecObj = 3, // @ C003_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	k4_SensorWallOrnClickWithSpecObjRemoved = 4, // @ C004_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	k5_SensorWallAndOrGate = 5, // @ C005_SENSOR_WALL_AND_OR_GATE    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	k6_SensorWallCountdown = 6, // @ C006_SENSOR_WALL_COUNTDOWN    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	k7_SensorWallSingleProjLauncherNewObj = 7, // @ C007_SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_NEW_OBJECT    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	k8_SensorWallSingleProjLauncherExplosion = 8, // @ C008_SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_EXPLOSION    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	k9_SensorWallDoubleProjLauncherNewObj = 9, // @ C009_SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_NEW_OBJECT    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	k10_SensorWallDoubleProjLauncherExplosion = 10, // @ C010_SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_EXPLOSION    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	k11_SensorWallOrnClickWithSpecObjRemovedRotateSensors = 11, // @ C011_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED_ROTATE_SENSORS   /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	k12_SensorWallObjGeneratorRotateSensors = 12, // @ C012_SENSOR_WALL_OBJECT_GENERATOR_ROTATE_SENSORS    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	k13_SensorWallSingleObjStorageRotateSensors = 13, // @ C013_SENSOR_WALL_SINGLE_OBJECT_STORAGE_ROTATE_SENSORS    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	k14_SensorWallSingleProjLauncherSquareObj = 14, // @ C014_SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	k15_SensorWallDoubleProjLauncherSquareObj = 15, // @ C015_SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	k16_SensorWallObjExchanger = 16, // @ C016_SENSOR_WALL_OBJECT_EXCHANGER    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	k17_SensorWallOrnClickWithSpecObjRemovedSensor = 17, // @ C017_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED_REMOVE_SENSOR    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	k18_SensorWallEndGame = 18, // @ C018_SENSOR_WALL_END_GAME    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	k127_SensorWallChampionPortrait = 127 // @ C127_SENSOR_WALL_CHAMPION_PORTRAIT    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
};

class Sensor {
	Thing _nextThing;
	uint16 _datAndType;
	uint16 _attributes; // A
	uint16 _action; // B
public:
	explicit Sensor(uint16 *rawDat) : _nextThing(rawDat[0]), _datAndType(rawDat[1]), _attributes(rawDat[2]), _action(rawDat[3]) {}

	Thing getNextThing() { return _nextThing; }
	void setNextThing(Thing thing) { _nextThing = thing; }
	SensorType getType() { return (SensorType)(_datAndType & 0x7F); } // @ M39_TYPE
	uint16 getData() { return (_datAndType >> 7) & 0x1FF; } // @ M40_DATA
	static uint16 getDataMask1(uint16 data) { return (data >> 7) & 0xF; } // @ M42_MASK1
	static uint16 getDataMask2(uint16 data) { return (data >> 11) & 0xF; } // @ M43_MASK2
	void setData(uint16 dat) { _datAndType = dat; } // @ M41_SET_DATA
	void setTypeDisabled() { _datAndType &= 0xFF80; } // @ M44_SET_TYPE_DISABLED



	bool getOnlyOnce() { return (_attributes >> 2) & 1; }
	uint16 getEffectA() { return (_attributes >> 3) & 0x3; }
	bool getRevertEffectA() { return (_attributes >> 5) & 0x1; }
	bool getAudibleA() { return (_attributes >> 6) & 0x1; }
	uint16 getValue() { return (_attributes >> 7) & 0xF; }
	bool getLocalEffect() { return (_attributes >> 11) & 1; }
	uint16 getOrnOrdinal() { return _attributes >> 12; }

	uint16 getTargetMapY() { return (_action >> 11); }
	uint16 getTargetMapX() { return (_action >> 6) & 0x1F; }
	Direction getTargetCell() { return (Direction)((_action >> 4) & 3); }
	uint16 M45_healthMultiplier() { return ((_action >> 4) & 0xF); } // @ M45_HEALTH_MULTIPLIER
	uint16 M46_ticks() { return ((_action >> 4) >> 4) & 0xFFF; } // @ M46_TICKS
	uint16 M47_kineticEnergy() { return ((_action >> 4) & 0xFF); }// @ M47_KINETIC_ENERGY
	uint16 M48_stepEnergy() { return ((_action >> 4) >> 8) & 0xFF; }// @ M48_STEP_ENERGY
	uint16 M49_localEffect() { return (_action >> 4); } // @ M49_LOCAL_EFFECT

	void setDatAndTypeWithOr(uint16 val) { _datAndType |= val;  }

}; // @ SENSOR


#define k0x8000_randomDrop 0x8000 // @ MASK0x8000_RANDOM_DROP 


enum WeaponType {
	k2_WeaponTypeTorch = 2, // @ C02_WEAPON_TORCH
	k8_WeaponTypeDagger = 8, // @ C08_WEAPON_DAGGER
	k9_WeaponTypeFalchion = 9, // @ C09_WEAPON_FALCHION
	k10_WeaponTypeSword = 10, // @ C10_WEAPON_SWORD
	k23_WeaponTypeClub = 23, // @ C23_WEAPON_CLUB
	k24_WeaponTypeStoneClub = 24, // @ C24_WEAPON_STONE_CLUB
	k27_WeaponTypeArrow = 27, // @ C27_WEAPON_ARROW
	k28_WeaponTypeSlayer = 28, // @ C28_WEAPON_SLAYER
	k30_WeaponTypeRock = 30, // @ C30_WEAPON_ROCK
	k31_WeaponTypePoisonDart = 31, // @ C31_WEAPON_POISON_DART
	k32_WeaponTypeThrowingStar = 32 // @ C32_WEAPON_THROWING_STAR
};
class Weapon {
	Thing _nextThing;
	uint16 _desc;
public:
	explicit Weapon(uint16 *rawDat) : _nextThing(rawDat[0]), _desc(rawDat[1]) {}

	WeaponType getType() { return (WeaponType)(_desc & 0x7F); }
	void setType(uint16 val) { _desc = (_desc & ~0x7F) | (val & 0x7F); }
	bool isLit() { return (_desc >> 15) & 1; }
	void setLit(bool val) {
		if (val)
			_desc |= (1 << 15);
		else
			_desc &= (~(1 << 15));
	}
	uint16 getChargeCount() { return (_desc >> 10) & 0xF; }
	uint16 setChargeCount(uint16 val) { _desc = (_desc & ~(0xF << 10)) | ((val & 0xF) << 10); return (val & 0xF); }
	Thing getNextThing() { return _nextThing; }
	void setNextThing(Thing val) { _nextThing = val;  }
	uint16 getCursed() { return (_desc >> 8) & 1; }
	void setCursed(uint16 val) { _desc = (_desc & ~(1 << 8)) | ((val & 1) << 8); }
	uint16 getPoisoned() { return (_desc >> 9) & 1; }
	uint16 getBroken() { return (_desc >> 14) & 1; }
	uint16 getDoNotDiscard() { return (_desc >> 7) & 1; }
	void setDoNotDiscard(uint16 val) { _desc = (_desc & ~(1 << 7)) | ((val & 1) << 7); }
}; // @ WEAPON

enum ArmourType {
	k30_ArmourTypeWoodenShield = 30, // @ C30_ARMOUR_WOODEN_SHIELD
	k38_ArmourTypeArmet = 38, // @ C38_ARMOUR_ARMET
	k39_ArmourTypeTorsoPlate = 39, // @ C39_ARMOUR_TORSO_PLATE
	k40_ArmourTypeLegPlate = 40, // @ C40_ARMOUR_LEG_PLATE
	k41_ArmourTypeFootPlate = 41 // @ C41_ARMOUR_FOOT_PLATE
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
	uint16 getDoNotDiscard() { return (_attributes >> 7) & 1; }
	uint16 getChargeCount() { return (_attributes >> 9) & 0xF; }
	void setChargeCount(uint16 val) { _attributes = (_attributes & ~(0xF << 9)) | ((val & 0xF) << 9); }
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
	k3_PotionTypeVen = 3, // @ C03_POTION_VEN_POTION,
	k6_PotionTypeRos = 6, // @ C06_POTION_ROS_POTION,
	k7_PotionTypeKu = 7, // @ C07_POTION_KU_POTION,
	k8_PotionTypeDane = 8, // @ C08_POTION_DANE_POTION,
	k9_PotionTypeNeta = 9, // @ C09_POTION_NETA_POTION,
	k10_PotionTypeAntivenin = 10, // @ C10_POTION_ANTIVENIN,
	k11_PotionTypeMon = 11, // @ C11_POTION_MON_POTION,
	k12_PotionTypeYa = 12, // @ C12_POTION_YA_POTION,
	k13_PotionTypeEe = 13, // @ C13_POTION_EE_POTION,
	k14_PotionTypeVi = 14, // @ C14_POTION_VI_POTION,
	k15_PotionTypeWaterFlask = 15, // @ C15_POTION_WATER_FLASK,
	k19_PotionTypeFulBomb = 19, // @ C19_POTION_FUL_BOMB,
	k20_PotionTypeEmptyFlask = 20 // @ C20_POTION_EMPTY_FLASK,
};
class Potion {
public:
	Thing _nextThing;
	uint16 _attributes;
	explicit Potion(uint16 *rawDat) : _nextThing(rawDat[0]), _attributes(rawDat[1]) {}

	PotionType getType() { return (PotionType)((_attributes >> 8) & 0x7F); }
	void setType(PotionType val) { _attributes = (_attributes & ~(0x7F << 8)) | ((val & 0x7F) << 8); }
	Thing getNextThing() { return _nextThing; }
	uint16 getPower() { return _attributes & 0xFF; }
	void setPower(uint16 val) { _attributes = (_attributes & ~0xFF) | (val & 0xFF); }
	uint16 getDoNotDiscard() { return (_attributes >> 15) & 1; }
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
	k1_JunkTypeWaterskin = 1, // @ C01_JUNK_WATERSKIN,
	k5_JunkTypeBones = 5, // @ C05_JUNK_BONES,
	k25_JunkTypeBoulder = 25, // @ C25_JUNK_BOULDER,
	k33_JunkTypeScreamerSlice = 33, // @ C33_JUNK_SCREAMER_SLICE,
	k34_JunkTypeWormRound = 34, // @ C34_JUNK_WORM_ROUND,
	k35_JunkTypeDrumstickShank = 35, // @ C35_JUNK_DRUMSTICK_SHANK,
	k36_JunkTypeDragonSteak = 36, // @ C36_JUNK_DRAGON_STEAK,
	k42_JunkTypeMagicalBoxBlue = 42, // @ C42_JUNK_MAGICAL_BOX_BLUE,
	k43_JunkTypeMagicalBoxGreen = 43, // @ C43_JUNK_MAGICAL_BOX_GREEN,
	k51_JunkTypeZokathra = 51 // @ C51_JUNK_ZOKATHRA,
};

class Junk {
	Thing _nextThing;
	uint16 _attributes;
public:
	explicit Junk(uint16 *rawDat) : _nextThing(rawDat[0]), _attributes(rawDat[1]) {}

	JunkType getType() { return (JunkType)(_attributes & 0x7F); }
	void setType(uint16 val) { _attributes = (_attributes & ~0x7F) | (val & 0x7F); }
	uint16 getChargeCount() { return (_attributes >> 14) & 0x3; }
	void setChargeCount(uint16 val) { _attributes = (_attributes & ~(0x3 << 14)) | ((val & 0x3) << 14); }
	uint16 getDoNotDiscard() { return (_attributes >> 7) & 1; }
	void setDoNotDiscard(uint16 val) { _attributes = (_attributes & ~(1 << 7)) | ((val & 1) << 7); }

	Thing getNextThing() { return _nextThing; }
	void setNextThing(Thing thing) { _nextThing = thing; }
}; // @ JUNK

#define kM1_soundModeDoNotPlaySound -1 // @ CM1_MODE_DO_NOT_PLAY_SOUND  
#define k0_soundModePlayImmediately 0 // @ C00_MODE_PLAY_IMMEDIATELY    
#define k1_soundModePlayIfPrioritized 1 // @ C01_MODE_PLAY_IF_PRIORITIZED 
#define k2_soundModePlayOneTickLater 2 // @ C02_MODE_PLAY_ONE_TICK_LATER 

class Projectile {
public:
	Thing _nextThing;
	Thing _slot;
	uint16 _kineticEnergy;
	uint16 _attack;
	uint16 _eventIndex;
	explicit Projectile(uint16 *rawDat) : _nextThing(rawDat[0]), _slot(rawDat[1]), _kineticEnergy(rawDat[2]),
		_attack(rawDat[3]), _eventIndex(rawDat[4]) {}

}; // @ PROJECTILE

#define k0_ExplosionType_Fireball 0 // @ C000_EXPLOSION_FIREBALL           
#define k1_ExplosionType_Slime 1 // @ C001_EXPLOSION_SLIME              
#define k2_ExplosionType_LightningBolt 2 // @ C002_EXPLOSION_LIGHTNING_BOLT     
#define k3_ExplosionType_HarmNonMaterial 3 // @ C003_EXPLOSION_HARM_NON_MATERIAL  
#define k4_ExplosionType_OpenDoor 4 // @ C004_EXPLOSION_OPEN_DOOR          
#define k6_ExplosionType_PoisonBolt 6 // @ C006_EXPLOSION_POISON_BOLT        
#define k7_ExplosionType_PoisonCloud 7 // @ C007_EXPLOSION_POISON_CLOUD       
#define k40_ExplosionType_Smoke 40 // @ C040_EXPLOSION_SMOKE             
#define k50_ExplosionType_Fluxcage 50 // @ C050_EXPLOSION_FLUXCAGE          
#define k100_ExplosionType_RebirthStep1 100 // @ C100_EXPLOSION_REBIRTH_STEP1    
#define k101_ExplosionType_RebirthStep2 101 // @ C101_EXPLOSION_REBIRTH_STEP2    

class Explosion {
	Thing _nextThing;
	uint16 _attributes;
public:
	explicit Explosion(uint16 *rawDat) : _nextThing(rawDat[0]), _attributes(rawDat[1]) {}

	Thing getNextThing() { return _nextThing; }
	Thing setNextThing(Thing val) { return _nextThing = val; }
	uint16 getType() { return _attributes & 0x7F; }
	uint16 setType(uint16 val) { _attributes = (_attributes & ~0x7F) | (val & 0x7F); return (val & 0x7F); }
	uint16 getAttack() { return (_attributes >> 8) & 0xFF; }
	void setAttack(uint16 val) { _attributes = (_attributes & ~(0xFF << 8)) | ((val & 0xFF) << 8); }
	uint16 getCentered() { return (_attributes >> 7) & 0x1; }
	void setCentered(uint16 val) { _attributes = (_attributes & ~(1 << 7)) | ((val & 1) << 7); }
}; // @ EXPLOSION


enum SquareMask {
	k0x0001_WallWestRandOrnAllowed = 0x1, // @ MASK0x0001_WALL_WEST_RANDOM_ORNAMENT_ALLOWED 
	k0x0002_WallSouthRandOrnAllowed = 0x2, // @ MASK0x0002_WALL_SOUTH_RANDOM_ORNAMENT_ALLOWED 
	k0x0004_WallEastRandOrnAllowed = 0x4, // @ MASK0x0004_WALL_EAST_RANDOM_ORNAMENT_ALLOWED 
	k0x0008_WallNorthRandOrnAllowed = 0x8, // @ MASK0x0008_WALL_NORTH_RANDOM_ORNAMENT_ALLOWED 
	k0x0008_CorridorRandOrnAllowed = 0x8, // @ MASK0x0008_CORRIDOR_RANDOM_ORNAMENT_ALLOWED 
	k0x0001_PitImaginary = 0x1, // @ MASK0x0001_PIT_IMAGINARY 
	k0x0004_PitInvisible = 0x4, // @ MASK0x0004_PIT_INVISIBLE 
	k0x0008_PitOpen = 0x8, // @ MASK0x0008_PIT_OPEN 
	k0x0004_StairsUp = 0x4, // @ MASK0x0004_STAIRS_UP 
	k0x0008_StairsNorthSouthOrient = 0x8, // @ MASK0x0008_STAIRS_NORTH_SOUTH_ORIENTATION 
	k0x0008_DoorNorthSouthOrient = 0x8, // @ MASK0x0008_DOOR_NORTH_SOUTH_ORIENTATION 
	k0x0004_TeleporterVisible = 0x4, // @ MASK0x0004_TELEPORTER_VISIBLE 
	k0x0008_TeleporterOpen = 0x8, // @ MASK0x0008_TELEPORTER_OPEN 
	k0x0001_FakeWallImaginary = 0x1, // @ MASK0x0001_FAKEWALL_IMAGINARY 
	k0x0004_FakeWallOpen = 0x4, // @ MASK0x0004_FAKEWALL_OPEN 
	k0x0008_FakeWallRandOrnOrFootPAllowed = 0x8, // @ MASK0x0008_FAKEWALL_RANDOM_ORNAMENT_OR_FOOTPRINTS_ALLOWED 
	k0x0010_ThingListPresent = 0x10, // @ MASK0x0010_THING_LIST_PRESENT 
	k0x8000_DecodeEvenIfInvisible = 0x8000 // @ MASK0x8000_DECODE_EVEN_IF_INVISIBLE 
};

enum SquareType {
	kM2_ChampionElemType = -2, // @ CM2_ELEMENT_CHAMPION 
	kM1_CreatureElemType = -1, // @ CM1_ELEMENT_CREATURE 
	k0_WallElemType = 0, // @ C00_ELEMENT_WALL 
	k1_CorridorElemType = 1, // @ C01_ELEMENT_CORRIDOR 
	k2_PitElemType = 2, // @ C02_ELEMENT_PIT 
	k3_StairsElemType = 3, // @ C03_ELEMENT_STAIRS 
	k4_DoorElemType = 4, // @ C04_ELEMENT_DOOR 
	k5_TeleporterElemType = 5, // @ C05_ELEMENT_TELEPORTER 
	k6_FakeWallElemType = 6, // @ C06_ELEMENT_FAKEWALL 
	k16_DoorSideElemType = 16, // @ C16_ELEMENT_DOOR_SIDE 
	k17_DoorFrontElemType = 17, // @ C17_ELEMENT_DOOR_FRONT 
	k18_StairsSideElemType = 18, // @ C18_ELEMENT_STAIRS_SIDE 
	k19_StairsFrontElemType = 19 // @ C19_ELEMENT_STAIRS_FRONT 
}; // @ C[-2..19]_ELEMENT_...

#define k0x8000_championBones 0x8000 // @ MASK0x8000_CHAMPION_BONES 
#define k0x7FFF_thingType 0x7FFF // @ MASK0x7FFF_THING_TYPE     

class Square {
	byte _data;
public:
	explicit Square(byte dat = 0) : _data(dat) {}
	explicit Square(SquareType type) { setType(type); }
	explicit Square(byte element, byte mask) : _data((element << 5) | mask) {}
	Square &set(byte dat) { this->_data = dat; return *this; }
	Square &set(SquareMask mask) { _data |= mask; return *this; }
	byte get(SquareMask mask) { return _data & mask; }
	byte getDoorState() { return _data & 0x7; } // @ M36_DOOR_STATE
	void setDoorState(byte state) { _data = ((_data & ~0x7) | state); } // @ M37_SET_DOOR_STATE
	SquareType getType() { return (SquareType)(_data >> 5); } // @ M34_SQUARE_TYPE
	void setType(SquareType type) { _data = (_data & 0x1F) | type << 5; }
	byte toByte() { return _data; } // I don't like 'em casts
}; // wrapper for bytes which are used as squares

struct DungeonFileHeader {
	uint16 _ornamentRandomSeed;
	uint16 _rawMapDataSize;
	uint8 _mapCount;
	uint16 _textDataWordCount;
	uint16 _partyStartLocation;
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


class DoorInfo {
public:
	byte _attributes;
	byte _defense;
	DoorInfo(byte b1, byte b2) : _attributes(b1), _defense(b2) {}
	DoorInfo() { resetToZero(); }
	void resetToZero() { _attributes = _defense = 0; }
}; // @ DOOR_INFO

class Group;

extern byte g235_ThingDataWordCount[16];

class DungeonMan {
	DMEngine *_vm;

	DungeonMan(const DungeonMan &other); // no implementation on purpose
	void operator=(const DungeonMan &rhs); // no implementation on purpose

	Square f152_getRelSquare(Direction dir, int16 stepsForward, int16 stepsRight, int16 posX, int16 posY); // @ F0152_DUNGEON_GetRelativeSquare

	void f455_decompressDungeonFile(); // @ F0455_FLOPPY_DecompressDungeon

	int16 f160_getSquareFirstThingIndex(int16 mapX, int16 mapY); // @ F0160_DUNGEON_GetSquareFirstThingIndex

	int16 f170_getRandomOrnOrdinal(bool allowed, int16 count, int16 mapX, int16 mapY, int16 modulo); // @ F0170_DUNGEON_GetRandomOrnamentOrdinal
	void f171_setSquareAspectOrnOrdinals(uint16 *aspectArray, bool leftAllowed, bool frontAllowed, bool rightAllowed, int16 dir,
										 int16 mapX, int16 mapY, bool isFakeWall); // @ F0171_DUNGEON_SetSquareAspectRandomWallOrnamentOrdinals


public:
	explicit DungeonMan(DMEngine *dmEngine);
	~DungeonMan();

	Square f151_getSquare(int16 mapX, int16 mapY); // @ F0151_DUNGEON_GetSquare
	void f173_setCurrentMap(uint16 mapIndex); // @ F0173_DUNGEON_SetCurrentMap
	Thing f161_getSquareFirstThing(int16 mapX, int16 mapY); // @ F0161_DUNGEON_GetSquareFirstThing
	Thing f159_getNextThing(Thing thing); // @ F0159_DUNGEON_GetNextThing(THING P0280_T_Thing)
	uint16 *f156_getThingData(Thing thing); // @ F0156_DUNGEON_GetThingData
	uint16 *f157_getSquareFirstThingData(int16 mapX, int16 mapY); // @ F0157_DUNGEON_GetSquareFirstThingData

	// TODO: this does stuff other than load the file!
	void f434_loadDungeonFile(Common::InSaveFile *file);	// @ F0434_STARTEND_IsLoadDungeonSuccessful_CPSC
	void f174_setCurrentMapAndPartyMap(uint16 mapIndex); // @ F0174_DUNGEON_SetCurrentMapAndPartyMap

	bool f149_isWallOrnAnAlcove(int16 wallOrnIndex); // @ F0149_DUNGEON_IsWallOrnamentAnAlcove
	void f150_mapCoordsAfterRelMovement(Direction dir, int16 stepsForward, int16 stepsRight, int16 &posX, int16 &posY); // @ F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement
	SquareType f153_getRelSquareType(Direction dir, int16 stepsForward, int16 stepsRight, int16 posX, int16 posY) {
		return Square(f152_getRelSquare(dir, stepsForward, stepsRight, posX, posY)).getType();
	} // @ F0153_DUNGEON_GetRelativeSquareType
	void f172_setSquareAspect(uint16 *aspectArray, Direction dir, int16 mapX, int16 mapY); // @ F0172_DUNGEON_SetSquareAspect
	void f168_decodeText(char *destString, Thing thing, TextType type); // F0168_DUNGEON_DecodeText
	Thing f166_getUnusedThing(uint16 thingType); // @ F0166_DUNGEON_GetUnusedThing


	uint16 f140_getObjectWeight(Thing thing); // @ F0140_DUNGEON_GetObjectWeight
	int16 f141_getObjectInfoIndex(Thing thing); // @ F0141_DUNGEON_GetObjectInfoIndex
	void f163_linkThingToList(Thing thingToLink, Thing thingInList, int16 mapX, int16 mapY); // @ F0163_DUNGEON_LinkThingToList
	WeaponInfo *f158_getWeaponInfo(Thing thing); // @ F0158_DUNGEON_GetWeaponInfo
	int16 f142_getProjectileAspect(Thing thing); // @ F0142_DUNGEON_GetProjectileAspect
	int16 f154_getLocationAfterLevelChange(int16 mapIndex, int16 levelDelta, int16 *mapX, int16 *mapY); // @ F0154_DUNGEON_GetLocationAfterLevelChange
	Thing f162_getSquareFirstObject(int16 mapX, int16 mapY); // @ F0162_DUNGEON_GetSquareFirstObject
	uint16 f143_getArmourDefense(ArmourInfo *armourInfo, bool useSharpDefense); // @ F0143_DUNGEON_GetArmourDefense
	Thing f165_getDiscardTHing(uint16 thingType); // @ F0165_DUNGEON_GetDiscardedThing
	uint16 f144_getCreatureAttributes(Thing thing); // @ F0144_DUNGEON_GetCreatureAttributes
	void f146_setGroupCells(Group *group, uint16 cells, uint16 mapIndex); // @ F0146_DUNGEON_SetGroupCells
	void f148_setGroupDirections(Group *group, int16 dir, uint16 mapIndex); // @ F0148_DUNGEON_SetGroupDirections
	bool f139_isCreatureAllowedOnMap(Thing thing, uint16 mapIndex); // @ F0139_DUNGEON_IsCreatureAllowedOnMap
	void f164_unlinkThingFromList(Thing thingToUnlink, Thing thingInList, int16 mapX, int16 mapY); // @ F0164_DUNGEON_UnlinkThingFromList
	int16 f155_getStairsExitDirection(int16 mapX, int16 mapY); // @ F0155_DUNGEON_GetStairsExitDirection
	Thing f167_getObjForProjectileLaucherOrObjGen(uint16 iconIndex); // @ F0167_DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator
	int16 f169_getRandomOrnamentIndex(uint16 val1, uint16 val2, int16 modulo); // @ F0169_DUNGEON_GetRandomOrnamentIndex


	uint32 _rawDunFileDataSize;	 // @ probably NONE
	byte *_rawDunFileData; // @ ???
	DungeonFileHeader _g278_dungeonFileHeader; // @ G0278_ps_DungeonHeader


	uint16 *_g281_dungeonMapsFirstColumnIndex; // @ G0281_pui_DungeonMapsFirstColumnIndex
	uint16 _g282_dungeonColumCount; // @ G0282_ui_DungeonColumnCount
	uint16 *_g280_dungeonColumnsCumulativeSquareThingCount; // @ G0280_pui_DungeonColumnsCumulativeSquareThingCount
	Thing *_g283_squareFirstThings; // @ G0283_pT_SquareFirstThings
	uint16 *_g260_dungeonTextData; // @ G0260_pui_DungeonTextData
	uint16 *_g284_thingData[16]; // @ G0284_apuc_ThingData
	byte ***_g279_dungeonMapData; // @ G0279_pppuc_DungeonMapData


	Direction _g308_partyDir; // @ G0308_i_PartyDirection
	int16 _g306_partyMapX; // @ G0306_i_PartyMapX
	int16 _g307_partyMapY; // @ G0307_i_PartyMapY
	uint8 _g309_partyMapIndex; // @ G0309_i_PartyMapIndex
	int16 _g272_currMapIndex; // @ G0272_i_CurrentMapIndex
	byte **_g271_currMapData; // @ G0271_ppuc_CurrentMapData
	Map *_g269_currMap; // @ G0269_ps_CurrentMap
	uint16 _g273_currMapWidth; // @ G0273_i_CurrentMapWidth
	uint16 _g274_currMapHeight; // @ G0274_i_CurrentMapHeight
	uint16 *_g270_currMapColCumulativeSquareFirstThingCount; // @G0270_pui_CurrentMapColumnsCumulativeSquareFirstThingCount


	Map *_g277_dungeonMaps; // @ G0277_ps_DungeonMaps
	byte *_g276_dungeonRawMapData; // @ G0276_puc_DungeonRawMapData

	int16 _g265_currMapInscriptionWallOrnIndex; // @ G0265_i_CurrentMapInscriptionWallOrnamentIndex
	Box _g291_dungeonViewClickableBoxes[6]; // G0291_aauc_DungeonViewClickableBoxes
	bool _g286_isFacingAlcove; // @ G0286_B_FacingAlcove
	bool _g287_isFacingViAltar; // @ G0287_B_FacingViAltar
	bool _g288_isFacingFountain; // @ G0288_B_FacingFountain
	ElementType _g285_squareAheadElement; // @ G0285_i_SquareAheadElement 
	Thing _g292_pileTopObject[5]; // @ G0292_aT_PileTopObject
	DoorInfo _g275_currMapDoorInfo[2]; // @ G0275_as_CurrentMapDoorInfo
};

}

#endif
