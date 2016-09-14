#ifndef CRYO_CRYO_H
#define CRYO_CRYO_H

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/random.h"
#include "engines/engine.h"
#include "gui/debugger.h"
#include "graphics/surface.h"
#include "graphics/screen.h"
#include "cryo/eden.h"

namespace Cryo {

class Console;

// our engine debug channels
enum {
	kCryoDebugExample = 1 << 0,
	kCryoDebugExample2 = 1 << 1
	                     // next new channel must be 1 << 2 (4)
	                     // the current limitation is 32 debug channels (1 << 31 is the last one)
};

class CryoEngine : public Engine {
public:
	CryoEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~CryoEngine();

	virtual Common::Error run();

	// Detection related functions
	const ADGameDescription *_gameDescription;
	const char *getGameId() const;
	Common::Platform getPlatform() const;

	// We need random numbers
	Common::RandomSource *_rnd;

	Graphics::Surface _screen;
	EdenGame game;

private:
	Console *_console;
};

extern CryoEngine *g_ed;

// Example console class
class Console : public GUI::Debugger {
public:
	Console(CryoEngine *vm) {}
	virtual ~Console(void) {}
};

} // End of namespace Cryo

#endif
