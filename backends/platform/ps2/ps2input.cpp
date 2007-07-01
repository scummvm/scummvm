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

#include <kernel.h>
#include <malloc.h>
#include <assert.h>
#include <libmouse.h>
#include "backends/platform/ps2/rpckbd.h"
#include "backends/platform/ps2/ps2input.h"
#include "backends/platform/ps2/ps2pad.h"
#include "backends/platform/ps2/systemps2.h"
#include "common/events.h"
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
	if (key >= Common::KEYCODE_F1 && key <= Common::KEYCODE_F9) {
		return key - Common::KEYCODE_F1 + Common::ASCII_F1;
	} else if (key >= Common::KEYCODE_KP0 && key <= Common::KEYCODE_KP9) {
		return key - Common::KEYCODE_KP0 + '0';
	} else if (key >= Common::KEYCODE_UP && key <= Common::KEYCODE_PAGEDOWN) {
		return key;
	} else if (key >= 'a' && key <= 'z' && mod & Common::KBD_SHIFT) {
		return key & ~0x20;
	} else if (key >= Common::KEYCODE_NUMLOCK && key <= Common::KEYCODE_EURO) {
		return 0;
	}
	return key;
}

bool Ps2Input::pollEvent(Common::Event *event) {
	bool checkPadMouse, checkPadKbd;
	checkPadMouse = checkPadKbd = _pad->padAlive();

	if (_mouseLoaded && (PS2MouseEnum() > 0)) { // usb mouse connected
		mouse_data mData;
		PS2MouseRead(&mData);
		if ((_posX != mData.x) || (_posY != mData.y)) {
			event->mouse.x = _posX = mData.x;
			event->mouse.y = _posY = mData.y;
			event->type = Common::EVENT_MOUSEMOVE;
            return true;
		}
		if (mData.buttons != _mButtons) {
			uint16 change = _mButtons ^ mData.buttons;
			_mButtons = mData.buttons;
			if (change & (PS2MOUSE_BTN1 | PS2MOUSE_BTN2)) {
				if (change & PS2MOUSE_BTN1)
					event->type = (_mButtons & PS2MOUSE_BTN1) ? Common::EVENT_LBUTTONDOWN : Common::EVENT_LBUTTONUP;
				else
					event->type = (_mButtons & PS2MOUSE_BTN2) ? Common::EVENT_RBUTTONDOWN : Common::EVENT_RBUTTONUP;
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
				if ((_usbToSdlk[key.key] == Common::KEYCODE_LSHIFT) || (_usbToSdlk[key.key] == Common::KEYCODE_RSHIFT)) {
					if (key.state & 1)
						_keyFlags |= Common::KBD_SHIFT;
					else
						_keyFlags &= ~Common::KBD_SHIFT;
				} else if ((_usbToSdlk[key.key] == Common::KEYCODE_LCTRL) || (_usbToSdlk[key.key] == Common::KEYCODE_RCTRL)) {
					if (key.state & 1)
						_keyFlags |= Common::KBD_CTRL;
					else
						_keyFlags &= ~Common::KBD_CTRL;
				} else if ((_usbToSdlk[key.key] == Common::KEYCODE_LALT) || (_usbToSdlk[key.key] == Common::KEYCODE_RALT)) {
					if (key.state & 1)
						_keyFlags |= Common::KBD_ALT;
					else
						_keyFlags &= ~Common::KBD_ALT;
				}
				if (key.state & 1) // down
					event->type = Common::EVENT_KEYDOWN;
				else
					event->type = Common::EVENT_KEYUP;
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
					event->type = (buttons & PAD_CROSS) ?  Common::EVENT_LBUTTONDOWN : Common::EVENT_LBUTTONUP;
				else
					event->type = (buttons & PAD_CIRCLE) ? Common::EVENT_RBUTTONDOWN : Common::EVENT_RBUTTONUP;
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
					event->type = Common::EVENT_MOUSEMOVE;
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

bool Ps2Input::getKeyEvent(Common::Event *event, uint16 buttonCode, bool down) {
	// for simulating key presses with the pad
	if (buttonCode) {
		uint8 entry = 0;
		while (!(buttonCode & 1)) {
			entry++;
			buttonCode >>= 1;
		}
		if (_padCodes[entry]) {
			event->type = (down) ? Common::EVENT_KEYDOWN : Common::EVENT_KEYUP;
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
	/* 04 */	Common::KEYCODE_a,
	/* 05 */	Common::KEYCODE_b,
	/* 06 */	Common::KEYCODE_c,
	/* 07 */	Common::KEYCODE_d,
	/* 08 */	Common::KEYCODE_e,
	/* 09 */	Common::KEYCODE_f,
	/* 0A */	Common::KEYCODE_g,
	/* 0B */	Common::KEYCODE_h,
	/* 0C */	Common::KEYCODE_i,
	/* 0D */	Common::KEYCODE_j,
	/* 0E */	Common::KEYCODE_k,
	/* 0F */	Common::KEYCODE_l,
	/* 10 */	Common::KEYCODE_m,
	/* 11 */	Common::KEYCODE_n,
	/* 12 */	Common::KEYCODE_o,
	/* 13 */	Common::KEYCODE_p,
	/* 14 */	Common::KEYCODE_q,
	/* 15 */	Common::KEYCODE_r,
	/* 16 */	Common::KEYCODE_s,
	/* 17 */	Common::KEYCODE_t,
	/* 18 */	Common::KEYCODE_u,
	/* 19 */	Common::KEYCODE_v,
	/* 1A */	Common::KEYCODE_w,
	/* 1B */	Common::KEYCODE_x,
	/* 1C */	Common::KEYCODE_y,
	/* 1D */	Common::KEYCODE_z,
	/* 1E */	Common::KEYCODE_1,
	/* 1F */	Common::KEYCODE_2,
	/* 20 */	Common::KEYCODE_3,
	/* 21 */	Common::KEYCODE_4,
	/* 22 */	Common::KEYCODE_5,
	/* 23 */	Common::KEYCODE_6,
	/* 24 */	Common::KEYCODE_7,
	/* 25 */	Common::KEYCODE_8,
	/* 26 */	Common::KEYCODE_9,
	/* 27 */	Common::KEYCODE_0,
	/* 28 */	Common::KEYCODE_RETURN,
	/* 29 */	Common::KEYCODE_ESCAPE,
	/* 2A */	Common::KEYCODE_BACKSPACE,
	/* 2B */	Common::KEYCODE_TAB,
	/* 2C */	Common::KEYCODE_SPACE,
	/* 2D */	Common::KEYCODE_MINUS,
	/* 2E */	Common::KEYCODE_EQUALS,
	/* 2F */	Common::KEYCODE_LEFTBRACKET,
	/* 30 */	Common::KEYCODE_RIGHTBRACKET,
	/* 31 */	Common::KEYCODE_BACKSLASH,
	/* 32 */	Common::KEYCODE_HASH,
	/* 33 */	Common::KEYCODE_SEMICOLON,
	/* 34 */	Common::KEYCODE_QUOTE,
	/* 35 */	Common::KEYCODE_BACKQUOTE,
	/* 36 */	Common::KEYCODE_COMMA,
	/* 37 */	Common::KEYCODE_PERIOD,
	/* 38 */	Common::KEYCODE_SLASH,
	/* 39 */	Common::KEYCODE_CAPSLOCK,
	/* 3A */	Common::KEYCODE_F1,
	/* 3B */	Common::KEYCODE_F2,
	/* 3C */	Common::KEYCODE_F3,
	/* 3D */	Common::KEYCODE_F4,
	/* 3E */	Common::KEYCODE_F5,
	/* 3F */	Common::KEYCODE_F6,
	/* 40 */	Common::KEYCODE_F7,
	/* 41 */	Common::KEYCODE_F8,
	/* 42 */	Common::KEYCODE_F9,
	/* 43 */	Common::KEYCODE_F10,
	/* 44 */	Common::KEYCODE_F11,
	/* 45 */	Common::KEYCODE_F12,
	/* 46 */	Common::KEYCODE_PRINT,
	/* 47 */	Common::KEYCODE_SCROLLOCK,
	/* 48 */	Common::KEYCODE_PAUSE,
	/* 49 */	Common::KEYCODE_INSERT,
	/* 4A */	Common::KEYCODE_HOME,
	/* 4B */	Common::KEYCODE_PAGEUP,
	/* 4C */	Common::KEYCODE_DELETE,
	/* 4D */	Common::KEYCODE_END,
	/* 4E */	Common::KEYCODE_PAGEDOWN,
	/* 4F */	Common::KEYCODE_RIGHT,
	/* 50 */	Common::KEYCODE_LEFT,
	/* 51 */	Common::KEYCODE_DOWN,
	/* 52 */	Common::KEYCODE_UP,
	/* 53 */	Common::KEYCODE_NUMLOCK,
	/* 54 */	Common::KEYCODE_KP_DIVIDE,
	/* 55 */	Common::KEYCODE_KP_MULTIPLY,
	/* 56 */	Common::KEYCODE_KP_MINUS,
	/* 57 */	Common::KEYCODE_KP_PLUS,
	/* 58 */	Common::KEYCODE_KP_ENTER,
	/* 59 */	Common::KEYCODE_KP1,
	/* 5A */	Common::KEYCODE_KP2,
	/* 5B */	Common::KEYCODE_KP3,
	/* 5C */	Common::KEYCODE_KP4,
	/* 5D */	Common::KEYCODE_KP5,
	/* 5E */	Common::KEYCODE_KP6,
	/* 5F */	Common::KEYCODE_KP7,
	/* 60 */	Common::KEYCODE_KP8,
	/* 61 */	Common::KEYCODE_KP9,
	/* 62 */	Common::KEYCODE_KP0,
	/* 63 */	Common::KEYCODE_KP_PERIOD,
	/* 64 */	0,
	/* 65 */	0,
	/* 66 */	0,
	/* 67 */	Common::KEYCODE_KP_EQUALS,
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
	/* E0 */    Common::KEYCODE_LCTRL,
	/* E1 */	Common::KEYCODE_LSHIFT,
	/* E2 */	Common::KEYCODE_LALT,
	/* E3 */	0,
	/* E4 */    Common::KEYCODE_RCTRL,
	/* E5 */	Common::KEYCODE_RSHIFT,
	/* E6 */	Common::KEYCODE_RALT,
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
	Common::KEYCODE_1,		 // Select
	0,			 // L3
	0,			 // R3
	Common::KEYCODE_F5,	 // Start
	0,			 //	Up
	0,			 //	Right
	0,			 //	Down
	0,			 //	Left
	Common::KEYCODE_KP0,	 //	L2
	0,			 //	R2
	Common::KEYCODE_n,		 //	L1
	Common::KEYCODE_y,		 //	R1
	Common::KEYCODE_ESCAPE, // Triangle
	0,			 //	Circle  => Right mouse button
	0,			 //	Cross	=> Left mouse button
	Common::KEYCODE_RETURN	 //	Square
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


