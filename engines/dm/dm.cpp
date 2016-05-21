#include "common/scummsys.h"
#include "common/system.h"

#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"

#include "engines/util.h"
#include "engines/engine.h"
#include "graphics/palette.h"
#include "common/file.h"

#include "dm/dm.h"
#include "dm/gfx.h"
#include "dm/dungeonman.h"

namespace DM {


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
	_dungeonMan->setCurrentMapAndPartyMap(0);




	_displayMan->loadCurrentMapGraphics();

	_displayMan->loadPalette(gPalCredits);

	byte pos[] = {kDirSouth, 1, 3, kDirSouth, 1, 4, kDirSouth, 1, 5, kDirSouth, 1, 6, kDirSouth, 1, 7, kDirEast, 1, 7};

	uint16 i = 0; //TODO: testing, please delete me
	while (true) {
		_displayMan->clearScreen(kColorBlack);
		_displayMan->drawDungeon((direction)pos[i*3 + 0], pos[i*3 + 1], pos[i*3 + 2]);
		//_displayMan->drawDungeon(_dungeonMan->_currMap.partyDir, _dungeonMan->_currMap.partyPosX, _dungeonMan->_currMap.partyPosY + i);
		_displayMan->updateScreen();
		_system->delayMillis(1000); //TODO: testing, please set me to 10
		if(i == 0)
			_system->delayMillis(0000); //TODO: testing, please set me to 10
		if (++i == 6) i = 0;
	}


	return Common::kNoError;
}

} // End of namespace DM
