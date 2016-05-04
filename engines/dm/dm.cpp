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

	_displayMan->loadPalette(palCredits);

	uint16 width = _displayMan->getImageWidth(1);
	uint16 height = _displayMan->getImageHeight(1);
	byte *cleanByteImg0Data = new byte[width * height];
	_displayMan->loadIntoBitmap(1, cleanByteImg0Data);
	_displayMan->blitToScreen(cleanByteImg0Data, width, height, 0, 0);


	while (true) {
		_displayMan->updateScreen();
		_system->delayMillis(10);
	}

	delete[] cleanByteImg0Data;

	return Common::kNoError;
}

} // End of namespace DM
