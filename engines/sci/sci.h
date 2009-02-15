#ifndef SCI_H
#define SCI_H

#include "engines/engine.h"
#include "gui/debugger.h"

//namespace Sci {

// our engine debug levels
enum {
	SCI_DEBUG_RESOURCES = 1 << 0,
	SCI_DEBUG_todo = 1 << 1
};

struct GameFlags {
	//int gameType;
	//int gameId;
	//uint32 features;
	// SCI Version
	// Resource Map Version
	// etc...
};

struct SciGameDescription {
	ADGameDescription desc;
	GameFlags flags;
};

//class Console;

class SciEngine : public Engine {
public:
	SciEngine(OSystem *syst, const SciGameDescription *desc);
	~SciEngine();

	virtual Common::Error init(void);
	virtual Common::Error go(void);

private:
	//Console *_console;
};

/*
// Example console
class Console : public GUI::Debugger {
	public:
		//Console(SciEngine *vm);
		//virtual ~Console(void);
};
*/

//} // End of namespace Sci

#endif // SCI_H
