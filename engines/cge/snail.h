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

#ifndef __SNAIL__
#define __SNAIL__

#include "cge/jbw.h"
#include "cge/cge.h"

namespace CGE {

#define POCKET_X    174
#define POCKET_Y    176
#define POCKET_DX   18
#define POCKET_DY   22
#define POCKET_NX   8
#define POCKET_NY   1

#define POCKET_SX   8
#define POCKET_SY   3

#define SNINSERT(c,r,v,p)   Snail->InsCom(c,r,v,p)
#define SNPOST(c,r,v,p)     Snail->AddCom(c,r,v,p)
#define SNPOST_(c,r,v,p)    Snail_->AddCom(c,r,v,p)

#define SNAIL_FRAME_RATE 62
#define SNAIL_FRAME_DELAY (1000 / SNAIL_FRAME_RATE)

typedef struct  {
	uint8 Horz, Vert;
} BAR;


struct SCB {
	uint8  *Ptr;
	uint16  Siz;
	SCB    *Nxt;
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

class SNAIL {
public:
	struct COM {
		SNCOM Com;
		int Ref;
		int Val;
		void *Ptr;
	} *SNList;
	uint8 Head, Tail;
	bool Turbo, Busy, TextDelay;
	uint32 _timerExpiry;
	static const char *ComTxt[];
	bool TalkEnable;
	SNAIL(CGEEngine *vm, bool turbo = false);
	~SNAIL();
	void RunCom(void);
	void AddCom(SNCOM com, int ref = 0, int val = 0, void *ptr = NULL);
	void InsCom(SNCOM com, int ref = 0, int val = 0, void *ptr = NULL);
	bool Idle(void);
private:
	CGEEngine *_vm;
};


void    SelectPocket(int n);
void    PocFul(void);


extern  SCB Scb;
extern  bool Flag[4];
extern  bool Game;
extern  bool Dark;
//extern  SNAIL *Snail;
//extern  SNAIL *Snail_;
extern  int Now;
extern  int Lev;
extern  int MaxCave;
extern  int PocPtr;
extern  BAR Barriers[];
extern  struct HXY {
	int _x;
	int _y;
} HeroXY[];

} // End of namespace CGE

#endif
