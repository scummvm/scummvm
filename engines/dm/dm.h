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

#ifndef DM_H
#define DM_H

#include "common/random.h"
#include "engines/engine.h"
#include "common/savefile.h"
#include "common/str.h"
#include "engines/savestate.h"

#include "console.h"
#include <common/memstream.h>
#include <advancedDetector.h>


struct ADGameDescription;

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

enum OriginalSaveFormat {
	k_saveFormat_accept_any = -1,
	k_saveFormat_endOfList = 0,
	k_saveFormat_dm_atari_st = 1,
	k_saveFormat_dm_amiga__2_x_pc98_x68000_fm_towns_csb_atari_st = 2,
	k_saveFormat_dm_apple_iigs = 3,
	k_saveFormat_dm_amiga_36_pc_csb_amiga_pc98_x68000_fm_towns = 5,
	k_saveFormat_total
};

enum OriginalSavePlatform {
	k_savePlatform_accept_any = -1,
	k_savePlatform_endOfList = 0,
	k_savePlatform_atari_st = 1, // @ C1_PLATFORM_ATARI_ST   
	k_savePlatform_apple_iigs = 2, // @ C2_PLATFORM_APPLE_IIGS 
	k_savePlatform_amiga = 3, // @ C3_PLATFORM_AMIGA      
	k_savePlatform_pc98 = 5, // @ C5_PLATFORM_PC98       
	k_savePlatform_x68000 = 6, // @ C6_PLATFORM_X68000     
	k_savePlatform_fm_towns_en = 7, // @ C7_PLATFORM_FM_TOWNS_EN
	k_savePlatform_fm_towns_jp = 8, // @ C8_PLATFORM_FM_TOWNS_JP
	k_savePlatform_pc = 9, // @ C9_PLATFORM_PC
	k_savePlatform_total
};

enum SaveTarget {
	k_saveTarget_accept_any = -1,
	k_saveTarget_endOfList = 0,
	k_saveTarget_DM21 = 1,
	k_saveTarget_total
};

struct DMADGameDescription {
	ADGameDescription _desc;

	SaveTarget _saveTargetToWrite;
	OriginalSaveFormat _origSaveFormatToWrite;
	OriginalSavePlatform _origPlatformToWrite;

	SaveTarget _saveTargetToAccept[k_saveTarget_total + 1];
	OriginalSaveFormat _saveFormatToAccept[k_saveFormat_total + 1];
	OriginalSavePlatform _origPlatformToAccept[k_savePlatform_total + 1];
};

void warning(bool repeat, const char *s, ...);

enum Direction {
	kDirNorth = 0,
	kDirEast = 1,
	kDirSouth = 2,
	kDirWest = 3
};

const char *debugGetDirectionName(Direction dir);

enum ThingType {
	kM1_PartyThingType = -1,  // @ CM1_THING_TYPE_PARTY
	k0_DoorThingType = 0, // @ C00_THING_TYPE_DOOR       
	k1_TeleporterThingType = 1, // @ C01_THING_TYPE_TELEPORTER 
	k2_TextstringType = 2, // @ C02_THING_TYPE_TEXTSTRING 
	k3_SensorThingType = 3,  // @ C03_THING_TYPE_SENSOR     
	k4_GroupThingType = 4, // @ C04_THING_TYPE_GROUP 
	k5_WeaponThingType = 5, // @ C05_THING_TYPE_WEAPON
	k6_ArmourThingType = 6, // @ C06_THING_TYPE_ARMOUR
	k7_ScrollThingType = 7,  // @ C07_THING_TYPE_SCROLL     
	k8_PotionThingType = 8,  // @ C08_THING_TYPE_POTION     
	k9_ContainerThingType = 9,  // @ C09_THING_TYPE_CONTAINER  
	k10_JunkThingType = 10,  // @ C10_THING_TYPE_JUNK      
	k14_ProjectileThingType = 14,  // @ C14_THING_TYPE_PROJECTILE
	k15_ExplosionThingType = 15,  // @ C15_THING_TYPE_EXPLOSION 
	k16_ThingTypeTotal = 16 // +1 than the last (explosionThingType)
}; // @ C[00..15]_THING_TYPE_...


class Thing {
public:
	uint16 _data;
	static const Thing _none; // @ C0xFFFF_THING_NONE
	static const Thing _endOfList; // @ C0xFFFE_THING_ENDOFLIST
	static const Thing _firstExplosion; // @ C0xFF80_THING_FIRST_EXPLOSION            
	static const Thing _explFireBall; // @ C0xFF80_THING_EXPLOSION_FIREBALL         
	static const Thing _explSlime; // @ C0xFF81_THING_EXPLOSION_SLIME            
	static const Thing _explLightningBolt; // @ C0xFF82_THING_EXPLOSION_LIGHTNING_BOLT   
	static const Thing _explHarmNonMaterial; // @ C0xFF83_THING_EXPLOSION_HARM_NON_MATERIAL
	static const Thing _explOpenDoor; // @ C0xFF84_THING_EXPLOSION_OPEN_DOOR        
	static const Thing _explPoisonBolt; // @ C0xFF86_THING_EXPLOSION_POISON_BOLT      
	static const Thing _explPoisonCloud; // @ C0xFF87_THING_EXPLOSION_POISON_CLOUD     
	static const Thing _explSmoke; // @ C0xFFA8_THING_EXPLOSION_SMOKE            
	static const Thing _explFluxcage; // @ C0xFFB2_THING_EXPLOSION_FLUXCAGE         
	static const Thing _explRebirthStep1; // @ C0xFFE4_THING_EXPLOSION_REBIRTH_STEP1    
	static const Thing _explRebirthStep2; // @ C0xFFE5_THING_EXPLOSION_REBIRTH_STEP2    
	static const Thing _party; // @ C0xFFFF_THING_PARTY  

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

void turnDirRight(Direction &dir);
void turnDirLeft(Direction &dir);
Direction returnOppositeDir(Direction dir);	// @ M18_OPPOSITE
uint16 returnPrevVal(uint16 val); // @ M19_PREVIOUS
uint16 returnNextVal(uint16 val); // @ M17_NEXT
bool isOrientedWestEast(Direction dir);	// @ M16_IS_ORIENTED_WEST_EAST

#define setFlag(val, mask) ((val) |= (mask))
#define getFlag(val, mask) ((val) & (mask))
#define clearFlag(val, mask) ((val) &= (~(mask))) // @ M09_CLEAR

uint16 toggleFlag(uint16 &val, uint16 mask); // @ M10_TOGGLE
uint16 bitmapByteCount(uint16 pixelWidth, uint16 height);  // @ M75_BITMAP_BYTE_COUNT
uint16 normalizeModulo4(uint16 val); // @ M21_NORMALIZE
int32 filterTime(int32 map_time); // @ M30_TIME
int32 setMapAndTime(int32 &map_time, uint32 map, uint32 time); // @ M33_SET_MAP_AND_TIME
uint16 getMap(int32 map_time); // @ M29_MAP
Thing thingWithNewCell(Thing thing, int16 cell); // @ M15_THING_WITH_NEW_CELL
int16 getDistance(int16 mapx1, int16 mapy1, int16 mapx2, int16 mapy2);// @ M38_DISTANCE

enum Cell {
	kM1_CellAny = -1, // @ CM1_CELL_ANY      
	k0_CellNorthWest = 0, // @ C00_CELL_NORTHWEST 
	k1_CellNorthEast = 1, // @ C01_CELL_NORTHEAST 
	k2_CellSouthEast = 2, // @ C02_CELL_SOUTHEAST 
	k3_CellSouthWest = 3 // @ C03_CELL_SOUTHWEST 
};

#define kM1_mapIndexNone -1 // @ CM1_MAP_INDEX_NONE       
#define k255_mapIndexEntrance 255 // @ C255_MAP_INDEX_ENTRANCE 

//TODO: Directly use CLIP
template<typename T>
inline T getBoundedValue(T min, T val, T max) {
	return CLIP<T>(min, val, max);
} // @ F0026_MAIN_GetBoundedValue

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

#define k0_modeLoadSavedGame 0 // @ C000_MODE_LOAD_SAVED_GAME        
#define k1_modeLoadDungeon 1 // @ C001_MODE_LOAD_DUNGEON           
#define k99_modeWaitingOnEntrance 99 // @ C099_MODE_WAITING_ON_ENTRANCE   
#define k202_modeEntranceDrawCredits 202 // @ C202_MODE_ENTRANCE_DRAW_CREDITS 

enum LoadgameResponse {
	kM1_LoadgameFailure = -1, // @ CM1_LOAD_GAME_FAILURE
	k1_LoadgameSuccess = 1// @ C01_LOAD_GAME_SUCCESS
};

struct SaveGameHeader {
	byte _version;
	SaveStateDescriptor _descr;
};

class DMEngine : public Engine {
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
public:
	explicit DMEngine(OSystem *syst, const DMADGameDescription *gameDesc);
	~DMEngine();
	virtual bool hasFeature(EngineFeature f) const;

	virtual Common::Error loadGameState(int slot);
	virtual bool canLoadGameStateCurrently();

	GUI::Debugger *getDebugger() { return _console; }

	void delay(uint16 verticalBlank); // @ F0022_MAIN_Delay
	uint16 getScaledProduct(uint16 val, uint16 scale, uint16 vale2); // @ F0030_MAIN_GetScaledProduct
	uint16 getRandomNumber(uint32 max) { return _rnd->getRandomNumber(max - 1); }
	int16 ordinalToIndex(int16 val); // @ M01_ORDINAL_TO_INDEX
	int16 indexToOrdinal(int16 val); // @ M00_INDEX_TO_ORDINAL
	void displayErrorAndStop(int16 errorIndex); // @ F0019_MAIN_DisplayErrorAndStop
	virtual Common::Error run(); // @ main
	void saveGame(); // @ F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF
	LoadgameResponse loadgame(int16 slot); // @ F0435_STARTEND_LoadGame_CPSF
	void processEntrance(); // @ F0441_STARTEND_ProcessEntrance
	void endGame(bool doNotDrawCreditsOnly); // @ F0444_STARTEND_Endgame

	void openEntranceDoors(); // @ F0438_STARTEND_OpenEntranceDoors
	void drawTittle(); // @ F0437_STARTEND_DrawTitle
	void entranceDrawCredits();
	void fuseSequnce(); // @ F0446_STARTEND_FuseSequence
	void fuseSequenceUpdate(); // @ F0445_STARTEND_FuseSequenceUpdate
	Common::Language getGameLanguage();

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

	int16 _newGameFl; // @ G0298_B_NewGame
	bool _restartGameRequest; // @ G0523_B_RestartGameRequested

	bool _stopWaitingForPlayerInput; // @ G0321_B_StopWaitingForPlayerInput
	bool _gameTimeTicking; // @ G0301_B_GameTimeTicking
	bool _restartGameAllowed; // @ G0524_B_RestartGameAllowed
	int32 _gameId; // @ G0525_l_GameID, probably useless here
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
};

bool readSaveGameHeader(Common::InSaveFile* in, SaveGameHeader* header);

} // End of namespace DM

#endif
