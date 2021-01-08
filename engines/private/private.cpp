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
#include "common/str.h"

#include "engines/util.h"

#include "image/bmp.h"
#include "graphics/cursorman.h"

#include "private/private.h"
#include "private/grammar.tab.h"
#include "private/grammar.h"

namespace Private {

PrivateEngine *g_private = NULL;

extern int parse(char*);

static const byte MOUSECURSOR_SCI[] = {
    1,1,0,0,0,0,0,0,0,0,0,
    1,2,1,0,0,0,0,0,0,0,0,
    1,2,2,1,0,0,0,0,0,0,0,
    1,2,2,2,1,0,0,0,0,0,0,
    1,2,2,2,2,1,0,0,0,0,0,
    1,2,2,2,2,2,1,0,0,0,0,
    1,2,2,2,2,2,2,1,0,0,0,
    1,2,2,2,2,2,2,2,1,0,0,
    1,2,2,2,2,2,2,2,2,1,0,
    1,2,2,2,2,2,2,2,2,2,1,
    1,2,2,2,2,2,1,0,0,0,0,
    1,2,1,0,1,2,2,1,0,0,0,
    1,1,0,0,1,2,2,1,0,0,0,
    0,0,0,0,0,1,2,2,1,0,0,
    0,0,0,0,0,1,2,2,1,0,0,
    0,0,0,0,0,0,1,2,2,1,0
};

static const byte cursorPalette[] = {
    0, 0, 0,           // Black / Transparent
    0x80, 0x80, 0x80,  // Gray
    0xff, 0xff, 0xff   // White
};

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
    //DebugMan.addDebugChannel(kPrivateDebugExample, "example", "this is just an example for a engine specific debug channel");
    //DebugMan.addDebugChannel(kPrivateDebugExample2, "example2", "also an example");

    // Don't forget to register your random source
    _rnd = new Common::RandomSource("private");

    g_private = this;

    _nextSetting = NULL;
    _nextMovie = NULL;
    _modified = false;
    _mode = -1;

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
    initFuncs();
    parse((char *) buf);
    assert(constants.size() > 0);

    // Initialize graphics using following:
    _screenW = 640;
    _screenH = 480;
    //_pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
    _pixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
    initGraphics(_screenW, _screenH, &_pixelFormat);

    CursorMan.replaceCursor(MOUSECURSOR_SCI, 11, 16, 0, 0, 0);
    CursorMan.replaceCursorPalette(cursorPalette, 0, 3);

    _image = new Image::BitmapDecoder();
    _compositeSurface = new Graphics::ManagedSurface();
    _compositeSurface->create(_screenW, _screenH, _pixelFormat);
    _compositeSurface->setTransparentColor(0x00ff00);

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
    //debug("PrivateEngine::go: Hello, World!");

    // This test will show up if -d1 and --debugflags=example are specified on the commandline
    //debugC(1, kPrivateDebugExample, "Example debug call");

    // This test will show up if --debugflags=example or --debugflags=example2 or both of them and -d3 are specified on the commandline
    //debugC(3, kPrivateDebugExample | kPrivateDebugExample2, "Example debug call two");

    // Simple main event loop
    Common::Event event;
    Common::Point mousePos;
    _videoDecoder = nullptr; //new Video::SmackerDecoder();

    _nextSetting = new Common::String("kGoIntro");

    while (!shouldQuit()) {
        g_system->getEventManager()->pollEvent(event);
        mousePos = g_system->getEventManager()->getMousePos();

        g_system->delayMillis(10);

        // Events
        switch (event.type) {
        case Common::EVENT_QUIT:
        case Common::EVENT_RETURN_TO_LAUNCHER:
            break;

        case Common::EVENT_LBUTTONDOWN:
            selectExit(mousePos);

        }

        // Movies
        if (_nextMovie != NULL) {
            _videoDecoder = new Video::SmackerDecoder();
            playVideo(*_nextMovie);
            _nextMovie = NULL;
            continue;

        }

        if (_videoDecoder) {
            stopSound();
            if (_videoDecoder->endOfVideo()) {
                _videoDecoder->close();
                delete _videoDecoder;
                _videoDecoder = nullptr;
            } else if (_videoDecoder->needsUpdate()) {
                drawScreen();
            }
            continue;
        }

        //if (_compositeSurface)
        //	drawScreen();

        if (_nextSetting != NULL) {
            debug("Executing %s", _nextSetting->c_str());
            _exits.clear();
            loadSetting(_nextSetting);
            _nextSetting = NULL;
            CursorMan.showMouse(false);
            execute(prog);
            CursorMan.showMouse(true);


        }

        g_system->updateScreen();

    }

    return Common::kNoError;
}

void PrivateEngine::selectExit(Common::Point mousePos) {
    Common::String *ns = NULL;
    int rs = 1 << 31;
    int cs = 0;

    for (ExitList::const_iterator it = _exits.begin(); it != _exits.end(); ++it) {
        const ExitInfo e = *it;
        cs = e.rect->width()*e.rect->height();
        if (e.rect->contains(mousePos)) {
            debug("Exit %s %d", e.nextSetting->c_str(), cs);
            if (cs < rs) {
                rs = cs;
                ns = e.nextSetting;
            }

        }
    }
    if (cs > 0)
        _nextSetting = ns;
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

Common::String PrivateEngine::convertPath(Common::String name) {
    Common::String path(name);
    Common::String s1("\\");
    Common::String s2("/");

    while (path.contains(s1))
        Common::replace(path, s1, s2);

    s1 = Common::String("\"");
    s2 = Common::String("");

    Common::replace(path, s1, s2);
    Common::replace(path, s1, s2);

    path.toLowercase();
    return path;
}

void PrivateEngine::playSound(const Common::String &name) {
    debugC(1, kPrivateDebugExample, "%s : %s", __FUNCTION__, name.c_str());

    Common::File *file = new Common::File();
    Common::String path = convertPath(name);

    if (!file->open(path))
        error("unable to find sound file %s", path.c_str());

    Audio::AudioStream *stream;
    stream = Audio::makeWAVStream(file, DisposeAfterUse::YES);
    stopSound();
    _mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, stream, -1, Audio::Mixer::kMaxChannelVolume);
}

void PrivateEngine::playVideo(const Common::String &name) {
    debugC(1, kPrivateDebugExample, "%s : %s", __FUNCTION__, name.c_str());
    Common::File *file = new Common::File();
    Common::String path = convertPath(name);

    if (!file->open(path))
        error("unable to find video file %s", path.c_str());

    if (!_videoDecoder->loadStream(file))
        error("unable to load video %s", path.c_str());
    _videoDecoder->start();

}

void PrivateEngine::stopSound() {
    debugC(3, kPrivateDebugExample, "%s", __FUNCTION__);
    if (_mixer->isSoundHandleActive(_soundHandle))
        _mixer->stopHandle(_soundHandle);
}

void PrivateEngine::loadImage(const Common::String &name, int x, int y) {
    debugC(1, kPrivateDebugExample, "%s : %s", __FUNCTION__, name.c_str());
    Common::File file;
    Common::String path = convertPath(name);
    if (!file.open(path))
        error("unable to load image %s", path.c_str());

    _image->loadStream(file);
    //debug("palette %d %d", _image->getPaletteStartIndex(), _image->getPaletteColorCount());
    //for (int i = 0; i < 30; i=i+3)
    //    debug("%x %x %x", *(_image->getPalette()+i), *(_image->getPalette()+i+1), *(_image->getPalette()+i+2));

    _compositeSurface->transBlitFrom(*_image->getSurface()->convertTo(_pixelFormat, _image->getPalette()), Common::Point(x,y), _pixelFormat.RGBToColor(0,255,0));
    drawScreen();
}


void PrivateEngine::drawScreen() {
    if (_videoDecoder ? _videoDecoder->needsUpdate() : false || _compositeSurface) {
        Graphics::Surface *screen = g_system->lockScreen();
        //screen->fillRect(Common::Rect(0, 0, g_system->getWidth(), g_system->getHeight()), 0);

        Graphics::ManagedSurface *surface = _compositeSurface;

        if (_videoDecoder) {
            Graphics::Surface *frame = new Graphics::Surface;
            frame->create(_screenW, _screenH, _pixelFormat);
            frame->copyFrom(*_videoDecoder->decodeNextFrame());
            surface->transBlitFrom(*frame->convertTo(_pixelFormat, _videoDecoder->getPalette()));
        }

        int w = surface->w; //CLIP<int>(surface->w, 0, _screenW);
        int h = surface->h; //CLIP<int>(surface->h, 0, _screenH);
        assert(w == _screenW && h == _screenH);

        screen->copyRectToSurface(*surface, 0, 0, Common::Rect(0, 0, _screenW, _screenH));

        g_system->unlockScreen();
        //if (_image->getPalette() != nullptr)
        //    g_system->getPaletteManager()->setPalette(_image->getPalette(), _image->getPaletteStartIndex(), _image->getPaletteColorCount());
        //if (_image->getPalette() != nullptr)
        //	g_system->getPaletteManager()->setPalette(_image->getPalette(), 0, 256);
        //g_system->getPaletteManager()->setPalette(_videoDecoder->getPalette(), 0, 256);
        g_system->updateScreen();
    }
}



} // End of namespace Private
