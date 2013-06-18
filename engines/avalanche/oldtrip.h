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

#ifndef __oldtrip_h__
#define __oldtrip_h__


#include "graph.h"
/*#include "Crt.h"*/

namespace Avalanche {

const integer maxgetset = 10;

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
	boolean quick, visible, homing, check_me;
	pointer behind; /* what's behind you */
	integer hx, hy; /* homing x & y coords */

	triptype *init(byte spritenum);   /* loads & sets up the sprite */
	void original();    /* just sets Quick to false */
	void andexor();    /* drops sprite onto screen 1 */
	void turn(byte whichway);      /* turns him round */
	void appear(integer wx, integer wy, byte wf); /* switches him on */
	void bounce();    /* bounces off walls. */
	void walk();    /* prepares for do_it, andexor, etc. */
	void do_it();    /* Actually copies the picture over */
	void getback();    /* gets background before sprite is drawn */
	void putback();    /* ...and wipes sprite from screen 1 */
	void walkto(integer xx, integer yy); /* home in on a point */
	void stophoming();    /* self-explanatory */
	void homestep();    /* calculates ix & iy for one homing step */
	void speed(shortint xx, shortint yy); /* sets ix & iy, non-homing, etc */
	void stopwalk();    /* Stops the sprite from moving */
	void chatter();    /* Sets up talk vars */
};

class getsettype {
public:
	array<1, maxgetset, fieldtype> gs;
	byte numleft;

	getsettype *init();
	void remember(fieldtype r);
	void recall();
};

const integer up = 0;
const integer right = 1;
const integer down = 2;
const integer left = 3;
const integer ur = 4;
const integer dr = 5;
const integer dl = 6;
const integer ul = 7;
const integer stopped = 8;

const integer numtr = 5; /* current max no. of sprites */

void trippancy();

void loadtrip();

void tripkey(char dir);

void apped(byte trn, byte np);

void fliproom(byte room, byte ped);

boolean infield(byte x);          /* returns True if you're within field "x" */

boolean neardoor();        /* returns True if you're near a door! */


#ifdef __trip4_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN array<1, numtr, triptype> tr;
#undef EXTERN
#define EXTERN extern

} // End of namespace Avalanche.

#endif