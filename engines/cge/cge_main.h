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

#ifndef __CGE_CGE__
#define __CGE_CGE__

#include "cge/wav.h"
#include "cge/vga13h.h"
#include "cge/events.h"
#include "cge/sound.h"

namespace CGE {
#define CAVE_X         4
#define CAVE_Y         166
#define CAVE_SX        0
#define CAVE_SY        0

#ifdef DEMO
#define CAVE_DX        23
#define CAVE_DY        29
#define CAVE_NX        3
#define CAVE_NY        1
#else
#define CAVE_DX        9
#define CAVE_DY        10
#define CAVE_NX        8
#define CAVE_NY        3
#endif
#define CAVE_MAX       (CAVE_NX * CAVE_NY)
#define PAIN           (_vm->_flag[0])

#define kInfoX         177
#define kInfoY         164
#define kInfoW         140
#define kButtonX       151
#define kButtonY       164
#define kMiniX         86
#define kMiniY         162
#define kLineMax       512
#define kDistMax       3
#define kIn0Ext        ".IN0"
#define kLgoExt        ".LGO"
#define kSvgExt        ".SVG"
#define kWalkSide      10
#define kBusyRef       500
#define kSystemRate    6   // 12 Hz
#define kHeroFun0      (40 * 12)
#define kHeroFun1      ( 2 * 12)
#define kGetNamePrompt 50
#define kGetNameTitle  51
#define kTSeq          96
//Useless?
//#define kBadSnd        97
//#define kBadMidi       98
#define kNoMusic       98
#define kBadSVG        99
#define kSeqHTalk      (kTSeq + 4)
#define kSeqTooFar     (kTSeq + 5)
#define kSeqNoWay      (kTSeq + 5)
#define kSeqPocketFull (kTSeq + 5)
#define kSeqOffUse     (kTSeq + 6)
#define kQuitTitle     200
#define kQuit          201
#define kNoQuit        202
#define kDemo          300
#define kOffUseCount   600
#define kOffUse        601
#define kNoWay         671
#define kTooFar        681
#define kPocketFull    691
#define kAltCtrlDel    777
#define kPanHeight     40
#define kScrWidth      320
#define kScrHeight     200
#define kWorldHeight   (kScrHeight - kPanHeight)

class System : public Sprite {
	int _lum;
public:
	int _funDel;

	System(CGEEngine *vm);

	void setPal();
	void funTouch();
	virtual void touch(uint16 mask, int x, int y);
	void tick();
private:
	CGEEngine *_vm;
};

class Square : public Sprite {
public:
	Square(CGEEngine *vm);
	virtual void touch(uint16 mask, int x, int y);
private:
	CGEEngine *_vm;
};

extern Vga *_vga;
extern Heart *_heart;
extern System *_sys;
extern int _offUseCount;
extern Sprite *_pocLight;
extern Keyboard *_keyboard;
extern Mouse *_mouse;
extern EventManager *_eventManager;
extern Sprite *_pocket[];
extern Sprite *_sprite;
extern Sprite *_miniCave;
extern Sprite *_shadow;
extern HorizLine *_horzLine;
extern InfoLine *_infoLine;
extern Sprite *_cavLight;
extern InfoLine *_debugLine;
extern Snail *_snail;
extern Snail *_snail_;
extern Fx *_fx;
extern Sound *_sound;

} // End of namespace CGE

#endif
