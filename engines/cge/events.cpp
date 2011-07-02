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
	Swap(_client, spr);
	return spr;
}

bool Keyboard::getKey(uint16 keycode, int &cgeCode) {
	if ((keycode == Common::KEYCODE_LCTRL) || (keycode == Common::KEYCODE_RCTRL)) {
		cgeCode = 29;
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

void Keyboard::NewKeyboard(Common::Event &event) {
	int keycode;
	if (!getKey(event.kbd.keycode, keycode))
		return;

	if (event.type == Common::EVENT_KEYUP) {
		// Key release
		_key[event.kbd.keycode] = false;
	} else if (event.type == Common::EVENT_KEYDOWN) {
		// Key press
		_key[event.kbd.keycode] = true;
		_current = Keyboard::_code[event.kbd.keycode];

		if (_client) {
			CGEEvent &evt = Evt[EvtHead++];
			evt._x = _current;	// Keycode
			evt._msk = KEYB;	// Event mask
			evt._ptr = _client;	// Sprite pointer
		}
	}
}

/*----------------- MOUSE interface -----------------*/

CGEEvent Evt[EVT_MAX];

uint16 EvtHead = 0, EvtTail = 0;

MOUSE_FUN *MOUSE::OldMouseFun  = NULL;
uint16     MOUSE::OldMouseMask = 0;


MOUSE::MOUSE(CGEEngine *vm, Bitmap **shpl) : Sprite(vm, shpl), Busy(NULL), Hold(NULL), hx(0), _vm(vm) {
	static Seq ms[] = {
		{ 0, 0, 0, 0, 1 },
		{ 1, 1, 0, 0, 1 }
	};

	setSeq(ms);

	/* TODO Mouse handling
	// Mouse reset
	_AX = 0x0000; // soft & hard reset (0x0021 soft reset does not work)
	__int__(0x33);
	Exist = (_AX != 0);
	Buttons = _BX;
*/
	gotoxy(SCR_WID/2, SCR_HIG/2);
	_z = 127;
	step(1);

	Exist = true;
	warning("STUB: MOUSE::MOUSE");
}


MOUSE::~MOUSE(void) {
	Off();
}


//void MOUSE::SetFun (void)
//{
//}


void MOUSE::On(void) {
	/*
	  if (SeqPtr && Exist)
	    {
	      _CX = X + X;  // horizontal position
	      _DX = Y;      // vertical position
	      _AX = 0x0004; // Set Mouse Position
	      __int__(0x33);
	      // set new mouse fun
	      _ES = FP_SEG(NewMouseFun);
	      _DX = FP_OFF(NewMouseFun);
	      _CX = 0x001F; // 11111b = all events
	      _AX = 0x0014; // Swap User-Interrupt Vector
	      __int__(0x33);
	      // save old mouse fun
	      OldMouseMask = _CX;
	      OldMouseFun = (MOUSE_FUN *) MK_FP(_ES, _DX);

	      // set X bounds
	      _DX = (SCR_WID - W) * 2;      // right limit
	      _CX = 0;              // left limit
	      _AX = 0x0007;         // note: each pixel = 2
	      __int__(0x33);

	      // set Y bounds
	      _DX = SCR_HIG - H;        // bottom limit
	      _CX = 0;              // top limit
	      _AX = 0x0008;
	      __int__(0x33);

	      Step(0);
	      if (Busy) Busy->Step(0);
	    }
	*/
	warning("STUB: MOUSE::On");
}


void MOUSE::Off(void) {
/*
	if (SeqPtr == 0)
	{
	  if (Exist)
	{
	  // bring back old mouse fun
	  _ES = FP_SEG(OldMouseFun);
	  _DX = FP_OFF(OldMouseFun);
	  _CX = OldMouseMask;
	  _AX = 0x0014;     // Swap User-Interrupt Vector
	  __int__(0x33);
	}
	  Step(1);
	  if (Busy) Busy->Step(1);
	}
	*/
	warning("STUB: MOUSE::Off");
}


void MOUSE::ClrEvt(Sprite *spr) {
	if (spr) {
		uint16 e;
		for (e = EvtTail; e != EvtHead; e = (e + 1) % EVT_MAX)
			if (Evt[e]._ptr == spr)
				Evt[e]._msk = 0;
	} else
		EvtTail = EvtHead;
}


void MOUSE::Tick(void) {
	step();
	while (EvtTail != EvtHead) {
		CGEEvent e = Evt[EvtTail];
		if (e._msk) {
			if (Hold && e._ptr != Hold)
				Hold->touch(e._msk | ATTN, e._x - Hold->_x, e._y - Hold->_y);

			// update mouse cursor position
			if (e._msk & ROLL)
				gotoxy(e._x, e._y);

			// activate current touched SPRITE
			if (e._ptr) {
				if (e._msk & KEYB)
					e._ptr->touch(e._msk, e._x, e._y);
				else
					e._ptr->touch(e._msk, e._x - e._ptr->_x, e._y - e._ptr->_y);
			} else if (Sys)
					Sys->touch(e._msk, e._x, e._y);

			if (e._msk & L_DN) {
				Hold = e._ptr;
				if (Hold) {
					Hold->_flags._hold = true;
					hx = e._x - Hold->_x;
					hy = e._y - Hold->_y;
				}
			}

			if (e._msk & L_UP) {
				if (Hold) {
					Hold->_flags._hold = false;
					Hold = NULL;
				}
			}
			///Touched = e.Ptr;

			// discard Text if button released
			if (e._msk & (L_UP | R_UP))
				KillText();
		}
		EvtTail = (EvtTail + 1) % EVT_MAX;
	}
	if (Hold)
		Hold->gotoxy(_x - hx, _y - hy);
}

/*----------------- EventManager interface -----------------*/

EventManager::EventManager() {
	_quitFlag = false;
}

void EventManager::poll() {
	while (g_system->getEventManager()->pollEvent(_event)) {
		switch (_event.type) {
		case Common::EVENT_QUIT:
			_quitFlag = true;
			return;
		case Common::EVENT_KEYDOWN:
		case Common::EVENT_KEYUP:
			_keyboard->NewKeyboard(_event);
			break;
		}
	}
}

} // End of namespace CGE
