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

/* Trip Oop (Trippancy 4 Andexor */
#include "graph.h"
/*#include "Crt.h"*/

namespace Avalanche {

const integer up = 0;
const integer right = 1;
const integer down = 2;
const integer left = 3;

const integer numtr = 1; /* current max no. of sprites */

struct adxtype {
	varying_string<12> name; /* name of character */
	byte num; /* number of pictures */
	byte xl, yl; /* x & y lengths of pictures */
	byte seq; /* how many in one stride */
	word size; /* the size of one picture */
	byte fgc, bgc; /* foreground & background bubble colours */
};

class triptype {
public:
	adxtype a; /* vital statistics */
	byte face, step;
	integer x, y; /* current xy coords */
	integer ox, oy; /* last xy coords */
	integer tax, tay; /* "behind" taken at... */
	shortint ix, iy; /* amount to move sprite by, each step */
	matrix<1, 24, 0, 1, pointer> pic; /* the pictures themselves */
	boolean quick, visible, homing;
	pointer behind; /* what's behind you */
	integer hx, hy; /* homing x & y coords */

	triptype *init(byte spritenum);   /* loads & sets up the sprite */
	void original();    /* just sets Quick to false */
	void andexor();    /* drops sprite onto screen 1 */
	void turn(byte whichway);      /* turns him round */
	void appear(integer wx, integer wy, byte wf); /* switches him on */
	void walk();    /* prepares for do_it, andexor, etc. */
	void do_it();    /* Actually copies the picture over */
	void getback();    /* gets background before sprite is drawn */
	void putback();    /* ...and wipes sprite from screen 1 */
	void walkto(integer xx, integer yy); /* home in on a point */
	void stophoming();    /* self-explanatory */
	void homestep();    /* calculates ix & iy for one homing step */
	void speed(shortint xx, shortint yy); /* sets ix & iy, non-homing, etc */
	void halt();    /* Stops the sprite from moving */
};

integer gd, gm;
array<1, 1, triptype> tr;

void copier(integer x1, integer y1, integer x2, integer y2, integer x3, integer y3, integer x4, integer y4);


static boolean dropin(integer xc, integer yc, integer x1, integer y1, integer x2, integer y2)
/* Dropin returns True if the point xc,yc falls within the 1-2 rectangle. */
{
	boolean dropin_result;
	;
	dropin_result = ((xc >= x1) && (xc <= x2) && (yc >= y1) && (yc <= y2));
	return dropin_result;
}



static void transfer(integer x1, integer y1, integer x2, integer y2) {
	pointer p, q;
	word s;
	;
	s = imagesize(x1, y1, x2, y2);
	setfillstyle(1, 0);
	mark(q);
	getmem(p, s);
	setactivepage(1);
	getimage(x1, y1, x2, y2, p);
	setactivepage(0);
	putimage(x1, y1, p, copyput);
	setactivepage(1);
	release(q);
}



static integer lesser(integer a, integer b) {
	integer lesser_result;
	;
	if (a < b)  lesser_result = a;
	else lesser_result = b;
	return lesser_result;
}



static integer greater(integer a, integer b) {
	integer greater_result;
	;
	if (a > b)  greater_result = a;
	else greater_result = b;
	return greater_result;
}

void copier(integer x1, integer y1, integer x2, integer y2, integer x3, integer y3, integer x4, integer y4)

{
	;
	if (dropin(x3, y3, x1, y1, x2, y2)
	        || dropin(x3, y4, x1, y1, x2, y2)
	        || dropin(x4, y3, x1, y1, x2, y2)
	        || dropin(x4, y4, x1, y1, x2, y2)) {
		;     /* Overlaps */
		transfer(lesser(x1, x3), lesser(y1, y3), greater(x2, x4), greater(y2, y4));
	} else {
		;     /* Doesn't overlap- copy both of them seperately */
		transfer(x3, y3, x4, y4); /* backwards- why not...? */
		transfer(x1, y1, x2, y2);
	}
}

void setup() {
	integer gd, gm;
	;
	gd = 3;
	gm = 0;
	initgraph(gd, gm, "");
	for (gd = 0; gd <= 1; gd ++) {
		;
		setactivepage(gd);
		setfillstyle(9, 1);
		bar(0, 0, 640, 200);
	}
	for (gm = 1; gm <= numtr; gm ++) tr[gm].original();
}

triptype *triptype::init(byte spritenum) {
	integer gd, gm;
	word s;
	untyped_file f;
	varying_string<2> xx;
	pointer p, q;
	word bigsize;
	byte sort, n;
	;
	str(spritenum, xx);
	assign(f, string("v:sprite") + xx + ".avd");
	reset(f, 1);
	seek(f, 59);
	blockread(f, a, sizeof(a));
	blockread(f, bigsize, 2);
	setvisualpage(3);
	setactivepage(3);
	for (sort = 0; sort <= 1; sort ++) {
		;
		mark(q);
		getmem(p, bigsize);
		blockread(f, p, bigsize);
		putimage(0, 0, p, 0);
		release(q);
		n = 1;
		{
			adxtype &with = a;
			for (gm = 0; gm <= (with.num / with.seq) - 1; gm ++) /* directions */
				for (gd = 0; gd <= with.seq - 1; gd ++) { /* steps */
					;
					getmem(pic[n][sort], a.size); /* grab the memory */
					getimage((gm / 2) * (with.xl * 6) + gd * with.xl, (gm % 2)*with.yl,
					         (gm / 2) * (with.xl * 6) + gd * with.xl + with.xl - 1, (gm % 2)*with.yl + with.yl - 1,
					         pic[n][sort]); /* grab the pic */
					putimage((gm / 2) * (with.xl * 6) + gd * with.xl, (gm % 2)*with.yl,
					         pic[n][sort], notput); /* test the pic */
					n += 1;
				}
		}
	}
	close(f);
	setactivepage(0);
	setvisualpage(0);
	x = 0;
	y = 0;
	quick = true;
	visible = false;
	getmem(behind, a.size);
	homing = false;
	ix = 0;
	iy = 0;
	return this;
}

void triptype::original() {
	;
	quick = false;
}

void triptype::getback() {
	;
	tax = x;
	tay = y;
	getimage(x, y, x + a.xl, y + a.yl, behind);
}

void triptype::andexor() {
	byte picnum; /* Picnum, Picnic, what ye heck */
	;
	picnum = face * a.seq + step + 1;
	putimage(x, y, pic[picnum][0], andput);
	putimage(x, y, pic[picnum][1], xorput);
}

void triptype::turn(byte whichway) {
	;
	face = whichway;
	step = 0;
}

void triptype::appear(integer wx, integer wy, byte wf) {
	;
	x = wx;
	y = wy;
	ox = wx;
	oy = wy;
	turn(wf);
	visible = true;
}

void triptype::walk() {
	;
	ox = x;
	oy = y;
	if (homing)  homestep();
	x = x + ix;
	y = y + iy;
	step += 1;
	if (step == a.seq)  step = 0;
	getback();
}

void triptype::do_it() {
	;
	copier(ox, oy, ox + a.xl, oy + a.yl, x, y, x + a.xl, y + a.yl);
}

void triptype::putback() {
	;
	putimage(tax, tay, behind, 0);
}

void triptype::walkto(integer xx, integer yy) {
	;
	speed(xx - x, yy - y);
	hx = xx;
	hy = yy;
	homing = true;
}

void triptype::stophoming() {
	;
	homing = false;
}

void triptype::homestep() {
	integer temp;
	;
	if ((hx == x) && (hy == y)) {
		;     /* touching the target */
		homing = false;
		return;
	}
	ix = 0;
	iy = 0;
	if (hy != y) {
		;
		temp = hy - y;
		if (temp > 4)  iy = 4;
		else if (temp < -4)  iy = -4;
		else iy = temp;
	}
	if (hx != x) {
		;
		temp = hx - x;
		if (temp > 4)  ix = 4;
		else if (temp < -4)  ix = -4;
		else ix = temp;
	}
}

void triptype::speed(shortint xx, shortint yy) {
	;
	ix = xx;
	iy = yy;
	if ((ix == 0) && (iy == 0))  return; /* no movement */
	if (ix == 0) {
		;     /* No horz movement */
		if (iy < 0)  turn(up);
		else turn(down);
	} else {
		;
		if (ix < 0)  turn(left);
		else turn(right);
	}
}

void triptype::halt() {
	;
	ix = 0;
	iy = 0;
	homing = false;
}

void trip() {
	byte fv;
	;
	for (fv = 1; fv <= numtr; fv ++) {
		triptype &with = tr[fv];
		;
		walk();
		if (with.quick && with.visible)  andexor();
		do_it();
		putback();
	}
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	;
	setup();
	{
		triptype &with = tr[1];
		;
		init(1);
		appear(600, 100, left);
		do {
			/*
			speed(-5,0); repeat trip until keypressed or (x=  0);
			speed( 5,0); repeat trip until keypressed or (x=600);
			*/
			walkto(10, 10);
			do {
				trip();
			} while (!(keypressed() || ! with.homing));
			walkto(70, 150);
			do {
				trip();
			} while (!(keypressed() || ! with.homing));
			walkto(600, 77);
			do {
				trip();
			} while (!(keypressed() || ! with.homing));
		} while (!keypressed());
	}
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.