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

#ifndef __CGE_CGE_EVENTS__
#define __CGE_CGE_EVENTS__

#include "common/events.h"
#include "cge/game.h"
#include "cge/talk.h"
#include "cge/jbw.h"
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

class Keyboard {
private:
	bool getKey(Common::Event &event, int &cgeCode);
	uint16 _current;
public:
	static const uint16 _code[0x60];
	static const uint16 _scummVmCodes[0x60];

	Sprite *_client;
	bool _key[0x60];

	void newKeyboard(Common::Event &event);
	uint16 lastKey();
	Sprite *setClient(Sprite *spr);

	Keyboard();
	~Keyboard();
};

/*----------------- MOUSE interface -----------------*/

extern Talk *_talk;

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
	Common::Event _event;
	CGEEvent _eventQueue[kEventMax];
	uint16 _eventQueueHead;
	uint16 _eventQueueTail;

	void handleEvents();
public:
	bool _quitFlag;

	EventManager();
	void poll();	
	void clearEvent(Sprite *spr);

	CGEEvent &getNextEvent();
};

} // End of namespace CGE

#endif
