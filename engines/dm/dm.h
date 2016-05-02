#ifndef DM_H
#define DM_H

#include "common/random.h"
#include "engines/engine.h"
#include "gui/debugger.h"


namespace DM {

class Console;
class DisplayMan;

enum {
	// engine debug channels
	kDMDebugExample = 1 << 0
};

class DMEngine : public Engine {
public:
	DMEngine(OSystem *syst);
	~DMEngine();

	virtual Common::Error run();
	Common::Error go();

private:
	Console *_console;
	Common::RandomSource *_rnd;
	DisplayMan *_displayMan;
};

class Console : public GUI::Debugger {
public:
	Console(DMEngine *vm) {}
	virtual ~Console(void) {}
};

} // End of namespace DM

#endif
