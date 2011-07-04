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

#ifndef __CGE_SNAIL__
#define __CGE_SNAIL__

#include "cge/jbw.h"
#include "cge/cge.h"

namespace CGE {

#define POCKET_X    174
#define POCKET_Y    176
#define POCKET_DX   18
#define POCKET_DY   22
#define POCKET_NY   1

#define POCKET_SX   8
#define POCKET_SY   3

#define SNINSERT(c, r, v, p)   _snail->insCom(c, r, v, p)
#define SNPOST(c, r, v, p)     _snail->addCom(c, r, v, p)
#define SNPOST_(c, r, v, p)    _snail_->addCom(c, r, v, p)

#define SNAIL_FRAME_RATE 62
#define SNAIL_FRAME_DELAY (1000 / SNAIL_FRAME_RATE)

struct Bar {
	uint8 _horz;
	uint8 _vert;
};


enum SNCOM {
	SNLABEL,  SNPAUSE,  SNWAIT,    SNLEVEL,  SNHIDE,
	SNSAY,    SNINF,    SNTIME,    SNCAVE,   SNKILL,
	SNRSEQ,   SNSEQ,    SNSEND,    SNSWAP,   SNKEEP,
	SNGIVE,   SNIF,     SNGAME,    SNSETX0,  SNSETY0,
	SNSLAVE,  SNSETXY,  SNRELX,    SNRELY,   SNRELZ,
	SNSETX,   SNSETY,   SNSETZ,    SNTRANS,  SNPORT,
	SNNEXT,   SNNNEXT,  SNTNEXT,   SNRNNEXT, SNRTNEXT,
	SNRMNEAR, SNRMTAKE, SNFLAG,    SNSETREF, SNBACKPT,
	SNFLASH,  SNLIGHT,  SNSETHB,   SNSETVB,  SNWALK,
	SNREACH,  SNCOVER,  SNUNCOVER, SNCLEAR,  SNTALK,
	SNMOUSE,  SNSOUND,  SNCOUNT,   SNEXEC,   SNSTEP,
	SNZTRIM,  SNGHOST
};

enum SNLIST { NEAR, TAKE };

class Snail {
public:
	struct Com {
		SNCOM _com;
		int _ref;
		int _val;
		void *_ptr;
	} *_snList;
	uint8 _head;
	uint8 _tail;
	bool _turbo;
	bool _busy;
	bool _textDelay;
	uint32 _timerExpiry;
	static const char *_comTxt[];
	bool _talkEnable;
	Snail(CGEEngine *vm, bool turbo);
	~Snail();
	void runCom();
	void addCom(SNCOM com, int ref, int val, void *ptr);
	void insCom(SNCOM com, int ref, int val, void *ptr);
	bool idle();
private:
	CGEEngine *_vm;
};


void pocFul();


extern  bool _flag[4];
extern  bool _game;
extern  bool _dark;
extern  int _now;
extern  int _lev;
extern  int _maxCave;
extern  Bar _barriers[];
extern  struct Hxy {
	int _x;
	int _y;
} _heroXY[];

} // End of namespace CGE

#endif
