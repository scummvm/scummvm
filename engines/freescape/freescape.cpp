#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/system.h"

#include "engines/util.h"

#include "freescape/freescape.h"

#include "freescape/loaders/16bitBinaryLoader.h"
//#include "freescape/loaders/8bitBinaryLoader.h"

#define OFFSET_DARKSIDE 0xc9ce
#define OFFSET_DRILLER 0xcf3e
#define OFFSET_TOTALECLIPSE 0xcdb7

namespace Freescape {

FreescapeEngine::FreescapeEngine(OSystem *syst)
	: Engine(syst), _screenW(320), _screenH(200), _border(nullptr) {
	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from run().

	// Do not initialize graphics here
	// Do not initialize audio devices here
	_hasReceivedTime = false;

	_rotation[0] = 0.0f;
	_rotation[1] = 0.0f;
	_rotation[2] = 0.0f;

	_position[0] = 1000.0f;
	_position[1] = 300.0f;
	_position[2] = 1000.0f;

	_velocity[0] = 0.0f;
	_velocity[1] = 0.0f;
	_velocity[2] = 0.0f;

	// Here is the right place to set up the engine specific debug channels
	DebugMan.addDebugChannel(kFreescapeDebug, "example", "this is just an example for a engine specific debug channel");
	DebugMan.addDebugChannel(kFreescapeDebug2, "example2", "also an example");

	// Don't forget to register your random source
	_rnd = new Common::RandomSource("freescape");

	debug("FreescapeEngine::FreescapeEngine");
}

FreescapeEngine::~FreescapeEngine() {
	debug("QuuxEngine::~QuuxEngine");

	// Dispose your resources here
	delete _rnd;
	delete _areasByAreaID;
	delete _border;

	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();
}

void FreescapeEngine::drawBorder() {
	g_system->copyRectToScreen((const void *)_border->data(), _screenW, 0, 0, _screenW, _screenH);
	g_system->updateScreen();
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

	Binary binary = load16bitBinary("3DKIT.RUN");
	debug("%s", _targetName.c_str());

	_areasByAreaID = binary.areasByAreaID;
	_border = binary.border;

	// 16 colors palette from ultima
	static const byte PALETTE[16][3] = {
		{0, 0, 0}, {0xe0, 0xc0, 0x70}, {0xb0, 0x80, 0x40}, {0x80, 0x60, 0x10}, {0x50, 0x30, 0x00}, {0x20, 0x80, 0xd0}, {0x20, 0x50, 0xb0}, {0x20, 0x30, 0x70}, {0x30, 0x40, 0x30}, {0x30, 0x30, 0x30}, {0x40, 0x40, 0x40}, {0x60, 0x60, 0x60}, {0x70, 0x70, 0x70}, {0x90, 0x90, 0x90}, {0xa0, 0xa0, 0xa0}, {0xc0, 0xc0, 0xc0}};
	g_system->getPaletteManager()->setPalette(&PALETTE[0][0], 0, 16);

	// Create debugger console. It requires GFX to be initialized
	//Console *console = new Console(this);
	//setDebugger(console);

	debug("FreescapeEngine::init");
	drawBorder();
	// Simple main event loop
	Common::Event evt;
	while (!shouldQuit()) {
		g_system->getEventManager()->pollEvent(evt);
		g_system->delayMillis(10);
	}

	return Common::kNoError;
}

bool FreescapeEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher) ||
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

} // namespace Freescape
