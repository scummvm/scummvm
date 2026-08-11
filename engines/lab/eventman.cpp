/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "common/events.h"

#include "graphics/cursorman.h"

#include "lab/lab.h"

#include "lab/dispman.h"
#include "lab/eventman.h"
#include "lab/image.h"
#include "lab/interface.h"

namespace Lab {

static const byte mouseData[] = {
	1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 7, 1, 0, 0, 0, 0, 0, 0, 0,
	1, 7, 7, 1, 0, 0, 0, 0, 0, 0,
	1, 7, 7, 7, 1, 0, 0, 0, 0, 0,
	1, 7, 7, 7, 7, 1, 0, 0, 0, 0,
	1, 7, 7, 7, 7, 7, 1, 0, 0, 0,
	1, 7, 7, 7, 7, 7, 7, 1, 0, 0,
	1, 7, 7, 7, 7, 7, 7, 7, 1, 0,
	1, 7, 7, 7, 7, 7, 1, 1, 1, 1,
	1, 7, 7, 1, 7, 7, 1, 0, 0, 0,
	1, 7, 1, 0, 1, 7, 7, 1, 0, 0,
	1, 1, 0, 0, 1, 7, 7, 1, 0, 0,
	0, 0, 0, 0, 0, 1, 7, 7, 1, 0,
	0, 0, 0, 0, 0, 1, 7, 7, 1, 0,
	0, 0, 0, 0, 0, 0, 1, 1, 0, 0
};

#define MOUSE_WIDTH 10
#define MOUSE_HEIGHT 15

EventManager::EventManager(LabEngine *vm) : _vm(vm) {
	_leftClick = false;
	_rightClick = false;
	_buttonHit = false;
	_mousePos = Common::Point(0, 0);
	_actionPressed = kActionNone;
}

void EventManager::initMouse() {
	CursorMan.pushCursor(mouseData, MOUSE_WIDTH, MOUSE_HEIGHT, 0, 0, 0);
	CursorMan.showMouse(false);

	setMousePos(Common::Point(_vm->_graphics->_screenWidth / 2, _vm->_graphics->_screenHeight / 2));
}

void EventManager::mouseShow() {
	CursorMan.showMouse(true);
}

void EventManager::mouseHide() {
	CursorMan.showMouse(false);
}

void EventManager::setMousePos(Common::Point pos) {
	if (_vm->_isHiRes)
		_vm->_system->warpMouse(pos.x, pos.y);
	else
		_vm->_system->warpMouse(pos.x * 2, pos.y);
}

void EventManager::processInput() {
	Common::Event event;

	while (_vm->_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_LBUTTONDOWN:
			if (_vm->_interface->checkButtonHit(_mousePos))
				_buttonHit = true;
			else
				_leftClick = true;
			break;
		case Common::EVENT_RBUTTONDOWN:
			_rightClick = true;
			break;
		case Common::EVENT_MOUSEMOVE:
			_mousePos = event.mouse;
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			switch (event.customType) {
			case kActionSoundLower:
				_vm->changeVolume(-1);
				break;
			case kActionSoundRaise:
				_vm->changeVolume(1);
				break;
			default:
				_actionPressed = event.customType;
				break;
			}
			break;
		case Common::EVENT_KEYDOWN:
			_actionPressed = kActionQuitDialogNo; // Used for "Press any key to continue" scenarios as well.
			break;
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
		default:
			break;
		}
	}
}

IntuiMessage *EventManager::getMsg() {
	static IntuiMessage message;

	_vm->_interface->handlePressedButton();
	processInput();

	if (_buttonHit) {
		Button *lastButtonHit = _vm->_interface->checkButtonHit(_mousePos);
		_buttonHit = false;
		if (lastButtonHit) {
			_vm->_interface->handlePressedButton();
			message._msgClass = kMessageButtonUp;
			message._code = lastButtonHit->_buttonId;
			message._qualifier = 0; // This does not seem to be used anywhere. Since qualifiers can be detected by keymapper anyways, this is set to 0.

			return &message;
		} else
			return nullptr;
	} else if (_leftClick || _rightClick) {
		message._msgClass = (_leftClick) ? kMessageLeftClick : kMessageRightClick;
		message._qualifier = 0;
		message._mouse = _mousePos;
		_leftClick = _rightClick = false;
		return &message;
	} else if (_actionPressed != kActionNone) {
		Button *curButton = _vm->_interface->checkNumButtonHit(_actionPressed);

		if (curButton) {
			message._msgClass = kMessageButtonUp;
			message._code = curButton->_buttonId;
		} else {
			message._msgClass = kMessageAction;
			message._code = _actionPressed;
		}

		message._qualifier = 0; // This does not seem to be used anywhere. Since qualifiers can be detected by keymapper anyways, this is set to 0.
		message._mouse = _mousePos;

		_actionPressed = kActionNone;

		return &message;
	} else
		return nullptr;
}

Common::Point EventManager::updateAndGetMousePos() {
	processInput();

	return _mousePos;
}

void EventManager::simulateEvent() {
	// Simulate an event by setting an unused action.
	_actionPressed = kActionDummy;
}

} // End of namespace Lab
