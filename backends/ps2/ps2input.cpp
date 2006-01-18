/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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
 * $Header$
 *
 */

#include <kernel.h>
#include <malloc.h>
#include <assert.h>
#include <libmouse.h>
#include "backends/ps2/rpckbd.h"
#include "backends/ps2/ps2input.h"
#include "backends/ps2/ps2pad.h"
#include "backends/ps2/systemps2.h"
#include "backends/ps2/sdlkeys.h"
#include "common/system.h"

Ps2Input::Ps2Input(OSystem_PS2 *system, bool mouseLoaded, bool kbdLoaded) {
	_system = system;
	_mouseLoaded = mouseLoaded;
	_kbdLoaded = kbdLoaded;
	_pad = new Ps2Pad(system);
	_lastPadCheck = 0;
	_posX = _posY = _mButtons = _padLastButtons = 0;
	_padAccel = 0;
	_minx = _miny = 0;
	_maxy = 239;
	_maxx = 319;
	_keyFlags = 0;
	if (_mouseLoaded) {
		if (PS2MouseInit() >= 0) {
			PS2MouseSetReadMode(PS2MOUSE_READMODE_ABS);
			printf("PS2Mouse initialized\n");
		} else { // shouldn't happen if the drivers were correctly loaded
			printf("unable to initialize PS2Mouse!\n");
			_mouseLoaded = false;
		}
	}
	if (_kbdLoaded) {
		if (PS2KbdInit() >= 0) {
			PS2KbdSetReadmode(PS2KBD_READMODE_RAW);
			printf("PS2Kbd initialized\n");
		} else {
			printf("unable to initialize PS2Kbd!\n");
			_kbdLoaded = false;
		}
	}
}

Ps2Input::~Ps2Input(void) {
}

void Ps2Input::newRange(uint16 minx, uint16 miny, uint16 maxx, uint16 maxy) {
	_minx = minx;
	_miny = miny;
	_maxx = maxx;
	_maxy = maxy;
	if (_mouseLoaded)
		PS2MouseSetBoundary(minx, maxx, miny, maxy);
	warpTo(_posX, _posY);
}

void Ps2Input::warpTo(uint16 x, uint16 y) {
	if ((x >= _minx) && (x <= _maxx) && (y >= _miny) && (y <= _maxy)) {
		_posX = x;
		_posY = y;
	} else {
		_posX = (x < _minx) ? (_minx) : ((x > _maxx) ? (_maxx) : (x));
		_posY = (y < _miny) ? (_miny) : ((y > _maxy) ? (_maxy) : (y));
	}
	if (_mouseLoaded)
		PS2MouseSetPosition(_posX, _posY);
}

#define JOY_THRESHOLD 30
#define PAD_CHECK_TIME 20

int Ps2Input::mapKey(int key, int mod) { // copied from sdl backend
	if (key >= SDLK_F1 && key <= SDLK_F9) {
		return key - SDLK_F1 + 315;
	} else if (key >= SDLK_KP0 && key <= SDLK_KP9) {
		return key - SDLK_KP0 + '0';
	} else if (key >= SDLK_UP && key <= SDLK_PAGEDOWN) {
		return key;
	} else if (key >= 'a' && key <= 'z' && mod & OSystem::KBD_SHIFT) {
		return key & ~0x20;
	} else if (key >= SDLK_NUMLOCK && key <= SDLK_EURO) {
		return 0;
	}
	return key;
}

bool Ps2Input::pollEvent(OSystem::Event *event) {
	bool checkPadMouse, checkPadKbd;
	checkPadMouse = checkPadKbd = _pad->padAlive();

	if (_mouseLoaded && (PS2MouseEnum() > 0)) { // usb mouse connected
		mouse_data mData;
		PS2MouseRead(&mData);
		if ((_posX != mData.x) || (_posY != mData.y)) {
			event->mouse.x = _posX = mData.x;
			event->mouse.y = _posY = mData.y;
			event->type = OSystem::EVENT_MOUSEMOVE;
            return true;
		}
		if (mData.buttons != _mButtons) {
			uint16 change = _mButtons ^ mData.buttons;
			_mButtons = mData.buttons;
			if (change & (PS2MOUSE_BTN1 | PS2MOUSE_BTN2)) {
				if (change & PS2MOUSE_BTN1)
					event->type = (_mButtons & PS2MOUSE_BTN1) ? OSystem::EVENT_LBUTTONDOWN : OSystem::EVENT_LBUTTONUP;
				else
					event->type = (_mButtons & PS2MOUSE_BTN2) ? OSystem::EVENT_RBUTTONDOWN : OSystem::EVENT_RBUTTONUP;
				event->mouse.x = _posX;
				event->mouse.y = _posY;
				return true;
			}
		}
		checkPadMouse = false;
	}
	if (_kbdLoaded) { // there's no way to tell if there's actually a keyboard connected
		PS2KbdRawKey key;
		if (PS2KbdReadRaw(&key) == 1) {
			if (_usbToSdlk[key.key]) {
				if ((_usbToSdlk[key.key] == SDLK_LSHIFT) || (_usbToSdlk[key.key] == SDLK_RSHIFT)) {
					if (key.state & 1)
						_keyFlags |= OSystem::KBD_SHIFT;
					else
						_keyFlags &= ~OSystem::KBD_SHIFT;
				} else if ((_usbToSdlk[key.key] == SDLK_LCTRL) || (_usbToSdlk[key.key] == SDLK_RCTRL)) {
					if (key.state & 1)
						_keyFlags |= OSystem::KBD_CTRL;
					else
						_keyFlags &= ~OSystem::KBD_CTRL;
				} else if ((_usbToSdlk[key.key] == SDLK_LALT) || (_usbToSdlk[key.key] == SDLK_RALT)) {
					if (key.state & 1)
						_keyFlags |= OSystem::KBD_ALT;
					else
						_keyFlags &= ~OSystem::KBD_ALT;
				}
				if (key.state & 1) // down
					event->type = OSystem::EVENT_KEYDOWN;
				else
					event->type = OSystem::EVENT_KEYUP;
				event->kbd.flags = 0;
				event->kbd.keycode = _usbToSdlk[key.key];
				event->kbd.ascii = mapKey(_usbToSdlk[key.key], _keyFlags);
				return true;
			} else
				printf("unknown keycode %02X - %02X\n", key.state, key.key);
		}
	}
	if (checkPadMouse || checkPadKbd) {
		// no usb mouse, simulate it using the pad
        uint16 buttons;
		int16 joyh, joyv;
		_pad->readPad(&buttons, &joyh, &joyv);
		uint16 btnChange = buttons ^ _padLastButtons;

		if (checkPadMouse) {
			if (btnChange & (PAD_CROSS | PAD_CIRCLE)) {
				if (btnChange & PAD_CROSS)
					event->type = (buttons & PAD_CROSS) ?  OSystem::EVENT_LBUTTONDOWN : OSystem::EVENT_LBUTTONUP;
				else
					event->type = (buttons & PAD_CIRCLE) ? OSystem::EVENT_RBUTTONDOWN : OSystem::EVENT_RBUTTONUP;
				event->mouse.x = _posX;
				event->mouse.y = _posY;
				_padLastButtons = buttons;
				return true;
			}
			uint32 time = _system->getMillis();
			if (time - _lastPadCheck > PAD_CHECK_TIME) {
				_lastPadCheck = time;
				int16 newX = _posX;
				int16 newY = _posY;
				if ((ABS(joyh) > JOY_THRESHOLD) || (ABS(joyv) > JOY_THRESHOLD)) {
					newX += joyh / 20;
					newY += joyv / 20;
				} else if (buttons & PAD_DIR_MASK) {
					if (_padLastButtons & PAD_DIR_MASK) {
						if (_padAccel < 16)
							_padAccel++;
					} else
						_padAccel = 0;
					_padLastButtons = buttons;
					if (buttons & PAD_LEFT)
						newX -= _padAccel >> 2;
					if (buttons & PAD_RIGHT)
						newX += _padAccel >> 2;
					if (buttons & PAD_UP)
						newY -= _padAccel >> 2;
					if (buttons & PAD_DOWN)
						newY += _padAccel >> 2;
				}
				newX = ((newX < (int16)_minx) ? (_minx) : ((newX > (int16)_maxx) ? (_maxx) : ((int16)newX)));
				newY = ((newY < (int16)_miny) ? (_miny) : ((newY > (int16)_maxy) ? (_maxy) : ((int16)newY)));
				if ((_posX != newX) || (_posY != newY)) {
					event->type = OSystem::EVENT_MOUSEMOVE;
					event->mouse.x = _posX = newX;
					event->mouse.y = _posY = newY;
					return true;
				}
			}
		}
		if (checkPadKbd) {
			if (getKeyEvent(event, btnChange, (btnChange & buttons) != 0)) {
				_padLastButtons = buttons;
				return true;
			}
		}
	}
	return false;
}

bool Ps2Input::getKeyEvent(OSystem::Event *event, uint16 buttonCode, bool down) {
	// for simulating key presses with the pad
	if (buttonCode) {
		uint8 entry = 0;
		while (!(buttonCode & 1)) {
			entry++;
			buttonCode >>= 1;
		}
		if (_padCodes[entry]) {
			event->type = (down) ? OSystem::EVENT_KEYDOWN : OSystem::EVENT_KEYUP;
			event->kbd.keycode = _padCodes[entry];
			event->kbd.flags = _padFlags[entry];
			event->kbd.ascii = mapKey(_padCodes[entry], _padFlags[entry]);
			return true;
		}
	}
	return false;
}

const int Ps2Input::_usbToSdlk[0x100] = {
	/* 00 */	0,
	/* 01 */	0,
	/* 02 */	0,
	/* 03 */	0,
	/* 04 */	SDLK_a,
	/* 05 */	SDLK_b,
	/* 06 */	SDLK_c,
	/* 07 */	SDLK_d,
	/* 08 */	SDLK_e,
	/* 09 */	SDLK_f,
	/* 0A */	SDLK_g,
	/* 0B */	SDLK_h,
	/* 0C */	SDLK_i,
	/* 0D */	SDLK_j,
	/* 0E */	SDLK_k,
	/* 0F */	SDLK_l,
	/* 10 */	SDLK_m,
	/* 11 */	SDLK_n,
	/* 12 */	SDLK_o,
	/* 13 */	SDLK_p,
	/* 14 */	SDLK_q,
	/* 15 */	SDLK_r,
	/* 16 */	SDLK_s,
	/* 17 */	SDLK_t,
	/* 18 */	SDLK_u,
	/* 19 */	SDLK_v,
	/* 1A */	SDLK_w,
	/* 1B */	SDLK_x,
	/* 1C */	SDLK_y,
	/* 1D */	SDLK_z,
	/* 1E */	SDLK_1,
	/* 1F */	SDLK_2,
	/* 20 */	SDLK_3,
	/* 21 */	SDLK_4,
	/* 22 */	SDLK_5,
	/* 23 */	SDLK_6,
	/* 24 */	SDLK_7,
	/* 25 */	SDLK_8,
	/* 26 */	SDLK_9,
	/* 27 */	SDLK_0,
	/* 28 */	SDLK_RETURN,
	/* 29 */	SDLK_ESCAPE,
	/* 2A */	SDLK_BACKSPACE,
	/* 2B */	SDLK_TAB,
	/* 2C */	SDLK_SPACE,
	/* 2D */	SDLK_MINUS,
	/* 2E */	SDLK_EQUALS,
	/* 2F */	SDLK_LEFTBRACKET,
	/* 30 */	SDLK_RIGHTBRACKET,
	/* 31 */	SDLK_BACKSLASH,
	/* 32 */	SDLK_HASH,
	/* 33 */	SDLK_SEMICOLON,
	/* 34 */	SDLK_QUOTE,
	/* 35 */	SDLK_BACKQUOTE,
	/* 36 */	SDLK_COMMA,
	/* 37 */	SDLK_PERIOD,
	/* 38 */	SDLK_SLASH,
	/* 39 */	SDLK_CAPSLOCK,
	/* 3A */	SDLK_F1,
	/* 3B */	SDLK_F2,
	/* 3C */	SDLK_F3,
	/* 3D */	SDLK_F4,
	/* 3E */	SDLK_F5,
	/* 3F */	SDLK_F6,
	/* 40 */	SDLK_F7,
	/* 41 */	SDLK_F8,
	/* 42 */	SDLK_F9,
	/* 43 */	SDLK_F10,
	/* 44 */	SDLK_F11,
	/* 45 */	SDLK_F12,
	/* 46 */	SDLK_PRINT,
	/* 47 */	SDLK_SCROLLOCK,
	/* 48 */	SDLK_PAUSE,
	/* 49 */	SDLK_INSERT,
	/* 4A */	SDLK_HOME,
	/* 4B */	SDLK_PAGEUP,
	/* 4C */	SDLK_DELETE,
	/* 4D */	SDLK_END,
	/* 4E */	SDLK_PAGEDOWN,
	/* 4F */	SDLK_RIGHT,
	/* 50 */	SDLK_LEFT,
	/* 51 */	SDLK_DOWN,
	/* 52 */	SDLK_UP,
	/* 53 */	SDLK_NUMLOCK,
	/* 54 */	SDLK_KP_DIVIDE,
	/* 55 */	SDLK_KP_MULTIPLY,
	/* 56 */	SDLK_KP_MINUS,
	/* 57 */	SDLK_KP_PLUS,
	/* 58 */	SDLK_KP_ENTER,
	/* 59 */	SDLK_KP1,
	/* 5A */	SDLK_KP2,
	/* 5B */	SDLK_KP3,
	/* 5C */	SDLK_KP4,
	/* 5D */	SDLK_KP5,
	/* 5E */	SDLK_KP6,
	/* 5F */	SDLK_KP7,
	/* 60 */	SDLK_KP8,
	/* 61 */	SDLK_KP9,
	/* 62 */	SDLK_KP0,
	/* 63 */	SDLK_KP_PERIOD,
	/* 64 */	0,
	/* 65 */	0,
	/* 66 */	0,
	/* 67 */	SDLK_KP_EQUALS,
	/* 68 */	0,
	/* 69 */	0,
	/* 6A */	0,
	/* 6B */	0,
	/* 6C */	0,
	/* 6D */	0,
	/* 6E */	0,
	/* 6F */	0,
	/* 70 */	0,
	/* 71 */	0,
	/* 72 */	0,
	/* 73 */	0,
	/* 74 */	0,
	/* 75 */	0,
	/* 76 */	0,
	/* 77 */	0,
	/* 78 */	0,
	/* 79 */	0,
	/* 7A */	0,
	/* 7B */	0,
	/* 7C */	0,
	/* 7D */	0,
	/* 7E */	0,
	/* 7F */	0,
	/* 80 */	0,
	/* 81 */	0,
	/* 82 */	0,
	/* 83 */	0,
	/* 84 */	0,
	/* 85 */	0,
	/* 86 */	0,
	/* 87 */	0,
	/* 88 */	0,
	/* 89 */	0,
	/* 8A */	0,
	/* 8B */	0,
	/* 8C */	0,
	/* 8D */	0,
	/* 8E */	0,
	/* 8F */	0,
	/* 90 */	0,
	/* 91 */	0,
	/* 92 */	0,
	/* 93 */	0,
	/* 94 */	0,
	/* 95 */	0,
	/* 96 */	0,
	/* 97 */	0,
	/* 98 */	0,
	/* 99 */	0,
	/* 9A */	0,
	/* 9B */	0,
	/* 9C */	0,
	/* 9D */	0,
	/* 9E */	0,
	/* 9F */	0,
	/* A0 */	0,
	/* A1 */	0,
	/* A2 */	0,
	/* A3 */	0,
	/* A4 */	0,
	/* A5 */	0,
	/* A6 */	0,
	/* A7 */	0,
	/* A8 */	0,
	/* A9 */	0,
	/* AA */	0,
	/* AB */	0,
	/* AC */	0,
	/* AD */	0,
	/* AE */	0,
	/* AF */	0,
	/* B0 */	0,
	/* B1 */	0,
	/* B2 */	0,
	/* B3 */	0,
	/* B4 */	0,
	/* B5 */	0,
	/* B6 */	0,
	/* B7 */	0,
	/* B8 */	0,
	/* B9 */	0,
	/* BA */	0,
	/* BB */	0,
	/* BC */	0,
	/* BD */	0,
	/* BE */	0,
	/* BF */	0,
	/* C0 */	0,
	/* C1 */	0,
	/* C2 */	0,
	/* C3 */	0,
	/* C4 */	0,
	/* C5 */	0,
	/* C6 */	0,
	/* C7 */	0,
	/* C8 */	0,
	/* C9 */	0,
	/* CA */	0,
	/* CB */	0,
	/* CC */	0,
	/* CD */	0,
	/* CE */	0,
	/* CF */	0,
	/* D0 */	0,
	/* D1 */	0,
	/* D2 */	0,
	/* D3 */	0,
	/* D4 */	0,
	/* D5 */	0,
	/* D6 */	0,
	/* D7 */	0,
	/* D8 */	0,
	/* D9 */	0,
	/* DA */	0,
	/* DB */	0,
	/* DC */	0,
	/* DD */	0,
	/* DE */	0,
	/* DF */	0,
	/* E0 */    SDLK_LCTRL,
	/* E1 */	SDLK_LSHIFT,
	/* E2 */	SDLK_LALT,
	/* E3 */	0,
	/* E4 */    SDLK_RCTRL,
	/* E5 */	SDLK_RSHIFT,
	/* E6 */	SDLK_RALT,
	/* E7 */	0,
	/* E8 */	0,
	/* E9 */	0,
	/* EA */	0,
	/* EB */	0,
	/* EC */	0,
	/* ED */	0,
	/* EE */	0,
	/* EF */	0,
	/* F0 */	0,
	/* F1 */	0,
	/* F2 */	0,
	/* F3 */	0,
	/* F4 */	0,
	/* F5 */	0,
	/* F6 */	0,
	/* F7 */	0,
	/* F8 */	0,
	/* F9 */	0,
	/* FA */	0,
	/* FB */	0,
	/* FC */	0,
	/* FD */	0,
	/* FE */	0,
	/* FF */	0
};

const int Ps2Input::_padCodes[16] = {
	SDLK_1,		 // Select
	0,			 // L3
	0,			 // R3
	SDLK_F5,	 // Start
	0,			 //	Up
	0,			 //	Right
	0,			 //	Down
	0,			 //	Left
	SDLK_KP0,	 //	L2
	0,			 //	R2
	SDLK_n,		 //	L1
	SDLK_y,		 //	R1
	SDLK_ESCAPE, // Triangle
	0,			 //	Circle  => Right mouse button
	0,			 //	Cross	=> Left mouse button
	SDLK_RETURN	 //	Square
};

const int Ps2Input::_padFlags[16] = {
	0,			 // Select
	0,			 // L3
	0,			 // R3
	0,			 // Start
	0,			 //	Up
	0,			 //	Right
	0,			 //	Down
	0,			 //	Left
	0,			 //	L2
	0,			 //	R2
	0,			 //	L1
	0,			 //	R1
	0,			 // Triangle
	0,			 //	Circle
	0,			 //	Cross
	0			 //	Square
};


