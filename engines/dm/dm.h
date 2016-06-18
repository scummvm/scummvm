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


enum direction {
	kDirNorth = 0,
	kDirEast = 1,
	kDirSouth = 2,
	kDirWest = 3
};

// TODO: refactor direction into a class
extern int8 dirIntoStepCountEast[4];
extern int8 dirIntoStepCountNorth[4];

void turnDirRight(direction &dir);
void turnDirLeft(direction &dir);
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
	uint16 data;
public:
	static const Thing thingNone;
	static const Thing thingEndOfList;

	Thing() : data(0) {}
	Thing(uint16 d) { set(d); }

	void set(uint16 d) {
		data = d;
	}

	byte getCell() const { return data >> 14; }
	ThingType getType() const { return (ThingType)((data >> 10) & 0xF); }
	uint16 getIndex() const { return data & 0x1FF; }
	uint16 toUint16() const { return data; } // I don't like 'em cast operators
	bool operator==(const Thing &rhs) const { return data == rhs.data; }
	bool operator!=(const Thing &rhs) const { return data != rhs.data; }
}; // @ THING


enum {
	// engine debug channels
	kDMDebugExample = 1 << 0
};

class DMEngine : public Engine {
	void startGame(); // @ F0462_START_StartGame_CPSF
public:
	DMEngine(OSystem *syst);
	~DMEngine();

	virtual Common::Error run();

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
	bool _stopWaitingForPlayerInput; // G0321_B_StopWaitingForPlayerInput
	bool _gameTimeTicking; // @ G0301_B_GameTimeTicking
	bool _restartGameAllowed; // @ G0524_B_RestartGameAllowed
	uint32 _gameId; // @ G0525_l_GameID, probably useless here
};

class Console : public GUI::Debugger {
public:
	Console(DMEngine *vm) {}
	virtual ~Console(void) {}
};

} // End of namespace DM

#endif
