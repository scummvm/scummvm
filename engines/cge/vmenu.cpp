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

#include "cge/vmenu.h"
#include "cge/mouse.h"
#include "cge/cge_main.h"
#include <string.h>

namespace CGE {


#define RELIEF      1
#if RELIEF
#define MB_LT       LGRAY
#define MB_RB       DGRAY
#else
#define MB_LT       DGRAY
#define MB_RB       LGRAY
#endif




MENU_BAR::MENU_BAR(CGEEngine *vm, uint16 w) : TALK(vm), _vm(vm) {
	int h = FONT_HIG + 2 * MB_VM, i = (w += 2 * MB_HM) * h;
	uint8 *p = farnew(uint8, i), * p1, * p2;

	memset(p + w, TRANS, i - 2 * w);
	memset(p, MB_LT, w);
	memset(p + i - w, MB_RB, w);
	p1 = p;
	p2 = p + i - 1;
	for (i = 0; i < h; i++) {
		*p1 = MB_LT;
		*p2 = MB_RB;
		p1 += w;
		p2 -= w;
	}
	TS[0] = new BITMAP(w, h, p);
	SetShapeList(TS);
	Flags.Slav = true;
	Flags.Tran = true;
	Flags.Kill = true;
	Flags.BDel = true;
}


static  char   *vmgt;


char *VMGather(CHOICE *list) {
	CHOICE *cp;
	int len = 0, h = 0;

	for (cp = list; cp->Text; cp++) {
		len += strlen(cp->Text);
		++h;
	}
	vmgt = new char[len + h];
	if (vmgt) {
		*vmgt = '\0';
		for (cp = list; cp->Text; cp++) {
			if (*vmgt)
				strcat(vmgt, "|");
			strcat(vmgt, cp->Text);
			++ h;
		}
	}
	return vmgt;
}


VMENU *VMENU::Addr = NULL;
int    VMENU::Recent   = -1;


VMENU::VMENU(CGEEngine *vm, CHOICE *list, int x, int y)
	: TALK(vm, VMGather(list), RECT), Menu(list), Bar(NULL), _vm(vm) {
	CHOICE *cp;

	Addr = this;
	delete[] vmgt;
	Items = 0;
	for (cp = list; cp->Text; cp++)
		++Items;
	Flags.BDel = true;
	Flags.Kill = true;
	if (x < 0 || y < 0)
		Center();
	else
		Goto(x - W / 2, y - (TEXT_VM + FONT_HIG / 2));
	Vga->ShowQ->Insert(this, Vga->ShowQ->Last());
	Bar = new MENU_BAR(_vm, W - 2 * TEXT_HM);
	Bar->Goto(X + TEXT_HM - MB_HM, Y + TEXT_VM - MB_VM);
	Vga->ShowQ->Insert(Bar, Vga->ShowQ->Last());
}


VMENU::~VMENU(void) {
	Addr = NULL;
}


void VMENU::Touch(uint16 mask, int x, int y) {
	uint16 h = FONT_HIG + TEXT_LS;
	bool ok = false;

	if (Items) {
		SPRITE::Touch(mask, x, y);

		y -= TEXT_VM - 1;
		int n = 0;
		if (y >= 0) {
			n = y / h;
			if (n < Items)
				ok = (x >= TEXT_HM && x < W - TEXT_HM/* && y % h < FONT_HIG*/);
			else
				n = Items - 1;
		}

		Bar->Goto(X + TEXT_HM - MB_HM, Y + TEXT_VM + n * h - MB_VM);

		if (ok && (mask & L_UP)) {
			Items = 0;
			SNPOST_(SNKILL, -1, 0, this);
			//Menu[Recent = n].Proc();
			warning("Missing call to proc()");
		}
	}
}

} // End of namespace CGE
