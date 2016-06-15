#ifndef DUNGEONMAN_H
#define DUNGEONMAN_H

#include "dm.h"
#include "dungeonman.h"
#include "gfx.h"


namespace DM {


int16 ordinalToIndex(int16 val); // @ M01_ORDINAL_TO_INDEX
int16 indexToOrdinal(int16 val); // @ M00_INDEX_TO_ORDINAL


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
	byte creatureAspectIndex;
	byte attackSoundOrdinal;
	uint16 attributes; /* Bits 15-14 Unreferenced */
	uint16 graphicInfo; /* Bits 11 and 6 Unreferenced */
	byte movementTicks; /* Value 255 means the creature cannot move */
	byte attackTicks; /* Minimum ticks between attacks */
	byte defense;
	byte baseHealth;
	byte attack;
	byte poisonAttack;
	byte dexterity;
	uint16 Ranges; /* Bits 7-4 Unreferenced */
	uint16 Properties;
	uint16 Resistances; /* Bits 15-12 and 3-0 Unreferenced */
	uint16 AnimationTicks; /* Bits 15-12 Unreferenced */
	uint16 WoundProbabilities; /* Contains 4 probabilities to wound a champion's Head (Bits 15-12), Legs (Bits 11-8), Torso (Bits 7-4) and Feet (Bits 3-0) */
	byte AttackType;
}; // @ CREATURE_INFO


extern CreatureInfo gCreatureInfo[kCreatureTypeCount];

class Door {
	Thing nextThing;
	uint16 attributes;
public:
	Door(uint16 *rawDat) : nextThing(rawDat[0]), attributes(rawDat[1]) {}
	Thing getNextThing() { return nextThing; }
	bool isMeleeDestructible() { return (attributes >> 8) & 1; }
	bool isMagicDestructible() { return (attributes >> 7) & 1; }
	bool hasButton() { return (attributes >> 6) & 1; }
	bool opensVertically() { return (attributes >> 5) & 1; }
	byte getOrnOrdinal() { return (attributes >> 1) & 0xF; }
	byte getType() { return attributes & 1; }
}; // @ DOOR

enum TeleporterScope {
	kTelepScopeCreatures = 1, // @ MASK0x0001_SCOPE_CREATURES
	kTelepScopeObjOrParty = 2 // @ MASK0x0002_SCOPE_OBJECTS_OR_PARTY
};


class Teleporter {
	Thing nextThing;
	uint16 attributes;
	uint16 destMapIndex;
public:
	Teleporter(uint16 *rawDat) : nextThing(rawDat[0]), attributes(rawDat[1]), destMapIndex(rawDat[2]) {}
	Thing getNextThing() { return nextThing; }
	bool makesSound() { return (attributes >> 15) & 1; }
	TeleporterScope getScope() { return (TeleporterScope)((attributes >> 13) & 1); }
	bool absRotation() { return (attributes >> 12) & 1; }
	direction getRotationDir() { return (direction)((attributes >> 10) & 1); }
	byte getDestY() { return (attributes >> 5) & 0xF; }
	byte getDestX() { return attributes & 0xF; }
	uint16 getDestMapIndex() { return destMapIndex >> 8; }
}; // @ TELEPORTER



class TextString {
	Thing nextThing;
	uint16 textDataRef;
public:
	TextString(uint16 *rawDat) : nextThing(rawDat[0]), textDataRef(rawDat[1]) {}

	Thing getNextThing() { return nextThing; }
	uint16 getWordOffset() { return textDataRef >> 3; }
	bool isVisible() { return textDataRef & 1; }
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
	Thing nextThing;
	uint16 datAndType;
	uint16 attributes;
	uint16 action;
public:
	Sensor(uint16 *rawDat) : nextThing(rawDat[0]), datAndType(rawDat[1]), attributes(rawDat[2]), action(rawDat[3]) {}

	Thing getNextThing() { return nextThing; }
	SensorType getType() { return (SensorType)(datAndType & 0x7F); } // @ M39_TYPE
	uint16 getData() { return datAndType >> 7; } // @ M40_DATA
	uint16 getDataMask1() { return (datAndType >> 7) & 0xF; } // @ M42_MASK1
	uint16 getDataMask2() { return (datAndType >> 11) & 0xF; } // @ M43_MASK2
	void setData(int16 dat) { datAndType = (datAndType & 0x7F) | (dat << 7); } // @ M41_SET_DATA
	void setTypeDisabled() { datAndType &= 0xFF80; } // @ M44_SET_TYPE_DISABLED
	uint16 getOrnOrdinal() { return attributes >> 12; }
	bool isLocalAction() { return (attributes >> 11) & 1; }
	uint16 getDelay() { return (attributes >> 7) & 0xF; }
	bool hasSound() { return (attributes >> 6) & 1; }
	bool shouldRevert() { return (attributes >> 5) & 1; }
	SensorActionType getActionType() { return (SensorActionType)((attributes >> 3) & 3); }
	bool isSingleUse() { return (attributes >> 2) & 1; }
	uint16 getRemoteMapY() { return (action >> 11); }
	uint16 getRemoteMapX() { return (action >> 6) & 0x1F; }
	direction getRemoteDir() { return (direction)((action >> 4) & 3); }
	uint16 getLocalAction() { return (action >> 4); }
	// some macros missing, i got bored
}; // @ SENSOR

class Group {
	Thing nextThing;
	Thing possessionID;
	byte type;
	byte position;
	uint16 health[4];
	uint16 attributes;
public:
	Group(uint16 *rawDat) : nextThing(rawDat[0]), possessionID(rawDat[1]), type(rawDat[2]),
		position(rawDat[3]), attributes(rawDat[8]) {
		health[0] = rawDat[4];
		health[1] = rawDat[5];
		health[2] = rawDat[6];
		health[3] = rawDat[7];
	}
	Thing getNextThing() { return nextThing; }
}; // @ GROUP

class Weapon {
	Thing nextThing;
	uint16 desc;
public:
	Weapon(uint16 *rawDat) : nextThing(rawDat[0]), desc(rawDat[1]) {}

	Thing getNextThing() { return nextThing; }
}; // @ WEAPON

class Armour {
	Thing nextThing;
	uint16 attributes;
public:
	Armour(uint16 *rawDat) : nextThing(rawDat[0]), attributes(rawDat[1]) {}

	Thing getNextThing() { return nextThing; }
}; // @ ARMOUR

class Scroll {
	Thing nextThing;
	uint16 attributes;
public:
	Scroll(uint16 *rawDat) : nextThing(rawDat[0]), attributes(rawDat[1]) {}
	void set(Thing next, uint16 attribs) {
		nextThing = next;
		attributes = attribs;
	}
	Thing getNextThing() { return nextThing; }
}; // @ SCROLL

class Potion {
	Thing nextThing;
	uint16 attributes;
public:
	Potion(uint16 *rawDat) : nextThing(rawDat[0]), attributes(rawDat[1]) {}

	Thing getNextThing() { return nextThing; }
}; // @ POTION

class Container {
	Thing nextThing;
	Thing nextContainedThing;
	uint16 type;
public:
	Container(uint16 *rawDat) : nextThing(rawDat[0]), nextContainedThing(rawDat[1]), type(rawDat[2]) {}

	Thing getNextThing() { return nextThing; }
}; // @ CONTAINER

class Junk {
	Thing nextThing;
	uint16 attributes;
public:
	Junk(uint16 *rawDat) : nextThing(rawDat[0]), attributes(rawDat[1]) {}

	Thing getNextThing() { return nextThing; }
}; // @ JUNK

class Projectile {
	Thing nextThing;
	Thing object;
	byte kineticEnergy;
	byte damageEnergy;
	uint16 timerIndex;
public:
	Projectile(uint16 *rawDat) : nextThing(rawDat[0]), object(rawDat[1]), kineticEnergy(rawDat[2]),
		damageEnergy(rawDat[3]), timerIndex(rawDat[4]) {}

	Thing getNextThing() { return nextThing; }
}; // @ PROJECTILE

class Explosion {
	Thing nextThing;
	uint16 attributes;
public:
	Explosion(uint16 *rawDat) : nextThing(rawDat[0]), attributes(rawDat[1]) {}

	Thing getNextThing() { return nextThing; }
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
	byte data;
public:
	Square(byte dat = 0) : data(dat) {}
	Square(SquareType type) { setType(type); }
	Square &set(byte dat) { this->data = dat; return *this; }
	Square &set(SquareMask mask) { data |= mask; return *this; }
	byte get(SquareMask mask) { return data & mask; }
	byte getDoorState() { return data & 0x7; } // @ M36_DOOR_STATE
	Square &setDoorState(byte state) { data = ((data & ~0x7) | state); } // @ M37_SET_DOOR_STATE
	SquareType getType() { return (SquareType)(data >> 5); } // @ M34_SQUARE_TYPE
	Square &setType(SquareType type) { data = (data & 0x1F) | type << 5; return *this; }
	byte toByte() { return data; } // I don't like 'em casts
};

struct DungeonFileHeader {
	uint16 dungeonId; // @ G0526_ui_DungeonID
	// equal to dungeonId
	uint16 ornamentRandomSeed;
	uint32 rawMapDataSize;
	uint8 mapCount;
	uint16 textDataWordCount;
	direction partyStartDir; // @ InitialPartyLocation
	uint16 partyStartPosX, partyStartPosY;
	uint16 squareFirstThingCount; // @ SquareFirstThingCount
	uint16 thingCounts[16]; // @ ThingCount[16]
}; // @ DUNGEON_HEADER

struct Map {
	uint32 rawDunDataOffset;
	uint8 offsetMapX, offsetMapY;

	uint8 level; // only used in DMII
	uint8 width, height; // !!! THESRE ARE INCLUSIVE BOUNDARIES
	// orn short for Ornament
	uint8 wallOrnCount; /* May be used in a Sensor on a Wall or closed Fake Wall square */
	uint8 randWallOrnCount; /* Used only on some Wall squares and some closed Fake Wall squares */
	uint8 floorOrnCount; /* May be used in a Sensor on a Pit, open Fake Wall, Corridor or Teleporter square */
	uint8 randFloorOrnCount; /* Used only on some Corridor squares and some open Fake Wall squares */

	uint8 doorOrnCount;
	uint8 creatureTypeCount;
	uint8 difficulty;

	FloorSet floorSet;
	WallSet wallSet;
	uint8 doorSet0, doorSet1;
}; // @ MAP

struct DungeonData {
	// I have no idea the heck is this
	uint16 *mapsFirstColumnIndex; // @ G0281_pui_DungeonMapsFirstColumnIndex
	uint16 columCount; // @ G0282_ui_DungeonColumnCount

	// I have no idea the heck is this
	uint16 *columnsCumulativeSquareThingCount; // @ G0280_pui_DungeonColumnsCumulativeSquareThingCount
	Thing *squareFirstThings; // @ G0283_pT_SquareFirstThings
	uint16 *textData; // @ G0260_pui_DungeonTextData

	uint16 **thingsData[16]; // @ G0284_apuc_ThingData

	byte ***mapData; // @ G0279_pppuc_DungeonMapData

	// TODO: ??? is this doing here
	uint16 eventMaximumCount; // @ G0369_ui_EventMaximumCount
}; // @ AGGREGATE

struct CurrMapData {
	direction partyDir; // @ G0308_i_PartyDirection
	int16 partyPosX; // @ G0306_i_PartyMapX
	int16 partyPosY; // @ G0307_i_PartyMapY
	uint8 currPartyMapIndex; // @ G0309_i_PartyMapIndex

	uint8 index; // @ G0272_i_CurrentMapIndex
	byte **data; // @ G0271_ppuc_CurrentMapData
	Map *map; // @ G0269_ps_CurrentMap
	uint16 width; // @ G0273_i_CurrentMapWidth
	uint16 height; // @ G0274_i_CurrentMapHeight
	uint16 *colCumulativeSquareFirstThingCount; // @G0270_pui_CurrentMapColumnsCumulativeSquareFirstThingCount
}; // @ AGGREGATE

struct Messages {
	bool newGame; // @ G0298_B_NewGame
	bool restartGameRequest; // @ G0523_B_RestartGameRequested
}; // @ AGGREGATE

class DungeonMan {
	DMEngine *_vm;

	DungeonMan(const DungeonMan &other); // no implementation on purpose
	void operator=(const DungeonMan &rhs); // no implementation on purpose

	Square getSquare(int16 mapX, int16 mapY); // @ F0151_DUNGEON_GetSquare
	Square getRelSquare(direction dir, int16 stepsForward, int16 stepsRight, int16 posX, int16 posY); // @ F0152_DUNGEON_GetRelativeSquare

	void decompressDungeonFile(); // @ F0455_FLOPPY_DecompressDungeon

	int16 getSquareFirstThingIndex(int16 mapX, int16 mapY); // @ F0160_DUNGEON_GetSquareFirstThingIndex
	Thing getSquareFirstThing(int16 mapX, int16 mapY); // @ F0161_DUNGEON_GetSquareFirstThing

	int16 getRandomOrnOrdinal(bool allowed, int16 count, int16 mapX, int16 mapY, int16 modulo); // @ F0170_DUNGEON_GetRandomOrnamentOrdinal
	void setSquareAspectOrnOrdinals(uint16 *aspectArray, bool leftAllowed, bool frontAllowed, bool rightAllowed, direction dir,
									int16 mapX, int16 mapY, bool isFakeWall); // @ F0171_DUNGEON_SetSquareAspectRandomWallOrnamentOrdinals


	void setCurrentMap(uint16 mapIndex); // @ F0173_DUNGEON_SetCurrentMap

	Thing getNextThing(Thing thing); // @ F0159_DUNGEON_GetNextThing(THING P0280_T_Thing)
	uint16 *getThingData(Thing thing); // @ unsigned char* F0156_DUNGEON_GetThingData(register THING P0276_T_Thing)
public:
	DungeonMan(DMEngine *dmEngine);
	~DungeonMan();
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
	uint16 _dungeonViewClickableBoxes[6][4]; // G0291_aauc_DungeonViewClickableBoxes
	bool _isFacingAlcove; // @ G0286_B_FacingAlcove
	bool _isFacingViAltar; // @ G0287_B_FacingViAltar
	bool _isFacingFountain; // @ G0288_B_FacingFountain
};

};


#endif
