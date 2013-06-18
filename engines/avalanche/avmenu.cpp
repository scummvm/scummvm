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
/*#include "Tommys.h"*/

namespace Avalanche {

typedef matrix < '\0', '\377', 0, 15, byte > fonttype;

fonttype font;
word storage_seg, storage_ofs;
byte result;
string registrant;

void icons() {
	untyped_file f;
	word gd, gm;
	byte bit;
	byte v;

	assign(f, "menu.avd");
	reset(f, 1);

	for (bit = 0; bit <= 3; bit ++) {
		port[0x3c4] = 2;
		port[0x3ce] = 4;
		port[0x3c5] = 1 << bit;
		port[0x3cf] = bit;
		for (gd = 70; gd <= 266; gd ++)
			blockread(f, mem[0xa000 * gd * 80 + 6], 9);

		for (gd = 0; gd <= 32; gd ++) {
			blockread(f, v, 1);
			for (gm = 0; gm <= 5; gm ++)
				mem[0xa000 * 73 + ((70 + gd + gm * 33) * 80)] = v; /* 79 */
		}

	}
	close(f);

	bit = getpixel(0, 0);

	setfillstyle(1, 7);
	for (gd = 0; gd <= 5; gd ++) bar(114, 73 + gd * 33, 583, 99 + gd * 33);
	setfillstyle(1, 15);
	for (gd = 0; gd <= 5; gd ++) bar(114, 70 + gd * 33, 583, 72 + gd * 33);
	setfillstyle(1, 8);
	for (gd = 0; gd <= 5; gd ++) bar(114, 100 + gd * 33, 583, 102 + gd * 33);

}

void load_regi_info();


static char decode1(char c) {
	byte b;

	char decode1_result;
	b = ord(c) - 32;
	decode1_result = chr(((b & 0xf) << 3) + ((cardinal)(b & 0x70) >> 4));
	return decode1_result;
}



static char decode2(char c) {
	char decode2_result;
	decode2_result = chr(((ord(c) & 0xf) << 2) + 0x43);
	return decode2_result;
}



static boolean checker(string proper, string check) {
	byte fv;
	boolean ok;

	boolean checker_result;
	ok = true;
	for (fv = 1; fv <= length(proper); fv ++)
		if ((ord(proper[fv]) & 0xf) != ((cardinal)(ord(check[fv]) - 0x43) >> 2))
			ok = false;

	checker_result = ok;
	return checker_result;
}

void load_regi_info() {
	text t;
	byte fv;
	string x;
	byte namelen, numlen;
	string namechk, numchk;
	string reginum;


	/*$I-*/
	assign(t, "register.dat");
	reset(t);
	/*$I+*/

	if (ioresult != 0) {
		registrant = "(Unregistered evaluation copy.)";
		return;
	}

	for (fv = 1; fv <= 53; fv ++) t >> NL;
	t >> x >> NL;
	close(t);

	namelen = 107 - ord(x[1]);
	numlen = 107 - ord(x[2]);

	registrant = copy(x, 3, namelen);
	reginum = copy(x, 4 + namelen, numlen);
	namechk = copy(x, 4 + namelen + numlen, namelen);
	numchk = copy(x, 4 + namelen + numlen + namelen, numlen);

	for (fv = 1; fv <= namelen; fv ++) registrant[fv] = decode1(registrant[fv]);
	for (fv = 1; fv <= numlen; fv ++) reginum[fv] = decode1(reginum[fv]);

	if ((! checker(registrant, namechk)) || (! checker(reginum, numchk)))
		registrant = "\?\"!\? ((.)";
	else
		registrant = registrant + " (" + reginum + ").";

}

void flesh_colours() {
	;        /* assembler;
asm
  mov ax,$1012;
  mov bx,21;                 { 21 = light pink (why?) */
	/* mov cx,1;
	mov dx,seg    @flesh;
	mov es,dx;
	mov dx,offset @flesh;
	int $10;

	mov dx,seg    @darkflesh;
	mov es,dx;
	mov dx,offset @darkflesh;
	mov bx,5;                 { 5 = dark pink. */
	/*int $10;

	jmp @TheEnd;

	@flesh:
	db 56,35,35;

	@darkflesh:
	db 43,22,22;

	@TheEnd: */
}

void setup() {
	integer gd, gm;
	file<fonttype> ff;

	if (paramstr(1) != "jsb")  exit(255);
	checkbreak = false;
	val(paramstr(2), storage_seg, gd);
	val(paramstr(3), storage_ofs, gd);

	assign(ff, "avalot.fnt");
	reset(ff);
	ff >> font;
	close(ff);

	gd = 3;
	gm = 1;
	initgraph(gd, gm, "");
	setvisualpage(1);

	icons();
}

void big(word x, word y, string z, boolean notted);
static word start, image;


static void generate(byte from, boolean &notted) {
	byte fv;

	image = 0;
	for (fv = 0; fv <= 7; fv ++)
		image += (from & (1 << fv)) << fv;

	image += image << 1;
	image = hi(image) + lo(image) * 256;
	if (notted)  image = ~ image;
}

void big(word x, word y, string z, boolean notted) {
	byte fv, ff;
	byte bit;

	start = x + y * 80;

	for (fv = 1; fv <= length(z); fv ++) {
		for (ff = 1; ff <= 12; ff ++) {
			generate(font[z[fv]][ff + 1], notted);
			for (bit = 0; bit <= 2; bit ++) {
				port[0x3c4] = 2;
				port[0x3ce] = 4;
				port[0x3c5] = 1 << bit;
				port[0x3cf] = bit;
				memw[0xa000 * start +   ff * 160] = image;
				memw[0xa000 * start + 80 + ff * 160] = image;
			}
		}
		start += 2;
	}
	bit = getpixel(0, 0);
}

void centre(integer y, string z) {
	big(40 - (length(z)), y, z, false);
}

void option(byte which, string what) {
	big(16, 41 + which * 33, string((char)(which + 48)) + ')', true);
	big(24, 41 + which * 33, what, true);
}

void invert(integer x1, integer y1, integer x2, integer y2) {
	word s;
	pointer p;

	s = imagesize(x1, y1, x2, y2);
	getmem(p, s);
	getimage(x1, y1, x2, y2, p);
	putimage(x1, y1, p, 4);
	sound(y1);
	delay(30);
	sound(600 - y2);
	delay(20);
	nosound;
	delay(200);
	putimage(x1, y1, p, 0);
	delay(250);
}

void wait() {
	word x;
	char r;
	boolean pressed;

	x = 0;
	pressed = false;
	do {
		setfillstyle(6, 15);
		bar(x  , 330, x - 1, 337);
		setfillstyle(1, 0);
		bar(x - 2, 330, x - 3, 337);
		delay(40);
		x += 1;

		if (keypressed()) {
			r = readkey();
			if (r == '\0') {
				r = readkey(); /* and...? */
			} else {
				/* Not an extended keystroke. */
				if (set::of(range('1', '6'), cspace, cescape, creturn, eos).has(r))  pressed = true;
			}
		}

	} while (!((x == 640) || pressed));

	if ((r == cspace) || (r == creturn))  r = '1';
	if (r == cescape)  r = '6';
	if (pressed) {
		result = ord(r) - 48;
		invert(48, 37 + result * 33, 114, 69 + result * 33);
	} else result = 177;
}

void show_up() {
	setvisualpage(0);
}

void loadmenu() {
	untyped_file f;
	byte bit;

	assign(f, "mainmenu.avd");
	reset(f, 1);
	for (bit = 0; bit <= 3; bit ++) {
		port[0x3c4] = 2;
		port[0x3ce] = 4;
		port[0x3c5] = 1 << bit;
		port[0x3cf] = bit;
		blockread(f, mem[0xa000 * 0], 59 * 80);
	}
	close(f);
	bit = getpixel(0, 0);
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	setup();
	loadmenu();
	load_regi_info();
	option(1, "Play the game.");
	option(2, "Read the background.");
	option(3, "Preview... perhaps...");
	option(4, "View the documentation.");
	option(5, "Registration info.");
	option(6, "Exit back to DOS.");
	centre(275, registrant);
	centre(303, "Make your choice, or wait for the demo.");

	show_up();
	wait();
	mem[storage_seg * storage_ofs] = result;
	closegraph();
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.