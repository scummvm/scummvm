#include "common/scummsys.h"
 
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/EventRecorder.h"
#include "common/file.h"
#include "common/fs.h"
 
#include "engines/util.h"
 
#include "testbed/testbed.h"
 
namespace Quux {
 
QuuxEngine::QuuxEngine(OSystem *syst) 
 : Engine(syst) {
	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from init().
 
	// Do not initialize graphics here
 
	// However this is the place to specify all default directories
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "sound");
 
	// Here is the right place to set up the engine specific debug levels
	DebugMan.addDebugChannel(kQuuxDebugExample, "example", "this is just an example for a engine specific debug level");
	DebugMan.addDebugChannel(kQuuxDebugExample2, "example2", "also an example");
 
	// Don't forget to register your random source
	g_eventRec.registerRandomSource(_rnd, "quux");
 
	printf("QuuxEngine::QuuxEngine\n");
}
 
QuuxEngine::~QuuxEngine() {
	// Dispose your resources here
	printf("QuuxEngine::~QuuxEngine\n");
 
	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();
}
 
Common::Error QuuxEngine::run() {
	// Initialize graphics using following:
	initGraphics(320, 200, false);
 
	// You could use backend transactions directly as an alternative,
	// but it isn't recommended, until you want to handle the error values
	// from OSystem::endGFXTransaction yourself.
	// This is just an example template:
	//_system->beginGFXTransaction();
	//	// This setup the graphics mode according to users seetings
	//	initCommonGFX(false);
	//
	//	// Specify dimensions of game graphics window.
	//	// In this example: 320x200
	//	_system->initSize(320, 200);
	//FIXME: You really want to handle
	//OSystem::kTransactionSizeChangeFailed here
	//_system->endGFXTransaction();
 
	// Create debugger console. It requires GFX to be initialized
	_console = new Console(this);
 
	// Additional setup.
	printf("QuuxEngine::init\n");
 
	// Your main even loop should be (invoked from) here.
	printf("QuuxEngine::go: Hello, World!\n");
 
	// This test will show up if -d1 and --debugflags=example are specified on the commandline
	debugC(1, kQuuxDebugExample, "Example debug call");
 
	// This test will show up if --debugflags=example or --debugflags=example2 or both of them and -d3 are specified on the commandline
	debugC(3, kQuuxDebugExample | kQuuxDebugExample2, "Example debug call two");
 
	return Common::kNoError;
}
 
} // End of namespace Quux
