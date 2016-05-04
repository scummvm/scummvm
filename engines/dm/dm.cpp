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




	byte *palette = new byte[256 * 3];
	for (int i = 0; i < 16; ++i)
		palette[i * 3] = palette[i * 3 + 1] = palette[i * 3 + 2] = i * 16;

	_displayMan->setPalette(palette, 16);

	byte *buffer = new byte[320 * 200];
	for (int i = 0; i < 320 * 100; ++i)
		buffer[i] = 4;
	for (int i = 320 * 100; i < 320 * 200; ++i)
		buffer[i] = 6;

	_system->copyRectToScreen(buffer, 320, 0, 0, 320, 200);
	_system->updateScreen();


	uint16 width = _displayMan->getImageWidth(75);
	uint16 height = _displayMan->getImageHeight(75);
	byte *cleanByteImg0Data = new byte[width * height];
	_displayMan->loadIntoBitmap(75, cleanByteImg0Data);
	_displayMan->blitToScreen(cleanByteImg0Data, width, height, 30, 30);


	while (true) {
		_displayMan->updateScreen();
	}


	delete[] buffer;
	delete[] cleanByteImg0Data;

	return Common::kNoError;
}

} // End of namespace DM
