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

#include "cge/mixer.h"
#include "cge/text.h"
#include "cge/snail.h"
#include "cge/events.h"
#include "cge/snddrv.h"
#include "cge/cge_main.h"

namespace CGE {

extern Mouse *Mouse;

bool Mixer::_appear = false;


Mixer::Mixer(CGEEngine *vm, int x, int y) : Sprite(vm, NULL), _fall(MIX_FALL), _vm(vm) {
	_appear = true;
	_mb[0] = new Bitmap("VOLUME", true);
	_mb[1] = NULL;
	setShapeList(_mb);
	setName(_text->getText(MIX_NAME));
	_flags._syst = true;
	_flags._kill = true;
	_flags._bDel = true;
	gotoxy(x, y);
	_z = MIX_Z;

	// slaves

	uint i;
	Seq ls[MIX_MAX]; 

	for (i = 0; i < MIX_MAX; i++) {
		static char fn[] = "V00";
		wtom(i, fn + 1, 10, 2);
		_lb[i] = new Bitmap(fn, true);
		ls[i]._now = ls[i]._next = i;
		ls[i]._dx = ls[i]._dy = ls[i]._dly = 0;
	}
	_lb[i] = NULL;

	for (i = 0; i < ArrayCount(_led); i++) {
		register Sprite *spr = new Sprite(_vm, _lb);

		Seq *seq = (Seq *)malloc(MIX_MAX * sizeof(Seq));
		Common::copy(ls, ls + MIX_MAX, seq);
		spr->setSeq(seq);

		spr->gotoxy(x + 2 + 12 * i, y + 8);
		spr->_flags._tran = true;
		spr->_flags._kill = true;
		spr->_flags._bDel = false;
		spr->_z = MIX_Z;
		_led[i] = spr;
	}
	_led[ArrayCount(_led) - 1]->_flags._bDel = true;

	_vga->_showQ->insert(this);
	for (i = 0; i < ArrayCount(_led); i++)
		_vga->_showQ->insert(_led[i]);

	//--- reset balance
	i = (_sndDrvInfo.Vol4._ml + _sndDrvInfo.Vol4._mr) / 2;
	_sndDrvInfo.Vol4._ml = i;
	_sndDrvInfo.Vol4._mr = i;
	i = (_sndDrvInfo.Vol4._dl + _sndDrvInfo.Vol4._dr) / 2;
	_sndDrvInfo.Vol4._dl = i;
	_sndDrvInfo.Vol4._dr = i;
	update();
	_time = MIX_DELAY;
}

Mixer::~Mixer() {
	_appear = false;
}


#pragma argsused
void Mixer::touch(uint16 mask, int x, int y) {
	Sprite::touch(mask, x, y);
	if (mask & L_UP) {
		uint8 *vol = (&_sndDrvInfo.Vol2._d) + (x < _w / 2);
		if (y < MIX_BHIG) {
			if (*vol < 0xFF)
				*vol += 0x11;
		} else if (y >= _h - MIX_BHIG) {
			if (*vol > 0x00)
				*vol -= 0x11;
		}
		update();
	}
}


void Mixer::tick() {
	int x = _mouse->_x;
	int y = _mouse->_y;
	if (spriteAt(x, y) == this) {
		_fall = MIX_FALL;
		if (_flags._hold)
			touch(L_UP, x - _x, y - _y);
	} else {
		if (_fall)
			_fall--;
		else {
			for (uint i = 0; i < ArrayCount(_led); i++)
				SNPOST_(SNKILL, -1, 0, _led[i]);
			SNPOST_(SNKILL, -1, 0, this);
		}
	}
	_time = MIX_DELAY;
}


void Mixer::update() {
	_led[0]->step(_sndDrvInfo.Vol4._ml);
	_led[1]->step(_sndDrvInfo.Vol4._dl);

	SNPOST2_(SNEXEC, -1, 0, SNDSETVOLUME);
}

} // End of namespace CGE
