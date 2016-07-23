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
#include "gui/debugger.h"
#include "common/savefile.h"
#include "common/str.h"
#include "engines/savestate.h"



namespace DM {

class Console;
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


void warning(bool repeat, const char *s, ...);

enum direction {
	kDirNorth = 0,
	kDirEast = 1,
	kDirSouth = 2,
	kDirWest = 3
};



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
	explicit Thing(uint16 d) { set(d); }

	void set(uint16 d) {
		_data = d;
	}

	byte getCell() const { return _data >> 14; }
	ThingType getType() const { return (ThingType)((_data >> 10) & 0xF); }
	uint16 getIndex() const { return _data & 0x3FF; }
	uint16 getTypeAndIndex() { return _data & 0x3FFF; }
	uint16 toUint16() const { return _data; } // I don't like 'em cast operators
	bool operator==(const Thing &rhs) const { return _data == rhs._data; }
	bool operator!=(const Thing &rhs) const { return _data != rhs._data; }
}; // @ THING



void turnDirRight(direction &dir);
void turnDirLeft(direction &dir);
direction returnOppositeDir(direction dir);	// @ M18_OPPOSITE
uint16 returnPrevVal(uint16 val); // @ M19_PREVIOUS
uint16 returnNextVal(uint16 val); // @ M17_NEXT
bool isOrientedWestEast(direction dir);	// @ M16_IS_ORIENTED_WEST_EAST


#define setFlag(val, mask) ((val) |= (mask))


#define getFlag(val, mask) ((val) & (mask))


#define clearFlag(val, mask) ((val) &= (~(mask))) // @ M09_CLEAR

uint16 toggleFlag(uint16 &val, uint16 mask); // @ M10_TOGGLE
uint16 M75_bitmapByteCount(uint16 pixelWidth, uint16 height);  // @ M75_BITMAP_BYTE_COUNT
uint16 M21_normalizeModulo4(uint16 val); // @ M21_NORMALIZE
int32 M30_time(int32 map_time); // @ M30_TIME
int32 M33_setMapAndTime(int32 &map_time, uint32 map, uint32 time); // @ M33_SET_MAP_AND_TIME
uint16 M29_map(int32 map_time); // @ M29_MAP
Thing M15_thingWithNewCell(Thing thing, int16 cell); // @ M15_THING_WITH_NEW_CELL
int16 M38_distance(int16 mapx1, int16 mapy1, int16 mapx2, int16 mapy2);// @ M38_DISTANCE


enum Cell {
	kM1_CellAny = -1, // @ CM1_CELL_ANY      
	k0_CellNorthWest = 0, // @ C00_CELL_NORTHWEST 
	k1_CellNorthEast = 1, // @ C01_CELL_NORTHEAST 
	k2_CellSouthEast = 2, // @ C02_CELL_SOUTHEAST 
	k3_CellSouthWest = 3 // @ C03_CELL_SOUTHWEST 
};

#define kM1_mapIndexNone -1 // @ CM1_MAP_INDEX_NONE       
#define k255_mapIndexEntrance 255 // @ C255_MAP_INDEX_ENTRANCE 


enum {
	// engine debug channels
	kDMDebugExample = 1 << 0,
	kDMDebugUselessCode = 1 << 1,
	kDMDebugOftenCalledWarning = 2 << 2
};




template<typename T>
inline T f26_getBoundedValue(T min, T val, T max) {
	return MIN(MAX(min, val), max);
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
	void f462_startGame(); // @ F0462_START_StartGame_CPSF
	void f3_processNewPartyMap(uint16 mapIndex); // @ F0003_MAIN_ProcessNewPartyMap_CPSE
	void f463_initializeGame(); // @ F0463_START_InitializeGame_CPSADEF
	void f448_initMemoryManager(); // @ F0448_STARTUP1_InitializeMemoryManager_CPSADEF
	void f2_gameloop(); // @ F0002_MAIN_GameLoop_CPSDF
	void initArrays();
	Common::String getSavefileName(uint16 slot);
	void writeSaveGameHeader(Common::OutSaveFile *out, const Common::String &saveName);
	bool readSaveGameHeader(Common::InSaveFile *file, SaveGameHeader *header);
	void f439_drawEntrance(); // @ F0439_STARTEND_DrawEntrance
public:
	explicit DMEngine(OSystem *syst);
	~DMEngine();
	virtual bool hasFeature(EngineFeature f) const;

	void f22_delay(uint16 verticalBlank); // @ F0022_MAIN_Delay
	uint16 f30_getScaledProduct(uint16 val, uint16 scale, uint16 vale2); // @ F0030_MAIN_GetScaledProduct
	uint16 getRandomNumber(uint32 max) { return _rnd->getRandomNumber(max - 1); }
	int16 M1_ordinalToIndex(int16 val); // @ M01_ORDINAL_TO_INDEX
	int16 M0_indexToOrdinal(int16 val); // @ M00_INDEX_TO_ORDINAL
	void f19_displayErrorAndStop(int16 errorIndex); // @ F0019_MAIN_DisplayErrorAndStop
	virtual Common::Error run(); // @ main
	void f433_processCommand140_saveGame(uint16 slot, const Common::String desc); // @ F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF
	LoadgameResponse f435_loadgame(int16 slot); // @ F0435_STARTEND_LoadGame_CPSF
	void f441_processEntrance(); // @ F0441_STARTEND_ProcessEntrance

	void f064_SOUND_RequestPlay_CPSD(uint16 P0088_ui_SoundIndex, int16 P0089_i_MapX, int16 P0090_i_MapY, uint16 P0091_ui_Mode) { warning(true, "STUB: f064_SOUND_RequestPlay_CPSD"); }

private:
	int16 _g528_saveFormat; // @ G0528_i_Format
	int16 _g527_platform; // @ G0527_i_Platform
	uint16 _g526_dungeonId; // @ G0526_ui_DungeonID
	Console *_console;
	byte *_g562_entranceDoorAnimSteps[10]; // @ G0562_apuc_Bitmap_EntranceDoorAnimationSteps
	byte *_g564_interfaceCredits; // @ G0564_puc_Graphic5_InterfaceCredits
public:
	Common::RandomSource *_rnd;
	DisplayMan *_displayMan;
	DungeonMan *_dungeonMan;
	EventManager *_eventMan;
	MenuMan *_menuMan;
	ChampionMan *_championMan;
	ObjectMan *_objectMan;
	InventoryMan *_inventoryMan;
	TextMan *_textMan;
	MovesensMan *_movsens;
	GroupMan *_groupMan;
	Timeline *_timeline;
	ProjExpl *_projexpl;


	int16 _g298_newGame; // @ G0298_B_NewGame
	bool _g523_restartGameRequest; // @ G0523_B_RestartGameRequested

	bool _g321_stopWaitingForPlayerInput; // @ G0321_B_StopWaitingForPlayerInput
	bool _g301_gameTimeTicking; // @ G0301_B_GameTimeTicking
	bool _g524_restartGameAllowed; // @ G0524_B_RestartGameAllowed
	int32 _g525_gameId; // @ G0525_l_GameID, probably useless here
	bool _g331_pressingEye; // @ G0331_B_PressingEye
	bool _g332_stopPressingEye; // @ G0332_B_StopPressingEye
	bool _g333_pressingMouth; // @ G0333_B_PressingMouth
	bool _g334_stopPressingMouth; // @ G0334_B_StopPressingMouth
	bool _g340_highlightBoxInversionRequested; // @ G0340_B_HighlightBoxInversionRequested
	int16 _g311_projectileDisableMovementTicks; // @ G0311_i_ProjectileDisabledMovementTicks
	int16 _g312_lastProjectileDisabledMovementDirection; // @ G0312_i_LastProjectileDisabledMovementDirection
	bool _g302_gameWon; // @ G0302_B_GameWon
	int16 _g327_newPartyMapIndex; // @ G0327_i_NewPartyMapIndex
	bool _g325_setMousePointerToObjectInMainLoop; // @ G0325_B_SetMousePointerToObjectInMainLoop
	int16 _g310_disabledMovementTicks; // @ G0310_i_DisabledMovementTicks

	int8 _dirIntoStepCountEast[4]; // @ G0233_ai_Graphic559_DirectionToStepEastCount
	int8 _dirIntoStepCountNorth[4]; // @ G0234_ai_Graphic559_DirectionToStepNorthCount
	uint32 _g313_gameTime; // @ G0313_ul_GameTime
	char _g353_stringBuildBuffer[128]; // @ G0353_ac_StringBuildBuffer
	int16 _g318_waitForInputMaxVerticalBlankCount; // @ G0318_i_WaitForInputMaximumVerticalBlankCount
};

class Console : public GUI::Debugger {
public:
	explicit Console(DMEngine *vm) {}
	virtual ~Console(void) {}
};

} // End of namespace DM

#endif
