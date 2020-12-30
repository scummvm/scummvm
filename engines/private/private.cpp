#include "common/scummsys.h"

#include "audio/decoders/wave.h"
#include "audio/audiostream.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/system.h"

#include "engines/util.h"

#include "private/private.h"
#include "private/grammar.tab.h"
#include "private/grammar.h"

extern int yyparse();

namespace Private {

extern int parse(char*);

PrivateEngine::PrivateEngine(OSystem *syst)
	: Engine(syst) {
	// Put your engine in a sane state, but do nothing big yet;
	// in particular, do not load data from files; rather, if you
	// need to do such things, do them from run().

	// Do not initialize graphics here
	// Do not initialize audio devices here

	// However this is the place to specify all default directories
	//const Common::FSNode gameDataDir(ConfMan.get("path"));
        //SearchMan.addSubDirectoryMatching(gameDataDir, "..");

	// Here is the right place to set up the engine specific debug channels
	DebugMan.addDebugChannel(kPrivateDebugExample, "example", "this is just an example for a engine specific debug channel");
	DebugMan.addDebugChannel(kPrivateDebugExample2, "example2", "also an example");

	// Don't forget to register your random source
	_rnd = new Common::RandomSource("private");

	debug("PrivateEngine::PrivateEngine");
}

PrivateEngine::~PrivateEngine() {
	debug("PrivateEngine::~PrivateEngine");

	// Dispose your resources here
	delete _rnd;

	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();
}

Common::Error PrivateEngine::run() {
	Common::File *file = new Common::File();
	assert(file->open("GAME.DAT"));
	void *buf = malloc(191000); 
	file->read(buf, 191000);
	parse((char *) buf);

	// Initialize graphics using following:
	_screenW = 640;
	_screenH = 480;
	initGraphics(_screenW, _screenH);

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

	// Create debugger console. It requires GFX to be initialized
	Console *console = new Console(this);
	setDebugger(console);

	// Additional setup.
	debug("PrivateEngine::init");

	// Your main even loop should be (invoked from) here.
	debug("PrivateEngine::go: Hello, World!");

	// This test will show up if -d1 and --debugflags=example are specified on the commandline
	debugC(1, kPrivateDebugExample, "Example debug call");

	// This test will show up if --debugflags=example or --debugflags=example2 or both of them and -d3 are specified on the commandline
	debugC(3, kPrivateDebugExample | kPrivateDebugExample2, "Example debug call two");

	// Simple main event loop
	Common::Event evt;
        _videoDecoder = new Video::SmackerDecoder();
  	playVideo("intro/intro.smk");
	while (!shouldQuit()) {
		g_system->getEventManager()->pollEvent(evt);
		g_system->delayMillis(10);

		if (_videoDecoder) {
			if (_videoDecoder->endOfVideo()) {
				_videoDecoder->close();
				delete _videoDecoder;
				_videoDecoder = nullptr;
			} else if (_videoDecoder->needsUpdate()) {
				drawScreen();
			}
		}

	}

	return Common::kNoError;
}

bool PrivateEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error PrivateEngine::loadGameStream(Common::SeekableReadStream *stream) {
	Common::Serializer s(stream, nullptr);
	syncGameStream(s);
	return Common::kNoError;
}

Common::Error PrivateEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	Common::Serializer s(nullptr, stream);
	syncGameStream(s);
	return Common::kNoError;
}

void PrivateEngine::syncGameStream(Common::Serializer &s) {
	// Use methods of Serializer to save/load fields
	int dummy = 0;
	s.syncAsUint16LE(dummy);
}

void PrivateEngine::playSound(const Common::String &name) {
	debugC(1, kPrivateDebugExample, "%s : %s", __FUNCTION__, name.c_str());
	Common::File *file = new Common::File();
	if (!file->open(name))
		error("unable to find sound file %s", name.c_str());

	Audio::AudioStream *stream;
	stream = Audio::makeWAVStream(file, DisposeAfterUse::YES);
	stopSound();
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, stream, -1, Audio::Mixer::kMaxChannelVolume);
}

void PrivateEngine::playVideo(const Common::String &name) {
	debugC(1, kPrivateDebugExample, "%s : %s", __FUNCTION__, name.c_str());
	Common::File *file = new Common::File();
	if (!file->open(name))
		error("unable to find video file %s", name.c_str());

	if (!_videoDecoder->loadStream(file))
	        error("unable to load video %s", name.c_str());
	_videoDecoder->start();

}

void PrivateEngine::stopSound() {
	debugC(3, kPrivateDebugExample, "%s", __FUNCTION__);
	if (_mixer->isSoundHandleActive(_soundHandle))
		_mixer->stopHandle(_soundHandle);
}

void PrivateEngine::drawScreen() {
	if (_videoDecoder && _videoDecoder->needsUpdate()) {
		Graphics::Surface *screen = g_system->lockScreen();
		screen->fillRect(Common::Rect(0, 0, g_system->getWidth(), g_system->getHeight()), 0);

		const Graphics::Surface *surface;
		surface = _videoDecoder->decodeNextFrame();

		int w = surface->w; //CLIP<int>(surface->w, 0, _screenW);
		int h = surface->h; //CLIP<int>(surface->h, 0, _screenH);

		//int x = (_screenW - w) / 2;
		//int y = (_screenH - h) / 2;
     	        //debug("%d %d %d %d", w, h, x, y);

		screen->copyRectToSurface(*surface, 0, 0, Common::Rect(0, 0, w, h));

		g_system->unlockScreen();
 	        g_system->getPaletteManager()->setPalette(_videoDecoder->getPalette(), 0, 256);	
		g_system->updateScreen();
	}
}



} // End of namespace Private
