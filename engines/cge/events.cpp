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
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janusz B. Wisniewski and L.K. Avalon
 */

#include "gui/saveload.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "engines/advancedDetector.h"
#include "cge/events.h"
#include "cge/events.h"
#include "cge/text.h"
#include "cge/cge_main.h"

namespace CGE {

/*----------------- KEYBOARD interface -----------------*/

Keyboard::Keyboard(CGEEngine *vm) : _client(nullptr), _vm(vm) {
	_keyAlt = false;
}

Keyboard::~Keyboard() {
}

Sprite *Keyboard::setClient(Sprite *spr) {
	SWAP(_client, spr);
	return spr;
}

void Keyboard::handleAction(Common::Event &event) {
	_keyAlt = false;

	switch (event.customType) {
	case kActionInfo:
		// Display ScummVM version and translation strings
		for (int i = 0; i < 5; i++)
			_vm->_commandHandler->addCommand(kCmdInf, 1, kShowScummVMVersion + i, nullptr);
		break;
	case kActionSave:
		_vm->saveGameDialog();
		break;
	case kActionLoad:
		_vm->loadGameDialog();
		break;
	case kActionQuit:
		_vm->quit();
		break;
	case kActionEscape:
		if (_client) {
			CGEEvent &evt = _vm->_eventManager->getNextEvent();
			evt._x = 0;
			evt._y = 0;
			evt._mask = kEventEsc;    // Event mask
			evt._spritePtr = _client; // Sprite pointer
		}
		break;
	case kActionAltDice:
		_keyAlt = true;
		break;
	case kActionInv1:
	case kActionInv2:
	case kActionInv3:
	case kActionInv4:
	case kActionInv5:
	case kActionInv6:
	case kActionInv7:
	case kActionInv8:
		_vm->selectPocket(event.customType - kActionInv1);
		break;
	case kActionLevel0:
	case kActionLevel1:
	case kActionLevel2:
	case kActionLevel3:
	case kActionLevel4:
		_vm->_commandHandler->addCommand(kCmdLevel, -1, event.customType - kActionLevel0, nullptr);
		break;
	default:
		break;
	}
}

/*----------------- MOUSE interface -----------------*/

Mouse::Mouse(CGEEngine *vm) : Sprite(vm, nullptr), _busy(nullptr), _hold(nullptr), _hx(0), _vm(vm) {
	_hold = nullptr;
	_hx = 0;
	_hy = 0;
	_exist = true;
	_buttons = 0;
	_busy = nullptr;
	_active = false;
	_flags._kill = false;

	const Seq ms[] = {
		{ 0, 0, 0, 0, 1 },
		{ 1, 1, 0, 0, 1 }
	};
	Seq *seq = (Seq *)malloc(2 * sizeof(Seq));
	Common::copy(ms, ms + 2, seq);
	setSeq(seq);

	BitmapPtr *MC = new BitmapPtr[3];
	MC[0] = new Bitmap(_vm, "MOUSE");
	MC[1] = new Bitmap(_vm, "DUMMY");
	MC[2] = nullptr;
	setShapeList(MC);

	gotoxy(kScrWidth / 2, kScrHeight / 2);
	_z = 127;
	step(1);
}

Mouse::~Mouse() {
	off();
}

void Mouse::on() {
	if (_seqPtr && _exist) {
		_active = true;
		step(0);
		if (_busy)
			_busy->step(0);
	}
}

void Mouse::off() {
	if (_seqPtr == 0) {
		if (_exist) {
			_active = false;
		}

		step(1);
		if (_busy)
			_busy->step(1);
	}
}

void Mouse::newMouse(Common::Event &event) {
	if (!_active)
		return;

	CGEEvent &evt = _vm->_eventManager->getNextEvent();
	evt._x = event.mouse.x;
	evt._y = event.mouse.y;
	evt._spritePtr = _vm->spriteAt(evt._x, evt._y);

	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		evt._mask = kMouseRoll;
		break;
	case Common::EVENT_LBUTTONDOWN:
		evt._mask = kMouseLeftDown;
		_buttons |= 1;
		break;
	case Common::EVENT_LBUTTONUP:
		evt._mask = kMouseLeftUp;
		_buttons &= ~1;
		break;
	case Common::EVENT_RBUTTONDOWN:
		evt._mask = kMouseRightDown;
		_buttons |= 2;
		break;
	case Common::EVENT_RBUTTONUP:
		evt._mask = kMouseRightUp;
		_buttons &= ~2;
		break;
	default:
		break;
	}
}

/*----------------- EventManager interface -----------------*/

EventManager::EventManager(CGEEngine *vm) : _vm(vm){
	_eventQueueHead = 0;
	_eventQueueTail = 0;
	for (uint16 k = 0; k < kEventMax; k++) {
		_eventQueue[k]._mask = 0;
		_eventQueue[k]._x = 0;
		_eventQueue[k]._y = 0;
		_eventQueue[k]._spritePtr = nullptr;
	}
	_event.joystick.axis = 0;
	_event.joystick.position = 0;
	_event.joystick.button = 0;
}

void EventManager::poll() {
	while (g_system->getEventManager()->pollEvent(_event)) {
		switch (_event.type) {
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			// Handle keyboard events
			_vm->_keyboard->handleAction(_event);
			handleEvents();
			break;
		case Common::EVENT_MOUSEMOVE:
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONDOWN:
		case Common::EVENT_RBUTTONUP:
			// Handle mouse events
			_vm->_mouse->newMouse(_event);
			handleEvents();
			break;
		default:
			break;
		}
	}
}

void EventManager::handleEvents() {
	while (_eventQueueTail != _eventQueueHead) {
		CGEEvent e = _eventQueue[_eventQueueTail];
		if (e._mask) {
			if (_vm->_mouse->_hold && e._spritePtr != _vm->_mouse->_hold)
				_vm->_mouse->_hold->touch(e._mask | kEventAttn, e._x - _vm->_mouse->_hold->_x, e._y - _vm->_mouse->_hold->_y);

			// update mouse cursor position
			if (e._mask & kMouseRoll)
				_vm->_mouse->gotoxy(e._x, e._y);

			// activate current touched SPRITE
			if (e._spritePtr) {
				e._spritePtr->touch(e._mask, e._x - e._spritePtr->_x, e._y - e._spritePtr->_y);
			} else if (_vm->_sys)
				_vm->_sys->touch(e._mask, e._x, e._y);

			if (e._mask & kMouseLeftDown) {
				_vm->_mouse->_hold = e._spritePtr;
				if (_vm->_mouse->_hold) {
					_vm->_mouse->_hold->_flags._hold = true;

					if (_vm->_mouse->_hold->_flags._drag) {
						_vm->_mouse->_hx = e._x - _vm->_mouse->_hold->_x;
						_vm->_mouse->_hy = e._y - _vm->_mouse->_hold->_y;
					}
				}
			}

			if (e._mask & kMouseLeftUp) {
				if (_vm->_mouse->_hold) {
					_vm->_mouse->_hold->_flags._hold = false;
					_vm->_mouse->_hold = nullptr;
				}
			}
			///Touched = e.Ptr;

			// discard Text if button released
			if (e._mask & (kMouseLeftUp | kMouseRightUp))
				_vm->killText();
		}
		_eventQueueTail = (_eventQueueTail + 1) % kEventMax;
	}
	if (_vm->_mouse->_hold) {
		if (_vm->_mouse->_hold->_flags._drag)
			_vm->_mouse->_hold->gotoxy(_vm->_mouse->_x - _vm->_mouse->_hx, _vm->_mouse->_y - _vm->_mouse->_hy);
	}
}

void EventManager::clearEvent(Sprite *spr) {
	if (spr) {
		for (uint16 e = _eventQueueTail; e != _eventQueueHead; e = (e + 1) % kEventMax)
			if (_eventQueue[e]._spritePtr == spr)
				_eventQueue[e]._mask = 0;
	} else
		_eventQueueTail = _eventQueueHead;
}

CGEEvent &EventManager::getNextEvent() {
	CGEEvent &evt = _eventQueue[_eventQueueHead];
	_eventQueueHead = (_eventQueueHead + 1) % kEventMax;

	return evt;
}

} // End of namespace CGE
