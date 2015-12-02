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

#include "lab/lab.h"
#include "lab/image.h"
#include "lab/mouse.h"
#include "lab/interface.h"

namespace Lab {

static byte MouseData[] = {1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
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
						   0, 0, 0, 0, 0, 0, 1, 1, 0, 0};

#define MOUSE_WIDTH 10
#define MOUSE_HEIGHT 15

/*****************************************************************************/
/* Checks whether or not the cords fall within one of the gadgets in a list  */
/* of gadgets.                                                               */
/*****************************************************************************/
Gadget *EventManager::checkGadgetHit(Gadget *gadgetList, Common::Point pos) {
	while (gadgetList != NULL) {
		if ((pos.x >= gadgetList->x) && (pos.y >= gadgetList->y) &&
		    (pos.x <= (gadgetList->x + gadgetList->Im->Width)) &&
		    (pos.y <= (gadgetList->y + gadgetList->Im->Height)) &&
		     !(GADGETOFF & gadgetList->GadgetFlags)) {
			if (_vm->_isHiRes) {
				_hitGadget = gadgetList;
			} else {
				mouseHide();
				gadgetList->ImAlt->drawImage(gadgetList->x, gadgetList->y);
				mouseShow();

				for (uint16 i = 0; i < 3; i++)
					_vm->waitTOF();

				mouseHide();
				gadgetList->Im->drawImage(gadgetList->x, gadgetList->y);
				mouseShow();
			}

			return gadgetList;
		} else {
			gadgetList = gadgetList->NextGadget;
		}
	}

	return NULL;
}



void EventManager::attachGadgetList(Gadget *gadgetList) {
	if (_screenGadgetList != gadgetList)
		_lastGadgetHit = nullptr;

	_screenGadgetList = gadgetList;
}

EventManager::EventManager(LabEngine *vm) : _vm(vm) {
	_leftClick = false;
	_rightClick = false;

	_mouseHidden = true;
	_numHidden   = 1;
	_lastGadgetHit = nullptr;
	_screenGadgetList = nullptr;
	_hitGadget = nullptr;
	_nextKeyIn = 0;
	_nextKeyOut = 0;
	_mousePos = Common::Point(0, 0);
	_mouseAtEdge = false;

	for (int i = 0; i < 64; i++)
		_keyBuf[i] = 0;

}

void EventManager::mouseHandler(int flag, Common::Point pos) {
	if (_numHidden >= 2)
		return;

	if (flag & 0x02) { /* Left mouse button click */
		Gadget *tmp = NULL;
		if (_screenGadgetList)
			tmp = checkGadgetHit(_screenGadgetList, _vm->_isHiRes ? pos : Common::Point(pos.x / 2, pos.y));

		if (tmp)
			_lastGadgetHit = tmp;
		else
			_leftClick = true;
	}

	if (flag & 0x08) /* Right mouse button click */
		_rightClick = true;
}

void EventManager::updateMouse() {
	bool doUpdateDisplay = false;

	if (!_mouseHidden)
		doUpdateDisplay = true;

	if (_hitGadget) {
		mouseHide();
		_hitGadget->ImAlt->drawImage(_hitGadget->x, _hitGadget->y);
		mouseShow();

		for (uint16 i = 0; i < 3; i++)
			_vm->waitTOF();

		mouseHide();
		_hitGadget->Im->drawImage(_hitGadget->x, _hitGadget->y);
		mouseShow();
		doUpdateDisplay = true;
		_hitGadget = nullptr;
	}

	if (doUpdateDisplay)
		_vm->screenUpdate();
}


/*****************************************************************************/
/* Initializes the mouse.                                                    */
/*****************************************************************************/
void EventManager::initMouse() {
	g_system->setMouseCursor(MouseData, MOUSE_WIDTH, MOUSE_HEIGHT, 0, 0, 0);
	g_system->showMouse(false);

	setMousePos(Common::Point(0, 0));
}


/*****************************************************************************/
/* Shows the mouse.                                                          */
/*****************************************************************************/
void EventManager::mouseShow() {
	if (_numHidden)
		_numHidden--;

	if ((_numHidden == 0) && _mouseHidden) {
		processInput();
		_mouseHidden = false;
	}

	g_system->showMouse(true);
}

/*****************************************************************************/
/* Hides the mouse.                                                          */
/*****************************************************************************/
void EventManager::mouseHide() {
	_numHidden++;

	if (_numHidden && !_mouseHidden) {
		_mouseHidden = true;

		g_system->showMouse(false);
	}
}

/*****************************************************************************/
/* Gets the current mouse co-ordinates.  NOTE: On IBM version, will scale    */
/* from virtual to screen co-ordinates automatically.                        */
/*****************************************************************************/
Common::Point EventManager::getMousePos() {
	if (_vm->_isHiRes)
		return _mousePos;
	else
		return Common::Point(_mousePos.x / 2, _mousePos.y);
}


/*****************************************************************************/
/* Moves the mouse to new co-ordinates.                                      */
/*****************************************************************************/
void EventManager::setMousePos(Common::Point pos) {
	if (_vm->_isHiRes)
		g_system->warpMouse(pos.x, pos.y);
	else
		g_system->warpMouse(pos.x * 2, pos.y);

	if (!_mouseHidden)
		processInput();
}


/*****************************************************************************/
/* Checks whether or not the mouse buttons have been pressed, and the last   */
/* co-ordinates of the button press.  leftbutton tells whether to check the  */
/* left or right button.                                                     */
/*****************************************************************************/
bool EventManager::mouseButton(uint16 *x, uint16 *y, bool leftbutton) {
	if (leftbutton) {
		if (_leftClick) {
			*x = (!_vm->_isHiRes) ? (uint16)_mousePos.x / 2 : (uint16)_mousePos.x;
			*y = (uint16)_mousePos.y;
			_leftClick = false;
			return true;
		}
	} else {
		if (_rightClick) {
			*x = (!_vm->_isHiRes) ? (uint16)_mousePos.x / 2 : (uint16)_mousePos.x;
			*y = (uint16)_mousePos.y;
			_rightClick = false;
			return true;
		}
	}

	return false;
}

Gadget *EventManager::mouseGadget() {
	Gadget *temp = _lastGadgetHit;

	_lastGadgetHit = nullptr;
	return temp;
}

/*****************************************************************************/
/* Checks whether or not a key has been pressed.                             */
/*****************************************************************************/
bool EventManager::keyPress(uint16 *keyCode) {
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

void EventManager::processInput(bool can_delay) {
	Common::Event event;

	if (1 /*!g_IgnoreProcessInput*/) {
		int flags = 0;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_RBUTTONDOWN:
				flags |= 8;
				mouseHandler(flags, _mousePos);
				break;

			case Common::EVENT_LBUTTONDOWN:
				flags |= 2;
				mouseHandler(flags, _mousePos);
				break;

			case Common::EVENT_MOUSEMOVE: {
				int lastMouseAtEdge = _mouseAtEdge;
				_mouseAtEdge = false;
				_mousePos.x = event.mouse.x;
				if (event.mouse.x <= 0) {
					_mousePos.x = 0;
					_mouseAtEdge = true;
				}
				if (_mousePos.x > _vm->_screenWidth - 1) {
					_mousePos.x = _vm->_screenWidth;
					_mouseAtEdge = true;
				}

				_mousePos.y = event.mouse.y;
				if (event.mouse.y <= 0) {
					_mousePos.y = 0;
					_mouseAtEdge = true;
				}
				if (_mousePos.y > _vm->_screenHeight - 1) {
					_mousePos.y = _vm->_screenHeight;
					_mouseAtEdge = true;
				}

				if (!lastMouseAtEdge || !_mouseAtEdge)
					mouseHandler(1, _mousePos);
				}
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

				default: {
					int n = ((((unsigned int)((_nextKeyIn + 1) >> 31) >> 26) + (byte)_nextKeyIn + 1) & 0x3F)
						- ((unsigned int)((_nextKeyIn + 1) >> 31) >> 26);
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

			g_system->copyRectToScreen(_vm->_displayBuffer, _vm->_screenWidth, 0, 0, _vm->_screenWidth, _vm->_screenHeight);
			g_system->updateScreen();
		}
	}

	if (can_delay)
		g_system->delayMillis(10);
}

uint16 EventManager::getNextChar() {
	uint16 c = 0;

	processInput();
	if (_nextKeyIn != _nextKeyOut) {
		c = _keyBuf[_nextKeyOut];
		_nextKeyOut = ((((unsigned int)((_nextKeyOut + 1) >> 31) >> 26) + (byte)_nextKeyOut + 1) & 0x3F)
			- ((unsigned int)((_nextKeyOut + 1) >> 31) >> 26);
	}

	return c;
}

Common::Point EventManager::updateAndGetMousePos() {
	processInput();

	return _mousePos;
}
} // End of namespace Lab
