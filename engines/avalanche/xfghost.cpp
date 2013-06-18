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
/*#include "Crt.h"*/

namespace Avalanche {

const array<1, 44, char> chunkheader =
    string("Spooky file! Nearly a Chunk... (c) MT.") + '\32' + '\261' + '\x30' + '\x1' + '\x75' + '\261';

const integer n = -1; /* "No change" in new-whatever */

const integer aa = -2; /* This is an aargh. */

const integer aargh_x_ofs = -177;
const integer aargh_y_ofs = 52;

enum flavourtype {ch_ega, ch_bgi, ch_natural, ch_two, ch_one, last_flavourtype};

struct chunkblocktype {
	flavourtype flavour;
	integer x, y;
	integer xl, yl;
	longint size;
};

integer gd, gm, x, y;
untyped_file f;
byte bit;
byte a;  /*absolute $A000:0;*/
palettetype cc;
char r;
text t;
word s;
pointer p;

untyped_file chunkfile;
chunkblocktype cb;

pointer a_p;
word a_s;

void open_chunk() {
	;
	assign(chunkfile, "v:spooky.avd");
	rewrite(chunkfile, 1);
	blockwrite(chunkfile, chunkheader, sizeof(chunkheader));
}

void close_chunk() {
	;
	close(chunkfile);
}

void grab(integer x1, integer y1, integer x2, integer y2, flavourtype how, integer newx, integer newy) {
	pointer p;
	word s;
	integer y;
	byte bit;

	;
#ifndef DRYRUN
	{
		;
		cb.flavour = how;
		switch (newx) {
		case n:
			cb.x = x1;
			break;
		case aa:
			cb.x = x1 + aargh_x_ofs;
			break;
		default:
			cb.x = newx;
		}

		switch (newy) {
		case n:
			cb.y = y1;
			break;
		case aa:
			cb.y = y1 + aargh_y_ofs;
			break;
		default:
			cb.y = newy;
		}

		cb.xl = x2 - x1;
		if (set_of_enum(flavourtype)::of(ch_ega, ch_one, ch_two, eos).has(how))  cb.xl = ((cb.xl + 7) / 8) * 8;
		cb.yl = y2 - y1;
	}

	if (how != ch_natural) {
		;
		s = imagesize(x1, y1, x2, y2);
		getmem(p, s);
		getimage(x1, y1, x2, y2, p);
	}

	rectangle(x1, y1, x2, y2);

	switch (how) {
	case ch_bgi:
		cb.size = s;
		break;
	}

	blockwrite(chunkfile, cb, sizeof(cb));

	switch (how) {
	case ch_bgi:
		blockwrite(chunkfile, p, s);
		break;
	case ch_ega: {
		;
		setactivepage(1);
		cleardevice();
		putimage(0, 0, p, 0);
		setactivepage(0);

		for (bit = 0; bit <= 3; bit ++)
			for (y = 0; y <= cb.yl; y ++) {
				;
				port[0x3c4] = 2;
				port[0x3ce] = 4;
				port[0x3c5] = 1 << bit;
				port[0x3cf] = bit;
				blockwrite(chunkfile, mem[0xa400 * y * 80], cb.xl / 8);
			}

		y = getpixel(0, 0);
	}
	break;
	case ch_two: {
		; /* Same as EGA, but with only 2 planes. */
		setactivepage(1);
		cleardevice();
		putimage(0, 0, p, 0);
		setactivepage(0);

		for (bit = 2; bit <= 3; bit ++) /* << Bit to grab? */
			for (y = 0; y <= cb.yl; y ++) {
				;
				port[0x3c4] = 2;
				port[0x3ce] = 4;
				port[0x3c5] = 1 << bit;
				port[0x3cf] = bit;
				blockwrite(chunkfile, mem[0xa400 * y * 80], cb.xl / 8);
			}

		y = getpixel(0, 0);
	}
	break;
	case ch_one: {
		; /* ...but with only one plane! */
		setactivepage(1);
		cleardevice();
		putimage(0, 0, p, 0);
		setactivepage(0);

		for (bit = 3; bit <= 3; bit ++)
			for (y = 0; y <= cb.yl; y ++) {
				;
				port[0x3c4] = 2;
				port[0x3ce] = 4;
				port[0x3c5] = 1 << bit;
				port[0x3cf] = bit;
				blockwrite(chunkfile, mem[0xa400 * y * 80], cb.xl / 8);
			}

		y = getpixel(0, 0);
	}
	break;
	}

	freemem(p, s);
#endif
	rectangle(x1, y1, x2, y2);

}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	;
#ifndef DRYRUN
	open_chunk();
#endif

	gd = 3;
	gm = 0;
	initgraph(gd, gm, "c:\\bp\\bgi");
	assign(f, "c:\\sleep4\\colour.ptx");
	reset(f, 1);
	for (bit = 0; bit <= 3; bit ++) {
		;
		port[0x3c4] = 2;
		port[0x3ce] = 4;
		port[0x3c5] = 1 << bit;
		port[0x3cf] = bit;
		blockread(f, a, 16000); /* 28000 */
	}
	close(f);

	setwritemode(xorput);

	/* Grab the big ghost. */

	grab(0,  0, 160, 65, ch_two, n, n); /* First column, */
	grab(0, 67, 172, 127, ch_two, n, n);
	grab(0, 132, 158, 190, ch_two, n, n);

	a_s = imagesize(349, 36, 361, 43);
	getmem(a_p, a_s);
	getimage(349, 36, 361, 43, a_p);
	setfillstyle(1, 0);
	bar(349, 36, 361, 43);

	grab(173, 66, 347, 124, ch_two, n, n); /* Second column. */
	grab(173,  6, 352, 64, ch_two, n, n);

	putimage(349, 36, a_p, 0);

	/* Grab Avvy's eyes and the exclamation mark. */

	grab(605, 10, 620, 12, ch_bgi, n, n); /* Eyes looking left */
	grab(622, 10, 638, 12, ch_bgi, n, n); /* Ditto looking right (eye eye, sir) */
	grab(611,  0, 616,  5, ch_bgi, n, n); /* ! */

	/* Grab the cobweb. */

	grab(535, 25, 639, 75, ch_one, n, 0);
	/* ^^^ Interesting point here: the ch_EGA save routine pads with black
	  space to the RIGHT of the object. Since this cobweb needs to be right-
	  justified, we must decrease x1 until xl is a multiple of 8. */

	/* Grab Mark's signature. */

	grab(462, 61, 525, 65, ch_ega, 576, 195);

	/* Grab the open door. */

	grab(180, 132, 294, 180, ch_ega, 520, 127);

	/* Grab the bat. */

	grab(354,  0, 474, 28, ch_bgi, n, n);
	grab(484,  0, 526, 23, ch_bgi, n, n);
	grab(542,  2, 564, 22, ch_bgi, n, n);

	/* Grab the big fade-in face. */

	grab(350, 71, 420, 105, ch_ega, n, n); /* Top line. */
	grab(421, 71, 491, 105, ch_ega, n, n);

	grab(350, 107, 419, 141, ch_ega, n, n); /* Second line. */
	grab(421, 107, 490, 141, ch_ega, n, n);

	grab(350, 143, 420, 177, ch_ega, n, n); /* Third line. */
	grab(422, 143, 489, 177, ch_ega, n, n);

	/* Grab the "AARGH!" */

	grab(349, 36, 361, 43, ch_bgi, aa, aa); /* A */
	grab(366, 31, 385, 46, ch_bgi, aa, aa); /* Aa */
	grab(394, 34, 415, 52, ch_bgi, aa, aa); /* Aar */
	grab(428, 33, 457, 57, ch_bgi, aa, aa); /* Aarg */
	grab(471, 30, 508, 59, ch_bgi, aa, aa); /* Aargh */
	grab(524, 30, 524, 58, ch_bgi, aa, aa); /* Aargh! */

	for (gd = 0; gd <= 4; gd ++)
		grab(509, 76 + gd * 22, 551, 96 + gd * 22, ch_bgi, n, n); /* The big green eyes. */

	for (gd = 5; gd >= 0; gd --)
		grab(181 + gd * 34, 186, 214 + gd * 34, 199, ch_bgi, n, n); /* The red greldet. */

	for (gd = 0; gd <= 5; gd ++)
		grab(390 + gd * 34, 186, 423 + gd * 34, 199, ch_bgi, n, n); /* The blue greldet. */

#ifndef DRYRUN
	close_chunk();
#endif
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.