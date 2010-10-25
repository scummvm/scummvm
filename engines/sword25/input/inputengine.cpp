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
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#define BS_LOG_PREFIX "INPUTENGINE"

#include "common/algorithm.h"
#include "common/events.h"
#include "common/system.h"
#include "common/util.h"
#include "sword25/kernel/kernel.h"
#include "sword25/kernel/callbackregistry.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/input/inputengine.h"

namespace Sword25 {

#define DOUBLE_CLICK_TIME 500
#define DOUBLE_CLICK_RECT_SIZE 4

InputEngine::InputEngine(Kernel *pKernel) :
	Service(pKernel),
	_currentState(0),
	_leftMouseDown(false),
	_rightMouseDown(false),
	_mouseX(0),
	_mouseY(0),
	_leftDoubleClick(false),
	_doubleClickTime(DOUBLE_CLICK_TIME),
	_doubleClickRectWidth(DOUBLE_CLICK_RECT_SIZE),
	_doubleClickRectHeight(DOUBLE_CLICK_RECT_SIZE),
	_lastLeftClickTime(0),
	_lastLeftClickMouseX(0),
	_lastLeftClickMouseY(0) {
	memset(_keyboardState[0], 0, sizeof(_keyboardState[0]));
	memset(_keyboardState[1], 0, sizeof(_keyboardState[1]));
	_leftMouseState[0] = false;
	_leftMouseState[1] = false;
	_rightMouseState[0] = false;
	_rightMouseState[1] = false;

	if (!registerScriptBindings())
		BS_LOG_ERRORLN("Script bindings could not be registered.");
	else
		BS_LOGLN("Script bindings registered.");
}

InputEngine::~InputEngine() {
	unregisterScriptBindings();
}

Service *InputEngine_CreateObject(Kernel *pKernel) {
	return new InputEngine(pKernel);
}

bool InputEngine::init() {
	// No initialisation needed
	return true;
}

void InputEngine::update() {
	Common::Event event;
	_currentState ^= 1;

	// Loop through processing any pending events
	bool handleEvents = true;
	while (handleEvents && g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_LBUTTONUP:
			_leftMouseDown = event.type == Common::EVENT_LBUTTONDOWN;
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			handleEvents = false;
			break;
		case Common::EVENT_RBUTTONDOWN:
		case Common::EVENT_RBUTTONUP:
			_rightMouseDown = event.type == Common::EVENT_RBUTTONDOWN;
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			handleEvents = false;
			break;

		case Common::EVENT_MOUSEMOVE:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			break;

		case Common::EVENT_KEYDOWN:
		case Common::EVENT_KEYUP:
			alterKeyboardState(event.kbd.keycode, (event.type == Common::EVENT_KEYDOWN) ? 0x80 : 0);
			break;

		default:
			break;
		}
	}

	_leftMouseState[_currentState] = _leftMouseDown;
	_rightMouseState[_currentState] = _rightMouseDown;

	testForLeftDoubleClick();
}

bool InputEngine::isLeftMouseDown() {
	return _leftMouseDown;
}

bool InputEngine::isRightMouseDown() {
	return _rightMouseDown;
}

void InputEngine::testForLeftDoubleClick() {
	_leftDoubleClick = false;

	// Only bother checking for a double click if the left mouse button was clicked
	if (wasLeftMouseDown()) {
		// Get the time now
		uint now = Kernel::getInstance()->getMilliTicks();

		// A double click is signalled if
		// 1. The two clicks are close enough together
		// 2. The mouse cursor hasn't moved much
		if (now - _lastLeftClickTime <= _doubleClickTime &&
		        ABS(_mouseX - _lastLeftClickMouseX) <= _doubleClickRectWidth / 2 &&
		        ABS(_mouseY - _lastLeftClickMouseY) <= _doubleClickRectHeight / 2) {
			_leftDoubleClick = true;

			// Reset the time and position of the last click, so that clicking is not
			// interpreted as the first click of a further double-click
			_lastLeftClickTime = 0;
			_lastLeftClickMouseX = 0;
			_lastLeftClickMouseY = 0;
		} else {
			// There is no double click. Remember the position and time of the click,
			// in case it's the first click of a double-click sequence
			_lastLeftClickTime = now;
			_lastLeftClickMouseX = _mouseX;
			_lastLeftClickMouseY = _mouseY;
		}
	}
}

void InputEngine::alterKeyboardState(int keycode, byte newState) {
	_keyboardState[_currentState][keycode] = newState;
}

bool InputEngine::isLeftDoubleClick() {
	return _leftDoubleClick;
}

bool InputEngine::wasLeftMouseDown() {
	return (_leftMouseState[_currentState] == false) && (_leftMouseState[_currentState ^ 1] == true);
}

bool InputEngine::wasRightMouseDown() {
	return (_rightMouseState[_currentState] == false) && (_rightMouseState[_currentState ^ 1] == true);
}

int InputEngine::getMouseX() {
	return _mouseX;
}

int InputEngine::getMouseY() {
	return _mouseY;
}

bool InputEngine::isKeyDown(uint keyCode) {
	return (_keyboardState[_currentState][keyCode] & 0x80) != 0;
}

bool InputEngine::wasKeyDown(uint keyCode) {
	return ((_keyboardState[_currentState][keyCode] & 0x80) == 0) &&
	       ((_keyboardState[_currentState ^ 1][keyCode] & 0x80) != 0);
}

void InputEngine::setMouseX(int posX) {
	_mouseX = posX;
	g_system->warpMouse(_mouseX, _mouseY);
}

void InputEngine::setMouseY(int posY) {
	_mouseY = posY;
	g_system->warpMouse(_mouseX, _mouseY);
}

bool InputEngine::registerCharacterCallback(CharacterCallback callback) {
	if (Common::find(_characterCallbacks.begin(), _characterCallbacks.end(), callback) == _characterCallbacks.end()) {
		_characterCallbacks.push_back(callback);
		return true;
	} else {
		BS_LOG_WARNINGLN("Tried to register an CharacterCallback that was already registered.");
		return false;
	}
}

bool InputEngine::unregisterCharacterCallback(CharacterCallback callback) {
	Common::List<CharacterCallback>::iterator callbackIter = Common::find(_characterCallbacks.begin(),
	        _characterCallbacks.end(), callback);
	if (callbackIter != _characterCallbacks.end()) {
		_characterCallbacks.erase(callbackIter);
		return true;
	} else {
		BS_LOG_WARNINGLN("Tried to unregister an CharacterCallback that was not previously registered.");
		return false;
	}
}

bool InputEngine::registerCommandCallback(CommandCallback callback) {
	if (Common::find(_commandCallbacks.begin(), _commandCallbacks.end(), callback) == _commandCallbacks.end()) {
		_commandCallbacks.push_back(callback);
		return true;
	} else {
		BS_LOG_WARNINGLN("Tried to register an CommandCallback that was already registered.");
		return false;
	}
}

bool InputEngine::unregisterCommandCallback(CommandCallback callback) {
	Common::List<CommandCallback>::iterator callbackIter =
	    Common::find(_commandCallbacks.begin(), _commandCallbacks.end(), callback);
	if (callbackIter != _commandCallbacks.end()) {
		_commandCallbacks.erase(callbackIter);
		return true;
	} else {
		BS_LOG_WARNINGLN("Tried to unregister an CommandCallback that was not previously registered.");
		return false;
	}
}

void InputEngine::reportCharacter(byte character) {
	Common::List<CharacterCallback>::const_iterator callbackIter = _characterCallbacks.begin();
	while (callbackIter != _characterCallbacks.end()) {
		// Iterator vor dem Aufruf erhöhen und im Folgendem auf einer Kopie arbeiten.
		// Dieses Vorgehen ist notwendig da der Iterator möglicherweise von der Callbackfunktion durch das Deregistrieren des Callbacks
		// invalidiert wird.
		Common::List<CharacterCallback>::const_iterator curCallbackIter = callbackIter;
		++callbackIter;

		(*curCallbackIter)(character);
	}
}

void InputEngine::reportCommand(KEY_COMMANDS command) {
	Common::List<CommandCallback>::const_iterator callbackIter = _commandCallbacks.begin();
	while (callbackIter != _commandCallbacks.end()) {
		// Iterator vor dem Aufruf erhöhen und im Folgendem auf einer Kopie arbeiten.
		// Dieses Vorgehen ist notwendig da der Iterator möglicherweise von der Callbackfunktion durch das Deregistrieren des Callbacks
		// invalidiert wird.
		Common::List<CommandCallback>::const_iterator curCallbackIter = callbackIter;
		++callbackIter;

		(*curCallbackIter)(command);
	}
}

bool InputEngine::persist(OutputPersistenceBlock &writer) {
	// Anzahl an Command-Callbacks persistieren.
	writer.write(_commandCallbacks.size());

	// Alle Command-Callbacks einzeln persistieren.
	{
		Common::List<CommandCallback>::const_iterator It = _commandCallbacks.begin();
		while (It != _commandCallbacks.end()) {
			writer.write(CallbackRegistry::instance().resolveCallbackPointer(*It));
			++It;
		}
	}

	// Anzahl an Character-Callbacks persistieren.
	writer.write(_characterCallbacks.size());

	// Alle Character-Callbacks einzeln persistieren.
	{
		Common::List<CharacterCallback>::const_iterator It = _characterCallbacks.begin();
		while (It != _characterCallbacks.end()) {
			writer.write(CallbackRegistry::instance().resolveCallbackPointer(*It));
			++It;
		}
	}

	return true;
}

bool InputEngine::unpersist(InputPersistenceBlock &reader) {
	// Command-Callbackliste leeren.
	_commandCallbacks.clear();

	// Anzahl an Command-Callbacks lesen.
	uint commandCallbackCount;
	reader.read(commandCallbackCount);

	// Alle Command-Callbacks wieder herstellen.
	for (uint i = 0; i < commandCallbackCount; ++i) {
		Common::String callbackFunctionName;
		reader.read(callbackFunctionName);

		_commandCallbacks.push_back(reinterpret_cast<CommandCallback>(
		                                 CallbackRegistry::instance().resolveCallbackFunction(callbackFunctionName)));
	}

	// Character-Callbackliste leeren.
	_characterCallbacks.clear();

	// Anzahl an Character-Callbacks lesen.
	uint characterCallbackCount;
	reader.read(characterCallbackCount);

	// Alle Character-Callbacks wieder herstellen.
	for (uint i = 0; i < characterCallbackCount; ++i) {
		Common::String callbackFunctionName;
		reader.read(callbackFunctionName);

		_characterCallbacks.push_back(reinterpret_cast<CharacterCallback>(CallbackRegistry::instance().resolveCallbackFunction(callbackFunctionName)));
	}

	return reader.isGood();
}

} // End of namespace Sword25
