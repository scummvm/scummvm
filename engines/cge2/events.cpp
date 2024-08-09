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
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janusz B. Wisniewski and L.K. Avalon
 */

#include "gui/saveload.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "cge2/events.h"
#include "cge2/text.h"
#include "cge2/cge2_main.h"

namespace CGE2 {

/*----------------- KEYBOARD interface -----------------*/

Keyboard::Keyboard(CGE2Engine *vm) : _client(nullptr), _vm(vm) {
}

Keyboard::~Keyboard() {
}

Sprite *Keyboard::setClient(Sprite *spr) {
	SWAP(_client, spr);
	return spr;
}

void Keyboard::handleAction(Common::Event &event) {
	switch (event.customType) {
	case kActionInfo:
		// Display ScummVM version and translation strings
		for (int i = 0; i < 3; i++)
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
			CGE2Event &evt = _vm->_eventManager->getNextEvent();
			evt._x = 0;
			evt._y = 0;
			evt._mask = kEventEsc;    // Event mask
			evt._spritePtr = _client; // Sprite pointer
		}
		break;
	default:
		break;
	}
}

/*----------------- MOUSE interface -----------------*/

Mouse::Mouse(CGE2Engine *vm) : Sprite(vm), _busy(nullptr), _hold(nullptr), _hx(0), _point(vm), _vm(vm) {
	_hold = nullptr;
	_hx = 0;
	_hy = 0;
	_exist = true;
	_buttons = 0;
	_busy = nullptr;
	_active = false;
	_flags._kill = false;

	setSeq(_stdSeq8);

	BitmapPtr MC = new Bitmap[2];
	MC[0] = Bitmap(_vm, "MOUSE");
	MC[1] = Bitmap(_vm, "DUMMY");
	setShapeList(MC, 2);

	step(1);
	on();
	off();
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
		if (_exist)
			_active = false;

		step(1);
		if (_busy)
			_busy->step(1);
	}
}

void Mouse::newMouse(Common::Event &event) {
	if (!_active)
		return;

	CGE2Event &evt = _vm->_eventManager->getNextEvent();
	evt._x = event.mouse.x;
	evt._y = event.mouse.y;
	evt._spritePtr = _vm->spriteAt(V2D(_vm, evt._x, evt._y));

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

EventManager::EventManager(CGE2Engine *vm) : _vm(vm) {
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
		_event.mouse.y = kWorldHeight - _event.mouse.y;
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
		CGE2Event e = _eventQueue[_eventQueueTail];
		_vm->_mouse->_point = V2D(_vm, e._x, e._y);
		if (e._mask) {
			if (e._mask & kMouseMask) {
				e._spritePtr = _vm->spriteAt(_vm->_mouse->_point);
				e._x += (_vm->_mouse->_siz.x >> 1);
				e._y -= _vm->_mouse->_siz.y;
				if (_vm->_mouse->_hold && (e._spritePtr != _vm->_mouse->_hold)) {
					_vm->_mouse->_hold->touch(e._mask | kEventAttn,
						V2D(_vm, e._x - _vm->_mouse->_hold->_pos2D.x, e._y - _vm->_mouse->_hold->_pos2D.y));
				}
				// update mouse cursor position
				if (e._mask & kMouseRoll)
					_vm->_mouse->gotoxyz(V2D(_vm, e._x, e._y));
			}

			// activate current touched SPRITE
			if (e._spritePtr) {
				e._spritePtr->touch(e._mask, _vm->_mouse->_point - e._spritePtr->_pos2D);
			} else if (_vm->_sys)
				_vm->_sys->touch(e._mask, _vm->_mouse->_point);

			// discard Text if button released
			if (e._mask & (kMouseLeftUp | kMouseRightUp))
				_vm->killText();
		}
		_eventQueueTail = (_eventQueueTail + 1) % kEventMax;
	}
}

void EventManager::clearEvent(Sprite *spr) {
	if (spr) {
		for (uint16 e = _eventQueueTail; e != _eventQueueHead; e = (e + 1) % kEventMax) {
			if (_eventQueue[e]._spritePtr == spr)
				_eventQueue[e]._mask = 0;
		}
	} else
		_eventQueueTail = _eventQueueHead;
}

CGE2Event &EventManager::getNextEvent() {
	CGE2Event &evt = _eventQueue[_eventQueueHead];
	_eventQueueHead = (_eventQueueHead + 1) % kEventMax;

	return evt;
}

} // End of namespace CGE2
