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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "backends/common/virtual-keyboard.h"
#include "backends/common/virtual-keyboard-parser.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/unzip.h"
#include "graphics/cursorman.h"
#include "graphics/imageman.h"
#include "graphics/surface-keycolored.h"
#include "gui/newgui.h"

namespace Common {

VirtualKeyboard::VirtualKeyboard() : _currentMode(0), _keyDown(0) {
	assert(g_system);
	_system = g_system;

	_parser = new VirtualKeyboardParser(this);
	_loaded = _displaying = _drag = false;
	_lastScreenChanged = _system->getScreenChangeID();

	memset(_cursor, 0xFF, sizeof(_cursor));
}

VirtualKeyboard::~VirtualKeyboard() {
	// TODO: clean up event data pointers
	deleteEventData();
	delete _parser;
}

void VirtualKeyboard::reset() {
	// TODO: clean up event data pointers
	deleteEventData();
	_modes.clear();
	_initialMode = _currentMode = 0;
	_kbdBound.left = _kbdBound.top
		= _kbdBound.right = _kbdBound.bottom = 0;
	_hAlignment = kAlignCentre;
	_vAlignment = kAlignBottom;
	_keyQueue.clear();
	_keyDown = 0;
	_keyFlags = 0;
	_displaying = _drag = false;
	_firstRun = true;
	_lastScreenChanged = _system->getScreenChangeID();
}

void VirtualKeyboard::deleteEventData() {
	ModeMap::iterator it_m;
	EventMap::iterator it_e;
	for (it_m = _modes.begin(); it_m != _modes.end(); it_m++) {
		EventMap *evt = &(it_m->_value.events);
		for (it_e = evt->begin(); it_e != evt->end(); it_e++)
			delete it_e->_value.data;
	}
}

bool VirtualKeyboard::loadKeyboardPack(Common::String packName) {

	if (Common::File::exists(packName + ".xml")) {
		// uncompressed keyboard pack
		if (!_parser->loadFile(packName + ".xml"))
			return false;
		
	} else if (Common::File::exists(packName + ".zip")) {
		// compressed keyboard pack
#ifdef USE_ZLIB
		unzFile zipFile = unzOpen((packName + ".zip").c_str());
		if (zipFile && unzLocateFile(zipFile, (packName + ".xml").c_str(), 2) == UNZ_OK) {
			unz_file_info fileInfo;
			unzOpenCurrentFile(zipFile);
			unzGetCurrentFileInfo(zipFile, &fileInfo, NULL, 0, NULL, 0, NULL, 0);
			byte *buffer = new byte[fileInfo.uncompressed_size+1];
			assert(buffer);
			memset(buffer, 0, (fileInfo.uncompressed_size+1)*sizeof(byte));
			unzReadCurrentFile(zipFile, buffer, fileInfo.uncompressed_size);
			unzCloseCurrentFile(zipFile);
			if (!_parser->loadBuffer(buffer, fileInfo.uncompressed_size+1, true)) {
				unzClose(zipFile);
				return false;
			}
		} else {
			warning("Could not find %s.xml file in %s.zip keyboard pack", packName.c_str(), packName.c_str());
			unzClose(zipFile);
			return false;
		}
		unzClose(zipFile);

		ImageMan.addArchive(packName + ".zip");
#else
		return false;
#endif
	} else {
		warning("Keyboard pack not found");
		return false;
	}

	_parser->setParseMode(kParseFull);
	_loaded = _parser->parse();
	if (_loaded)
		printf("Keyboard pack '%s' loaded successfully!\n", packName.c_str());

	return _loaded;
}

void VirtualKeyboard::setDefaultPosition()
{
	int16 scrW = _system->getOverlayWidth(), scrH = _system->getOverlayHeight(); 
	int16 kbdW = _kbdBound.width(), kbdH = _kbdBound.height();
	int16 posX = 0, posY = 0;
	if (scrW != kbdW) {
		switch (_hAlignment) {
		case kAlignLeft:
			posX = 0;
			break;
		case kAlignCentre:
			posX = (scrW - kbdW) / 2;
			break;
		case kAlignRight:
			posX = scrW - kbdW;
			break;
		}
	}
	if (scrH != kbdH) {
		switch (_vAlignment) {
		case kAlignTop:
			posY = 0;
			break;
		case kAlignMiddle:
			posY = (scrH - kbdH) / 2;
			break;
		case kAlignBottom:
			posY = scrH - kbdH;
			break;
		}
	}
	_kbdBound.moveTo(posX, posY);
}

bool VirtualKeyboard::checkModeResolutions()
{
	_parser->setParseMode(kParseCheckResolutions);
	_loaded = _parser->parse();
	return _loaded;
}
	
void VirtualKeyboard::move(int16 x, int16 y) {
	// snap to edge of screen
	if (ABS(x) < SNAP_WIDTH)
		x = 0;
	int16 x2 = _system->getOverlayWidth() - _kbdBound.width();
	if (ABS(x - x2) < SNAP_WIDTH)
		x = x2;
	if (ABS(y) < SNAP_WIDTH)
		y = 0;
	int16 y2 = _system->getOverlayHeight() - _kbdBound.height();
	if (ABS(y - y2) < SNAP_WIDTH)
		y = y2;

	_kbdBound.moveTo(x, y);
}

Common::String VirtualKeyboard::findArea(int16 x, int16 y) {
	x -= _kbdBound.left;
	y -= _kbdBound.top;
	if (x < 0 || x > _kbdBound.width()) return "";
	if (y < 0 || y > _kbdBound.height()) return "";
	return _currentMode->imageMap.findMapArea(x, y);
}

void VirtualKeyboard::processClick(const Common::String& area) {
	if (!_currentMode->events.contains(area)) return;
	Event evt = _currentMode->events[area];

	switch (evt.type) {
	case kEventKey: {
		// add virtual keypress to queue
		Common::KeyState key = *(Common::KeyState*)evt.data;
		key.flags ^= _keyFlags;
		if ((key.keycode >= Common::KEYCODE_a) && (key.keycode <= Common::KEYCODE_z))
			key.ascii = (key.flags & Common::KBD_SHIFT) ? key.keycode - 32 : key.keycode;
		_keyQueue.insertKey(key);
		_keyFlags = 0;
		break;
	}
	case kEventModifier:
		_keyFlags ^= *(byte*)(evt.data);
		break;
	case kEventSwitchMode:
		// switch to new mode
		switchMode(*(Common::String *)evt.data);
		_keyFlags = 0;
		break;
	case kEventClose:
		// close virtual keyboard
		_displaying = false;
		break;
	}
}

void VirtualKeyboard::switchMode(Mode *newMode) {
	_kbdBound.setWidth(newMode->image->w);
	_kbdBound.setHeight(newMode->image->h);
	_currentMode = newMode;
	_needRedraw = true;
}

void VirtualKeyboard::switchMode(const Common::String& newMode) {
	if (!_modes.contains(newMode)) {
		warning("Keyboard mode '%s' unknown", newMode.c_str());
		return;
	}
	switchMode(&_modes[newMode]);
}

void VirtualKeyboard::show() {
	if (!_loaded) {
		// if not loaded then load default "vkeybd" pack
		if (!loadKeyboardPack("vkeybd")) {
			warning("Keyboard not loaded therefore can't be shown");
			return;
		}
	}
	if (_lastScreenChanged != _system->getScreenChangeID())
		screenChanged();
	switchMode(_initialMode);
	_displaying = true;
	if (_firstRun) {
		_firstRun = false;
		setDefaultPosition();
	}

	if (!g_gui.isActive()) {
		_system->showOverlay();
		_system->clearOverlay();
	}
	_overlayBackup.create(_system->getOverlayWidth(), _system->getOverlayHeight(), sizeof(OverlayColor));
	_system->grabOverlay((OverlayColor*)_overlayBackup.pixels, _overlayBackup.w);
	setupCursor();

	runLoop();

	removeCursor();
	_system->copyRectToOverlay((OverlayColor*)_overlayBackup.pixels, _overlayBackup.w, 0, 0, _overlayBackup.w, _overlayBackup.h);
	if (!g_gui.isActive()) _system->hideOverlay();
	_overlayBackup.free();
}

void VirtualKeyboard::hide() {
	_displaying = false;
}

void VirtualKeyboard::screenChanged() {
	_lastScreenChanged = _system->getScreenChangeID();
	if (!checkModeResolutions())
		_displaying = false;
}

void VirtualKeyboard::runLoop() {
	Common::EventManager *eventMan = _system->getEventManager();

	while (_displaying) {
		if (_needRedraw) redraw();

		animateCursor();
		_system->updateScreen();
		Common::Event event;
		while (eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_LBUTTONDOWN:
				if (_kbdBound.contains(event.mouse)) {
					_areaDown = findArea(event.mouse.x, event.mouse.y);
					if (_areaDown.empty()) {
						_drag = true;
						_dragPoint.x = event.mouse.x - _kbdBound.left;
						_dragPoint.y = event.mouse.y - _kbdBound.top;
					}
				} else
					_areaDown.clear();
				break;
			case Common::EVENT_LBUTTONUP:
				if (_drag) _drag = false;
				if (!_areaDown.empty() && _areaDown == findArea(event.mouse.x, event.mouse.y)) {
					processClick(_areaDown);
					_areaDown.clear();
				}
				break;
			case Common::EVENT_MOUSEMOVE:
				if (_drag) {
					move(event.mouse.x - _dragPoint.x, 
						 event.mouse.y - _dragPoint.y);
					_needRedraw = true;
				}
				break;
			case Common::EVENT_SCREEN_CHANGED:
				screenChanged();
				break;
			case Common::EVENT_QUIT:
				_system->quit();
				return;
			default:
				break;
			}
		}
		// Delay for a moment
		_system->delayMillis(10);
	}

	// push keydown & keyup events into the event manager
	Common::Event evt;
	evt.synthetic = false;
	while (!_keyQueue.empty()) {
		evt.kbd = _keyQueue.pop();
		evt.type = Common::EVENT_KEYDOWN;
		eventMan->pushEvent(evt);
		evt.type = Common::EVENT_KEYUP;
		eventMan->pushEvent(evt);
	}
}

void VirtualKeyboard::redraw() {
	Graphics::SurfaceKeyColored surf;

	surf.create(_system->getOverlayWidth(), _system->getOverlayHeight(), sizeof(OverlayColor));
	
	memcpy(surf.pixels, _overlayBackup.pixels, surf.w * surf.h * sizeof(OverlayColor));
	surf.blit(_currentMode->image, _kbdBound.left, _kbdBound.top, _currentMode->transparentColor);

	_system->copyRectToOverlay((OverlayColor*)surf.pixels, surf.w, 0, 0, surf.w, surf.h);

	surf.free();

	_needRedraw = false;
}

void VirtualKeyboard::setupCursor() {
	const byte palette[] = {
		255, 255, 255, 0,
		255, 255, 255, 0,
		171, 171, 171, 0,
		87,  87,  87, 0
	};

	CursorMan.pushCursorPalette(palette, 0, 4);
	CursorMan.pushCursor(NULL, 0, 0, 0, 0);
	CursorMan.showMouse(true);
}

void VirtualKeyboard::animateCursor() {
	int time = _system->getMillis();
	if (time > _cursorAnimateTimer + kCursorAnimateDelay) {
		for (int i = 0; i < 15; i++) {
			if ((i < 6) || (i > 8)) {
				_cursor[16 * 7 + i] = _cursorAnimateCounter;
				_cursor[16 * i + 7] = _cursorAnimateCounter;
			}
		}

		CursorMan.replaceCursor(_cursor, 16, 16, 7, 7);

		_cursorAnimateTimer = time;
		_cursorAnimateCounter = (_cursorAnimateCounter + 1) % 4;
	}
}

void VirtualKeyboard::removeCursor() {
	CursorMan.popCursor();
	CursorMan.popCursorPalette();
}

VirtualKeyboard::Queue::Queue() {
	_keyPos = _keys.begin();
	_strPos = 0;
}

void VirtualKeyboard::Queue::insertKey(KeyState key) {
	switch (key.keycode) {
	case KEYCODE_LEFT:
		moveLeft();
		return;
	case KEYCODE_RIGHT:
		moveRight();
		return;
	case KEYCODE_BACKSPACE:
		deleteKey();
		return;
	}

	String keyStr;
	if (key.keycode >= 32 && key.keycode <= 126) {
		if (key.flags & KBD_CTRL)
			keyStr += "Ctrl+";
		if (key.flags & KBD_ALT)
			keyStr += "Alt+";
		if (key.flags & KBD_SHIFT && (key.ascii < 65 || key.ascii > 90))
			keyStr += "Shift+";
		keyStr += (char)key.ascii;
	}

	const char *k = keyStr.c_str();
	while (char ch = *k++)
		_str.insertChar(ch, _strPos++);

	VirtualKeyPress kp;
	kp.key = key;
	kp.strLen = keyStr.size();
	_keys.insert(_keyPos, kp);
	_keyPos++;
}

void VirtualKeyboard::Queue::deleteKey() {
	if (_keyPos == _keys.begin())
		return;
	List<VirtualKeyPress>::iterator it = _keyPos;
	it--;
	_strPos -= it->strLen;
	while((it->strLen)-- > 0)
		_str.deleteChar(_strPos);
	_keys.erase(it);
}

void VirtualKeyboard::Queue::moveLeft() {
	if (_keyPos == _keys.begin())
		return;
	_keyPos--;
	_strPos -= _keyPos->strLen;
}

void VirtualKeyboard::Queue::moveRight() {
	List<VirtualKeyPress>::iterator it = _keyPos;
	it++;
	if (it == _keys.end())
		return;
	_strPos += _keyPos->strLen;
	_keyPos	= it;
}

KeyState VirtualKeyboard::Queue::pop() {
	KeyState ret = _keys.begin()->key;
	_keys.pop_front();
	return ret;
}

void VirtualKeyboard::Queue::clear() {
	_keys.clear();
	_keyPos = _keys.begin();
	_str.clear();
	_strPos = 0;
}

bool VirtualKeyboard::Queue::empty()
{ 
	return _keys.empty();
}

String VirtualKeyboard::Queue::getString()
{
	return _str;
}

} // end of namespace Common
