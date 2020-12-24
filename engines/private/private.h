#ifndef PRIVATE_H
#define PRIVATE_H

#include "common/random.h"
#include "common/serializer.h"
#include "engines/engine.h"
#include "gui/debugger.h"

namespace Private {

class Console;

// our engine debug channels
enum {
	kPrivateDebugExample = 1 << 0,
	kPrivateDebugExample2 = 1 << 1
	// next new channel must be 1 << 2 (4)
	// the current limitation is 32 debug channels (1 << 31 is the last one)
};

class PrivateEngine : public Engine {
private:
	// We need random numbers
	Common::RandomSource *_rnd;
public:
	PrivateEngine(OSystem *syst);
	~PrivateEngine();

	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
	bool canLoadGameStateCurrently() override { return true; }
	bool canSaveGameStateCurrently() override { return true; }
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	void syncGameStream(Common::Serializer &s);
};

// Example console class
class Console : public GUI::Debugger {
public:
	Console(PrivateEngine *vm) {
	}
	virtual ~Console(void) {
	}
};

} // End of namespace Quux

#endif
