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

#include "private/cursors.h"
#include "private/private.h"
#include "private/grammar.tab.h"
#include "private/grammar.h"


namespace Private {

PrivateEngine *g_private = NULL;

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
    //DebugMan.addDebugChannel(kPrivateDebugExample, "example", "this is just an example for a engine specific debug channel");
    //DebugMan.addDebugChannel(kPrivateDebugExample2, "example2", "also an example");

    // Don't forget to register your random source
    _rnd = new Common::RandomSource("private");

    g_private = this;

    _nextSetting = NULL;
    _nextMovie = NULL;
    _modified = false;
    _mode = -1;
    _frame = new Common::String("inface/general/inface2.bmp");


}

PrivateEngine::~PrivateEngine() {
    debug("PrivateEngine::~PrivateEngine");

    // Dispose your resources here
    delete _rnd;

    // Remove all of our debug levels here
    DebugMan.clearAllDebugChannels();
}

Common::Error PrivateEngine::run() {

    assert(_installerArchive.open("SUPPORT/ASSETS.Z"));
    Common::SeekableReadStream *file = NULL;

    if (_installerArchive.hasFile("GAME.DAT")) // if the full game is used
        file = _installerArchive.createReadStreamForMember("GAME.DAT");
    else if (_installerArchive.hasFile("GAME.TXT")) // if the demo is used
        file = _installerArchive.createReadStreamForMember("GAME.TXT");

    assert(file != NULL);
    void *buf = malloc(191000);
    file->read(buf, 191000);

    // Initialize stuff
    initInsts();
    initFuncs();
    initCursors();

    parse((char *) buf);
    assert(constants.size() > 0);

    // Initialize graphics using following:
    _screenW = 640;
    _screenH = 480;
    //_pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
    _pixelFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
    _transparentColor = _pixelFormat.RGBToColor(0,255,0);
    initGraphics(_screenW, _screenH, &_pixelFormat);

    CursorMan.replaceCursor(_cursors.getVal("default"), 11, 16, 0, 0, 0, true);
    CursorMan.replaceCursorPalette(cursorPalette, 0, 3);
    CursorMan.showMouse(true);

    _origin = new Common::Point(0, 0);
    _image = new Image::BitmapDecoder();
    _compositeSurface = new Graphics::ManagedSurface();
    _compositeSurface->create(_screenW, _screenH, _pixelFormat);
    _compositeSurface->setTransparentColor(_transparentColor);

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
        if (_mode == 1)
            drawScreenFrame();

        while (g_system->getEventManager()->pollEvent(event)) {
            mousePos = g_system->getEventManager()->getMousePos();
            // Events
            switch (event.type) {
            case Common::EVENT_KEYDOWN:
                if (event.kbd.keycode == Common::KEYCODE_ESCAPE && _videoDecoder)
                    skipVideo();
                break;

            case Common::EVENT_QUIT:
            case Common::EVENT_RETURN_TO_LAUNCHER:
                break;

            case Common::EVENT_LBUTTONDOWN:
                selectMask(mousePos);
                if (!_nextSetting)
                    selectExit(mousePos);
                break;

            case Common::EVENT_MOUSEMOVE:
                CursorMan.replaceCursor(_cursors.getVal("default"), 11, 16, 0, 0, 0, true);
                cursorExit(mousePos);
                cursorMask(mousePos);
                //
                break;

            default:
            {}

            }
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

        if (_nextSetting != NULL) {
            debug("Executing %s", _nextSetting->c_str());
            _exits.clear();
            _masks.clear();
            loadSetting(_nextSetting);
            _nextSetting = NULL;
            //CursorMan.showMouse(false);
            execute(prog);
            CursorMan.replaceCursor(_cursors.getVal("default"), 11, 16, 0, 0, 0, true);
            //CursorMan.showMouse(true);
        }

        g_system->updateScreen();
        g_system->delayMillis(10);
    }

    return Common::kNoError;
}

bool PrivateEngine::cursorExit(Common::Point mousePos) {
    //debug("Mousepos %d %d", mousePos.x, mousePos.y);
    mousePos = mousePos - *_origin;
    if (mousePos.x < 0 || mousePos.y < 0)
        return false;

    ExitInfo e;
    bool inside = false;
    for (ExitList::iterator it = _exits.begin(); it != _exits.end(); ++it) {
        e = *it;
        if (e.rect->contains(mousePos)) {
            inside = true;
            if (e.cursor != NULL)
                CursorMan.replaceCursor(_cursors.getVal(*e.cursor), 32, 32, 0, 0, 0, true);
        }
    }
    //if (!inside)
    //    CursorMan.replaceCursor(_cursors.getVal("default"), 11, 16, 0, 0, 0, true);

    return inside;
}

bool PrivateEngine::cursorMask(Common::Point mousePos) {
    //debug("Mousepos %d %d", mousePos.x, mousePos.y);
    mousePos = mousePos - *_origin;
    if (mousePos.x < 0 || mousePos.y < 0)
        return false;

    MaskInfo m;
    bool inside = false;
    for (MaskList::iterator it = _masks.begin(); it != _masks.end(); ++it) {
        m = *it;

        if (mousePos.x > m.surf->h || mousePos.y > m.surf->w)
            continue;

        //debug("Testing mask %s", m.nextSetting->c_str());
        if ( *((uint32*) m.surf->getBasePtr(mousePos.x, mousePos.y)) != _transparentColor) {
            //debug("Inside!");
            if (m.nextSetting != NULL) { // TODO: check this
                inside = true;
                debug("Rendering cursor mask %s", m.cursor->c_str());
                assert(_cursors.contains(*m.cursor));
                CursorMan.replaceCursor(_cursors.getVal(*m.cursor), 32, 32, 0, 0, 0, true);
                break;
            }

        }
    }
    //if (!inside)
    //    CursorMan.replaceCursor(_cursors.getVal("default"), 11, 16, 0, 0, 0, true);
    return inside;
}


void PrivateEngine::selectExit(Common::Point mousePos) {
    //debug("Mousepos %d %d", mousePos.x, mousePos.y);
    mousePos = mousePos - *_origin;
    if (mousePos.x < 0 || mousePos.y < 0)
        return;

    Common::String *ns = NULL;
    int rs = 100000000;
    int cs = 0;
    ExitInfo e;
    for (ExitList::iterator it = _exits.begin(); it != _exits.end(); ++it) {
        e = *it;
        cs = e.rect->width()*e.rect->height();
        //debug("Testing exit %s %d", e.nextSetting->c_str(), cs);
        if (e.rect->contains(mousePos)) {
            debug("Inside! %d %d", cs, rs);
            if (cs < rs && e.nextSetting != NULL) { // TODO: check this
                debug("Found Exit %s %d", e.nextSetting->c_str(), cs);
                rs = cs;
                ns = e.nextSetting;
            }

        }
    }
    if (ns != NULL) {
        debug("Exit selected %s", ns->c_str());
        _nextSetting = ns;
    }
}

void PrivateEngine::selectMask(Common::Point mousePos) {
    //debug("Mousepos %d %d", mousePos.x, mousePos.y);
    mousePos = mousePos - *_origin;
    if (mousePos.x < 0 || mousePos.y < 0)
        return;

    Common::String *ns = NULL;
    MaskInfo m;
    for (MaskList::iterator it = _masks.begin(); it != _masks.end(); ++it) {
        m = *it;

        //debug("Testing mask %s", m.nextSetting->c_str());
        if ( *((uint32*) m.surf->getBasePtr(mousePos.x, mousePos.y)) != _transparentColor) {
            debug("Inside!");
            if (m.nextSetting != NULL) { // TODO: check this
                debug("Found Mask %s", m.nextSetting->c_str());
                ns = m.nextSetting;
            }

            if (m.flag != NULL) { // TODO: check this
                setSymbol(m.flag, 1);
            }

            break;

        }
    }
    if (ns != NULL) {
        debug("Mask selected %s", ns->c_str());
        _nextSetting = ns;
    }
}

bool PrivateEngine::hasFeature(EngineFeature f) const {
    return
        (f == kSupportsReturnToLauncher);
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

    Audio::LoopingAudioStream *stream;
    stream = new Audio::LoopingAudioStream(Audio::makeWAVStream(file, DisposeAfterUse::YES), 1);
    stopSound();
    _mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, stream, -1, Audio::Mixer::kMaxChannelVolume);
}

void PrivateEngine::playVideo(const Common::String &name) {
    debug("%s : %s", __FUNCTION__, name.c_str());
    Common::File *file = new Common::File();
    Common::String path = convertPath(name);

    if (!file->open(path))
        error("unable to find video file %s", path.c_str());

    if (!_videoDecoder->loadStream(file))
        error("unable to load video %s", path.c_str());
    _videoDecoder->start();

}

void PrivateEngine::skipVideo() {
    _videoDecoder->close();
    delete _videoDecoder;
    _videoDecoder = nullptr;
}


void PrivateEngine::stopSound() {
    debugC(3, kPrivateDebugExample, "%s", __FUNCTION__);
    if (_mixer->isSoundHandleActive(_soundHandle))
        _mixer->stopHandle(_soundHandle);
}

void PrivateEngine::loadImage(const Common::String &name, int x, int y, bool drawn) {
    debugC(1, kPrivateDebugExample, "%s : %s", __FUNCTION__, name.c_str());
    Common::File file;
    Common::String path = convertPath(name);
    if (!file.open(path))
        error("unable to load image %s", path.c_str());

    _image->loadStream(file);
    //debug("palette %d %d", _image->getPaletteStartIndex(), _image->getPaletteColorCount());
    //for (int i = 0; i < 30; i=i+3)
    //    debug("%x %x %x", *(_image->getPalette()+i), *(_image->getPalette()+i+1), *(_image->getPalette()+i+2));

    _compositeSurface->transBlitFrom(*_image->getSurface()->convertTo(_pixelFormat, _image->getPalette()), *_origin + Common::Point(x,y), _transparentColor);
    drawScreen();
}

void PrivateEngine::drawScreenFrame() {
    Common::File file;
    Common::String path = convertPath(*_frame);
    if (!file.open(path))
        error("unable to load image %s", path.c_str());

    _image->loadStream(file);
    _compositeSurface->transBlitFrom(*_image->getSurface()->convertTo(_pixelFormat, _image->getPalette()), Common::Point(0,0));
    //drawScreen();
}


Graphics::ManagedSurface *PrivateEngine::loadMask(const Common::String &name, int x, int y, bool drawn) {
    debugC(1, kPrivateDebugExample, "%s : %s", __FUNCTION__, name.c_str());
    Common::File file;
    Common::String path = convertPath(name);
    if (!file.open(path))
        error("unable to load mask %s", path.c_str());

    _image->loadStream(file);
    Graphics::ManagedSurface *surf = new Graphics::ManagedSurface();
    surf->create(_screenW, _screenH, _pixelFormat);
    surf->transBlitFrom(*_image->getSurface()->convertTo(_pixelFormat, _image->getPalette()), Common::Point(x,y));

    if (drawn) {
        _compositeSurface->transBlitFrom(surf->rawSurface(), *_origin + Common::Point(x,y), _transparentColor);
        drawScreen();
    }

    return surf;
}

void PrivateEngine::drawScreen() {
    Graphics::Surface *screen = g_system->lockScreen();
    Graphics::ManagedSurface *surface = _compositeSurface;
    int w = surface->w;
    int h = surface->h;

    if (_videoDecoder) {
        Graphics::Surface *frame = new Graphics::Surface;
        frame->create(_screenW, _screenH, _pixelFormat);
        frame->copyFrom(*_videoDecoder->decodeNextFrame());
        const Common::Point o(_origin->x, _origin->y);
        surface->transBlitFrom(*frame->convertTo(_pixelFormat, _videoDecoder->getPalette()), o);
    }

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

bool PrivateEngine::getRandomBool(uint p) {
    uint r = _rnd->getRandomNumber(100);
    return (r <= p);
}


} // End of namespace Private
