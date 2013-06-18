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

#define __trip3_implementation__
/* Project Minstrel- Trippancy routines */

#include "trip3.h"


#include "Graph.h"
/*#include "Crt.h"*/

/*$S+*/

namespace Avalanche {

const integer avvy = 1;
const integer test = 177;

struct triprec {
	byte handle; /* who is it? */
	integer x, y; /* current x&y */
	byte xm, ym; /* x&y margins */
	shortint ix, iy; /* inc x&y */
	byte stage; /* animation */
	integer xl, yl; /* x&y length */
	boolean prime; /* true on first move */
	boolean alive; /* true if it moves */
};

array<1, 10, triprec> tr;
byte tramt;
array<1, 20000, byte> blue3;
array<1, 24, word> pozzes;

const integer up = 1;
const integer right = 2;
const integer down = 3;
const integer left = 4;
const integer ur = 5;
const integer dr = 6;
const integer dl = 7;
const integer ul = 8;

/*                                                                              */
/*       EGA Graphic Primitive for Turbo Pascal 3.01A, Version 01FEB86.         */
/*       (C) 1986 by Kent Cedola, 2015 Meadow Lake Ct., Norfolk, VA, 23518      */
/*                                                                              */
/*       Description: Write a array of colors in a vertical line.  The current  */
/*       merge setting is used to control the combining of bits.                */
/*                                                                              */
void gpwtcol(void *buf, integer n) {    /* Cedola */
	;
	/* inline
	  ($1E/$A1/GDCUR_Y/$D1/$E0/$D1/$E0/$03/$06/GDCUR_Y/$05/$A000/$8E/$C0/$8B/$3E/
	   GDCUR_X/$8B/$CF/$D1/$EF/$D1/$EF/$D1/$EF/$BA/$03CE/$8A/$26/GDMERGE/$B0/$03/
	   $EF/$B8/$0205/$EF/$B0/$08/$EE/$42/$B0/$80/$80/$E1/$07/$D2/$C8/$EE/$8B/$4E/
	   $04/$C5/$76/$06/$8A/$24/$46/$26/$8A/$1D/$26/$88/$25/$83/$C7/$50/$E2/$F2/
	   $B0/$FF/$EE/$4A/$B8/>$05/$EF/$B8/>$03/$EF/$1F); */
}


void loadtrip() {
	untyped_file inf;
	;
	assign(inf, "t:avvy.trp");
	reset(inf, 1);
	seek(inf, 0x27);
	blockread(inf, pozzes, sizeof(pozzes));
	blockread(inf, blue3, sizeof(blue3));
	close(inf);
}

void enter(byte what_handle, integer xx, integer yy, integer lx, integer ly, byte mmx, byte mmy, byte st) {
	;
	tramt += 1;
	{
		triprec &with = tr[tramt];
		;
		with.handle = what_handle;
		with.ix = 0;
		with.iy = 0;
		with.x = xx;
		with.y = yy;
		with.xl = lx;
		with.yl = ly;
		with.xm = mmx;
		with.ym = mmy;
		with.stage = st;
		with.prime = true;
		with.alive = true;
	}
}

void plot(byte stage, integer ox, integer oy) { /* orig x & y. Page is always 1/UNSEEN. */
	byte x, y, len;
	word count;
	;
	count = pozzes[stage];
	do {
		len = blue3[count];
		if (len == 177)  return;
		x = blue3[count + 1];
		y = blue3[count + 2];
		count += 3;
		{
			;
			gdcur_x = x + ox;
			gdcur_y = y + oy;
			/* fiddle xy coords to match page 1 */
			gdcur_y += 205;   /* 203 */ gdcur_x -= 128; /* 114 */
			if (gdcur_x < 0) {
				;
				gdcur_x += 640;
				gdcur_y -= 1;
			}
			gpwtcol(&blue3[count], len);
			count += len;
		}
	} while (!false);
}

void trippancy() {
	byte fv;
	pointer p, saved1, saved2;
	word s;
	array<1, 10, pointer> q;
	boolean allstill;
	;
	if ((cw != 177) || (! dropsok) || keypressed())  return;

	/* Do the Avvy Walk */

	switch (dna.rw) {
	case up:
		budge(avvy, 0, -3, anim * 4 - 3);
		break;
	case down:
		budge(avvy, 0, 3, anim * 4 - 1);
		break;
	case right:
		budge(avvy, 5, 0, anim * 4 - 2);
		break;
	case left:
		budge(avvy, -5, 0, anim * 4);
		break;
	case ul:
		budge(avvy, -5, -3, anim * 4);
		break;
	case dl:
		budge(avvy, -5, 3, anim * 4);
		break;
	case ur:
		budge(avvy, 5, -3, anim * 4 - 2);
		break;
	case dr:
		budge(avvy, 5, 3, anim * 4 - 2);
		break;
	}

	for (fv = 1; fv <= tramt; fv ++) {
		triprec &with = tr[fv];
		boundscheck(with.x, with.y, with.xm, with.ym);
	}

	allstill = true;
	for (fv = 1; fv <= tramt; fv ++) {
		triprec &with = tr[fv];
		if (((with.alive) && (!((with.ix == 0) && (with.iy == 0)))) || with.prime)  allstill = false;
	}
	if (allstill)  return;

	if (dna.rw > 0) {
		;
		anim += 1;
		if (anim == 7)  anim = 1;
	}

	/* Trippancy Step 1 - Grab moon array of unmargined sprites (phew) */
	mark(saved1);
	setactivepage(1);
	off();
	for (fv = 1; fv <= tramt; fv ++) {
		triprec &with = tr[fv];
		;
		s = imagesize(with.x - with.xm, with.y - with.ym, with.x + with.xl + with.xm, with.y + with.yl + with.ym);
		getmem(q[fv], s);
		getimage(with.x - with.xm, with.y - with.ym, with.x + with.xl + with.xm, with.y + with.yl + with.ym, q[fv]);
	}
	/* Step 2 - Plot sprites on 1/UNSEEN */
	for (fv = 1; fv <= tramt; fv ++) {
		triprec &with = tr[fv];
		;
		plot(with.stage, with.x, with.y);
	}
	/* Step 3 - Copy all eligible from 1/UNSEEN to 0/SEEN */
	mark(saved2);
	for (fv = 1; fv <= tramt; fv ++) {
		triprec &with = tr[fv];
		if (((with.alive) && (!((with.ix == 0) && (with.iy == 0)))) || with.prime) {
			;
			s = imagesize(with.x - with.xm, with.y - with.ym, with.x + with.xl + with.xm, with.y + with.yl + with.ym);
			getmem(p, s);
			setactivepage(1);
			getimage(with.x - with.xm, with.y - with.ym, with.x + with.xl + with.xm, with.y + with.yl + with.ym, p);
			setactivepage(0);
			putimage(with.x - with.xm, with.y - with.ym, p, 0);
			release(saved2);
			with.prime = false;
		}
	}
	/* Step 4 - Unplot sprites from 1/UNSEEN */
	setactivepage(1);
	for (fv = 1; fv <= tramt; fv ++) {
		triprec &with = tr[fv];
		;
		putimage(with.x - with.xm, with.y - with.ym, q[fv], 0);
		if (with.ix != 0)  with.x = with.x + with.ix;
		if (with.iy != 0)  with.y = with.y + with.iy;
		with.ix = 0;
		with.iy = 0;
		if (with.handle == avvy)  {
			dnatype &with1 = dna;  ;
			ux = with.x;
			uy = with.y;
		}
	}
	on();
	release(saved1);
	for (fv = 1; fv <= tramt; fv ++) { /* synch xy coords of mouths */
		triprec &with = tr[fv];
		;
		mouths[fv].x = with.x + 20;
		mouths[fv].y = with.y;
	}

	setactivepage(0);
}

void budge(byte who, shortint xx, shortint yy, byte frame) { /* Moving & animation controller */
	byte fv;
	;
	for (fv = 1; fv <= tramt; fv ++) {
		triprec &with = tr[fv];
		if (with.handle == who) {
			;
			with.ix = xx;
			with.iy = yy;
			with.stage = frame;
		}
	}
}

void tripkey(char dir) {
	;
	if (cw != 177)  return;
	{
		dnatype &with = dna;
		;
		switch (dir) {
		case 'H':
			if (with.rw != up)     {
				;
				with.rw = up;
				ww = up;
			} else with.rw = 0;
			break;
		case 'P':
			if (with.rw != down)   {
				;
				with.rw = down;
				ww = down;
			} else with.rw = 0;
			break;
		case 'K':
			if (with.rw != left)   {
				;
				with.rw = left;
				ww = left;
			} else with.rw = 0;
			break;
		case 'M':
			if (with.rw != right)  {
				;
				with.rw = right;
				ww = right;
			} else with.rw = 0;
			break;
		case 'I':
			if (with.rw != ur)     {
				;
				with.rw = ur;
				ww = right;
			} else with.rw = 0;
			break;
		case 'Q':
			if (with.rw != dr)     {
				;
				with.rw = dr;
				ww = right;
			} else with.rw = 0;
			break;
		case 'O':
			if (with.rw != dl)     {
				;
				with.rw = dl;
				ww = left;
			} else with.rw = 0;
			break;
		case 'G':
			if (with.rw != ul)     {
				;
				with.rw = ul;
				ww = left;
			} else with.rw = 0;
			break;
		}
		if (with.rw == 0) {
			;
			ux = ppos[0][0];
			uy = ppos[0][1];
			anim -= 1;
			if (anim == 0)  anim = 6;
		}
	}
}

void boundscheck(integer &x, integer &y, byte xm, byte ym) {
	;
	if (y > 127 - ym)  y = 127 - ym;
	if (y < ym + 10)  y = ym + 10;
	if (x < xm)  x = xm;
	if (x > 640 - xm)  x = 640 - xm;
}

class unit_trip3_initialize {
public:
	unit_trip3_initialize();
};
static unit_trip3_initialize trip3_constructor;

unit_trip3_initialize::unit_trip3_initialize() {
	; /* init portion of Trip3 */
	tramt = 0;
}

} // End of namespace Avalanche.