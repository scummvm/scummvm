#ifndef QUUX_H
#define QUUX_H
 
#include "common/random.h"
#include "engines/engine.h"
#include "gui/debugger.h"
 
namespace Quux {
 
class Console;
 
// our engine debug levels
enum {
	kQuuxDebugExample = 1 << 0,
	kQuuxDebugExample2 = 1 << 1
	// next new level must be 1 << 2 (4)
	// the current limitation is 32 debug levels (1 << 31 is the last one)
};
 
class QuuxEngine : public Engine {
public:
	QuuxEngine(OSystem *syst);
	~QuuxEngine();
 
	virtual Common::Error run();
 
private:
	Console *_console;
 
	// We need random numbers
	Common::RandomSource _rnd;
};
 
// Example console class
class Console : public GUI::Debugger {
public:
	Console(QuuxEngine *vm) {}
	virtual ~Console(void) {}
};
 
} // End of namespace Quux
 
#endif
