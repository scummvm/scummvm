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

#ifndef CGE_EVENTS_H
#define CGE_EVENTS_H

#include "common/events.h"
#include "cge/game.h"
#include "cge/talk.h"
#include "cge/vga13h.h"

namespace CGE {

/*----------------- KEYBOARD interface -----------------*/

#define kKeyCtrl    29
#define kKeyAlt     56
#define kEventMax   256

enum EventMask {
	kMouseRoll      = 1 << 0,
	kMouseLeftDown  = 1 << 1,
	kMouseLeftUp    = 1 << 2,
	kMouseRightDown = 1 << 3,
	kMouseRightUp   = 1 << 4,
	kEventAttn      = 1 << 5,
	kEventKeyb      = 1 << 7
};

enum Keys {
    NoKey       = 0, CtrlA, CtrlB, CtrlC, CtrlD, CtrlE, CtrlF, CtrlG, CtrlH,
    CtrlI, CtrlJ, CtrlK, CtrlL, CtrlM, CtrlN, CtrlO, CtrlP,
    CtrlQ, CtrlR, CtrlS, CtrlT, CtrlU, CtrlV, CtrlW, CtrlX,
    CtrlY, CtrlZ,
    BSp         = 8, Tab,
    Enter       = 13,
    Eof         = 26, Esc,
    AltQ        = 256 + 16, AltW, AltE, AltR, AltT, AltY, AltU, AltI, AltO, AltP,
    AltA        = 256 + 30, AltS, AltD, AltF, AltG, AltH, AltJ, AltK, AltL,
    AltZ        = 256 + 44, AltX, AltC, AltV, AltB, AltN, AltM,
    F11         = 256 + 87, F12,
    F1          = 256 + 59, F2, F3, F4, F5, F6, F7, F8, F9, F10,
    ShiftTab    = 256 + 15,
    ShiftF1     = 256 + 84, ShiftF2, ShiftF3, ShiftF4, ShiftF5,
    ShiftF6, ShiftF7, ShiftF8, ShiftF9, ShiftF10,
    CtrlF1      = 256 + 94, CtrlF2,  CtrlF3,  CtrlF4,  CtrlF5,
    CtrlF6,  CtrlF7,  CtrlF8,  CtrlF9,  CtrlF10,
    AltF1       = 256 + 104, AltF2,  AltF3,   AltF4,   AltF5,
    AltF6,  AltF7,   AltF8,   AltF9,   AltF10,
    Home        = 256 + 71, Up, PgUp,
    Left        = 256 + 75, Ctr, Right,
    End         = 256 + 79, Down, PgDn, Ins, Del,
    CtrlLeft    = 256 + 115, CtrlRight, CtrlEnd, CtrlPgDn, CtrlHome,
    CtrlPgUp    = 256 + 132,
    MouseLeft   = 512 + 1, MouseRight,
    TwiceLeft   = 512 + 256 + 1, TwiceRight
};

class Keyboard {
private:
	bool getKey(Common::Event &event, int &cgeCode);
	uint16 _current;
	CGEEngine *_vm;
public:
	static const uint16 _code[0x60];
	static const uint16 _scummVmCodes[0x60];

	Sprite *_client;
	bool _key[0x60];

	void newKeyboard(Common::Event &event);
	uint16 lastKey();
	Sprite *setClient(Sprite *spr);

	Keyboard(CGEEngine *vm);
	~Keyboard();
};

/*----------------- MOUSE interface -----------------*/

struct CGEEvent {
	uint16 _mask;
	uint16 _x;
	uint16 _y;
	Sprite *_spritePtr;
};

class Mouse : public Sprite {
public:
	Sprite *_hold;
	bool _active;
	int _hx;
	int _hy;
	bool _exist;
	int _buttons;
	Sprite *_busy;
	//Sprite *Touched;
	Mouse(CGEEngine *vm);
	~Mouse();
	void on();
	void off();
	void newMouse(Common::Event &event);
private:
	CGEEngine *_vm;
};

/*----------------- EventManager interface -----------------*/

class EventManager {
private:
	CGEEngine *_vm;
	Common::Event _event;
	CGEEvent _eventQueue[kEventMax];
	uint16 _eventQueueHead;
	uint16 _eventQueueTail;

	void handleEvents();
public:
	bool _quitFlag;

	EventManager(CGEEngine *vm);
	void poll();	
	void clearEvent(Sprite *spr);

	CGEEvent &getNextEvent();
};

} // End of namespace CGE

#endif
