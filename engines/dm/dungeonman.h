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

#ifndef DM_DUNGEONMAN_H
#define DM_DUNGEONMAN_H

#include "dm/dm.h"
#include "dm/gfx.h"

namespace DM {

/* Object info */
enum ObjectInfoIndexConst {
	kDMObjectInfoIndexFirstScroll = 0, // @ C000_OBJECT_INFO_INDEX_FIRST_SCROLL
	kDMObjectInfoIndexFirstContainer = 1, // @ C001_OBJECT_INFO_INDEX_FIRST_CONTAINER
	kDMObjectInfoIndexFirstPotion = 2, // @ C002_OBJECT_INFO_INDEX_FIRST_POTION
	kDMObjectInfoIndexFirstWeapon = 23, // @ C023_OBJECT_INFO_INDEX_FIRST_WEAPON
	kDMObjectInfoIndexFirstArmour = 69, // @ C069_OBJECT_INFO_INDEX_FIRST_ARMOUR
	kDMObjectInfoIndexFirstJunk = 127 // @ C127_OBJECT_INFO_INDEX_FIRST_JUNK
};

#define kDMMapXNotOnASquare -1 // @ CM1_MAPX_NOT_ON_A_SQUARE

enum ElementType {
	kDMElementTypeChampion = -2, // @ CM2_ELEMENT_CHAMPION /* Values -2 and -1 are only used as projectile impact types */
	kDMElementTypeCreature = -1, // @ CM1_ELEMENT_CREATURE
	kDMElementTypeWall = 0, // @ C00_ELEMENT_WALL /* Values 0-6 are used as square types and projectile impact types. Values 0-2 and 5-6 are also used for square aspect */
	kDMElementTypeCorridor = 1, // @ C01_ELEMENT_CORRIDOR
	kDMElementTypePit = 2, // @ C02_ELEMENT_PIT
	kDMElementTypeStairs = 3, // @ C03_ELEMENT_STAIRS
	kDMElementTypeDoor = 4, // @ C04_ELEMENT_DOOR
	kDMElementTypeTeleporter = 5, // @ C05_ELEMENT_TELEPORTER
	kDMElementTypeFakeWall = 6, // @ C06_ELEMENT_FAKEWALL
	kDMElementTypeDoorSide = 16, // @ C16_ELEMENT_DOOR_SIDE /* Values 16-19 are only used for square aspect */
	kDMElementTypeDoorFront = 17, // @ C17_ELEMENT_DOOR_FRONT
	kDMElementTypeStairsSide = 18, // @ C18_ELEMENT_STAIRS_SIDE
	kDMElementTypeStairsFront = 19  // @ C19_ELEMENT_STAIRS_FRONT
};

enum ObjectAllowedSlot {
	kDMMaskMouth = 0x0001, // @ MASK0x0001_MOUTH
	kDMMaskHead = 0x0002, // @ MASK0x0002_HEAD
	kDMMaskNeck = 0x0004, // @ MASK0x0004_NECK
	kDMMaskTorso = 0x0008, // @ MASK0x0008_TORSO
	kDMMaskLegs = 0x0010, // @ MASK0x0010_LEGS
	kDMMaskFeet = 0x0020, // @ MASK0x0020_FEET
	kDMMaskQuiverLine1 = 0x0040, // @ MASK0x0040_QUIVER_LINE1
	kDMMaskQuiverLine2 = 0x0080, // @ MASK0x0080_QUIVER_LINE2
	kDMMaskPouchPassAndThroughDoors = 0x0100, // @ MASK0x0100_POUCH_PASS_AND_THROUGH_DOORS
	kDMMaskHands = 0x0200, // @ MASK0x0200_HANDS
	kDMMaskContainer = 0x0400, // @ MASK0x0400_CONTAINER
	kDMMaskFootprints = 0x8000, // @ MASK0x8000_FOOTPRINTS
	kDMMaskRandomDrop = 0x8000 // @ MASK0x8000_RANDOM_DROP
};

enum ArmourAttribute {
	kDMArmourAttributeShield = 0x0080, // @ MASK0x0080_IS_A_SHIELD
	kDMArmourAttributeSharpDefense = 0x0007 // @ MASK0x0007_SHARP_DEFENSE
};

enum WeaponClass {
	kDMWeaponClassNone = -1,
/* Class 0: SWING weapons */
	kDMWeaponClassSwingWeapon = 0, // @ C000_CLASS_SWING_WEAPON
/* Class 1 to 15: THROW weapons */
	kDMWeaponClassDaggerAndAxes = 2, // @ C002_CLASS_DAGGER_AND_AXES
	kDMWeaponClassBowAmmunition = 10, // @ C010_CLASS_BOW_AMMUNITION
	kDMWeaponClassSlingAmmunition = 11, // @ C011_CLASS_SLING_AMMUNITION
	kDMWeaponClassPoisinDart = 12, // @ C012_CLASS_POISON_DART
/* Class 16 to 111: SHOOT weapons */
	kDMWeaponClassFirstBow = 16, // @ C016_CLASS_FIRST_BOW
	kDMWeaponClassLastBow = 31, // @ C031_CLASS_LAST_BOW
	kDMWeaponClassFirstSling = 32, // @ C032_CLASS_FIRST_SLING
	kDMWeaponClassLastSling = 47, // @ C047_CLASS_LAST_SLING
/* Class 112 to 255: Magic and special weapons */
	kDMWeaponClassFirstMagicWeapon = 112 // @ C112_CLASS_FIRST_MAGIC_WEAPON
};

enum TextType {
	/* Used for text on walls */
	kDMTextTypeInscription = 0, // @ C0_TEXT_TYPE_INSCRIPTION
	/* Used for messages displayed when the party walks on a square */
	kDMTextTypeMessage = 1, // @ C1_TEXT_TYPE_MESSAGE
	/* Used for text on scrolls and champion information */
	kDMTextTypeScroll = 2 // @ C2_TEXT_TYPE_SCROLL
};

enum SquareAspect {
	kDMSquareAspectElement = 0, // @ C0_ELEMENT
	kDMSquareAspectFirstGroupOrObject = 1, // @ C1_FIRST_GROUP_OR_OBJECT
	kDMSquareAspectRightWallOrnOrd = 2, // @ C2_RIGHT_WALL_ORNAMENT_ORDINAL
	kDMSquareFrontWallOrnOrd = 3, // @ C3_FRONT_WALL_ORNAMENT_ORDINAL
	kDMSquareAspectLeftWallOrnOrd = 4, // @ C4_LEFT_WALL_ORNAMENT_ORDINAL
	kDMSquareAspectPitInvisible = 2, // @ C2_PIT_INVISIBLE
	kDMSquareAspectTeleporterVisible = 2, // @ C2_TELEPORTER_VISIBLE
	kDMSquareAspectStairsUp = 2, // @ C2_STAIRS_UP
	kDMSquareAspectDoorState = 2, // @ C2_DOOR_STATE
	kDMSquareAspectDoorThingIndex = 3, // @ C3_DOOR_THING_INDEX
	kDMSquareAspectFloorOrn = 4 // @ C4_FLOOR_ORNAMENT_ORDINAL
};

#define kDMImmuneToFire 15 // @ C15_IMMUNE_TO_FIRE
#define kDMImmuneToPoison 15 // @ C15_IMMUNE_TO_POISON

enum TeleporterScope {
	kDMTeleporterScopeCreatures = 1, // @ MASK0x0001_SCOPE_CREATURES
	kDMTeleporterScopeObjectsOrParty = 2 // @ MASK0x0002_SCOPE_OBJECTS_OR_PARTY
};

enum SensorType {
	kDMSensorDisabled = 0, // @ C000_SENSOR_DISABLED    /* Never triggered, may be used for a floor or wall ornament */
	kDMSensorFloorTheronPartyCreatureObj = 1, // @ C001_SENSOR_FLOOR_THERON_PARTY_CREATURE_OBJECT    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	kDMSensorFloorTheronPartyCreature = 2, // @ C002_SENSOR_FLOOR_THERON_PARTY_CREATURE    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	kDMSensorFloorParty = 3, // @ C003_SENSOR_FLOOR_PARTY    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	kDMSensorFloorObj = 4, // @ C004_SENSOR_FLOOR_OBJECT    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	kDMSensorFloorPartyOnStairs = 5, // @ C005_SENSOR_FLOOR_PARTY_ON_STAIRS    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	kDMSensorFloorGroupGenerator = 6, // @ C006_SENSOR_FLOOR_GROUP_GENERATOR    /* Triggered by event F0245_TIMELINE_ProcessEvent5_Square_Corridor */
	kDMSensorFloorCreature = 7, // @ C007_SENSOR_FLOOR_CREATURE    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	kDMSensorFloorPartyPossession = 8, // @ C008_SENSOR_FLOOR_PARTY_POSSESSION    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	kDMSensorFloorVersionChecker = 9, // @ C009_SENSOR_FLOOR_VERSION_CHECKER    /* Triggered by party/thing F0276_SENSOR_ProcessThingAdditionOrRemoval */
	kDMSensorWallOrnClick = 1, // @ C001_SENSOR_WALL_ORNAMENT_CLICK    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	kDMSensorWallOrnClickWithAnyObj = 2, // @ C002_SENSOR_WALL_ORNAMENT_CLICK_WITH_ANY_OBJECT    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	kDMSensorWallOrnClickWithSpecObj = 3, // @ C003_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	kDMSensorWallOrnClickWithSpecObjRemoved = 4, // @ C004_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	kDMSensorWallAndOrGate = 5, // @ C005_SENSOR_WALL_AND_OR_GATE    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	kDMSensorWallCountdown = 6, // @ C006_SENSOR_WALL_COUNTDOWN    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	kDMSensorWallSingleProjLauncherNewObj = 7, // @ C007_SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_NEW_OBJECT    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	kDMSensorWallSingleProjLauncherExplosion = 8, // @ C008_SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_EXPLOSION    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	kDMSensorWallDoubleProjLauncherNewObj = 9, // @ C009_SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_NEW_OBJECT    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	kDMSensorWallDoubleProjLauncherExplosion = 10, // @ C010_SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_EXPLOSION    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	kDMSensorWallOrnClickWithSpecObjRemovedRotateSensors = 11, // @ C011_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED_ROTATE_SENSORS   /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	kDMSensorWallObjGeneratorRotateSensors = 12, // @ C012_SENSOR_WALL_OBJECT_GENERATOR_ROTATE_SENSORS    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	kDMSensorWallSingleObjStorageRotateSensors = 13, // @ C013_SENSOR_WALL_SINGLE_OBJECT_STORAGE_ROTATE_SENSORS    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	kDMSensorWallSingleProjLauncherSquareObj = 14, // @ C014_SENSOR_WALL_SINGLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	kDMSensorWallDoubleProjLauncherSquareObj = 15, // @ C015_SENSOR_WALL_DOUBLE_PROJECTILE_LAUNCHER_SQUARE_OBJECT    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	kDMSensorWallObjExchanger = 16, // @ C016_SENSOR_WALL_OBJECT_EXCHANGER    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	kDMSensorWallOrnClickWithSpecObjRemovedSensor = 17, // @ C017_SENSOR_WALL_ORNAMENT_CLICK_WITH_SPECIFIC_OBJECT_REMOVED_REMOVE_SENSOR    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
	kDMSensorWallEndGame = 18, // @ C018_SENSOR_WALL_END_GAME    /* Triggered by event F0248_TIMELINE_ProcessEvent6_Square_Wall */
	kDMSensorWallChampionPortrait = 127 // @ C127_SENSOR_WALL_CHAMPION_PORTRAIT    /* Triggered by player click F0275_SENSOR_IsTriggeredByClickOnWall */
};

enum WeaponType {
	kDMWeaponTorch = 2, // @ C02_WEAPON_TORCH
	kDMWeaponDagger = 8, // @ C08_WEAPON_DAGGER
	kDMWeaponFalchion = 9, // @ C09_WEAPON_FALCHION
	kDMWeaponSword = 10, // @ C10_WEAPON_SWORD
	kDMWeaponClub = 23, // @ C23_WEAPON_CLUB
	kDMWeaponStoneClub = 24, // @ C24_WEAPON_STONE_CLUB
	kDMWeaponArrow = 27, // @ C27_WEAPON_ARROW
	kDMWeaponSlayer = 28, // @ C28_WEAPON_SLAYER
	kDMWeaponRock = 30, // @ C30_WEAPON_ROCK
	kDMWeaponPoisonDart = 31, // @ C31_WEAPON_POISON_DART
	kDMWeaponThrowingStar = 32 // @ C32_WEAPON_THROWING_STAR
};

enum ArmourType {
	kDMArmourWoodenShield = 30, // @ C30_ARMOUR_WOODEN_SHIELD
	kDMArmourArmet = 38, // @ C38_ARMOUR_ARMET
	kDMArmourTorsoPlate = 39, // @ C39_ARMOUR_TORSO_PLATE
	kDMArmourLegPlate = 40, // @ C40_ARMOUR_LEG_PLATE
	kDMArmourFootPlate = 41 // @ C41_ARMOUR_FOOT_PLATE
};

enum PotionType {
	kDMPotionTypeVen = 3, // @ C03_POTION_VEN_POTION,
	kDMPotionTypeRos = 6, // @ C06_POTION_ROS_POTION,
	kDMPotionTypeKu = 7, // @ C07_POTION_KU_POTION,
	kDMPotionTypeDane = 8, // @ C08_POTION_DANE_POTION,
	kDMPotionTypeNeta = 9, // @ C09_POTION_NETA_POTION,
	kDMPotionTypeAntivenin = 10, // @ C10_POTION_ANTIVENIN,
	kDMPotionTypeMon = 11, // @ C11_POTION_MON_POTION,
	kDMPotionTypeYa = 12, // @ C12_POTION_YA_POTION,
	kDMPotionTypeEe = 13, // @ C13_POTION_EE_POTION,
	kDMPotionTypeVi = 14, // @ C14_POTION_VI_POTION,
	kDMPotionTypeWaterFlask = 15, // @ C15_POTION_WATER_FLASK,
	kDMPotionTypeFulBomb = 19, // @ C19_POTION_FUL_BOMB,
	kDMPotionTypeEmptyFlask = 20 // @ C20_POTION_EMPTY_FLASK,
};

enum JunkType {
	kDMJunkTypeWaterskin = 1, // @ C01_JUNK_WATERSKIN,
	kDMJunkTypeBones = 5, // @ C05_JUNK_BONES,
	kDMJunkTypeBoulder = 25, // @ C25_JUNK_BOULDER,
	kDMJunkTypeScreamerSlice = 33, // @ C33_JUNK_SCREAMER_SLICE,
	kDMJunkTypeWormRound = 34, // @ C34_JUNK_WORM_ROUND,
	kDMJunkTypeDrumstickShank = 35, // @ C35_JUNK_DRUMSTICK_SHANK,
	kDMJunkTypeDragonSteak = 36, // @ C36_JUNK_DRAGON_STEAK,
	kDMJunkTypeMagicalBoxBlue = 42, // @ C42_JUNK_MAGICAL_BOX_BLUE,
	kDMJunkTypeMagicalBoxGreen = 43, // @ C43_JUNK_MAGICAL_BOX_GREEN,
	kDMJunkTypeZokathra = 51 // @ C51_JUNK_ZOKATHRA,
};

enum ExplosionType {
	kDMExplosionTypeFireball = 0, // @ C000_EXPLOSION_FIREBALL
	kDMExplosionTypeSlime = 1, // @ C001_EXPLOSION_SLIME
	kDMExplosionTypeLightningBolt = 2, // @ C002_EXPLOSION_LIGHTNING_BOLT
	kDMExplosionTypeHarmNonMaterial = 3, // @ C003_EXPLOSION_HARM_NON_MATERIAL
	kDMExplosionTypeOpenDoor = 4, // @ C004_EXPLOSION_OPEN_DOOR
	kDMExplosionTypePoisonBolt = 6, // @ C006_EXPLOSION_POISON_BOLT
	kDMExplosionTypePoisonCloud = 7, // @ C007_EXPLOSION_POISON_CLOUD
	kDMExplosionTypeSmoke = 40, // @ C040_EXPLOSION_SMOKE
	kDMExplosionTypeFluxcage = 50, // @ C050_EXPLOSION_FLUXCAGE
	kDMExplosionTypeRebirthStep1 = 100, // @ C100_EXPLOSION_REBIRTH_STEP1
	kDMExplosionTypeRebirthStep2 = 101 // @ C101_EXPLOSION_REBIRTH_STEP2
};

enum SquareMask {
	kDMSquareMaskWallWestRandOrnament = 0x1, // @ MASK0x0001_WALL_WEST_RANDOM_ORNAMENT_ALLOWED
	kDMSquareMaslWallSouthRandOrnament = 0x2, // @ MASK0x0002_WALL_SOUTH_RANDOM_ORNAMENT_ALLOWED
	kDMSquareMaskWallEastRandOrnament = 0x4, // @ MASK0x0004_WALL_EAST_RANDOM_ORNAMENT_ALLOWED
	kDMSquareMaskWallNorthRandOrnament = 0x8, // @ MASK0x0008_WALL_NORTH_RANDOM_ORNAMENT_ALLOWED
	kDMSquareMaskCorridorRandOrnament = 0x8, // @ MASK0x0008_CORRIDOR_RANDOM_ORNAMENT_ALLOWED
	kDMSquareMaskPitImaginary = 0x1, // @ MASK0x0001_PIT_IMAGINARY
	kDMSquareMaskPitInvisible = 0x4, // @ MASK0x0004_PIT_INVISIBLE
	kDMSquareMaskPitOpen = 0x8, // @ MASK0x0008_PIT_OPEN
	kDMSquareMaskStairsUp = 0x4, // @ MASK0x0004_STAIRS_UP
	kDMSquareMaskStairsNorthSouth = 0x8, // @ MASK0x0008_STAIRS_NORTH_SOUTH_ORIENTATION
	kDMSquareMaskDoorNorthSouth = 0x8, // @ MASK0x0008_DOOR_NORTH_SOUTH_ORIENTATION
	kDMSquareMaskTeleporterVisible = 0x4, // @ MASK0x0004_TELEPORTER_VISIBLE
	kDMSquareMaskTeleporterOpen = 0x8, // @ MASK0x0008_TELEPORTER_OPEN
	kDMSquareMaskFakeWallImaginary = 0x1, // @ MASK0x0001_FAKEWALL_IMAGINARY
	kDMSquareMaskFakeWallOpen = 0x4, // @ MASK0x0004_FAKEWALL_OPEN
	kDMSquareMaskFakeWallRandOrnamentOrFootprintsAllowed = 0x8, // @ MASK0x0008_FAKEWALL_RANDOM_ORNAMENT_OR_FOOTPRINTS_ALLOWED
	kDMSquareMaskThingListPresent = 0x10 // @ MASK0x0010_THING_LIST_PRESENT
};

#define kDMMaskChampionBones 0x8000 // @ MASK0x8000_CHAMPION_BONES
#define kDMMaskThingType 0x7FFF // @ MASK0x7FFF_THING_TYPE

class ObjectInfoIndex {
public:
	int16 _type;
	uint16 _objectAspectIndex;
	uint16 _actionSetIndex;
	uint16 _allowedSlots;
	ObjectInfoIndex(int16 type, uint16 objectAspectIndex, uint16 actionSetIndex, uint16 allowedSlots)
		: _type(type), _objectAspectIndex(objectAspectIndex), _actionSetIndex(actionSetIndex), _allowedSlots(allowedSlots) {}
	ObjectInfoIndex() : _type(0), _objectAspectIndex(0), _actionSetIndex(0), _allowedSlots(0) {}
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

class ArmourInfo {
public:
	uint16 _weight;
	uint16 _defense;
	uint16 _attributes;

	ArmourInfo(uint16 weight, uint16 defense, uint16 attributes)
		:_weight(weight), _defense(defense), _attributes(attributes) {}
	ArmourInfo() :_weight(0), _defense(0), _attributes(0) {}

	uint16 getAttribute(ArmourAttribute attribute) { return _attributes & attribute; }
	void setAttribute(ArmourAttribute attribute) { _attributes |= attribute; }
}; // @ ARMOUR_INFO

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
	WeaponInfo() : _weight(0), _class(0), _strength(0), _kineticEnergy(0), _attributes(0) {}

	uint16 getShootAttack() { return _attributes & 0xFF; } // @ M65_SHOOT_ATTACK
	uint16 getProjectileAspectOrdinal() { return (_attributes >> 8) & 0x1F; } // @ M66_PROJECTILE_ASPECT_ORDINAL
}; // @ WEAPON_INFO

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

	uint16 getFearResistance() { return (_properties >> 4) & 0xF; }
	uint16 getExperience() { return (_properties >> 8) & 0xF; }
	uint16 getWariness() { return (_properties >> 12) & 0xF; }
	uint16 getFireResistance() { return (_resistances >> 4) & 0xF; }
	uint16 getPoisonResistance() { return (_resistances >> 8) & 0xF; }
	static uint16 getHeight(uint16 attrib) { return (attrib >> 7) & 0x3; }
	uint16 getSightRange() { return (_ranges) & 0xF; }
	uint16 getSmellRange() { return  (_ranges >> 8) & 0xF; }
	uint16 getAttackRange() { return (_ranges >> 12) & 0xF; }
}; // @ CREATURE_INFO

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

	bool getAttrOnlyOnce() { return (_attributes >> 2) & 1; }
	uint16 getAttrEffectA() { return (_attributes >> 3) & 0x3; }
	bool getAttrRevertEffectA() { return (_attributes >> 5) & 0x1; }
	bool getAttrAudibleA() { return (_attributes >> 6) & 0x1; }
	uint16 getAttrValue() { return (_attributes >> 7) & 0xF; }
	bool getAttrLocalEffect() { return (_attributes >> 11) & 1; }
	uint16 getAttrOrnOrdinal() { return _attributes >> 12; }

	uint16 getActionTargetMapY() { return (_action >> 11); }
	uint16 getActionTargetMapX() { return (_action >> 6) & 0x1F; }
	Cell getActionTargetCell() { return (Cell)((_action >> 4) & 3); }
	uint16 getActionHealthMultiplier() { return ((_action >> 4) & 0xF); } // @ M45_HEALTH_MULTIPLIER
	uint16 getActionTicks() { return ((_action >> 4) >> 4) & 0xFFF; } // @ M46_TICKS
	uint16 getActionKineticEnergy() { return ((_action >> 4) & 0xFF); }// @ M47_KINETIC_ENERGY
	uint16 getActionStepEnergy() { return ((_action >> 4) >> 8) & 0xFF; }// @ M48_STEP_ENERGY
	uint16 getActionLocalEffect() { return (_action >> 4); } // @ M49_LOCAL_EFFECT

	void setDatAndTypeWithOr(uint16 val) { _datAndType |= val;  }

}; // @ SENSOR

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

class Square {
	byte _data;
public:
	explicit Square(byte dat = 0) : _data(dat) {}
	explicit Square(ElementType type) { setType(type); }
	explicit Square(byte element, byte mask) : _data((element << 5) | mask) {}
	Square &set(byte dat) { this->_data = dat; return *this; }
	Square &set(SquareMask mask) { _data |= mask; return *this; }
	byte get(SquareMask mask) { return _data & mask; }
	byte getDoorState() { return _data & 0x7; } // @ M36_DOOR_STATE
	void setDoorState(byte state) { _data = ((_data & ~0x7) | state); } // @ M37_SET_DOOR_STATE
	ElementType getType() { return (ElementType)(_data >> 5); } // @ M34_SQUARE_TYPE
	void setType(ElementType type) { _data = (_data & 0x1F) | type << 5; }
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

class DungeonMan {
	DMEngine *_vm;

	DungeonMan(const DungeonMan &other); // no implementation on purpose
	void operator=(const DungeonMan &rhs); // no implementation on purpose

	Square getRelSquare(Direction dir, int16 stepsForward, int16 stepsRight, int16 posX, int16 posY); // @ F0152_DUNGEON_GetRelativeSquare

	void decompressDungeonFile(); // @ F0455_FLOPPY_DecompressDungeon

	int16 getSquareFirstThingIndex(int16 mapX, int16 mapY); // @ F0160_DUNGEON_GetSquareFirstThingIndex

	int16 getRandomOrnOrdinal(bool allowed, int16 count, int16 mapX, int16 mapY, int16 modulo); // @ F0170_DUNGEON_GetRandomOrnamentOrdinal
	void setSquareAspectOrnOrdinals(uint16 *aspectArray, bool leftAllowed, bool frontAllowed, bool rightAllowed, int16 dir,
										 int16 mapX, int16 mapY, bool isFakeWall); // @ F0171_DUNGEON_SetSquareAspectRandomWallOrnamentOrdinals

public:
	explicit DungeonMan(DMEngine *dmEngine);
	~DungeonMan();

	Square getSquare(int16 mapX, int16 mapY); // @ F0151_DUNGEON_GetSquare
	void setCurrentMap(uint16 mapIndex); // @ F0173_DUNGEON_SetCurrentMap
	Thing getSquareFirstThing(int16 mapX, int16 mapY); // @ F0161_DUNGEON_GetSquareFirstThing
	Thing getNextThing(Thing thing); // @ F0159_DUNGEON_GetNextThing(THING P0280_T_Thing)
	uint16 *getThingData(Thing thing); // @ F0156_DUNGEON_GetThingData
	uint16 *getSquareFirstThingData(int16 mapX, int16 mapY); // @ F0157_DUNGEON_GetSquareFirstThingData

	// TODO: this does stuff other than load the file!
	void loadDungeonFile(Common::InSaveFile *file);	// @ F0434_STARTEND_IsLoadDungeonSuccessful_CPSC
	void setCurrentMapAndPartyMap(uint16 mapIndex); // @ F0174_DUNGEON_SetCurrentMapAndPartyMap

	bool isWallOrnAnAlcove(int16 wallOrnIndex); // @ F0149_DUNGEON_IsWallOrnamentAnAlcove
	void mapCoordsAfterRelMovement(Direction dir, int16 stepsForward, int16 stepsRight, int16 &posX, int16 &posY); // @ F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement
	ElementType getRelSquareType(Direction dir, int16 stepsForward, int16 stepsRight, int16 posX, int16 posY) {
		return Square(getRelSquare(dir, stepsForward, stepsRight, posX, posY)).getType();
	} // @ F0153_DUNGEON_GetRelativeSquareType
	void setSquareAspect(uint16 *aspectArray, Direction dir, int16 mapX, int16 mapY); // @ F0172_DUNGEON_SetSquareAspect
	void decodeText(char *destString, Thing thing, TextType type); // F0168_DUNGEON_DecodeText
	Thing getUnusedThing(uint16 thingType); // @ F0166_DUNGEON_GetUnusedThing

	uint16 getObjectWeight(Thing thing); // @ F0140_DUNGEON_GetObjectWeight
	int16 getObjectInfoIndex(Thing thing); // @ F0141_DUNGEON_GetObjectInfoIndex
	void linkThingToList(Thing thingToLink, Thing thingInList, int16 mapX, int16 mapY); // @ F0163_DUNGEON_LinkThingToList
	WeaponInfo *getWeaponInfo(Thing thing); // @ F0158_DUNGEON_GetWeaponInfo
	int16 getProjectileAspect(Thing thing); // @ F0142_DUNGEON_GetProjectileAspect
	int16 getLocationAfterLevelChange(int16 mapIndex, int16 levelDelta, int16 *mapX, int16 *mapY); // @ F0154_DUNGEON_GetLocationAfterLevelChange
	Thing getSquareFirstObject(int16 mapX, int16 mapY); // @ F0162_DUNGEON_GetSquareFirstObject
	uint16 getArmourDefense(ArmourInfo *armourInfo, bool useSharpDefense); // @ F0143_DUNGEON_GetArmourDefense
	Thing getDiscardThing(uint16 thingType); // @ F0165_DUNGEON_GetDiscardedThing
	uint16 getCreatureAttributes(Thing thing); // @ F0144_DUNGEON_GetCreatureAttributes
	void setGroupCells(Group *group, uint16 cells, uint16 mapIndex); // @ F0146_DUNGEON_SetGroupCells
	void setGroupDirections(Group *group, int16 dir, uint16 mapIndex); // @ F0148_DUNGEON_SetGroupDirections
	bool isCreatureAllowedOnMap(Thing thing, uint16 mapIndex); // @ F0139_DUNGEON_IsCreatureAllowedOnMap
	void unlinkThingFromList(Thing thingToUnlink, Thing thingInList, int16 mapX, int16 mapY); // @ F0164_DUNGEON_UnlinkThingFromList
	int16 getStairsExitDirection(int16 mapX, int16 mapY); // @ F0155_DUNGEON_GetStairsExitDirection
	Thing getObjForProjectileLaucherOrObjGen(uint16 iconIndex); // @ F0167_DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator
	int16 getRandomOrnamentIndex(uint16 val1, uint16 val2, int16 modulo); // @ F0169_DUNGEON_GetRandomOrnamentIndex

	uint32 _rawDunFileDataSize;	 // @ probably NONE
	byte *_rawDunFileData; // @ ???
	DungeonFileHeader _dungeonFileHeader; // @ G0278_ps_DungeonHeader

	uint16 *_dungeonMapsFirstColumnIndex; // @ G0281_pui_DungeonMapsFirstColumnIndex
	uint16 _dungeonColumCount; // @ G0282_ui_DungeonColumnCount
	uint16 *_dungeonColumnsCumulativeSquareThingCount; // @ G0280_pui_DungeonColumnsCumulativeSquareThingCount
	Thing *_squareFirstThings; // @ G0283_pT_SquareFirstThings
	uint16 *_dungeonTextData; // @ G0260_pui_DungeonTextData
	uint16 *_thingData[16]; // @ G0284_apuc_ThingData
	byte ***_dungeonMapData; // @ G0279_pppuc_DungeonMapData

	Direction _partyDir; // @ G0308_i_PartyDirection
	int16 _partyMapX; // @ G0306_i_PartyMapX
	int16 _partyMapY; // @ G0307_i_PartyMapY
	uint8 _partyMapIndex; // @ G0309_i_PartyMapIndex
	int16 _currMapIndex; // @ G0272_i_CurrentMapIndex
	byte **_currMapData; // @ G0271_ppuc_CurrentMapData
	Map *_currMap; // @ G0269_ps_CurrentMap
	uint16 _currMapWidth; // @ G0273_i_CurrentMapWidth
	uint16 _currMapHeight; // @ G0274_i_CurrentMapHeight
	uint16 *_currMapColCumulativeSquareFirstThingCount; // @G0270_pui_CurrentMapColumnsCumulativeSquareFirstThingCount

	Map *_dungeonMaps; // @ G0277_ps_DungeonMaps
	byte *_dungeonRawMapData; // @ G0276_puc_DungeonRawMapData

	int16 _currMapInscriptionWallOrnIndex; // @ G0265_i_CurrentMapInscriptionWallOrnamentIndex
	Box _dungeonViewClickableBoxes[6]; // G0291_aauc_DungeonViewClickableBoxes
	bool _isFacingAlcove; // @ G0286_B_FacingAlcove
	bool _isFacingViAltar; // @ G0287_B_FacingViAltar
	bool _isFacingFountain; // @ G0288_B_FacingFountain
	ElementType _squareAheadElement; // @ G0285_i_SquareAheadElement
	Thing _pileTopObject[5]; // @ G0292_aT_PileTopObject
	DoorInfo _currMapDoorInfo[2]; // @ G0275_as_CurrentMapDoorInfo

	ObjectInfoIndex _objectInfos[180]; // @ G0237_as_Graphic559_ObjectInfo
	ArmourInfo _armourInfos[58]; // @ G0239_as_Graphic559_ArmourInfo
	WeaponInfo _weaponInfos[46]; // @ G0238_as_Graphic559_WeaponInfo
	CreatureInfo _creatureInfos[k27_CreatureTypeCount]; // @ G0243_as_Graphic559_CreatureInfo
	byte _thingDataWordCount[16]; // @ G0235_auc_Graphic559_ThingDataByteCount

	void setupConstants();
};

}

#endif
