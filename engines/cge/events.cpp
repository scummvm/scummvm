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
 */

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#include "common/events.h"
#include "cge/events.h"
#include "cge/events.h"
#include "cge/text.h"
#include "cge/cge_main.h"

namespace CGE {

CGEEvent Evt[EVT_MAX];

uint16 EvtHead = 0, EvtTail = 0;

/*----------------- KEYBOARD interface -----------------*/

const uint16 Keyboard::_code[0x60] = {
	0,               Esc,      '1',         '2',       '3',
	'4',             '5',      '6',         '7',       '8',
	'9',             '0',      '-',         '+',       BSp,
	Tab,             'Q',      'W',         'E',       'R',
	'T',             'Y',      'U',         'I',       'O',
	'P',             '[',      ']',         Enter,     0/*Ctrl*/,
	'A',             'S',      'D',         'F',       'G',
	'H',             'J',      'K',         'L',       ';',
	'\'',            '`',      0/*LShift*/, '\\',      'Z',
	'X',             'C',      'V',         'B',       'N',
	'M',             ',',      '.',         '/',       0/*RShift*/,
	'*',             0/*Alt*/, ' ',         0/*Caps*/, F1,
	F2,              F3,       F4,          F5,        F6,
	F7,              F8,       F9,          F10,       0/*NumLock*/,
	0/*ScrollLock*/, Home,     Up,          PgUp,      '-',
	Left,            Ctr,      Right,       '+',       End,
	Down,            PgDn,     Ins,         Del,       0 * 0x54,
	0 * 0x55,        0 * 0x56, F11,         F12,       0 * 0x59,
	0 * 0x5A,        0 * 0x5B, 0 * 0x5C,    0 * 0x5D,  0 * 0x5E,
	0 * 0x5F
};

const uint16 Keyboard::_scummVmCodes[0x60] = {
	0,					Common::KEYCODE_ESCAPE,	Common::KEYCODE_1,	Common::KEYCODE_2,	Common::KEYCODE_3,
	Common::KEYCODE_4,	Common::KEYCODE_5,	Common::KEYCODE_6,	Common::KEYCODE_7,	Common::KEYCODE_8,
	Common::KEYCODE_9,	Common::KEYCODE_0,	Common::KEYCODE_MINUS, Common::KEYCODE_PLUS,	Common::KEYCODE_BACKSPACE,
	Common::KEYCODE_TAB,	Common::KEYCODE_q,	Common::KEYCODE_w,	Common::KEYCODE_e,	Common::KEYCODE_r,
	Common::KEYCODE_t,	Common::KEYCODE_y,	Common::KEYCODE_u,	Common::KEYCODE_i,	Common::KEYCODE_o,
	Common::KEYCODE_p,	Common::KEYCODE_LEFTBRACKET,	Common::KEYCODE_RIGHTBRACKET,	Common::KEYCODE_RETURN,	0/*Ctrl*/,
	Common::KEYCODE_a,	Common::KEYCODE_s,	Common::KEYCODE_d,	Common::KEYCODE_f,	Common::KEYCODE_g,
	Common::KEYCODE_h,	Common::KEYCODE_j,	Common::KEYCODE_k,	Common::KEYCODE_l,	Common::KEYCODE_SEMICOLON,
	Common::KEYCODE_BACKSLASH,	Common::KEYCODE_TILDE,	Common::KEYCODE_LSHIFT,	Common::KEYCODE_BACKSLASH,	Common::KEYCODE_z,
	Common::KEYCODE_x,	Common::KEYCODE_c,	Common::KEYCODE_v,	Common::KEYCODE_b,	Common::KEYCODE_n,
	Common::KEYCODE_m,	Common::KEYCODE_COMMA,	Common::KEYCODE_PERIOD,	Common::KEYCODE_SLASH,	Common::KEYCODE_RSHIFT,
	Common::KEYCODE_KP_MULTIPLY,	0 /*Alt*/,	Common::KEYCODE_SPACE,	Common::KEYCODE_CAPSLOCK,	Common::KEYCODE_F1,
	Common::KEYCODE_F2,	Common::KEYCODE_F3,	Common::KEYCODE_F4,	Common::KEYCODE_F5,	Common::KEYCODE_F6,
	Common::KEYCODE_F7,	Common::KEYCODE_F8,	Common::KEYCODE_F9,	Common::KEYCODE_F10,	Common::KEYCODE_NUMLOCK,
	Common::KEYCODE_SCROLLOCK,	Common::KEYCODE_KP7,	Common::KEYCODE_KP8,	Common::KEYCODE_KP9,	Common::KEYCODE_KP_MINUS,
	Common::KEYCODE_KP4,	Common::KEYCODE_KP5,	Common::KEYCODE_KP6,	Common::KEYCODE_KP_PLUS,	Common::KEYCODE_KP1,
	Common::KEYCODE_KP2,	Common::KEYCODE_KP3,	Common::KEYCODE_KP0,	Common::KEYCODE_KP_PERIOD,	0,
	0,					0,					Common::KEYCODE_F11,	Common::KEYCODE_F12,	0,
	0,					0,					0,						0,						0,
	0
};

Keyboard::Keyboard() {
	_client = NULL;
	Common::set_to(&_key[0], &_key[0x60], false);
	_current = 0;
}

Keyboard::~Keyboard() {
}

Sprite *Keyboard::setClient(Sprite *spr) {
	swap(_client, spr);
	return spr;
}

bool Keyboard::getKey(uint16 keycode, int &cgeCode) {
	if ((keycode == Common::KEYCODE_LCTRL) || (keycode == Common::KEYCODE_RCTRL)) {
		cgeCode = 29;
		return true;
	}
	if (keycode == Common::KEYCODE_KP_ENTER) {
		cgeCode = 28;
		return true;
	}

	// Scan through the ScummVM mapping list
	for (int idx = 0; idx < 0x60; ++idx) {
		if (_scummVmCodes[idx] == keycode) {
			cgeCode = idx;
			return true;
		}
	}

	return false;
}

void Keyboard::newKeyboard(Common::Event &event) {
	int keycode;
	if (!getKey(event.kbd.keycode, keycode))
		return;

	if (event.type == Common::EVENT_KEYUP) {
		// Key release
		_key[keycode] = false;
	} else if (event.type == Common::EVENT_KEYDOWN) {
		// Key press
		_key[keycode] = true;
		_current = Keyboard::_code[keycode];

		if (_client) {
			CGEEvent &evt = Evt[EvtHead];
			EvtHead = (EvtHead + 1) % EVT_MAX;
			evt._x = _current;	// Keycode
			evt._msk = KEYB;	// Event mask
			evt._ptr = _client;	// Sprite pointer
		}
	}
}

/*----------------- MOUSE interface -----------------*/

Mouse::Mouse(CGEEngine *vm) : Sprite(vm, NULL), _busy(NULL), _hold(NULL), _hx(0), _vm(vm) {
	_hold = NULL;
	_hx = 0;
	_hy = 0;
	_exist = true;
	_buttons = 0;
	_busy = NULL;
	_active = false;
	_flags._kill = false;

	const Seq ms[] = {
		{ 0, 0, 0, 0, 1 },
		{ 1, 1, 0, 0, 1 }
	};
	Seq *seq = (Seq *)malloc(2 * sizeof(Seq));
	Common::copy(ms, ms + 2, seq);
	setSeq(seq);

	BMP_PTR *MC = new BMP_PTR[3];
	MC[0] = new Bitmap("MOUSE", true);
	MC[1] = new Bitmap("DUMMY", true);
	MC[2] = NULL;
	setShapeList(MC);

	gotoxy(kScrWidth/2, kScrHeight/2);
	_z = 127;
	step(1);
}


Mouse::~Mouse() {
	off();
}


//void Mouse::setFun()
//{
//}


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

	CGEEvent &evt = Evt[EvtHead];
	EvtHead = (EvtHead + 1) % EVT_MAX;
	evt._x = event.mouse.x;
	evt._y = event.mouse.y;
	evt._ptr = spriteAt(evt._x, evt._y);

	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		evt._msk = ROLL;
		break;
	case Common::EVENT_LBUTTONDOWN:
		evt._msk = L_DN;
		_buttons |= 1;
		break;
	case Common::EVENT_LBUTTONUP:
		evt._msk = L_UP;
		_buttons &= ~1;
		break;
	case Common::EVENT_RBUTTONDOWN:
		evt._msk = R_DN;
		_buttons |= 2;
		break;
	case Common::EVENT_RBUTTONUP:
		evt._msk = R_UP;
		_buttons &= ~2;
		break;
	default:
		break;
	}
}

/*----------------- EventManager interface -----------------*/

EventManager::EventManager() {
	_quitFlag = false;
}

void EventManager::poll() {
	while (g_system->getEventManager()->pollEvent(_event)) {
		switch (_event.type) {
		case Common::EVENT_QUIT:
			// Signal to quit
			_quitFlag = true;
			return;
		case Common::EVENT_KEYDOWN:
		case Common::EVENT_KEYUP:
			// Handle keyboard events
			_keyboard->newKeyboard(_event);
			handleEvents();
			break;
		case Common::EVENT_MOUSEMOVE:
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONDOWN:
		case Common::EVENT_RBUTTONUP:
			// Handle mouse events
			_mouse->newMouse(_event);
			handleEvents();
			break;
		default:
			break;
		}
	}
}

void EventManager::handleEvents() {
	while (EvtTail != EvtHead) {
		CGEEvent e = Evt[EvtTail];
		if (e._msk) {
			if (_mouse->_hold && e._ptr != _mouse->_hold)
				_mouse->_hold->touch(e._msk | ATTN, e._x - _mouse->_hold->_x, e._y - _mouse->_hold->_y);

			// update mouse cursor position
			if (e._msk & ROLL)
				_mouse->gotoxy(e._x, e._y);

			// activate current touched SPRITE
			if (e._ptr) {
				if (e._msk & KEYB)
					e._ptr->touch(e._msk, e._x, e._y);
				else
					e._ptr->touch(e._msk, e._x - e._ptr->_x, e._y - e._ptr->_y);
			} else if (_sys)
					_sys->touch(e._msk, e._x, e._y);

			if (e._msk & L_DN) {
				_mouse->_hold = e._ptr;
				if (_mouse->_hold) {
					_mouse->_hold->_flags._hold = true;

					if (_mouse->_hold->_flags._drag) {
						_mouse->_hx = e._x - _mouse->_hold->_x;
						_mouse->_hy = e._y - _mouse->_hold->_y;
					}
				}
			}

			if (e._msk & L_UP) {
				if (_mouse->_hold) {
					_mouse->_hold->_flags._hold = false;
					_mouse->_hold = NULL;
				}
			}
			///Touched = e.Ptr;

			// discard Text if button released
			if (e._msk & (L_UP | R_UP))
				killText();
		}
		EvtTail = (EvtTail + 1) % EVT_MAX;
	}
	if (_mouse->_hold) {
		if (_mouse->_hold->_flags._drag)
			_mouse->_hold->gotoxy(_mouse->_x - _mouse->_hx, _mouse->_y - _mouse->_hy);
	}
}

void EventManager::clrEvt(Sprite *spr) {
	if (spr) {
		uint16 e;
		for (e = EvtTail; e != EvtHead; e = (e + 1) % EVT_MAX)
			if (Evt[e]._ptr == spr)
				Evt[e]._msk = 0;
	} else
		EvtTail = EvtHead;
}

} // End of namespace CGE
