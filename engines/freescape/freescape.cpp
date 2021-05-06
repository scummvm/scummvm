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
#define OFFSET_DRILLER_EGA 0x9b40
#define OFFSET_DRILLER_CGA 0x7bb0
#define OFFSET_CASTLE 0xe472
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

void FreescapeEngine::drawBorder() {
	if (_border == nullptr)
		return;

	Texture *t = _gfx->createTexture(_border);
	const Common::Rect rect(0, 0, _screenW, _screenH);
	//g_system->copyRectToScreen(_borderSurf->getPixels(), _borderSurf->pitch, 0, 0, _screenW, _screenH);


	_gfx->drawTexturedRect2D(rect, rect, t, 1.0, false);
	_gfx->flipBuffer();
	_system->updateScreen();
	_gfx->freeTexture(t);
}

Common::Error FreescapeEngine::run() {
	// Initialize graphics:
	_gfx = Freescape::createRenderer(_system);
	_gfx->init();
	_gfx->clear();
	
	Binary binary;
	Common::String renderMode = "";

	if (_targetName == "3Dkit")
		binary = load16bitBinary("3DKIT.RUN");
	else if (_targetName == "3Dkitcube")
		binary = load16bitBinary("CUBE.RUN");
	else if (_targetName == "Driller") {
		renderMode = "ega";
		if (!ConfMan.hasKey("render_mode"))
			renderMode = "ega";

		if (renderMode == "ega")
			binary = load8bitBinary("DRILLE.EXE", OFFSET_DRILLER_EGA);
		else if (renderMode == "cga")
			binary = load8bitBinary("DRILLC.EXE", OFFSET_DRILLER_CGA);
		else
			error("Invalid render mode %s for Driller", renderMode.c_str());

	}
	else if (_targetName == "Castle")
		binary = load8bitBinary("CME.EXE", OFFSET_CASTLE);
	else
		error("%s is an invalid game", _targetName.c_str());

	if (binary.areasByAreaID) {
		_areasByAreaID = binary.areasByAreaID;
		if (binary.palette) {
			uint pSize = 0; 
			if (binary.bits == 16)
				pSize = 256;
			else if (binary.bits == 8)
				pSize = 16;
			else
				error("Invalid number of bits %d", binary.bits);

			Graphics::PixelBuffer *palette = new Graphics::PixelBuffer(_gfx->_palettePixelFormat, pSize, DisposeAfterUse::NO);
			*palette = binary.palette->data();
			_gfx->_palette = palette;
		}

		if (binary.border) {
			Graphics::PixelBuffer *border = new Graphics::PixelBuffer(_gfx->_originalPixelFormat, 320*200, DisposeAfterUse::NO);
			*border = binary.border->data();
			_border = _gfx->convertFromPalette(border);
		}

		_startArea = 1; //binary.startArea;

		assert(_areasByAreaID->contains(_startArea));
		Area *area = (*_areasByAreaID)[_startArea];
		assert(area);
		drawBorder();
		area->draw(_gfx);
	}	
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
