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

#ifndef DM_DM_H
#define DM_DM_H

#include "engines/engine.h"
#include "engines/savestate.h"

#include "common/random.h"
#include "common/savefile.h"
#include "common/str.h"
#include "common/memstream.h"

#include "advancedDetector.h"

#include "dm/console.h"
#include "dm/detection.h"

namespace DM {

class DisplayMan;
class DungeonMan;
class EventManager;
class MenuMan;
class ChampionMan;
class ObjectMan;
class InventoryMan;
class TextMan;
class MovesensMan;
class GroupMan;
class Timeline;
class ProjExpl;
class DialogMan;
class SoundMan;

enum Direction {
	kDMDirNorth = 0,
	kDMDirEast = 1,
	kDMDirSouth = 2,
	kDMDirWest = 3
};

enum ThingType {
	kDMThingTypeParty = -1,  // @ CM1_THING_TYPE_PARTY
	kDMThingTypeDoor = 0, // @ C00_THING_TYPE_DOOR
	kDMThingTypeTeleporter = 1, // @ C01_THING_TYPE_TELEPORTER
	kDMstringTypeText = 2, // @ C02_THING_TYPE_TEXTSTRING
	kDMThingTypeSensor = 3,  // @ C03_THING_TYPE_SENSOR
	kDMThingTypeGroup = 4, // @ C04_THING_TYPE_GROUP
	kDMThingTypeWeapon = 5, // @ C05_THING_TYPE_WEAPON
	kDMThingTypeArmour = 6, // @ C06_THING_TYPE_ARMOUR
	kDMThingTypeScroll = 7,  // @ C07_THING_TYPE_SCROLL
	kDMThingTypePotion = 8,  // @ C08_THING_TYPE_POTION
	kDMThingTypeContainer = 9,  // @ C09_THING_TYPE_CONTAINER
	kDMThingTypeJunk = 10,  // @ C10_THING_TYPE_JUNK
	kDMThingTypeProjectile = 14,  // @ C14_THING_TYPE_PROJECTILE
	kDMThingTypeExplosion = 15,  // @ C15_THING_TYPE_EXPLOSION
	kDMThingTypeTotal = 16 // +1 than the last (explosionThingType)
}; // @ C[00..15]_THING_TYPE_...

enum Cell {
	kDMCellAny = -1, // @ CM1_CELL_ANY
	kDMCellNorthWest = 0, // @ C00_CELL_NORTHWEST
	kDMCellNorthEast = 1, // @ C01_CELL_NORTHEAST
	kDMCellSouthEast = 2, // @ C02_CELL_SOUTHEAST
	kDMCellSouthWest = 3 // @ C03_CELL_SOUTHWEST
};

enum GameMode {
	kDMModeLoadSavedGame = 0, // @ C000_MODE_LOAD_SAVED_GAME
	kDMModeLoadDungeon = 1, // @ C001_MODE_LOAD_DUNGEON
	kDMModeWaitingOnEntrance = 99, // @ C099_MODE_WAITING_ON_ENTRANCE
	kDMModeEntranceDrawCredits = 202 // @ C202_MODE_ENTRANCE_DRAW_CREDITS
};

enum LoadgameResult {
	kDMLoadgameFailure = -1, // @ CM1_LOAD_GAME_FAILURE
	kDMLoadgameSuccess = 1// @ C01_LOAD_GAME_SUCCESS
};

enum MapIndice {
	kDMMapIndexNone = -1, // @ CM1_MAP_INDEX_NONE
	kDMMapIndexEntrance = 255 // @ C255_MAP_INDEX_ENTRANCE
};

#define kDMMaskDecodeEvenIfInvisible 0x8000 // @ MASK0x8000_DECODE_EVEN_IF_INVISIBLE
#define kDMMaskMergeCycles 0x8000 // @ MASK0x8000_MERGE_CYCLES

#define kDMSlotBoxInventoryFirstSlot 8  // @ C08_SLOT_BOX_INVENTORY_FIRST_SLOT
#define kDMSlotBoxInventoryActionHand 9 // @ C09_SLOT_BOX_INVENTORY_ACTION_HAND
#define kDMSlotBoxChestFirstSlot 38     // @ C38_SLOT_BOX_CHEST_FIRST_SLOT

class Thing {
public:
	uint16 _data;

	Thing() : _data(0) {}
	Thing(const Thing &other) { set(other._data); }
	explicit Thing(uint16 d) { set(d); }

	void set(uint16 d) {
		_data = d;
	}

	byte getCell() const { return _data >> 14; }
	ThingType getType() const { return (ThingType)((_data >> 10) & 0xF); }
	uint16 getIndex() const { return _data & 0x3FF; }

	void setCell(uint16 cell) { _data = (_data & ~(0x3 << 14)) | ((cell & 0x3) << 14); }
	void setType(uint16 type) { _data = (_data & ~(0xF << 10)) | ((type & 0xF) << 10); }
	void setIndex(uint16 index) { _data = (_data & ~0x3FF) | (index & 0x3FF); }

	uint16 getTypeAndIndex() { return _data & 0x3FFF; }
	uint16 toUint16() const { return _data; } // I don't like 'em cast operators
	bool operator==(const Thing &rhs) const { return _data == rhs._data; }
	bool operator!=(const Thing &rhs) const { return _data != rhs._data; }
}; // @ THING

#define setFlag(val, mask) ((val) |= (mask))
#define getFlag(val, mask) ((val) & (mask))
#define clearFlag(val, mask) ((val) &= (~(mask))) // @ M09_CLEAR

// Note: F0026_MAIN_GetBoundedValue<T> has been replaced by CLIP<T>

#define CALL_MEMBER_FN(object, ptrToMember)  ((object).*(ptrToMember))

struct SaveGameHeader {
	byte _version;
	SaveStateDescriptor _descr;
};

class DMEngine : public Engine {
private:
	void startGame(); // @ F0462_START_StartGame_CPSF
	void processNewPartyMap(uint16 mapIndex); // @ F0003_MAIN_ProcessNewPartyMap_CPSE
	void initializeGame(); // @ F0463_START_InitializeGame_CPSADEF
	void initMemoryManager(); // @ F0448_STARTUP1_InitializeMemoryManager_CPSADEF
	void gameloop(); // @ F0002_MAIN_GameLoop_CPSDF
	void initConstants();
	Common::String getSavefileName(uint16 slot);
	void writeSaveGameHeader(Common::OutSaveFile *out, const Common::String &saveName);
	bool writeCompleteSaveFile(int16 slot, Common::String &desc, int16 saveAndPlayChoice);
	void drawEntrance(); // @ F0439_STARTEND_DrawEntrance
	void fuseSequenceUpdate(); // @ F0445_STARTEND_FuseSequenceUpdate
	void processEntrance(); // @ F0441_STARTEND_ProcessEntrance
	void openEntranceDoors(); // @ F0438_STARTEND_OpenEntranceDoors
	void drawTittle(); // @ F0437_STARTEND_DrawTitle

public:
	explicit DMEngine(OSystem *syst, const DMADGameDescription *gameDesc);
	~DMEngine() override;
	bool hasFeature(EngineFeature f) const override;

	Common::Error loadGameState(int slot) override;
	bool canLoadGameStateCurrently() override;

	bool isDemo() const;

	void delay(uint16 verticalBlank); // @ F0022_MAIN_Delay
	uint16 getScaledProduct(uint16 val, uint16 scale, uint16 vale2); // @ F0030_MAIN_GetScaledProduct
	uint16 getRandomNumber(uint32 max) { return _rnd->getRandomNumber(max - 1); }
	int16 ordinalToIndex(int16 val); // @ M01_ORDINAL_TO_INDEX
	int16 indexToOrdinal(int16 val); // @ M00_INDEX_TO_ORDINAL
	Common::Error run() override; // @ main
	void saveGame(); // @ F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF
	LoadgameResult loadgame(int16 slot); // @ F0435_STARTEND_LoadGame_CPSF
	void endGame(bool doNotDrawCreditsOnly); // @ F0444_STARTEND_Endgame

	void entranceDrawCredits();
	void fuseSequence(); // @ F0446_STARTEND_FuseSequence
	Common::Language getGameLanguage();

	Direction turnDirRight(int16 dir); // @ M17_NEXT
	Direction turnDirLeft(int16 dir); // @ M19_PREVIOUS
	Direction returnOppositeDir(int16 dir);	// @ M18_OPPOSITE
	bool isOrientedWestEast(int16 dir);	// @ M16_IS_ORIENTED_WEST_EAST
	uint16 normalizeModulo4(int16 dir); // @ M21_NORMALIZE

	int32 filterTime(int32 mapTime); // @ M30_TIME
	int32 setMapAndTime(uint32 map, uint32 time); // @ M33_SET_MAP_AND_TIME
	uint16 getMap(int32 mapTime); // @ M29_MAP
	Thing thingWithNewCell(Thing thing, int16 cell); // @ M15_THING_WITH_NEW_CELL
	int16 getDistance(int16 mapx1, int16 mapy1, int16 mapx2, int16 mapy2); // @ M38_DISTANCE
	int32 setMap(int32 mapTime, uint32 map); // @ M31_setMap


private:
	uint16 _dungeonId; // @ G0526_ui_DungeonID
	byte *_entranceDoorAnimSteps[10]; // @ G0562_apuc_Bitmap_EntranceDoorAnimationSteps
	byte *_interfaceCredits; // @ G0564_puc_Graphic5_InterfaceCredits
	Common::RandomSource *_rnd;

	byte *_savedScreenForOpenEntranceDoors; // ad-hoc HACK
	const DMADGameDescription *_gameVersion;
	bool _canLoadFromGMM;
public:
	Console *_console;
	DisplayMan *_displayMan;
	DungeonMan *_dungeonMan;
	EventManager *_eventMan;
	MenuMan *_menuMan;
	ChampionMan *_championMan;
	ObjectMan *_objectMan;
	InventoryMan *_inventoryMan;
	TextMan *_textMan;
	MovesensMan *_moveSens;
	GroupMan *_groupMan;
	Timeline *_timeline;
	ProjExpl *_projexpl;
	DialogMan *_dialog;
	SoundMan *_sound;

	Common::MemoryWriteStreamDynamic *_saveThumbnail;

	bool _engineShouldQuit;
	int _loadSaveSlotAtRuntime;

	GameMode _gameMode; // @ G0298_B_NewGame
	bool _restartGameRequest; // @ G0523_B_RestartGameRequested

	bool _stopWaitingForPlayerInput; // @ G0321_B_StopWaitingForPlayerInput
	bool _gameTimeTicking; // @ G0301_B_GameTimeTicking
	bool _restartGameAllowed; // @ G0524_B_RestartGameAllowed
	bool _pressingEye; // @ G0331_B_PressingEye
	bool _stopPressingEye; // @ G0332_B_StopPressingEye
	bool _pressingMouth; // @ G0333_B_PressingMouth
	bool _stopPressingMouth; // @ G0334_B_StopPressingMouth
	bool _highlightBoxInversionRequested; // @ G0340_B_HighlightBoxInversionRequested
	int16 _projectileDisableMovementTicks; // @ G0311_i_ProjectileDisabledMovementTicks
	int16 _lastProjectileDisabledMovementDirection; // @ G0312_i_LastProjectileDisabledMovementDirection
	bool _gameWon; // @ G0302_B_GameWon
	int16 _newPartyMapIndex; // @ G0327_i_NewPartyMapIndex
	bool _setMousePointerToObjectInMainLoop; // @ G0325_B_SetMousePointerToObjectInMainLoop
	int16 _disabledMovementTicks; // @ G0310_i_DisabledMovementTicks

	int8 _dirIntoStepCountEast[4]; // @ G0233_ai_Graphic559_DirectionToStepEastCount
	int8 _dirIntoStepCountNorth[4]; // @ G0234_ai_Graphic559_DirectionToStepNorthCount
	int32 _gameTime; // @ G0313_ul_GameTime
	char _stringBuildBuffer[128]; // @ G0353_ac_StringBuildBuffer
	int16 _waitForInputMaxVerticalBlankCount; // @ G0318_i_WaitForInputMaximumVerticalBlankCount

	Thing _thingNone;				 // @ C0xFFFF_THING_NONE
	Thing _thingEndOfList;			 // @ C0xFFFE_THING_ENDOFLIST
	Thing _thingFirstExplosion;		 // @ C0xFF80_THING_FIRST_EXPLOSION
	Thing _thingExplFireBall;		 // @ C0xFF80_THING_EXPLOSION_FIREBALL
	Thing _thingExplSlime;			 // @ C0xFF81_THING_EXPLOSION_SLIME
	Thing _thingExplLightningBolt;	 // @ C0xFF82_THING_EXPLOSION_LIGHTNING_BOLT
	Thing _thingExplHarmNonMaterial; // @ C0xFF83_THING_EXPLOSION_HARM_NON_MATERIAL
	Thing _thingExplOpenDoor;		 // @ C0xFF84_THING_EXPLOSION_OPEN_DOOR
	Thing _thingExplPoisonBolt;		 // @ C0xFF86_THING_EXPLOSION_POISON_BOLT
	Thing _thingExplPoisonCloud;	 // @ C0xFF87_THING_EXPLOSION_POISON_CLOUD
	Thing _thingExplSmoke;			 // @ C0xFFA8_THING_EXPLOSION_SMOKE
	Thing _thingExplFluxcage;		 // @ C0xFFB2_THING_EXPLOSION_FLUXCAGE
	Thing _thingExplRebirthStep1;	 // @ C0xFFE4_THING_EXPLOSION_REBIRTH_STEP1
	Thing _thingExplRebirthStep2;	 // @ C0xFFE5_THING_EXPLOSION_REBIRTH_STEP2
	Thing _thingParty;				 // @ C0xFFFF_THING_PARTY
};

WARN_UNUSED_RESULT bool readSaveGameHeader(Common::InSaveFile *in, SaveGameHeader *header, bool skipThumbnail = true);

} // End of namespace DM

#endif
