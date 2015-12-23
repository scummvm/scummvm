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
 */

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "common/events.h"

#include "lab/lab.h"

#include "lab/dispman.h"
#include "lab/eventman.h"
#include "lab/image.h"

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

Button *EventManager::checkButtonHit(ButtonList *buttonList, Common::Point pos) {
	for (ButtonList::iterator buttonItr = buttonList->begin(); buttonItr != buttonList->end(); ++buttonItr) {
		Button *button = *buttonItr;
		Common::Rect buttonRect(button->_x, button->_y, button->_x + button->_image->_width - 1, button->_y + button->_image->_height - 1);

		if (buttonRect.contains(pos) && button->_isEnabled) {
			if (_vm->_isHiRes) {
				_hitButton = button;
			} else {
				mouseHide();
				button->_altImage->drawImage(button->_x, button->_y);
				mouseShow();

				for (uint16 i = 0; i < 3; i++)
					_vm->waitTOF();

				mouseHide();
				button->_image->drawImage(button->_x, button->_y);
				mouseShow();
			}

			return button;
		}
	}

	return nullptr;
}

void EventManager::attachButtonList(ButtonList *buttonList) {
	if (_screenButtonList != buttonList)
		_lastButtonHit = nullptr;

	_screenButtonList = buttonList;
}

Button *EventManager::getButton(uint16 id) {
	for (ButtonList::iterator buttonItr = _screenButtonList->begin(); buttonItr != _screenButtonList->end(); ++buttonItr) {
		Button *button = *buttonItr;
		if (button->_buttonId == id)
			return button;
	}

	return nullptr;
}

EventManager::EventManager(LabEngine *vm) : _vm(vm) {
	_leftClick = false;
	_rightClick = false;

	_mouseHidden = true;
	_lastButtonHit = nullptr;
	_screenButtonList = nullptr;
	_hitButton = nullptr;
	_mousePos = Common::Point(0, 0);

	_nextKeyIn = 0;
	_nextKeyOut = 0;

	for (int i = 0; i < 64; i++)
		_keyBuf[i] = Common::KEYCODE_INVALID;

}

void EventManager::updateMouse() {
	bool doUpdateDisplay = false;

	if (!_mouseHidden)
		doUpdateDisplay = true;

	if (_hitButton) {
		mouseHide();
		_hitButton->_altImage->drawImage(_hitButton->_x, _hitButton->_y);
		mouseShow();

		for (uint16 i = 0; i < 3; i++)
			_vm->waitTOF();

		mouseHide();
		_hitButton->_image->drawImage(_hitButton->_x, _hitButton->_y);
		mouseShow();
		doUpdateDisplay = true;
		_hitButton = nullptr;
	}

	if (doUpdateDisplay)
		_vm->_graphics->screenUpdate();
}

void EventManager::initMouse() {
	_vm->_system->setMouseCursor(mouseData, MOUSE_WIDTH, MOUSE_HEIGHT, 0, 0, 0);
	_vm->_system->showMouse(false);

	setMousePos(Common::Point(0, 0));
}

void EventManager::mouseShow() {
	if (_mouseHidden) {
		processInput();
		_mouseHidden = false;
	}

	_vm->_system->showMouse(true);
}

void EventManager::mouseHide() {
	if (!_mouseHidden) {
		_mouseHidden = true;

		_vm->_system->showMouse(false);
	}
}

Common::Point EventManager::getMousePos() {
	if (_vm->_isHiRes)
		return _mousePos;
	else
		return Common::Point(_mousePos.x / 2, _mousePos.y);
}

void EventManager::setMousePos(Common::Point pos) {
	if (_vm->_isHiRes)
		_vm->_system->warpMouse(pos.x, pos.y);
	else
		_vm->_system->warpMouse(pos.x * 2, pos.y);

	if (!_mouseHidden)
		processInput();
}

bool EventManager::keyPress(Common::KeyCode *keyCode) {
	if (haveNextChar()) {
		*keyCode = getNextChar();
		return true;
	}

	return false;
}

bool EventManager::haveNextChar() {
	processInput();
	return _nextKeyIn != _nextKeyOut;
}

void EventManager::processInput() {
	Common::Event event;
	Button *curButton = nullptr;

	while (_vm->_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_LBUTTONDOWN:
			if (_screenButtonList)
				curButton = checkButtonHit(_screenButtonList, _vm->_isHiRes ? _mousePos : Common::Point(_mousePos.x / 2, _mousePos.y));

			if (curButton)
				_lastButtonHit = curButton;
			else
				_leftClick = true;
			break;
		case Common::EVENT_RBUTTONDOWN:
			_rightClick = true;
			break;
		case Common::EVENT_MOUSEMOVE:
			_mousePos = event.mouse;
			break;
		case Common::EVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_LEFTBRACKET:
				_vm->changeVolume(-1);
				break;
			case Common::KEYCODE_RIGHTBRACKET:
				_vm->changeVolume(1);
				break;
			case Common::KEYCODE_z:
				//saveSettings();
				break;
			case Common::KEYCODE_d:
				if (event.kbd.hasFlags(Common::KBD_CTRL)) {
					// Open debugger console
					_vm->_console->attach();
					continue;
				}
				// Intentional fall through
			default: {
				int n = (_nextKeyIn + 1) % 64;
				if (n != _nextKeyOut) {
					_keyBuf[_nextKeyIn] = event.kbd.keycode;
					_nextKeyIn = n;
				}
				}
			}
			break;
		case Common::EVENT_QUIT:
		case Common::EVENT_RTL:
		default:
			break;
		}
	}

	_vm->_system->copyRectToScreen(_vm->_graphics->_displayBuffer, _vm->_graphics->_screenWidth, 0, 0, _vm->_graphics->_screenWidth, _vm->_graphics->_screenHeight);
	_vm->_console->onFrame();
	_vm->_system->updateScreen();
}

Common::KeyCode EventManager::getNextChar() {
	Common::KeyCode chr = Common::KEYCODE_INVALID;

	processInput();
	if (_nextKeyIn != _nextKeyOut) {
		chr = _keyBuf[_nextKeyOut];
		_nextKeyOut = (_nextKeyOut + 1) % 64;
	}

	return chr;
}

Common::Point EventManager::updateAndGetMousePos() {
	processInput();

	return _mousePos;
}

} // End of namespace Lab
