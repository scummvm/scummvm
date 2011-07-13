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

#include "common/scummsys.h"
#include "cge/walk.h"
#include "cge/sound.h"
#include "cge/startup.h"
#include "cge/config.h"
#include "cge/vga13h.h"
#include "cge/snail.h"
#include "cge/text.h"
#include "cge/game.h"
#include "cge/events.h"
#include "cge/cfile.h"
#include "cge/vol.h"
#include "cge/talk.h"
#include "cge/vmenu.h"
#include "cge/gettext.h"
#include "cge/mixer.h"
#include "cge/cge_main.h"
#include "cge/cge.h"
#include "common/str.h"

namespace CGE {

WALK *_hero;
Cluster Trace[MAX_FIND_LEVEL];
int FindLevel;

uint8 Cluster::_map[MAP_ZCNT][MAP_XCNT];


uint8 &Cluster::cell() {
	return _map[_b][_a];
}


bool Cluster::Protected() {
/*
	if (A == Barriers[Now]._vert || B == Barriers[Now]._horz)
		return true;

	_DX = (MAP_ZCNT << 8) + MAP_XCNT;
	_BX = (uint16) this;

	asm   mov ax,1
	asm   mov cl,[bx].(COUPLE)A
	asm   mov ch,[bx].(COUPLE)B
	asm   test    cx,0x8080       // (A < 0) || (B < 0)
	asm   jnz xit

	asm   cmp cl,dl
	asm   jge xit
	asm   cmp ch,dh
	asm   jge xit

	//  if (A < 0 || A >= MAP_XCNT || B < 0 || B >= MAP_ZCNT) return true;

	asm   mov al,dl
	asm   mul ch
	asm   xor ch,ch
	asm   add ax,cx
	asm   mov bx,ax
	_BX += (uint16) Map;
	//asm add bx,offset CLUSTER::Map
	asm   mov al,[bx]
	asm   and ax,0xFF
	asm   jz  xit
	asm   mov ax,1

	//  return Map[B][A] != 0;

	xit: return _AX;
	*/

	warning("STUB: CLUSTER::Protected()");
	return true;
}


Cluster XZ(int x, int y) {
	if (y < MAP_TOP)
		y = MAP_TOP;

	if (y > MAP_TOP + MAP_HIG - MAP_ZGRID)
		y = MAP_TOP + MAP_HIG - MAP_ZGRID;

	return Cluster(x / MAP_XGRID, (y - MAP_TOP) / MAP_ZGRID);
}


Cluster XZ(Couple xy) {
	signed char x, y;
	xy.split(x, y);
	return XZ(x, y);
}

WALK::WALK(CGEEngine *vm, BMP_PTR *shpl)
	: Sprite(vm, shpl), Dir(NO_DIR), _tracePtr(-1), _vm(vm) {
}


void WALK::tick() {
	if (_flags._hide)
		return;

	_here = XZ(_x + _w / 2, _y + _h);

	if (Dir != NO_DIR) {
		Sprite *spr;
		_sys->funTouch();
		for (spr = _vga->_showQ->first(); spr; spr = spr->_next) {
			if (distance(spr) < 2) {
				if (!spr->_flags._near) {
					_vm->feedSnail(spr, kNear);
					spr->_flags._near = true;
				}
			} else {
				spr->_flags._near = false;
			}
		}
	}

	if (_flags._hold || _tracePtr < 0)
		park();
	else {
		if (_here == Trace[_tracePtr]) {
			if (--_tracePtr < 0)
				park();
		} else {
			signed char dx, dz;
			(Trace[_tracePtr] - _here).split(dx, dz);
			DIR d = (dx) ? ((dx > 0) ? EE : WW) : ((dz > 0) ? SS : NN);
			turn(d);
		}
	}
	step();
	if ((Dir == WW && _x <= 0)           ||
	    (Dir == EE && _x + _w >= SCR_WID) ||
	    (Dir == SS && _y + _w >= WORLD_HIG - 2))
		park();
	else {
		signed char x;            // dummy var
		_here.split(x, _z);         // take current Z position
		SNPOST_(SNZTRIM, -1, 0, this);    // update Hero's pos in show queue
	}
}


int WALK::distance(Sprite *spr) {
	int dx, dz;
	dx = spr->_x - (_x + _w - WALKSIDE);
	if (dx < 0)
		dx = (_x + WALKSIDE) - (spr->_x + spr->_w);

	if (dx < 0)
		dx = 0;

	dx /= MAP_XGRID;
	dz = spr->_z - _z;
	if (dz < 0)
		dz = - dz;

	dx = dx * dx + dz * dz;
	for (dz = 1; dz * dz < dx; dz++)
		;

	return dz - 1;
}


void WALK::turn(DIR d) {
	DIR dir = (Dir == NO_DIR) ? SS : Dir;
	if (d != Dir) {
		step((d == dir) ? (1 + dir + dir) : (9 + 4 * dir + d));
		Dir = d;
	}
}


void WALK::park() {
	if (_time == 0)
		++_time;

	if (Dir != NO_DIR) {
		step(9 + 4 * Dir + Dir);
		Dir = NO_DIR;
		_tracePtr = -1;
	}
}


void WALK::findWay(Cluster c) {
	/*
	bool Find1Way();
	extern uint16 Target;

	if (c != _here) {
		for (FindLevel = 1; FindLevel <= MAX_FIND_LEVEL; FindLevel++) {
			signed char x, z;
			_here.split(x, z);
			Target = (z << 8) | x;
			c.split(x, z);
			_CX = (z << 8) | x;
			if (Find1Way())
				break;
		}
		_tracePtr = (FindLevel > MAX_FIND_LEVEL) ? -1 : (FindLevel - 1);
		if (_tracePtr < 0)
			NoWay();
		Time = 1;
	}
	*/
}


void WALK::findWay(Sprite *spr) {
	if (spr && spr != this) {
		int x = spr->_x;
		int z = spr->_z;
		if (spr->_flags._east)
			x += spr->_w + _w / 2 - WALKSIDE;
		else
			x -= _w / 2 - WALKSIDE;
		findWay(Cluster((x / MAP_XGRID),
		                ((z < MAP_ZCNT - MAX_DISTANCE) ? (z + 1)
		                 : (z - 1))));
	}
}


bool WALK::lower(Sprite *spr) {
	return (spr->_y > _y + (_h * 3) / 5);
}


void WALK::reach(Sprite *spr, int mode) {
	if (spr) {
		_hero->findWay(spr);
		if (mode < 0) {
			mode = spr->_flags._east;
			if (lower(spr))
				mode += 2;
		}
	}
	// note: insert SNAIL commands in reverse order
	SNINSERT(SNPAUSE, -1, 64, NULL);
	SNINSERT(SNSEQ, -1, TSEQ + mode, this);
	if (spr) {
		SNINSERT(SNWAIT,  -1, -1, _hero); /////--------$$$$$$$
		//SNINSERT(SNWALK, -1, -1, spr);
	}
	// sequence is not finished,
	// now it is just at sprite appear (disappear) point
}

} // End of namespace CGE
