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
#include "cge/mouse.h"
#include "cge/snddrv.h"
#include "cge/cge_main.h"
#include <string.h>

namespace CGE {

extern MOUSE Mouse;

bool   MIXER::Appear = false;


MIXER::MIXER(int x, int y) : SPRITE(NULL), Fall(MIX_FALL) {
	Appear = true;
	mb[0] = new BITMAP("VOLUME");
	mb[1] = NULL;
	SetShapeList(mb);
	SetName(Text->getText(MIX_NAME));
	Flags.Syst = true;
	Flags.Kill = true;
	Flags.BDel = true;
	Goto(x, y);
	Z = MIX_Z;

	// slaves

	int i;
	for (i = 0; i < MIX_MAX; i ++) {
		static char fn[] = "V00";
		wtom(i, fn + 1, 10, 2);
		lb[i] = new BITMAP(fn);
		ls[i].Now = ls[i].Next = i;
		ls[i].Dx = ls[i].Dy = ls[i].Dly = 0;
	}
	lb[i] = NULL;

	for (i = 0; i < ArrayCount(Led); i ++) {
		register SPRITE *spr = new SPRITE(lb);
		spr->SetSeq(ls);
		spr->Goto(x + 2 + 12 * i, y + 8);
		spr->Flags.Tran = true;
		spr->Flags.Kill = true;
		spr->Flags.BDel = false;
		spr->Z = MIX_Z;
		Led[i] = spr;
	}
	Led[ArrayCount(Led) - 1]->Flags.BDel = true;

	Vga->ShowQ->Insert(this);
	for (i = 0; i < ArrayCount(Led); i ++)
		Vga->ShowQ->Insert(Led[i]);

	//--- reset balance
	i = (SNDDrvInfo.VOL4.ML + SNDDrvInfo.VOL4.MR) / 2;
	SNDDrvInfo.VOL4.ML = i;
	SNDDrvInfo.VOL4.MR = i;
	i = (SNDDrvInfo.VOL4.DL + SNDDrvInfo.VOL4.DR) / 2;
	SNDDrvInfo.VOL4.DL = i;
	SNDDrvInfo.VOL4.DR = i;
	Update();
	Time = MIX_DELAY;
}

MIXER::~MIXER(void) {
	Appear = false;
}


#pragma argsused
void MIXER::Touch(uint16 mask, int x, int y) {
	SPRITE::Touch(mask, x, y);
	if (mask & L_UP) {
		uint8 *vol = (&SNDDrvInfo.VOL2.D) + (x < W / 2);
		if (y < MIX_BHIG) {
			if (*vol < 0xFF)
				*vol += 0x11;
		} else if (y >= H - MIX_BHIG) {
			if (*vol > 0x00)
				*vol -= 0x11;
		}
		Update();
	}
}


void MIXER::Tick(void) {
	int x = Mouse.X, y = Mouse.Y;
	if (SpriteAt(x, y) == this) {
		Fall = MIX_FALL;
		if (Flags.Hold)
			Touch(L_UP, x - X, y - Y);
	} else {
		if (Fall)
			--Fall;
		else {
			for (int i = 0; i < ArrayCount(Led); i ++)
				SNPOST_(SNKILL, -1, 0, Led[i]);
			SNPOST_(SNKILL, -1, 0, this);
		}
	}
	Time = MIX_DELAY;
}


void MIXER::Update(void) {
	Led[0]->Step(SNDDrvInfo.VOL4.ML);
	Led[1]->Step(SNDDrvInfo.VOL4.DL);

	//TODO Change the SNPOST message send to a special way to send function pointer
	//SNPOST_(SNEXEC, -1, 0, (void*)&SNDSetVolume);
	warning("STUB: MIXER::Update");
}

} // End of namespace CGE
