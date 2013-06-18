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

#include "graph.h"
/*#include "Tommys.h"*/
/*#include "Crt.h"*/

namespace Avalanche {

enum flavourtype {ch_ega, ch_bgi, last_flavourtype};

struct chunkblocktype {
	flavourtype flavour;
	integer x, y;
	integer xl, yl;
	longint size;
	boolean natural;

	boolean memorise; /* Hold it in memory? */
};

enum kind {walled, unwalled, last_kind};

const array<1, 44, char> chunkheader =
    string("Chunk-type AVD file, for an Avvy game.") + '\32' + '\261' + '\x30' + '\x1' + '\x75' + '\261';

const integer arraysize = 12000;

const kind w = walled;
const kind uw = unwalled;

array<1, 50, longint> offsets;
byte num_chunks, this_chunk;
integer gd, gm;
untyped_file f;
array<0, arraysize, byte> aa;

void rdln() {
	char r;
	return;
	do {
		r = readkey();
	} while (!(! keypressed()));
}

void load(kind k) {
	byte a1 /*absolute $A400:800*/;
	byte bit;
	untyped_file f;


	if (k == unwalled)  assign(f, "place29.avd");
	else assign(f, "walled.avd");

	reset(f, 1);
	seek(f, 177);
	for (bit = 0; bit <= 3; bit ++) {
		port[0x3c4] = 2;
		port[0x3ce] = 4;
		port[0x3c5] = 1 << bit;
		port[0x3cf] = bit;
		blockread(f, a1, 12080);
	}

	close(f);
	bit = getpixel(0, 0);
}

void finder() {
	char r;
	integer x, y;
	return;
	setfillstyle(0, 0);
	setcolor(14);
	x = 320;
	y = 100;
	setwritemode(xorput);
	do {
		bar(0, 0, 200, 10);
		outtextxy(0, 0, strf(x) + ',' + strf(y));
		line(x - 20, y, x + 20, y);
		line(x, y - 20, x, y + 20);
		do {
		} while (!keypressed());
		line(x - 20, y, x + 20, y);
		line(x, y - 20, x, y + 20);
		switch (readkey()) {
		case '\15':
			return;
			break;
		case '8':
			y -= 10;
			break;
		case '4':
			x -= 10;
			break;
		case '6':
			x += 10;
			break;
		case '2':
			y += 10;
			break;
		case '\0':
			switch (readkey()) {
			case cup:
				y -= 1;
				break;
			case cdown:
				y += 1;
				break;
			case cleft:
				x -= 1;
				break;
			case cright:
				x += 1;
				break;
			}
			break;
		}
	} while (!false);
}

void loadtemp(string which) {
	byte a0 /*absolute $A000:800*/;
	byte bit;
	untyped_file f;


	assign(f, string("corr") + which + "tmp.avd");
	reset(f, 1);
	seek(f, 177);
	for (bit = 0; bit <= 3; bit ++) {
		port[0x3c4] = 2;
		port[0x3ce] = 4;
		port[0x3c5] = 1 << bit;
		port[0x3cf] = bit;
		blockread(f, a0, 12080);
	}
	close(f);

	bit = getpixel(0, 0);

	finder();
}

void open_chunk() {
	assign(f, "chunk29.avd");
	rewrite(f, 1);
	blockwrite(f, chunkheader, sizeof(chunkheader));
	blockwrite(f, num_chunks, 1);
	blockwrite(f, offsets, num_chunks * 4);

	this_chunk = 0;
}

void close_chunk() {
	seek(f, 45);
	blockwrite(f, offsets, num_chunks * 4); /* make sure they're right! */
	close(f);
}

void mgrab(integer x1, integer y1, integer x2, integer y2, word size) {
	integer yy;
	word aapos;
	byte length, bit;

	if (size > arraysize) {
		restorecrtmode();
		output << "*** SORRY! *** Increase the arraysize constant to be greater" << NL;
		output << " than " << size << '.' << NL;
		exit(0);
	}

	aapos = 0;

	length = x2 - x1;

	for (bit = 0; bit <= 3; bit ++) {
		port[0x3c4] = 2;
		port[0x3ce] = 4;
		port[0x3c5] = 1 << bit;
		port[0x3cf] = bit;
		for (yy = y1; yy <= y2; yy ++) {
			move(mem[0xa400 * yy * 80 + x1], aa[aapos], length);
			aapos += length;
		}
	}
	bit = getpixel(0, 0);

}

void grab(integer x1, integer y1, integer x2, integer y2, integer realx, integer realy, flavourtype flav,
          boolean mem, boolean nat, kind k)
/* yes, I *do* know how to spell "really"! */
{
	word s;
	pointer p;
	chunkblocktype ch;

	/* rectangle(x1,y1,x2,y2); exit;*/
	this_chunk += 1;
	offsets[this_chunk] = filepos(f);


	s = imagesize(x1, y1, x2, y2);
	getmem(p, s);
	getimage(x1, y1, x2, y2, p);

	{
		ch.flavour = flav;
		ch.x = realx;
		ch.y = realy;

		ch.xl = x2 - x1;
		ch.yl = y2 - y1;
		ch.size = s;
		ch.memorise = mem;
		ch.natural = nat;
	}

	load(k);

	setvisualpage(1);
	setactivepage(1);
	rdln();
	putimage(ch.x, ch.y, p, 0);

	if (flav == ch_ega) {
		freemem(p, s);
		s = 4 * (((x2 / 8) - (x1 / 8)) + 2) * (y2 - y1 + 1);
		{
			ch.size = s;
			ch.x = ch.x / 8;
			ch.xl = ((realx - ch.x * 8) + (x2 - x1) + 7) / 8;
			mgrab(ch.x, ch.y, ch.x + ch.xl, ch.y + ch.yl, s);
		}
	} else
		/* For BGI pictures. */
	{
		ch.x = ch.x / 8;
		ch.xl = (ch.xl + 7) / 8;
		ch.size = imagesize(ch.x * 8, ch.y, (ch.x + ch.xl) * 8, ch.y + ch.yl);
	}

	rdln();
	setvisualpage(0);
	setactivepage(0);

	blockwrite(f, ch, sizeof(ch));

	switch (flav) {
	case ch_ega :
		if (! nat)  blockwrite(f, aa, s);
		break;
	case ch_bgi : {
		if (! nat)  blockwrite(f, p, s);
		freemem(p, s);
	}
	break;
	}
	/* rectangle(x1,y1,x2,y2);*/
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	gd = 3;
	gm = 0;
	initgraph(gd, gm, "c:\\bp\\bgi");
	setwritemode(xorput);

	loadtemp("2");

	num_chunks = 32;

	open_chunk();

	grab(0, 11, 100, 104, 260, 41, ch_ega, false, false, uw); /*1 Geida's door. */
	grab(103, 12, 203, 55, 207, 61, ch_ega, false, false, uw); /*2 "Ite mingite" sign. */
	grab(123, 59, 185, 103, 254, 90, ch_ega, false, false, uw); /*3 Normal N door (rt handle)*/
	grab(227, 10, 289, 56, 254, 88, ch_ega, false, false, uw); /*4 Normal N door (lt handle)*/
	grab(207, 63, 294, 105,  9, 47, ch_ega, false, false, w); /*5 Window in left wall. */
	grab(312, 10, 416, 56, 233, 88, ch_ega, false, false, uw); /*6 North archway */
	grab(331, 78, 384, 101, 32, 64, ch_ega, false, false, w); /*7 2 torches, L wall. */
	grab(607, 142, 621, 157, 233, 88, ch_ega, false, false, uw); /*8 1 torch, N wall. */
	grab(417, 11, 577, 59, 311, 61, ch_ega, false, false, uw); /*9 "The Wrong Way!" sign. */

	loadtemp("3");

	grab(0, 11, 62, 63, 557, 108, ch_ega, false, false, w); /*10 Near right candle */
	grab(64, 11, 120, 63, 18, 108, ch_ega, false, false, w); /*11 Near left candle */
	grab(122, 11, 169, 55, 93, 100, ch_ega, false, false, w); /*12 Far left candle */
	grab(171, 11, 222, 52, 500, 100, ch_ega, false, false, w); /*13 Far right candle */
	grab(32, 68, 84, 104, 285, 70, ch_ega, false, false, uw); /*14 Far window */
	grab(138, 65, 190, 92, 233, 88, ch_ega, false, false, uw); /*15 Baron du Lustie pic 1 */
	grab(244, 65, 296, 92, 103, 51, ch_ega, false, false, uw); /*16 Baron du Lustie pic 2 */
	grab(172, 54, 280, 63, 233, 88, ch_ega, false, false, uw); /*17 "Art Gallery" sign */
	grab(341, 18, 402, 47, 563, 48, ch_ega, false, false, w); /*18 Right wall torches */
	grab(528, 10, 639, 160, 528, 10, ch_ega, false, false, uw); /*19 Right wall */
	grab(430, 50, 526, 88, 543, 50, ch_ega, false, false, w); /*20 Window in right wall */
	grab(451, 91, 494, 152, 566, 91, ch_ega, false, false, w); /*21 Door in right wall */
	grab(238, 10, 307, 14, 484, 156, ch_ega, false, false, w); /*22 Near wall door: right */
	grab(239, 16, 300, 20, 300, 156, ch_ega, false, false, w); /*23 Near wall door: middle */
	grab(234, 22, 306, 26, 100, 156, ch_ega, false, false, w); /*24 Near wall door: left */
	grab(25, 113, 87, 156, 254, 90, ch_ega, false, false, w); /*25 Far door opening stage 1 */
	grab(131, 113, 193, 156, 254, 90, ch_ega, false, false, w); /*26 Far door opening stage 2 */
	grab(237, 113, 299, 156, 254, 90, ch_ega, false, false, w); /*27 Far door opening stage 3 */

	loadtemp("4");

	grab(0, 11, 112, 160,  0, 11, ch_ega, false, false, uw); /*28 Left wall */
	grab(144, 44, 197, 76, 30, 44, ch_ega, false, false, w); /*29 Shield on L wall. */
	grab(149, 90, 192, 152, 35, 90, ch_ega, false, false, w); /*30 Door in L wall. */
	grab(463, 28, 527, 43, 252, 100, ch_ega, false, false, w); /*31 Archway x 2 */
	grab(463, 79, 527, 94, 252, 100, ch_ega, false, false, w); /*32 Archway x 3 */

	close_chunk();
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.