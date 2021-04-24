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

#include "graphics/renderer.h"

#include "freescape/freescape.h"
#include "freescape/gfx.h"

#include "freescape/loaders/16bitBinaryLoader.h"
#include "freescape/loaders/8bitBinaryLoader.h"

#define OFFSET_DARKSIDE 0xc9ce
#define OFFSET_DRILLER 0x9b40
#define OFFSET_CASTLE 0x9b40
#define OFFSET_TOTALECLIPSE 0xcdb7

namespace Freescape {

FreescapeEngine *g_freescape = NULL;

FreescapeEngine::FreescapeEngine(OSystem *syst)
	: Engine(syst), _screenW(320), _screenH(200), _border(nullptr), _gfx(nullptr) {
	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from run().

	g_freescape = this;
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
	debug("FreescapeEngine::~FreescapeEngine");

	// Dispose your resources here
	delete _rnd;
	//delete _areasByAreaID;
	delete _border;
	delete _gfx;

	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();
}

void FreescapeEngine::convertBorder() {
	Graphics::PixelFormat srcFormat = Graphics::PixelFormat::createFormatCLUT8();
	_border = new Graphics::Surface();
	_border->create(_screenW, _screenH, srcFormat);
	_border->copyRectToSurface(_rawBorder->data(), _border->w, 0, 0, _border->w, _border->h);
	_border->convertToInPlace(_pixelFormat, _rawPalette->data());
}

void FreescapeEngine::drawBorder() {
	if (_border == nullptr)
		return;

	Texture *t = _gfx->createTexture(_border);
	const Common::Rect rect(0, 0, 320, 200);

	_gfx->drawTexturedRect2D(rect, rect, t, 1.0, false);
	_gfx->flipBuffer();
	_system->updateScreen();
	_gfx->freeTexture(t);
}

Common::Error FreescapeEngine::run() {
	// Initialize graphics:
	_pixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
	_gfx = Freescape::createRenderer(_system, &_pixelFormat);
	_gfx->init();
	_gfx->clear();
	
	Binary binary;
	if (_targetName == "3Dkit")
		binary = load16bitBinary("3DKIT.RUN");
	else if (_targetName == "Driller")
		binary = load8bitBinary("DRILLE.EXE", OFFSET_DRILLER);
	else if (_targetName == "Castle")
		binary = load8bitBinary("CME.EXE", OFFSET_CASTLE);
	else
		error("%s is an invalid game", _targetName.c_str());

	_areasByAreaID = binary.areasByAreaID;
	_rawBorder = binary.border;
	_rawPalette = binary.palette;
	_startArea = binary.startArea;
	convertBorder();

	debug("FreescapeEngine::init");
	// Simple main event loop
	Common::Event evt;

	drawBorder();
	assert(_areasByAreaID->contains(_startArea));
	Area *area = (*_areasByAreaID)[_startArea];
	assert(area);
	area->draw(_gfx);

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
