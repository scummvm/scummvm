#include "common/scummsys.h"
#include "common/system.h"
 
#include "engines/util.h"
 
#include "testbed/testbed.h"
#include "testbed/graphics.h"
 
namespace Testbed {
 
TestbedEngine::TestbedEngine(OSystem *syst) 
 : Engine(syst) {
	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from init().
 
	// Do not initialize graphics here
 
	// However this is the place to specify all default directories
 
	printf("TestbedEngine::TestbedEngine()\n");
}
 
TestbedEngine::~TestbedEngine() {
	// Dispose your resources here
	printf("TestbedEngine::~TestbedEngine()\n");
 
	// Remove all of our debug levels here
}

Common::Error TestbedEngine::run() {
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
 
	// Additional setup.
	printf("TestbedEngine::init\n");

	GFXTestSuite ts;
	ts.execute();

	// Your main even loop should be (invoked from) here.
	printf("TestbedEngine::go: Hello, World!\n");
	
	return Common::kNoError;
}
 
} // End of namespace Testbed
