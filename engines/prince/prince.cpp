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
#include "prince/object.h"
#include "prince/mob.h"
#include "prince/sound.h"
#include "prince/variatxt.h"
#include "prince/flags.h"

#include "video/flic_decoder.h"

namespace Prince {

Graphics::Surface *loadCursor(const char *curName)
{
	Common::SeekableReadStream *curStream = SearchMan.createReadStreamForMember(curName);
	if (!curStream) {
		error("Can't load %s", curName);
		return NULL;
	}

	curStream->skip(4);
	uint16 w = curStream->readUint16LE();
	uint16 h = curStream->readUint16LE();

	debug("Loading cursor %s, w %d, h %d", curName, w, h);

	Graphics::Surface *curSurface = new Graphics::Surface();
	curSurface->create(w, h, Graphics::PixelFormat::createFormatCLUT8());
	for (int ih = 0; ih < h; ++ih) {
		curStream->read(curSurface->getBasePtr(0, ih), w);
	}

	delete curStream;
	return curSurface;
}



PrinceEngine::PrinceEngine(OSystem *syst, const PrinceGameDescription *gameDesc) : 
	Engine(syst), _gameDescription(gameDesc), _graph(NULL), _script(NULL),
	_locationNr(0), _debugger(NULL), _objectList(NULL), _mobList(NULL), _midiPlayer(NULL),
	_cameraX(0), _newCameraX(0), _frameNr(0) {

	// Debug/console setup
	DebugMan.addDebugChannel(DebugChannel::kScript, "script", "Prince Script debug channel");
	DebugMan.addDebugChannel(DebugChannel::kEngine, "engine", "Prince Engine debug channel");

	DebugMan.enableDebugChannel("script");

	gDebugLevel = 10;
	

	_rnd = new Common::RandomSource("prince");
	_debugger = new Debugger(this);
	_midiPlayer = new MusicPlayer(this);

}

PrinceEngine::~PrinceEngine() {
	DebugMan.clearAllDebugChannels();

	delete _rnd;
	delete _debugger;
	delete _cur1;
	delete _cur2;
	delete _midiPlayer;
}

GUI::Debugger *PrinceEngine::getDebugger() {
	return _debugger;
}

Common::Error PrinceEngine::run() {
	_graph = new GraphicsMan(this);

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	
	debug("Adding all path: %s", gameDataDir.getPath().c_str());

	SearchMan.addSubDirectoryMatching(gameDataDir, "all", 0, 2);
	SearchMan.addSubDirectoryMatching(gameDataDir, "data/voices/output", 0, 2);

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

	Common::SeekableReadStream *variaTxtStream = SearchMan.createReadStreamForMember("variatxt.dat");

	if (!variaTxtStream) {
		error("Can't load variatxt.dat");
		return Common::kPathNotFile;
	}

	_variaTxt = new VariaTxt();
	_variaTxt->loadFromStream(*variaTxtStream);
	delete variaTxtStream;

	Common::SeekableReadStream *talkTxtStream = SearchMan.createReadStreamForMember("talktxt.dat");
	if (!talkTxtStream) {
		error("Can't load talkTxtStream");
		return Common::kPathDoesNotExist;
	}

	_talkTxtSize = talkTxtStream->size();
	_talkTxt = new byte[_talkTxtSize];
	talkTxtStream->read(_talkTxt, _talkTxtSize);

	delete talkTxtStream;


	_cur1 = loadCursor("mouse1.cur");
	_cur2 = loadCursor("mouse2.cur");
#if 0
	Common::SeekableReadStream *logoStream = SearchMan.createReadStreamForMember("logo.raw"); 
	if (logoStream)
	{
		MhwanhDecoder logo;
		logo.loadStream(*logoStream);
		_graph->setPalette(logo.getPalette());
		_graph->draw(0, 0, logo.getSurface());
		_graph->update();
		_system->delayMillis(700);
	}
	delete logoStream;
#endif
	mainLoop();

	return Common::kNoError;
}

class MobList {
public:
	bool loadFromStream(Common::SeekableReadStream &stream);

	Common::Array<Mob> _mobList;
};

bool MobList::loadFromStream(Common::SeekableReadStream &stream)
{
	Mob mob;
	while (mob.loadFromStream(stream))
		_mobList.push_back(mob);

	return true;
}

class ObjectList {
public:
	bool loadFromStream(Common::SeekableReadStream &stream);

	Common::Array<Object> _objList;
};

bool ObjectList::loadFromStream(Common::SeekableReadStream &stream)
{
	Object obj;
	while (obj.loadFromStream(stream))
		_objList.push_back(obj);

	return true;
}

bool PrinceEngine::loadLocation(uint16 locationNr) {
	debug("PrinceEngine::loadLocation %d", locationNr);
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.remove(Common::String::format("%02d", _locationNr));
	_locationNr = locationNr;

	const Common::String locationNrStr = Common::String::format("%02d", _locationNr);
	debug("loadLocation %s", locationNrStr.c_str());
	SearchMan.addSubDirectoryMatching(gameDataDir, locationNrStr, 0, 2);

	// load location background
	Common::SeekableReadStream *room = SearchMan.createReadStreamForMember("room");

	if (!room) {
		error("Can't load room bitmap");
		return false;
	}

	if(_roomBmp.loadStream(*room)) {
		debug("Room bitmap loaded");
		_sceneWidth = _roomBmp.getSurface()->w;
	}

	delete room;

	delete _mobList;
	_mobList = NULL;

	Common::SeekableReadStream *mobListStream = SearchMan.createReadStreamForMember("mob.lst");
	if (!mobListStream) {
		error("Can't read mob.lst");
		return false;
	}

	_mobList = new MobList();
	_mobList->loadFromStream(*mobListStream);

	delete mobListStream;

	delete _objectList;
	_objectList = NULL;

	Common::SeekableReadStream *objListStream = SearchMan.createReadStreamForMember("obj.lst");
	if (!objListStream) {
		error("Can't read obj.lst");
		return false;
	}

	_objectList = new ObjectList();
	_objectList->loadFromStream(*objListStream);
	delete objListStream;

	const char *musName = MusicPlayer::_musTable[MusicPlayer::_musRoomTable[locationNr]];
	_midiPlayer->loadMidi(musName);

	return true;
}

void PrinceEngine::changeCursor(uint16 curId)
{
	Graphics::Surface *curSurface = NULL;

	uint16 hotspotX = 0;
	uint16 hotspotY = 0;

	switch(curId) {
		case 0:
			CursorMan.showMouse(false);
			return;
		case 1:
			curSurface = _cur1;
			break;
		case 2:
			curSurface = _cur2;
			hotspotX = curSurface->w >> 1;
			hotspotY = curSurface->h >> 1;
			break;
	}

	CursorMan.replaceCursorPalette(_roomBmp.getPalette(), 0, 255);
	CursorMan.replaceCursor(
		curSurface->getBasePtr(0, 0), 
		curSurface->w, curSurface->h, 
		hotspotX, hotspotY, 
		255, false,
		&curSurface->format
	);
	CursorMan.showMouse(true);
}

bool PrinceEngine::playNextFrame() {
	if (!_flicPlayer.isVideoLoaded())
		return false;

	const Graphics::Surface *s = _flicPlayer.decodeNextFrame();
	if (s) {
		_graph->drawTransparent(s);
		_graph->change();
	} else if (_flicLooped) {
        _flicPlayer.rewind();
        playNextFrame();
    }

	return true;
}

bool PrinceEngine::loadAnim(uint16 animNr, bool loop) {
	Common::String streamName = Common::String::format("AN%02d", animNr);
	Common::SeekableReadStream * flicStream = SearchMan.createReadStreamForMember(streamName);

	if (!flicStream) {
		error("Can't open %s", streamName.c_str());
		return false;
	}

	if (!_flicPlayer.loadStream(flicStream)) {
		error("Can't load flic stream %s", streamName.c_str());
	}

	debug("%s loaded", streamName.c_str());
    _flicLooped = loop;
	_flicPlayer.start();
	playNextFrame();
	return true;
}

void PrinceEngine::scrollCameraLeft(int16 delta) {
    if (_newCameraX > 0) {
        if (_newCameraX < delta)
            _newCameraX = 0;
        else
            _newCameraX -= delta;
    }   
}

void PrinceEngine::scrollCameraRight(int16 delta) {
    if (_newCameraX != _sceneWidth - 640) {
        if (_sceneWidth - 640 < delta + _newCameraX)
            delta += (_sceneWidth - 640) - (delta + _newCameraX);
        _newCameraX += delta;
        debug(0, "PrinceEngine::scrollCameraRight() _newCameraX = %d; delta = %d", _newCameraX, delta);
    }   
}

void PrinceEngine::keyHandler(Common::Event event) {
	uint16 nChar = event.kbd.keycode;
	switch (nChar) {
	case Common::KEYCODE_d:
		if (event.kbd.hasFlags(Common::KBD_CTRL)) {
			getDebugger()->attach();
		}
		break;
	case Common::KEYCODE_LEFT:
		scrollCameraLeft(32);
		break;
	case Common::KEYCODE_RIGHT:
		scrollCameraRight(32);
		break;
	case Common::KEYCODE_ESCAPE:
		_script->setFlag(Flags::ESCAPED2, 1);
		break;
	}
}

void PrinceEngine::hotspot() {
	if (!_mobList)
		return;
	Common::Point mousepos = _system->getEventManager()->getMousePos();
	Common::Point mousePosCamera(mousepos.x + _cameraX, mousepos.y);

	for (Common::Array<Mob>::const_iterator it = _mobList->_mobList.begin()
		; it != _mobList->_mobList.end() ; ++it) {
		if (it->_visible)
			continue;
		if (it->_rect.contains(mousePosCamera)) {
			uint16 textW = 0;
			for (uint16 i = 0; i < it->_name.size(); ++i)
				textW += _font.getCharWidth(it->_name[i]);

			uint16 x = mousepos.x - textW/2;
			if (x > _graph->_frontScreen->w)
				x = 0;

			if (x + textW > _graph->_frontScreen->w)
				x = _graph->_frontScreen->w - textW;

			_font.drawString(
				_graph->_frontScreen, 
				it->_name, 
				x,
				mousepos.y - _font.getFontHeight(),
				_graph->_frontScreen->w,
				216
			);
			break;
		}
	}
}

void PrinceEngine::printAt(uint32 slot, uint8 color, const char *s, uint16 x, uint16 y) {

	debugC(1, DebugChannel::kEngine, "PrinceEngine::printAt slot %d, color %d, x %02d, y %02d, str %s", slot, color, x, y, s);

	Text &text = _textSlots[slot];
	text._str = s;
	text._x = x;
	text._y = y;
	text._color = color;
}

uint32 PrinceEngine::getTextWidth(const char *s) {
    uint16 textW = 0;
	while (*s) {
        textW += _font.getCharWidth(*s) + _font.getKerningOffset(0, 0);
		++s;
    }
    return textW;
}

void PrinceEngine::showTexts() {
	for (uint32 slot = 0; slot < MAXTEXTS; ++slot) {
		Text& text = _textSlots[slot];
		if (!text._str && !text._time)
			continue;

		Common::Array<Common::String> lines;
		_font.wordWrapText(text._str, _graph->_frontScreen->w, lines);

		for (uint8 i = 0; i < lines.size(); ++i) {
			_font.drawString(
				_graph->_frontScreen,
				lines[i],
				text._x - getTextWidth(lines[i].c_str())/2,
				text._y - (lines.size() - i) * (_font.getFontHeight()),
				_graph->_frontScreen->w,
				text._color
			);
		}

		--text._time;
		if (text._time == 0) {
			text._str = NULL;
		}
	}
}

void PrinceEngine::drawScreen() {
	const Graphics::Surface *roomSurface = _roomBmp.getSurface();	
	if (roomSurface) {
		_graph->setPalette(_roomBmp.getPalette());
		const Graphics::Surface visiblePart = roomSurface->getSubArea(Common::Rect(_cameraX, 0, roomSurface->w, roomSurface->h));
		_graph->draw(0, 0, &visiblePart);
	}

	playNextFrame();

	//if (_objectList)
	//	  _graph->drawTransparent(_objectList->getSurface());

	hotspot();

	showTexts();

	getDebugger()->onFrame();

	_graph->update();
}

void PrinceEngine::mainLoop() {

	while (!shouldQuit()) {
		uint32 currentTime = _system->getMillis();

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
		drawScreen();

		// Calculate the frame delay based off a desired frame time
		int delay = 1000/15 - int32(_system->getMillis() - currentTime);
		// Ensure non-negative
		delay = delay < 0 ? 0 : delay;
		_system->delayMillis(delay);
		
		_cameraX = _newCameraX;
		++_frameNr;
	}
}

} // End of namespace Prince
/* vim: set tabstop=4 expandtab!: */
