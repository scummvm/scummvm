/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
 
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/random.h"
#include "common/fs.h"
#include "common/keyboard.h"
#include "common/substream.h"

#include "graphics/cursorman.h"
#include "graphics/surface.h"
#include "graphics/palette.h"
#include "graphics/pixelformat.h"

#include "engines/util.h"
#include "engines/advancedDetector.h"

#include "audio/audiostream.h"

#include "prince/prince.h"
#include "prince/font.h"
#include "prince/graphics.h"
#include "prince/script.h"
#include "prince/debugger.h"

#include "video/flic_decoder.h"

namespace Prince {

PrinceEngine::PrinceEngine(OSystem *syst, const PrinceGameDescription *gameDesc) : 
    Engine(syst), _gameDescription(gameDesc), _graph(NULL), _script(NULL),
    _locationNr(0), _debugger(NULL) {
    _rnd = new Common::RandomSource("prince");
    _debugger = new Debugger(this);

}

PrinceEngine::~PrinceEngine() {
    DebugMan.clearAllDebugChannels();

    delete _rnd;
    delete _debugger;
}

GUI::Debugger *PrinceEngine::getDebugger()
{
    return _debugger;
}

Common::Error PrinceEngine::run() {

    _graph = new GraphicsMan(this);

    const Common::FSNode gameDataDir(ConfMan.get("path"));
    
    debug("Adding all path: %s", gameDataDir.getPath().c_str());

    SearchMan.addSubDirectoryMatching(gameDataDir, "all", 0, 2);

    Common::SeekableReadStream *font1stream = SearchMan.createReadStreamForMember("font1.raw");
    if (!font1stream) 
        return Common::kPathNotFile;

    if (_font.load(*font1stream)) {
        _font.getCharWidth(103);
    }
    delete font1stream;

    Common::SeekableReadStream * walizka = SearchMan.createReadStreamForMember("walizka");
    if (!walizka)
        return Common::kPathDoesNotExist;

    debug("Loading walizka");
    if (!_walizkaBmp.loadStream(*walizka)) {
        return Common::kPathDoesNotExist;
    }
       
    Common::SeekableReadStream * skryptStream = SearchMan.createReadStreamForMember("skrypt.dat"); 
    if (!skryptStream)
        return Common::kPathNotFile;

    debug("Loading skrypt");
    _script = new Script(this);
    _script->loadFromStream(*skryptStream);

    delete skryptStream;

    mainLoop();

    return Common::kNoError;
}

bool PrinceEngine::loadLocation(uint16 locationNr)
{
    debug("PrinceEngine::loadLocation %d", locationNr);
    const Common::FSNode gameDataDir(ConfMan.get("path"));
    SearchMan.remove(Common::String::format("%02d", _locationNr));
    _locationNr = locationNr;

    const Common::String locationNrStr = Common::String::format("%02d", _locationNr);
    debug("loadLocation %s", locationNrStr.c_str());
    SearchMan.addSubDirectoryMatching(gameDataDir, locationNrStr, 0, 2);

    // load location background
    Common::SeekableReadStream *room = SearchMan.createReadStreamForMember("room");

    if (!room)
    {
        error("Can't load room bitmap");
        return false;
    }

    if(_roomBmp.loadStream(*room))
    {
        debug("Room bitmap loaded");
        _system->getPaletteManager()->setPalette(_roomBmp.getPalette(), 0, 256);
    }

    delete room;

    return true;
}

bool PrinceEngine::playNextFrame()
{
    if (_flicPlayer.endOfVideo())
        _flicPlayer.rewind();
    const Graphics::Surface *s = _flicPlayer.decodeNextFrame();
    if (s)
    {
        _graph->drawTransparent(s);
        _graph->change();
    }

    return true;
}

bool PrinceEngine::loadAnim(uint16 animNr)
{
    Common::String streamName = Common::String::format("AN%02d", animNr);
    Common::SeekableReadStream * flicStream = SearchMan.createReadStreamForMember(streamName);

    if (!flicStream)
    {
        error("Can't open %s", streamName.c_str());
        return false;
    }

    if (!_flicPlayer.loadStream(flicStream))
    {
        error("Can't load flic stream %s", streamName.c_str());
    }

    debug("%s loaded", streamName.c_str());
    _flicPlayer.start();
    return true;
}

void PrinceEngine::keyHandler(Common::Event event) {
	uint16 nChar = event.kbd.keycode;
	if (event.kbd.hasFlags(Common::KBD_CTRL)) {
		switch (nChar) {
		case Common::KEYCODE_d:
			getDebugger()->attach();
			getDebugger()->onFrame();
			break;
        }
    }
}

void PrinceEngine::mainLoop() {

    while (!shouldQuit()) {
        Common::Event event;
        Common::EventManager *eventMan = _system->getEventManager();
        while (eventMan->pollEvent(event)) {
            switch (event.type) {
            case Common::EVENT_KEYDOWN:
                keyHandler(event);
                break;
            case Common::EVENT_KEYUP:
                break;
            case Common::EVENT_MOUSEMOVE:
                break;
            case Common::EVENT_LBUTTONDOWN:
            case Common::EVENT_RBUTTONDOWN:
                break;
            case Common::EVENT_LBUTTONUP:
            case Common::EVENT_RBUTTONUP:
                break;
            case Common::EVENT_QUIT:
                break;
            default:
                break;
            }
        }

        if (shouldQuit())
            return;

        _script->step();

        if (_roomBmp.getSurface())
            _graph->draw(_roomBmp.getSurface());

        playNextFrame();

        _graph->update();

        _system->delayMillis(40);

    }
}

} // End of namespace Prince
