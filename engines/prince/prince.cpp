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
#include "prince/font.h"
#include "prince/mhwanh.h"
#include "prince/cursor.h"
#include "prince/archive.h"

namespace Prince {

void PrinceEngine::debugEngine(const char *s, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

    va_start(va, s);
    vsnprintf(buf, STRINGBUFLEN, s, va);
    va_end(va);

	debug("Prince::Engine frame %08ld %s", _frameNr, buf);
}

PrinceEngine::PrinceEngine(OSystem *syst, const PrinceGameDescription *gameDesc) : 
	Engine(syst), _gameDescription(gameDesc), _graph(NULL), _script(NULL),
	_locationNr(0), _debugger(NULL), _midiPlayer(NULL),
	_cameraX(0), _newCameraX(0), _frameNr(0), _cursor1(NULL), _cursor2(NULL), _font(NULL),
	_walizkaBmp(NULL), _roomBmp(NULL), _voiceStream(NULL) {

	// Debug/console setup
	DebugMan.addDebugChannel(DebugChannel::kScript, "script", "Prince Script debug channel");
	DebugMan.addDebugChannel(DebugChannel::kEngine, "engine", "Prince Engine debug channel");

	DebugMan.enableDebugChannel("script");

	gDebugLevel = 10;
}

PrinceEngine::~PrinceEngine() {
	DebugMan.clearAllDebugChannels();

	delete _rnd;
	delete _debugger;
	delete _cursor1;
	delete _cursor2;
	delete _midiPlayer;
	delete _script;
	delete _font;
	delete _roomBmp;
	delete _walizkaBmp;
	delete _variaTxt;
	delete[] _talkTxt;
	delete _graph;
}

GUI::Debugger *PrinceEngine::getDebugger() {
	return _debugger;
}

template <typename T>
bool loadFromStream(T &resource, Common::SeekableReadStream &stream) {
	return resource.loadFromStream(stream);
}

template <>
bool loadFromStream<MhwanhDecoder>(MhwanhDecoder &image, Common::SeekableReadStream &stream) {
	return image.loadStream(stream);
}

template <>
bool loadFromStream<Graphics::BitmapDecoder>(Graphics::BitmapDecoder &image, Common::SeekableReadStream &stream) {
	return image.loadStream(stream);
}

template<typename T>
bool loadResource(T *resource, const char *resourceName, bool required = true) {
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(resourceName);
	if (!stream) {
		if (required) 
			error("Can't load %s", resourceName);
		return false;
	}

	bool ret = loadFromStream(*resource, *stream);

	delete stream;

	return ret;
} 

template <typename T>
bool loadResource(Common::Array<T> &array, const char *resourceName, bool required = true) {
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(resourceName);
	if (!stream) {
		if (required) {
			error("Can't load %s", resourceName);
		}
		return false;
	}

	typename Common::Array<T>::value_type t;
	while (t.loadFromStream(*stream))
		array.push_back(t);

	delete stream;
	return true;
}

void PrinceEngine::init() {

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	
	debugEngine("Adding all path: %s", gameDataDir.getPath().c_str());

	PtcArchive *all = new PtcArchive();
	if (!all->open("all/databank.ptc")) 
		error("Can't open all/databank.ptc");

	PtcArchive *voices = new PtcArchive();
	if (!voices->open("data/voices/databank.ptc"))
		error("Can't open data/voices/databank.ptc");

	SearchMan.add("all", all);
	SearchMan.add("data/voices", voices);

	_graph = new GraphicsMan(this);

	_rnd = new Common::RandomSource("prince");
	_debugger = new Debugger(this);

	SearchMan.addSubDirectoryMatching(gameDataDir, "all", 0, 2);
	SearchMan.addSubDirectoryMatching(gameDataDir, "data/voices", 0, 2);

	_midiPlayer = new MusicPlayer(this);
	 
	_font = new Font();
	loadResource(_font, "all/font1.raw");
	_walizkaBmp = new MhwanhDecoder();
	loadResource(_walizkaBmp, "all/walizka");

	_script = new Script(this);
	loadResource(_script, "all/skrypt.dat");

	_variaTxt = new VariaTxt();
	loadResource(_variaTxt, "all/variatxt.dat");
	
	_cursor1 = new Cursor();
	loadResource(_cursor1, "all/mouse1.cur");

	_cursor2 = new Cursor();
	loadResource(_cursor2, "all/mouse2.cur");

	Common::SeekableReadStream *talkTxtStream = SearchMan.createReadStreamForMember("all/talktxt.dat");
	if (!talkTxtStream) {
		error("Can't load talkTxtStream");
		return;
	}
	_talkTxtSize = talkTxtStream->size();
	_talkTxt = new byte[_talkTxtSize];
	talkTxtStream->read(_talkTxt, _talkTxtSize);

	delete talkTxtStream;
}

void PrinceEngine::showLogo() {
	MhwanhDecoder logo;
	if (loadResource(&logo, "logo.raw")) {
		_graph->setPalette(logo.getPalette());
		_graph->draw(0, 0, logo.getSurface());
		_graph->update();
		_system->delayMillis(700);
	}
}

Common::Error PrinceEngine::run() {

	init();

	showLogo();

//	return Common::kNoError;

	mainLoop();

	return Common::kNoError;
}

bool PrinceEngine::loadLocation(uint16 locationNr) {
	_cameraX = 0;
	_newCameraX = 0;
	_debugger->_locationNr = locationNr;
	debugEngine("PrinceEngine::loadLocation %d", locationNr);
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.remove(Common::String::format("%02d", _locationNr));
	_locationNr = locationNr;

	const Common::String locationNrStr = Common::String::format("%02d", _locationNr);
	debugEngine("loadLocation %s", locationNrStr.c_str());

	PtcArchive *locationArchive = new PtcArchive();
	if (!locationArchive->open(locationNrStr + "/databank.ptc"))
		error("Can't open location %s", locationNrStr.c_str());

	SearchMan.add(locationNrStr, locationArchive);

	delete _roomBmp;
	// load location background
	_roomBmp = new Graphics::BitmapDecoder();
	loadResource(_roomBmp, "room");
	if (_roomBmp->getSurface()) {
		_sceneWidth = _roomBmp->getSurface()->w;
	}

	_mobList.clear();
	loadResource(_mobList, "mob.lst", false);

	const char *musName = MusicPlayer::_musTable[MusicPlayer::_musRoomTable[locationNr]];
	_midiPlayer->loadMidi(musName);

	return true;
}

void PrinceEngine::changeCursor(uint16 curId) {
	_debugger->_cursorNr = curId;

	Graphics::Surface *curSurface = NULL;

	uint16 hotspotX = 0;
	uint16 hotspotY = 0;

	switch(curId) {
		case 0:
			CursorMan.showMouse(false);
			return;
		case 1:
			curSurface = _cursor1->getSurface();
			break;
		case 2:
			curSurface = _cursor2->getSurface();
			hotspotX = curSurface->w >> 1;
			hotspotY = curSurface->h >> 1;
			break;
	}

	CursorMan.replaceCursorPalette(_roomBmp->getPalette(), 0, 255);
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

void PrinceEngine::playSample(uint16 sampleId, uint16 loopType) {
	if (_voiceStream) {

		Audio::RewindableAudioStream *audioStream = Audio::makeWAVStream(_voiceStream, DisposeAfterUse::YES);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, audioStream, sampleId);
	}
}

void PrinceEngine::stopSample(uint16 sampleId) {
	_mixer->stopID(sampleId);
	_voiceStream = NULL;
}

bool PrinceEngine::loadVoice(uint32 slot, const Common::String &streamName) {
	debugEngine("Loading wav %s slot %d", streamName.c_str(), slot);

	if (slot > MAXTEXTS) {
		error("Text slot bigger than MAXTEXTS %d", MAXTEXTS);
		return false;
	}

	_voiceStream = SearchMan.createReadStreamForMember(streamName);
	if (!_voiceStream) {
		error("Can't open %s", streamName.c_str());
		return false;
	}

	uint32 id = _voiceStream->readUint32LE();
	if (id != 0x46464952) {
		error("It's not RIFF file %s", streamName.c_str());
		return false;
	}

	_voiceStream->skip(0x20);
	id = _voiceStream->readUint32LE();
	if (id != 0x61746164) {
		error("No data section in %s id %04x", streamName.c_str(), id);
		return false;
	}

	id = _voiceStream->readUint32LE();
	debugEngine("SetVoice slot %d time %04x", slot, id); 
	id <<= 3;
	id /= 22050;
	id += 2;

	_textSlots[slot]._time = id;

	debugEngine("SetVoice slot %d time %04x", slot, id); 
	_voiceStream->seek(0);

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

	debugEngine("%s loaded", streamName.c_str());
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
        debugEngine("PrinceEngine::scrollCameraRight() _newCameraX = %d; delta = %d", _newCameraX, delta);
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
		_script->setFlagValue(Flags::ESCAPED2, 1);
		break;
	}
}

void PrinceEngine::hotspot() {
	Common::Point mousepos = _system->getEventManager()->getMousePos();
	Common::Point mousePosCamera(mousepos.x + _cameraX, mousepos.y);

	for (Common::Array<Mob>::const_iterator it = _mobList.begin()
		; it != _mobList.end() ; ++it) {
		const Mob& mob = *it;
		if (mob._visible)
			continue;
		if (mob._rect.contains(mousePosCamera)) {
			uint16 textW = 0;
			for (uint16 i = 0; i < mob._name.size(); ++i)
				textW += _font->getCharWidth(mob._name[i]);

			uint16 x = mousepos.x - textW/2;
			if (x > _graph->_frontScreen->w)
				x = 0;

			if (x + textW > _graph->_frontScreen->w)
				x = _graph->_frontScreen->w - textW;

			uint16 y = mousepos.y - _font->getFontHeight();
			if (y > _graph->_frontScreen->h)
				y = _font->getFontHeight() - 2;

			_font->drawString(
				_graph->_frontScreen, 
				mob._name, 
				x,
				y,
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
        textW += _font->getCharWidth(*s) + _font->getKerningOffset(0, 0);
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
		_font->wordWrapText(text._str, _graph->_frontScreen->w, lines);

		for (uint8 i = 0; i < lines.size(); ++i) {
			_font->drawString(
				_graph->_frontScreen,
				lines[i],
				text._x - getTextWidth(lines[i].c_str())/2,
				text._y - (lines.size() - i) * (_font->getFontHeight()),
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
	const Graphics::Surface *roomSurface = _roomBmp->getSurface();	
	if (roomSurface) {
		_graph->setPalette(_roomBmp->getPalette());
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

	loadLocation(2);
	changeCursor(1);

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

		//_script->step();
		drawScreen();

		// Calculate the frame delay based off a desired frame time
		int delay = 1000/15 - int32(_system->getMillis() - currentTime);
		// Ensure non-negative
		delay = delay < 0 ? 0 : delay;
		_system->delayMillis(delay);
		
		_cameraX = _newCameraX;
		++_frameNr;

		if (_debugger->_locationNr != _locationNr)
			loadLocation(_debugger->_locationNr);
		if (_debugger->_cursorNr != _cursorNr)
			changeCursor(_debugger->_cursorNr);
	}
}

} // End of namespace Prince

/* vim: set tabstop=4 expandtab!: */
