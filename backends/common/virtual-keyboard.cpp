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
#include "backends/common/virtual-keyboard-gui.h"
#include "backends/common/virtual-keyboard-parser.h"
#include "graphics/imageman.h"

namespace Common {

VirtualKeyboard::VirtualKeyboard() : _currentMode(0) {
	assert(g_system);
	_system = g_system;

	_parser = new VirtualKeyboardParser(this);
	_kbdGUI = new VirtualKeyboardGUI(this);
	_loaded = false;
}

VirtualKeyboard::~VirtualKeyboard() {
	deleteEvents();
	delete _kbdGUI;
	delete _parser;
}

void VirtualKeyboard::deleteEvents() {
	ModeMap::iterator it_m;
	EventMap::iterator it_e;
	for (it_m = _modes.begin(); it_m != _modes.end(); it_m++) {
		EventMap *evt = &(it_m->_value.events);
		for (it_e = evt->begin(); it_e != evt->end(); it_e++)
			delete it_e->_value;
	}
}

void VirtualKeyboard::reset() {
	deleteEvents();
	_modes.clear();
	_initialMode = _currentMode = 0;
	_hAlignment = kAlignCentre;
	_vAlignment = kAlignBottom;
	_keyQueue.clear();
	_loaded = false;
	_kbdGUI->reset();
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

bool VirtualKeyboard::checkModeResolutions()
{
	_parser->setParseMode(kParseCheckResolutions);
	_loaded = _parser->parse();
	return _loaded;
}

Common::String VirtualKeyboard::findArea(int16 x, int16 y) {
	return _currentMode->imageMap.findMapArea(x, y);
}

void VirtualKeyboard::processAreaClick(const Common::String& area) {
	if (!_currentMode->events.contains(area)) return;
	Event *evt = _currentMode->events[area];

	switch (evt->type) {
	case kEventKey: {
		// add virtual keypress to queue
		_keyQueue.insertKey(*(Common::KeyState*)evt->data);
		break;
	}
	case kEventModifier:
		_keyQueue.toggleFlags(*(byte*)(evt->data));
		break;
	case kEventSwitchMode:
		// switch to new mode
		switchMode(*(Common::String *)evt->data);
		_keyQueue.clearFlags();
		break;
	case kEventClose:
		// close virtual keyboard
		_kbdGUI->hide();
		break;
	}
}

void VirtualKeyboard::switchMode(Mode *newMode) {
	_kbdGUI->initMode(newMode);
	_currentMode = newMode;
}

void VirtualKeyboard::switchMode(const Common::String& newMode) {
	if (!_modes.contains(newMode)) {
		warning("Keyboard mode '%s' unknown", newMode.c_str());
		return;
	}
	switchMode(&_modes[newMode]);
}

void VirtualKeyboard::handleMouseDown(int16 x, int16 y) {
	_areaDown = findArea(x, y);
	if (_areaDown.empty())
		_kbdGUI->startDrag(x, y);
}

void VirtualKeyboard::handleMouseUp(int16 x, int16 y) {
	if (!_areaDown.empty() && _areaDown == findArea(x, y)) {
		processAreaClick(_areaDown);
		_areaDown.clear();
	}
	_kbdGUI->endDrag();
}

void VirtualKeyboard::show() {
	if (!_loaded) {
		// if not loaded then load default "vkeybd" pack
		if (!loadKeyboardPack("vkeybd")) {
			warning("Keyboard not loaded therefore can't be shown");
			return;
		}
	}
	switchMode(_initialMode);

	_kbdGUI->run();

	EventManager *eventMan = _system->getEventManager();
	assert(eventMan);

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

void VirtualKeyboard::hide() {
	_kbdGUI->hide();
}

bool VirtualKeyboard::isDisplaying() { 
	return _kbdGUI->isDisplaying();
}

VirtualKeyboard::KeyPressQueue::KeyPressQueue() {
	_keyPos = _keys.end();
	_strPos = 0;
}

void VirtualKeyboard::KeyPressQueue::toggleFlags(byte fl) {
	_keyFlags ^= fl;
	_strChanged = true;
}

void VirtualKeyboard::KeyPressQueue::clearFlags() {
	_keyFlags = 0;
	_strChanged = true;
}

void VirtualKeyboard::KeyPressQueue::insertKey(KeyState key) {
	_strChanged = true;
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
	default:
		;
	}

	key.flags ^= _keyFlags;
	if ((key.keycode >= Common::KEYCODE_a) && (key.keycode <= Common::KEYCODE_z))
		key.ascii = (key.flags & Common::KBD_SHIFT) ? key.keycode - 32 : key.keycode;
	clearFlags();

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
}

void VirtualKeyboard::KeyPressQueue::deleteKey() {
	if (_keyPos == _keys.begin())
		return;
	List<VirtualKeyPress>::iterator it = _keyPos;
	it--;
	_strPos -= it->strLen;
	while((it->strLen)-- > 0)
		_str.deleteChar(_strPos);
	_keys.erase(it);
}

void VirtualKeyboard::KeyPressQueue::moveLeft() {
	if (_keyPos == _keys.begin())
		return;
	_keyPos--;
	_strPos -= _keyPos->strLen;
}

void VirtualKeyboard::KeyPressQueue::moveRight() {
	if (_keyPos == _keys.end())
		return;
	_strPos += _keyPos->strLen;
	_keyPos++;
}

KeyState VirtualKeyboard::KeyPressQueue::pop() {
	bool front = (_keyPos == _keys.begin());
	VirtualKeyPress kp = *(_keys.begin());
	_keys.pop_front();

	if (front)
		_keyPos = _keys.begin();
	else
		_strPos -= kp.strLen;

	while (kp.strLen-- > 0)
		_str.deleteChar(0);

	return kp.key;
}

void VirtualKeyboard::KeyPressQueue::clear() {
	_keys.clear();
	_keyPos = _keys.end();
	_str.clear();
	_strPos = 0;
	_keyFlags = 0;
}

bool VirtualKeyboard::KeyPressQueue::empty()
{ 
	return _keys.empty();
}

String VirtualKeyboard::KeyPressQueue::getString()
{
	String flags;
	if (_keyFlags & KBD_CTRL)
		flags += "Ctrl+";
	if (_keyFlags & KBD_ALT)
		flags += "Alt+";
	if (_keyFlags & KBD_SHIFT)
		flags += "Shift+";
	return _str + flags;
}

uint VirtualKeyboard::KeyPressQueue::getInsertIndex() {
	return _strPos;
}

bool VirtualKeyboard::KeyPressQueue::hasStringChanged() {
	bool ret = _strChanged;
	_strChanged = false;
	return ret;
}

} // end of namespace Common
