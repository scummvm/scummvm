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
#include "graphics/decoders/bmp.h"

#include "engines/util.h"
#include "engines/advancedDetector.h"

#include "audio/audiostream.h"

#include "prince/prince.h"
#include "prince/font.h"
#include "prince/mhwanh.h"
#include "prince/graphics.h"

namespace Prince {

PrinceEngine::PrinceEngine(OSystem *syst, const PrinceGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_rnd = new Common::RandomSource("prince");

}

PrinceEngine::~PrinceEngine() {
	DebugMan.clearAllDebugChannels();

	delete _rnd;
}

Common::Error PrinceEngine::run() {

    _graph = new GraphicsMan(this);

    const Common::FSNode gameDataDir(ConfMan.get("path"));
    
    debug("Adding all path: %s", gameDataDir.getPath().c_str());

	SearchMan.addSubDirectoryMatching(gameDataDir, "all", 0, 2);
	SearchMan.addSubDirectoryMatching(gameDataDir, "01", 0, 2);

    Common::SeekableReadStream * walizka = SearchMan.createReadStreamForMember("walizka");

    Common::SeekableReadStream *font1stream = SearchMan.createReadStreamForMember("font1.raw");
    if (!font1stream) 
        return Common::kPathNotFile;

    Font font1 = Font();
    if (font1.load(*font1stream))
    {
        font1.getCharWidth(103);
    }
    delete font1stream;

    Common::SeekableReadStream *room = SearchMan.createReadStreamForMember("room");

	//_frontScreen = new Graphics::Surface();
	//_frontScreen->create(640, 480, Graphics::PixelFormat::createFormatCLUT8());

    if (room)
    {
        Graphics::BitmapDecoder roomBmp;
        roomBmp.loadStream(*room);
        //_roomBackground = roomBmp.getSurface();
        _system->getPaletteManager()->setPalette(roomBmp.getPalette(), 0, 256);

        //font1.drawString(_frontScreen, "Hello World", 10, 10, 640, 1);

        MhwanhDecoder walizkaBmp;
        if (walizka)
        {
            debug("Loading walizka");
            if (walizkaBmp.loadStream(*walizka))
            {
                _graph->_roomBackground = walizkaBmp.getSurface();
                _graph->setPalette(walizkaBmp.getPalette());
            }
        }

        _graph->change();


        mainLoop();
    }
    delete room;

	return Common::kNoError;
}

void PrinceEngine::mainLoop() {
	//uint32 nextFrameTime = 0;
	while (!shouldQuit()) {
		Common::Event event;
		Common::EventManager *eventMan = _system->getEventManager();
		while (eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
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

        _graph->update();

		_system->delayMillis(40);

    }
}

} // End of namespace Prince
