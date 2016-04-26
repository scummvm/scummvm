#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "gui/EventRecorder.h"
#include "common/file.h"
#include "common/fs.h"

#include "engines/util.h"

#include "dm/dm.h"

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

	// regiser random source
	_rnd = new Common::RandomSource("quux");

	debug("DMEngine::DMEngine");
}

DMEngine::~DMEngine() {
	debug("DMEngine::~DMEngine");

	// dispose of resources
	delete _rnd;

	// clear debug channels
	DebugMan.clearAllDebugChannels();
}

Common::Error DMEngine::run() {
	// Init graphics
	initGraphics(320, 200, false);

	// Create debug console (it requirese GFX to be inited)
	_console = new Console(this);

	// Additional setup
	debug("DMEngine::init");

	// Run main loop
	debug("DMEngine:: start main loop");

	while (true)
		debug("Run!");

	return Common::kNoError;
}

} // End of namespace DM
