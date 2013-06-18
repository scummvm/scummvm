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

array<0, 3, palettetype> fxpal;

void load() {
	byte a0 /*absolute $A000:800*/;
	byte bit;
	untyped_file f;


	assign(f, "preview2.avd");
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

	settextjustify(1, 1);
	setcolor(3);
	outtextxy(320, 166, "...This is a preview of things to come...");
	setcolor(14);
	outtextxy(320, 176, "AVAROID");
	outtextxy(320, 183, "(a space so dizzy)");
	setcolor(9);
	outtextxy(320, 194, "the next Avvy adventure-- in 256 colours.");
	setcolor(7);
	outtextxy(590, 195, "Any key...");
}

void setup() {
	integer gd, gm;
	palettetype p;

	if (paramstr(1) != "jsb")  exit(255);
	gd = 3;
	gm = 0;
	initgraph(gd, gm, "");
	getpalette(fxpal[0]);

	fillchar(p.colors, sizeof(p.colors), '\0'); /* Blank out the screen. */
	p.size = 16;
	setallpalette(p);
}

void wait() {
	word w;
	char r;

	w = 0;
	do {
		delay(1);
		w += 1;
	} while (!(keypressed() || (w == 15000)));

	while (keypressed())  r = readkey(); /* Keyboard sink. */
}

void show(byte n) {
	setallpalette(fxpal[n]);
	delay(55);
}

shortint fades(shortint x) {
	byte r, g, b;

	shortint fades_result;
	r = x / 16;
	x = x % 16;
	g = x / 4;
	b = x % 4;
	if (r > 0)  r -= 1;
	if (g > 0)  g -= 1;
	if (b > 0)  b -= 1;
	fades_result = (16 * r + 4 * g + b);
	/* fades:=x-1;*/
	return fades_result;
}

void dawn();

static void calc(byte n) {
	byte fv;

	fxpal[n] = fxpal[n - 1];

	for (fv = 1; fv <= fxpal[n].size - 1; fv ++)
		fxpal[n].colors[fv] = fades(fxpal[n].colors[fv]);
}

void dawn() {
	byte fv;

	for (fv = 1; fv <= 3; fv ++) calc(fv);

	for (fv = 3; fv >= 0; fv --) show(fv);
}

void dusk() {
	byte fv;

	for (fv = 1; fv <= 3; fv ++) show(fv);
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	setup();
	load();
	dawn();
	wait();
	dusk();
	closegraph();
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.