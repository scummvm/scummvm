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


namespace DM {

class Console;
class DisplayMan;
class DungeonMan;
class EventManager;
class MenuMan;
class ChampionMan;
class LoadsaveMan;
class ObjectMan;
class InventoryMan;
class TextMan;
class MovesensMan;
class GroupMan;
class Timeline;


enum direction {
	kDirNorth = 0,
	kDirEast = 1,
	kDirSouth = 2,
	kDirWest = 3
};

void turnDirRight(direction &dir);
void turnDirLeft(direction &dir);
direction returnOppositeDir(direction dir);	// @ M18_OPPOSITE
uint16 returnPrevVal(uint16 val); // @ M19_PREVIOUS
uint16 returnNextVal(uint16 val); // @ M17_NEXT
bool isOrientedWestEast(direction dir);	// @ M16_IS_ORIENTED_WEST_EAST


uint16 getFlag(uint16 val, uint16 mask);
void setFlag(uint16 &val, uint16 mask);
void clearFlag(uint16 &val, uint16 mask);


enum ThingType {
	kPartyThingType = -1, // @ CM1_THING_TYPE_PARTY, special value
	kDoorThingType = 0,
	kTeleporterThingType = 1,
	kTextstringType = 2,
	kSensorThingType = 3,
	kGroupThingType = 4,
	kWeaponThingType = 5,
	kArmourThingType = 6,
	kScrollThingType = 7,
	kPotionThingType = 8,
	kContainerThingType = 9,
	kJunkThingType = 10,
	kProjectileThingType = 14,
	kExplosionThingType = 15,
	kThingTypeTotal = 16 // +1 than the last (explosionThingType)
}; // @ C[00..15]_THING_TYPE_...

enum Cell {
	kCellAny = -1, // @ CM1_CELL_ANY      
	kCellNorthWest = 0, // @ C00_CELL_NORTHWEST 
	kCellNorthEast = 1, // @ C01_CELL_NORTHEAST 
	kCellSouthEast = 2, // @ C02_CELL_SOUTHEAST 
	kCellSouthWest = 3 // @ C03_CELL_SOUTHWEST 
};

class Thing {
	uint16 _data;
public:
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
	uint16 toUint16() const { return _data; } // I don't like 'em cast operators
	bool operator==(const Thing &rhs) const { return _data == rhs._data; }
	bool operator!=(const Thing &rhs) const { return _data != rhs._data; }
}; // @ THING


enum {
	// engine debug channels
	kDMDebugExample = 1 << 0
};

class DMEngine : public Engine {
	void startGame(); // @ F0462_START_StartGame_CPSF
	void processNewPartyMap(uint16 mapIndex); // @ F0003_MAIN_ProcessNewPartyMap_CPSE
	void initializeGame(); // @ F0463_START_InitializeGame_CPSADEF
	void gameloop(); // @ F0002_MAIN_GameLoop_CPSDF
	void initArrays();

public:
	explicit DMEngine(OSystem *syst);
	~DMEngine();

	int16 ordinalToIndex(int16 val); // @ M01_ORDINAL_TO_INDEX
	int16 indexToOrdinal(int16 val); // @ M00_INDEX_TO_ORDINAL
	virtual Common::Error run(); // @ main

private:
	Console *_console;
public:
	Common::RandomSource *_rnd;
	DisplayMan *_displayMan;
	DungeonMan *_dungeonMan;
	EventManager *_eventMan;
	MenuMan *_menuMan;
	ChampionMan *_championMan;
	LoadsaveMan *_loadsaveMan;
	ObjectMan *_objectMan;
	InventoryMan *_inventoryMan;
	TextMan *_textMan;
	MovesensMan *_movsens;
	GroupMan *_groupMan;
	Timeline *_timeline;

	bool _stopWaitingForPlayerInput; // G0321_B_StopWaitingForPlayerInput
	bool _gameTimeTicking; // @ G0301_B_GameTimeTicking
	bool _restartGameAllowed; // @ G0524_B_RestartGameAllowed
	uint32 _gameId; // @ G0525_l_GameID, probably useless here
	bool _pressingEye; // @ G0331_B_PressingEye
	bool _stopPressingEye; // @ G0332_B_StopPressingEye
	bool _pressingMouth; // @ G0333_B_PressingMouth
	bool _stopPressingMouth; // @ G0334_B_StopPressingMouth
	bool _highlightBoxInversionRequested; // @ G0340_B_HighlightBoxInversionRequested

	// TODO: refactor direction into a class
	int8 _dirIntoStepCountEast[4];
	int8 _dirIntoStepCountNorth[4];
};

class Console : public GUI::Debugger {
public:
	explicit Console(DMEngine *vm) {}
	virtual ~Console(void) {}
};

} // End of namespace DM

#endif
