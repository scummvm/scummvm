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

#define KEYBD_INT   9
#define LSHIFT      42
#define RSHIFT      54
#define CTRL        29
#define ALT         56


class Keyboard {
private:
	bool getKey(uint16 keycode, int &cgeCode);
public:
	static const uint16 _code[0x60];
	static const uint16 _scummVmCodes[0x60];

	uint16 _current;
	Sprite *_client;
	bool _key[0x60];

	void NewKeyboard(Common::Event &event);
	uint16 last() {
		uint16 cur = _current;
		_current = 0;
		return cur;
	}
	Sprite *setClient(Sprite *spr);

	Keyboard();
	~Keyboard();
};

/*----------------- MOUSE interface -----------------*/

#define EVT_MAX     256
#define ROLL        0x01
#define L_DN        0x02
#define L_UP        0x04
#define R_DN        0x08
#define R_UP        0x10
#define ATTN        0x20 // 0x40
#define KEYB        0x80


extern TALK *Talk;

struct CGEEvent {
	uint16 _msk;
	uint16 _x;
	uint16 _y;
	Sprite *_ptr;
};

extern CGEEvent Evt[EVT_MAX];
extern uint16 EvtHead, EvtTail;


class MOUSE : public Sprite {
public:
	Sprite *Hold;
	bool _active;
	int hx, hy;
	bool Exist;
	int Buttons;
	Sprite *Busy;
	//Sprite *Touched;
	MOUSE(CGEEngine *vm, Bitmap **shpl = MC);
	~MOUSE();
	void On();
	void Off();
	void Tick();
	void NewMouse(Common::Event &event);
private:
	CGEEngine *_vm;
};

/*----------------- EventManager interface -----------------*/

class EventManager {
private:
	Common::Event _event;
	void handleEvents();
public:
	bool _quitFlag;

	EventManager();
	void poll();	
	static void ClrEvt(Sprite *spr = NULL);
};

} // End of namespace CGE

#endif
