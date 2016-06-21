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


enum direction {
	kDirNorth = 0,
	kDirEast = 1,
	kDirSouth = 2,
	kDirWest = 3
};

// TODO: refactor direction into a class
extern int8 _dirIntoStepCountEast[4];
extern int8 _dirIntoStepCountNorth[4];

void turnDirRight(direction &dir);
void turnDirLeft(direction &dir);
direction returnOppositeDir(direction dir);
bool isOrientedWestEast(direction dir);


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

class Thing {
	uint16 _data;
public:
	static const Thing _thingNone;
	static const Thing _thingEndOfList;

	Thing() : _data(0) {}
	Thing(uint16 d) { set(d); }

	void set(uint16 d) {
		_data = d;
	}

	byte getCell() const { return _data >> 14; }
	ThingType getType() const { return (ThingType)((_data >> 10) & 0xF); }
	uint16 getIndex() const { return _data & 0x1FF; }
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
public:
	DMEngine(OSystem *syst);
	~DMEngine();

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
	bool _stopWaitingForPlayerInput; // G0321_B_StopWaitingForPlayerInput
	bool _gameTimeTicking; // @ G0301_B_GameTimeTicking
	bool _restartGameAllowed; // @ G0524_B_RestartGameAllowed
	uint32 _gameId; // @ G0525_l_GameID, probably useless here
	bool _pressingEye; // @ G0331_B_PressingEye
	bool _stopPressingEye; // @ G0332_B_StopPressingEye
	bool _pressingMouth; // @ G0333_B_PressingMouth
	bool _stopPressingMouth; // @ G0334_B_StopPressingMouth
	bool _highlightBoxInversionRequested; // @ G0340_B_HighlightBoxInversionRequested
};

class Console : public GUI::Debugger {
public:
	Console(DMEngine *vm) {}
	virtual ~Console(void) {}
};

} // End of namespace DM

#endif
