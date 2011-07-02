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
#include <string.h>

namespace CGE {

extern MOUSE *Mouse;

bool Mixer::_appear = false;


Mixer::Mixer(CGEEngine *vm, int x, int y) : Sprite(vm, NULL), _fall(MIX_FALL), _vm(vm) {
	_appear = true;
	_mb[0] = new Bitmap("VOLUME", true);
	_mb[1] = NULL;
	setShapeList(_mb);
	setName(Text->getText(MIX_NAME));
	_flags._syst = true;
	_flags._kill = true;
	_flags._bDel = true;
	gotoxy(x, y);
	_z = MIX_Z;

	// slaves

	uint i;
	for (i = 0; i < MIX_MAX; i++) {
		static char fn[] = "V00";
		wtom(i, fn + 1, 10, 2);
		_lb[i] = new Bitmap(fn, true);
		_ls[i].Now = _ls[i].Next = i;
		_ls[i].Dx = _ls[i].Dy = _ls[i].Dly = 0;
	}
	_lb[i] = NULL;

	for (i = 0; i < ArrayCount(_led); i++) {
		register Sprite *spr = new Sprite(_vm, _lb);
		spr->setSeq(_ls);
		spr->gotoxy(x + 2 + 12 * i, y + 8);
		spr->_flags._tran = true;
		spr->_flags._kill = true;
		spr->_flags._bDel = false;
		spr->_z = MIX_Z;
		_led[i] = spr;
	}
	_led[ArrayCount(_led) - 1]->_flags._bDel = true;

	Vga->ShowQ->Insert(this);
	for (i = 0; i < ArrayCount(_led); i++)
		Vga->ShowQ->Insert(_led[i]);

	//--- reset balance
	i = (SNDDrvInfo.VOL4.ML + SNDDrvInfo.VOL4.MR) / 2;
	SNDDrvInfo.VOL4.ML = i;
	SNDDrvInfo.VOL4.MR = i;
	i = (SNDDrvInfo.VOL4.DL + SNDDrvInfo.VOL4.DR) / 2;
	SNDDrvInfo.VOL4.DL = i;
	SNDDrvInfo.VOL4.DR = i;
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
		uint8 *vol = (&SNDDrvInfo.VOL2.D) + (x < _w / 2);
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
	int x = Mouse->_x;
	int y = Mouse->_y;
	if (SpriteAt(x, y) == this) {
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


void Mixer::update(void) {
	_led[0]->step(SNDDrvInfo.VOL4.ML);
	_led[1]->step(SNDDrvInfo.VOL4.DL);

	//TODO Change the SNPOST message send to a special way to send function pointer
	//SNPOST_(SNEXEC, -1, 0, (void*)&SNDSetVolume);
	warning("STUB: Mixer::Update");
}

} // End of namespace CGE
