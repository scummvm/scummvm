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
#include "cge/events.h"
#include "cge/cge_main.h"

namespace CGE {


#define RELIEF      1
#if RELIEF
#define MB_LT       LGRAY
#define MB_RB       DGRAY
#else
#define MB_LT       DGRAY
#define MB_RB       LGRAY
#endif




MenuBar::MenuBar(CGEEngine *vm, uint16 w) : Talk(vm), _vm(vm) {
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

	_ts = new BMP_PTR[2];
	_ts[0] = new Bitmap(w, h, p);
	_ts[1] = NULL;
	setShapeList(_ts);

	_flags._slav = true;
	_flags._tran = true;
	_flags._kill = true;
	_flags._bDel = true;
}


static  char   *vmgt;


char *VMGather(Choice *list) {
	Choice *cp;
	int len = 0, h = 0;

	for (cp = list; cp->_text; cp++) {
		len += strlen(cp->_text);
		h++;
	}
	vmgt = new char[len + h];
	if (vmgt) {
		*vmgt = '\0';
		for (cp = list; cp->_text; cp++) {
			if (*vmgt)
				strcat(vmgt, "|");
			strcat(vmgt, cp->_text);
			h++;
		}
	}
	return vmgt;
}


Vmenu *Vmenu::_addr = NULL;
int    Vmenu::_recent   = -1;


Vmenu::Vmenu(CGEEngine *vm, Choice *list, int x, int y)
	: Talk(vm, VMGather(list), RECT), _menu(list), _bar(NULL), _vm(vm) {
	Choice *cp;

	_addr = this;
	delete[] vmgt;
	_items = 0;
	for (cp = list; cp->_text; cp++)
		_items++;
	_flags._bDel = true;
	_flags._kill = true;
	if (x < 0 || y < 0)
		center();
	else
		gotoxy(x - _w / 2, y - (TEXT_VM + FONT_HIG / 2));
	_vga->_showQ->insert(this, _vga->_showQ->last());
	_bar = new MenuBar(_vm, _w - 2 * TEXT_HM);
	_bar->gotoxy(_x + TEXT_HM - MB_HM, _y + TEXT_VM - MB_VM);
	_vga->_showQ->insert(_bar, _vga->_showQ->last());
}


Vmenu::~Vmenu() {
	_addr = NULL;
}


void Vmenu::touch(uint16 mask, int x, int y) {
	uint16 h = FONT_HIG + TEXT_LS;
	bool ok = false;

	if (_items) {
		Sprite::touch(mask, x, y);

		y -= TEXT_VM - 1;
		int n = 0;
		if (y >= 0) {
			n = y / h;
			if (n < _items)
				ok = (x >= TEXT_HM && x < _w - TEXT_HM/* && y % h < FONT_HIG*/);
			else
				n = _items - 1;
		}

		_bar->gotoxy(_x + TEXT_HM - MB_HM, _y + TEXT_VM + n * h - MB_VM);

		if (ok && (mask & L_UP)) {
			_items = 0;
			SNPOST_(SNKILL, -1, 0, this);
			//_menu[_recent = n].Proc();
			warning("Missing call to proc()");
		}
	}
}

} // End of namespace CGE
