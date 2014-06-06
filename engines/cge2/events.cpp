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
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#include "gui/saveload.h"
#include "gui/about.h"
#include "gui/message.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "engines/advancedDetector.h"
#include "cge2/events.h"
#include "cge2/text.h"
#include "cge2/cge2_main.h"

namespace CGE2 {

/*----------------- KEYBOARD interface -----------------*/

Keyboard::Keyboard(CGE2Engine *vm) : _client(NULL), _vm(vm) {
	warning("STUB: Keyboard::Keyboard() - Recheck the whole implementation!!!");
}

Keyboard::~Keyboard() {
}

Sprite *Keyboard::setClient(Sprite *spr) {
	warning("STUB: Keyboard::setClient()");
	return spr;
}

bool Keyboard::getKey(Common::Event &event) {
	warning("STUB: Keyboard::getKey()");
	return false;
}

void Keyboard::newKeyboard(Common::Event &event) {
	warning("STUB: Keyboard::newKeyboard()");
}

/*----------------- MOUSE interface -----------------*/

Mouse::Mouse(CGE2Engine *vm) : Sprite(vm), _busy(NULL), _hold(NULL), _hx(0), _vm(vm) {
	_hold = NULL;
	_hx = 0;
	_hy = 0;
	_exist = true;
	_buttons = 0;
	_busy = NULL;
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

	CGE2Event &evt = _vm->_eventManager->getNextEvent();
	evt._x = event.mouse.x;
	evt._y = event.mouse.y;
	evt._keyCode = Common::KEYCODE_INVALID;
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

EventManager::EventManager(CGE2Engine *vm) : _vm(vm){
	_eventQueueHead = 0;
	_eventQueueTail = 0;
	memset(&_eventQueue, 0, kEventMax * sizeof(CGE2Event));
	memset(&_event, 0, sizeof(Common::Event));
}

void EventManager::poll() {
	while (g_system->getEventManager()->pollEvent(_event)) {
		switch (_event.type) {
		case Common::EVENT_KEYDOWN:
		case Common::EVENT_KEYUP:
			// Handle keyboard events
			_vm->_keyboard->newKeyboard(_event);
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
		if (e._mask) {
			if (_vm->_mouse->_hold && e._spritePtr != _vm->_mouse->_hold)
				_vm->_mouse->_hold->touch(e._mask | kEventAttn, e._x - _vm->_mouse->_hold->_pos2D.x, e._y - _vm->_mouse->_hold->_pos2D.y, e._keyCode);

			// update mouse cursor position
			if (e._mask & kMouseRoll)
				_vm->_mouse->gotoxyz(e._x, kWorldHeight - e._y);

			// activate current touched SPRITE
			if (e._spritePtr) {
				if (e._mask & kEventKeyb)
					e._spritePtr->touch(e._mask, e._x, e._y, e._keyCode);
				else
					e._spritePtr->touch(e._mask, e._x - e._spritePtr->_pos2D.x, e._y - e._spritePtr->_pos2D.y, e._keyCode);
			} else if (_vm->_sys)
				_vm->_sys->touch(e._mask, e._x, e._y, e._keyCode);

			if (e._mask & kMouseLeftDown) {
				_vm->_mouse->_hold = e._spritePtr;
				if (_vm->_mouse->_hold) {
					_vm->_mouse->_hold->_flags._hold = true;

					if (_vm->_mouse->_hold->_flags._drag) {
						_vm->_mouse->_hx = e._x - _vm->_mouse->_hold->_pos2D.x;
						_vm->_mouse->_hy = e._y - _vm->_mouse->_hold->_pos2D.y;
					}
				}
			}

			if (e._mask & kMouseLeftUp) {
				if (_vm->_mouse->_hold) {
					_vm->_mouse->_hold->_flags._hold = false;
					_vm->_mouse->_hold = NULL;
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
			_vm->_mouse->_hold->gotoxyz(_vm->_mouse->_pos2D.x - _vm->_mouse->_hx, kWorldHeight - _vm->_mouse->_pos2D.y - _vm->_mouse->_hy);
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

CGE2Event &EventManager::getNextEvent() {
	CGE2Event &evt = _eventQueue[_eventQueueHead];
	_eventQueueHead = (_eventQueueHead + 1) % kEventMax;

	return evt;
}

} // End of namespace CGE2
