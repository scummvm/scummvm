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

const array<1, 44, char> chunkheader =
    string("Chunk-type AVD file, for an Avvy game.") + '\32' + '\261' + '\x30' + '\x1' + '\x75' + '\261';

const integer arraysize = 32000;

array<1, 30, longint> offsets;
byte num_chunks, this_chunk;
integer gd, gm;
untyped_file f;
array<0, arraysize, byte> aa;

void load() {
	byte a0 /*absolute $A000:800*/;
	byte a1 /*absolute $A400:800*/;
	byte bit;
	untyped_file f;

	assign(f, "place9.avd");
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

void load_temp(string which) {
	byte a0 /*absolute $A000:800*/;
	byte a1 /*absolute $A400:800*/;
	byte bit;
	untyped_file f;
	assign(f, which);
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
}

void open_chunk() {
	assign(f, "chunk9.avd");
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

void grab(integer x1, integer y1, integer x2, integer y2, integer realx, integer realy, boolean mem, boolean nat)
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
		if (nat)
			ch.flavour = ch_bgi;
		else ch.flavour = ch_ega; /* At the moment, Celer can't handle natural ch_EGAs. */
		ch.x = realx;
		ch.y = realy;

		ch.xl = x2 - x1;
		ch.yl = y2 - y1;
		ch.size = s;
		ch.memorise = mem;
		ch.natural = nat;
	}

	setvisualpage(1);
	setactivepage(1);
	input >> NL;
	putimage(ch.x, ch.y, p, 0);

	freemem(p, s);
	{
		if (ch.flavour == ch_bgi)  s = imagesize(ch.x * 8, ch.y, (ch.x + ch.xl) * 8, ch.y + ch.yl);
		else s = 4 * ((x2 - x1 + 7) / 8) * (y2 - y1 + 1);
		ch.size = s;
		ch.x = ch.x / 8;
		ch.xl = (ch.xl + 7) / 8;
		mgrab(ch.x, ch.y, ch.x + ch.xl, ch.y + ch.yl, s);
	}

	input >> NL;
	setvisualpage(0);
	setactivepage(0);

	blockwrite(f, ch, sizeof(ch));

	if (! nat)  blockwrite(f, aa, s);
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	gd = 3;
	gm = 0;
	initgraph(gd, gm, "c:\\bp\\bgi");
	load();
	load_temp("d:chunkbit.avd");
	setwritemode(xorput);

	num_chunks = 7;

	open_chunk();

	grab(78, 78, 232, 102, 212, 10, true, false); /* 154 across */
	grab(235, 78, 389, 102, 212, 10, true, false);
	grab(392, 78, 546, 102, 212, 10, true, false);
	grab(392, 78, 546, 102, 212, 10, true, true);

	load_temp("d:chunkbi3.avd");

	grab(437, 51, 475, 78, 147, 120, false, false); /* 5 = door half-open. */
	grab(397, 51, 435, 78, 147, 120, false, false); /* 6 = door open. */
	grab(397, 51, 435, 78, 147, 120, true, true); /* 7 = door shut. */

	close_chunk();
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.