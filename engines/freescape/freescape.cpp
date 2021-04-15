#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/system.h"

#include "engines/util.h"

#include "freescape/freescape.h"

#include "freescape/loaders/16bitBinaryLoader.h"
//#include "freescape/loaders/8bitBinaryLoader.h"

#define OFFSET_DARKSIDE       0xc9ce
#define OFFSET_DRILLER        0xcf3e
#define OFFSET_TOTALECLIPSE   0xcdb7

namespace Freescape {

FreescapeEngine::FreescapeEngine(OSystem *syst)
	: Engine(syst) {
	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from run().

	// Do not initialize graphics here
	// Do not initialize audio devices here

	// However this is the place to specify all default directories
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "sound");

	// Here is the right place to set up the engine specific debug channels
	DebugMan.addDebugChannel(kQuuxDebugExample, "example", "this is just an example for a engine specific debug channel");
	DebugMan.addDebugChannel(kQuuxDebugExample2, "example2", "also an example");

	// Don't forget to register your random source
	_rnd = new Common::RandomSource("freescape");

	debug("FreescapeEngine::FreescapeEngine");
}

FreescapeEngine::~FreescapeEngine() {
	debug("QuuxEngine::~QuuxEngine");

	// Dispose your resources here
	delete _rnd;

	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();
}

Common::Error FreescapeEngine::run() {
	// Initialize graphics using following:
	initGraphics(320, 200);

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

    Game *_game = load16bitBinary("3DKIT.RUN");
    Common::File *file = new Common::File();

    if (!file->open("3DKIT.RUN")) {
        delete file;
    	return Common::kNoError;
    }

	const int32 fileSize = file->size();
	byte *buf = (byte *)malloc(fileSize);
	file->read(buf, fileSize);

	_pixelFormat = g_system->getScreenFormat();
	// 16 colors palette from ultima
    static const byte PALETTE[16][3] = {
                { 0, 0, 0 },{ 0xe0, 0xc0, 0x70 },{ 0xb0, 0x80, 0x40 },{ 0x80, 0x60, 0x10 },
                { 0x50, 0x30, 0x00 },{ 0x20, 0x80, 0xd0 },{ 0x20, 0x50, 0xb0 },{ 0x20, 0x30, 0x70 },
                { 0x30, 0x40, 0x30 },{ 0x30, 0x30, 0x30 },{ 0x40, 0x40, 0x40 },{ 0x60, 0x60, 0x60 },
                { 0x70, 0x70, 0x70 },{ 0x90, 0x90, 0x90 },{ 0xa0, 0xa0, 0xa0 },{ 0xc0, 0xc0, 0xc0 }
    };
    g_system->getPaletteManager()->setPalette(&PALETTE[0][0], 0, 16);

	int i = 0;
	byte *p = buf;

	while(i < fileSize-4) {
		if(*((uint32*) p) == 0xfa002445) {
			debug("Border found at %x", i);
			g_system->copyRectToScreen((const void*) p, 320, 0, 0, 320, 200);
		}
		p++;
		i++;

	}

	// Create debugger console. It requires GFX to be initialized
	Console *console = new Console(this);
	setDebugger(console);

	debug("FreescapeEngine::init");

	// Simple main event loop
	Common::Event evt;
	while (!shouldQuit()) {
		g_system->getEventManager()->pollEvent(evt);
		g_system->delayMillis(10);
	}

	return Common::kNoError;
}

bool FreescapeEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error FreescapeEngine::loadGameStream(Common::SeekableReadStream *stream) {
	Common::Serializer s(stream, nullptr);
	syncGameStream(s);
	return Common::kNoError;
}

Common::Error FreescapeEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	Common::Serializer s(nullptr, stream);
	syncGameStream(s);
	return Common::kNoError;
}

void FreescapeEngine::syncGameStream(Common::Serializer &s) {
	// Use methods of Serializer to save/load fields
	int dummy = 0;
	s.syncAsUint16LE(dummy);
}

} // End of namespace Quux
