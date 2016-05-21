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
#include "dm/gfx.h"
#include "dm/dungeonman.h"

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

	debug("DMEngine::DMEngine");
}

DMEngine::~DMEngine() {
	debug("DMEngine::~DMEngine");

	// dispose of resources
	delete _rnd;
	delete _console;
	delete _displayMan;
	delete _dungeonMan;

	// clear debug channels
	DebugMan.clearAllDebugChannels();
}


Common::Error DMEngine::run() {
	initGraphics(320, 200, false);
	_console = new Console(this);
	_displayMan = new DisplayMan(this);
	_dungeonMan = new DungeonMan(this);

	_displayMan->setUpScreens(320, 200);

	_displayMan->loadGraphics();

	_dungeonMan->loadDungeonFile();
	int16 dummyMapIndex = 0;
	_dungeonMan->setCurrentMapAndPartyMap(dummyMapIndex);




	_displayMan->loadCurrentMapGraphics();

	_displayMan->loadPalette(gPalCredits);

	CurrMapData &currMap = _dungeonMan->_currMap;
	while (true) {
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN && !event.synthetic)
				switch (event.kbd.keycode) {
				case Common::KEYCODE_w:
					_dungeonMan->mapCoordsAfterRelMovement(currMap.partyDir, 1, 0, currMap.partyPosX, currMap.partyPosY);
					break;
				case Common::KEYCODE_a:
					_dungeonMan->mapCoordsAfterRelMovement(currMap.partyDir, 0, -1, currMap.partyPosX, currMap.partyPosY);
					break;
				case Common::KEYCODE_s:
					_dungeonMan->mapCoordsAfterRelMovement(currMap.partyDir, -1, 0, currMap.partyPosX, currMap.partyPosY);
					break;
				case Common::KEYCODE_d:
					_dungeonMan->mapCoordsAfterRelMovement(currMap.partyDir, 0, 1, currMap.partyPosX, currMap.partyPosY);
					break;
				case Common::KEYCODE_q:
					turnDirLeft(currMap.partyDir);
					break;
				case Common::KEYCODE_e:
					turnDirRight(currMap.partyDir);
					break;
				case Common::KEYCODE_UP:
					if (dummyMapIndex < 13)
						_dungeonMan->setCurrentMapAndPartyMap(++dummyMapIndex);
					break;
				case Common::KEYCODE_DOWN:
					if (dummyMapIndex > 0)
						_dungeonMan->setCurrentMapAndPartyMap(--dummyMapIndex);
					break;
				}
		}
		_displayMan->clearScreen(kColorBlack);
		_displayMan->drawDungeon(_dungeonMan->_currMap.partyDir, _dungeonMan->_currMap.partyPosX, _dungeonMan->_currMap.partyPosY);
		_displayMan->updateScreen();
		_system->delayMillis(10);
	}


	return Common::kNoError;
}

} // End of namespace DM
