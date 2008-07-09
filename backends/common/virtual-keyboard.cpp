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
#include "graphics/imageman.h"
#include "graphics/surface-keycolored.h"
#include "common/unzip.h"

namespace Common {

VirtualKeyboard::VirtualKeyboard() : _currentMode(0), _keyDown(0) {
	assert(g_system);
	_system = g_system;

	_parser = new VirtualKeyboardParser(this);
	_loaded = _displaying = _drag = false;
	_screenWidth = _system->getOverlayWidth();
	_screenHeight = _system->getOverlayHeight();
}

VirtualKeyboard::~VirtualKeyboard() {
	// TODO: clean up event data pointers
	delete _parser;
}

void VirtualKeyboard::reset() {
	// TODO: clean up event data pointers
	_modes.clear();
	_initialMode = _currentMode = 0;
	_kbdBound.left = _kbdBound.top
		= _kbdBound.right = _kbdBound.bottom = 0;
	_hAlignment = kAlignCentre;
	_vAlignment = kAlignBottom;
	_keyQueue.clear();
	_keyDown = 0;
	_displaying = _drag = false;
	_firstRun = true;
	_screenWidth = _system->getOverlayWidth();
	_screenHeight = _system->getOverlayHeight();
}

bool VirtualKeyboard::loadKeyboardPack(Common::String packName) {

	if (ConfMan.hasKey("extrapath"))
		Common::File::addDefaultDirectoryRecursive(ConfMan.get("extrapath"));

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
	int16 kbdW = _kbdBound.width(), kbdH = _kbdBound.height();
	int16 posX = 0, posY = 0;
	if (_screenWidth != kbdW) {
		switch (_hAlignment) {
		case kAlignLeft:
			posX = 0;
			break;
		case kAlignCentre:
			posX = (_screenWidth - kbdW) / 2;
			break;
		case kAlignRight:
			posX = _screenWidth - kbdW;
			break;
		}
	}
	if (_screenHeight != kbdH) {
		switch (_vAlignment) {
		case kAlignTop:
			posY = 0;
			break;
		case kAlignMiddle:
			posY = (_screenHeight - kbdH) / 2;
			break;
		case kAlignBottom:
			posY = _screenHeight - kbdH;
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
	case kEventKey:
		// add virtual keypress to queue
		_keyQueue.push_back(*(Common::KeyState*)evt.data);
		break;
	case kEventSwitchMode:
		// switch to new mode
		switchMode(*(Common::String *)evt.data);
		break;
	case kEventClose:
		// close virtual keyboard
		_displaying = false;
		break;
	}
}

void VirtualKeyboard::switchMode(Mode *newMode) {
	_currentMode = newMode;
	_kbdBound.setWidth(_currentMode->image->w);
	_kbdBound.setHeight(_currentMode->image->h);
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
		warning("Keyboard not loaded therefore can't be shown");
		return;
	}
	if (_screenWidth != _system->getOverlayWidth() || _screenHeight != _system->getOverlayHeight()) {
		_screenWidth = _system->getOverlayWidth();
		_screenHeight = _system->getOverlayHeight();
		if (!checkModeResolutions()) return;
	}
	switchMode(_initialMode);
	_displaying = true;
	if (_firstRun) {
		_firstRun = false;
		setDefaultPosition();
	}
	runLoop();
}

void VirtualKeyboard::hide() {
	_displaying = false;
}

void VirtualKeyboard::runLoop() {
	Common::EventManager *eventMan = _system->getEventManager();
	
	_system->showOverlay();

	while (_displaying) {
		if (_needRedraw) redraw();

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
				_screenWidth = _system->getOverlayWidth();
				_screenHeight = _system->getOverlayHeight();
				if (!checkModeResolutions())
					_displaying = false;
				break;
			case Common::EVENT_QUIT:
				_system->quit();
				return;
			default:
				break;
			}
			// TODO - remove this line ?
			if (!_displaying) break;
		}
	}
	// clear keyboard from overlay
	_system->hideOverlay();
}

void VirtualKeyboard::redraw() {
	Graphics::SurfaceKeyColored surf;

	surf.create(_system->getOverlayWidth(), _system->getOverlayHeight(), sizeof(OverlayColor));

	_system->clearOverlay();
	_system->grabOverlay((OverlayColor*)surf.pixels, surf.w);

	surf.blit(_currentMode->image, _kbdBound.left, _kbdBound.top, _system->RGBToColor(0xff, 0, 0xff));
	_system->copyRectToOverlay((OverlayColor*)surf.pixels, surf.w, 0, 0, surf.w, surf.h);

	surf.free();

	_needRedraw = false;
}

bool VirtualKeyboard::pollEvent(Common::Event &event) {
	if (_displaying || (_keyQueue.empty() && !_keyDown))
		return false;

	event.synthetic = false; // ???
	if (_keyDown) {
		event.type = Common::EVENT_KEYUP;
		event.kbd = *_keyDown;
		_keyQueue.remove_at(0);
		_keyDown = 0;
	} else {
		_keyDown = _keyQueue.begin();
		event.type = Common::EVENT_KEYDOWN;
		event.kbd = *_keyDown;
	}
	return true;
}

} // end of namespace GUI
