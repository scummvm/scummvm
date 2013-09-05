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
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* PINGO	Full-screen sub-parts of the game. */

#include "avalanche/avalanche.h"

#include "avalanche/pingo2.h"
#include "avalanche/gyro2.h"
#include "avalanche/lucerna2.h"
#include "avalanche/trip6.h"
#include "avalanche/scrolls2.h"

#include "common/textconsole.h"
#include "common/file.h"

namespace Avalanche {

Pingo::Pingo(AvalancheEngine *vm) {
	_vm = vm;
}

void Pingo::dplot(int16 x, int16 y, Common::String z) {
	warning("STUB: Pingo::dplot()");
}

void Pingo::bosskey() {
#if 0
const
	months : array[0..11] of char = 'JFMAMJJASOND';
	title = 'Net Profits';
	fish = #224; { à }
var fv:byte; gd,gm:int16; r:char;
begin;
	dusk; delavvy;
	setactivepage(3); mousepage(3); setvisualpage(3); off;
	cleardevice; setfillstyle(xhatchfill,11);
	settextstyle(1,0,4); settextjustify(1,1);
	dplot(320,10,title);
	settextstyle(1,0,0); setusercharsize(4,3,7,12);
	for fv:=0 to 11 do
	begin;
	dplot(26+fv*52,187,months[fv]);
	bar(fv*52,177-fv*14,51+fv*52,180);
	rectangle(fv*52,177-fv*14,51+fv*52,180);
	end;
	settextstyle(0,0,1);
	for fv:=1 to 177 do
	begin;
	gd:=random(630); gm:=random(160)+30;
	setcolor(lightred); outtextxy(gd  ,gm  ,fish);
	setcolor(yellow);   outtextxy(gd+1,gm-1,fish);
	end;
	newpointer(6); { TTHand }
	dawn; on; setbkcolor(1); repeat check until (mpress>0) or keypressed;
	while keypressed do r:=readkey; setbkcolor(0); settextjustify(0,0);
	dusk; setvisualpage(0); setactivepage(0); mousepage(0); dawn;
	copy02;
#endif

	warning("STUB: Pingo::bosskey()");
}

void Pingo::copy02() {   // taken from Wobble (below)
	warning("STUB: Pingo::copy02()");
}

void Pingo::copy03() {   // taken from Wobble (below)
	warning("STUB: Pingo::copy03()");
}

void Pingo::copypage(byte frp, byte top) { // taken from Copy02 (above)
	warning("STUB: Pingo::copypage()");
}

void Pingo::wobble() {
	warning("STUB: Pingo::wobble()");
}

void Pingo::zl(int16 x1, int16 y1, int16 x2, int16 y2) {
	warning("STUB: Pingo::zl()");
}

void Pingo::zonk() {
	warning("STUB: Pingo::zonk()");
}

void Pingo::winning_pic() {
	Common::File f;
	_vm->_lucerna->dusk();

	if (!f.open("finale.avd")) {
		warning("AVALANCHE: Lucerna: File not found: finale.avd");
		return;
	}

#if 0
	for (byte bit = 0; bit <= 3; bit++) {
	port[0x3c4] = 2;
	port[0x3ce] = 4;
	port[0x3c5] = 1 << bit;
	port[0x3cf] = bit;
	blockread(f, mem[0xa000 * 0], 16000);
	}
#endif

	warning("STUB: Pingo::winning_pic()");

	f.close();

	//setvisualpage(0);
	warning("STUB: Pingo::winning_pic()");

	_vm->_lucerna->dawn();

#if 0
	do {
		_vm->_gyro->check();
	} while (!(keypressed() || (mrelease > 0)));
	while (keypressed())
		char r = readkey();
	major_redraw();
#endif

	warning("STUB: Pingo::winning_pic()");
}

} // End of namespace Avalanche.
