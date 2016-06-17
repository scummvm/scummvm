#include "common/scummsys.h"
#include "common/system.h"

#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"

#include "engines/util.h"
#include "engines/engine.h"
#include "graphics/palette.h"
#include "common/file.h"
#include "common/events.h"

#include "dm/dm.h"
#include "gfx.h"
#include "dungeonman.h"
#include "eventman.h"
#include "menus.h"

namespace DM {

int8 dirIntoStepCountEast[4] = {0 /* North */, 1 /* East */, 0 /* West */, -1 /* South */};
int8 dirIntoStepCountNorth[4] = {-1 /* North */, 0 /* East */, 1 /* West */, 0 /* South */};

void turnDirRight(direction &dir) { dir = (direction)((dir + 1) & 3); }
void turnDirLeft(direction &dir) { dir = (direction)((dir - 1) & 3); }
bool isOrientedWestEast(direction dir) { return dir & 1; }


DMEngine::DMEngine(OSystem *syst) : Engine(syst), _console(nullptr) {
	// Do not load data files
	// Do not initialize graphics here
	// Do not initialize audio devices here
	// Do these from run

	//Specify all default directories
	//const Common::FSNode gameDataDir(ConfMan.get("example"));
	//SearchMan.addSubDirectoryMatching(gameDataDir, "example2");
	DebugMan.addDebugChannel(kDMDebugExample, "example", "example desc");

	// register random source
	_rnd = new Common::RandomSource("quux");

	_displayMan = nullptr;
	_dungeonMan = nullptr;
	_eventMan = nullptr;
	_menuMan = nullptr;
	_stopWaitingForPlayerInput = false;
	_gameTimeTicking = false;

	debug("DMEngine::DMEngine");
}

DMEngine::~DMEngine() {
	debug("DMEngine::~DMEngine");

	// dispose of resources
	delete _rnd;
	delete _console;
	delete _displayMan;
	delete _dungeonMan;
	delete _eventMan;
	delete _menuMan;

	// clear debug channels
	DebugMan.clearAllDebugChannels();
}


Common::Error DMEngine::run() {
	initGraphics(320, 200, false);
	_console = new Console(this);
	_displayMan = new DisplayMan(this);
	_dungeonMan = new DungeonMan(this);
	_eventMan = new EventManager(this);
	_menuMan = new MenuMan(this);

	_displayMan->setUpScreens(320, 200);

	_displayMan->loadGraphics();

	_dungeonMan->loadDungeonFile();
	int16 dummyMapIndex = 0;
	_dungeonMan->setCurrentMapAndPartyMap(dummyMapIndex);


	_displayMan->loadCurrentMapGraphics();
	_displayMan->loadPalette(gPalCredits);

	_eventMan->initMouse();
	_eventMan->showMouse(true);

	startGame();


	while (true) {
		_stopWaitingForPlayerInput = false;
		//do {
			_eventMan->processInput();
			_eventMan->processCommandQueue();
		//} while (!_stopWaitingForPlayerInput || !_gameTimeTicking);

		_displayMan->clearScreen(kColorBlack);
		_displayMan->drawDungeon(_dungeonMan->_currMap.partyDir, _dungeonMan->_currMap.partyPosX, _dungeonMan->_currMap.partyPosY);
		// DUMMY CODE:
		_menuMan->drawMovementArrows();
		_displayMan->updateScreen();
		_system->delayMillis(10);
	}


	return Common::kNoError;
}


void DMEngine::startGame() {
	_eventMan->_primaryMouseInput = gPrimaryMouseInput_Interface;
	_eventMan->_secondaryMouseInput = gSecondaryMouseInput_Movement;

	_menuMan->drawMovementArrows();
	_gameTimeTicking = true;
}

} // End of namespace DM
