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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1988-1989 Lankhor
 */

#include "mortevielle/var_mor.h"
#include "common/str.h"

namespace Mortevielle {

/*---------------------------------------------------------------------------*/
/*--------------------   PROCEDURES  AND  FONCTIONS   -----------------------*/
/*---------------------------------------------------------------------------*/

void hirs() {
	const byte tandy[14] = {113, 80, 90, 14, 63, 6, 50, 56, 2, 3, 6, 7, 0, 0};
	const byte herc[13] = {50, 40, 41, 9, 103, 3, 100, 100, 2, 3, 0, 0, 0};
	int i, j;

	switch (gd) {
	case cga : {
		graphcolormode;
		graphbackground(0);
		palette(1);
		res = 1;
	}
	break;
	case ams : {
		hires;
		inline_((float)(0xb8) / 6 / 0 /   /*  =>  mov ax,6 */
		        0xcd / 0x10);     /*  =>  int 16   */
		port[0x3d9] = 15;
		port[0x3df] = 0;
		port[0x3dd] = 15;
		res = 2;
	}
	break;
	case ega : {
		inline_((float)(0xb8) / 14 / 0 /     /*  MOV AX, 14   ; mode video 14 = 640*200 16 couleurs */
		        0xcd / 0x10);        /*  INT 16  */
		res = 2;
	}
	break;
	case her : {
		port[0x3bf] = 3;
		port[0x3b8] = 2;
		for (i = 0; i <= 12; i ++) {
			port[0x3b4] = i;
			port[0x3b5] = herc[i];
		}
		inline_((float)(0xfc) / 0xb9 / 0 / 0x80 / 0xb8 / 0 / 0xb0 / 0x8e / 0xc0 / 0x31 / 0xff / 0x31 / 0xc0 / 0xf3 / 0xab);
		port[0x3b8] = 10;
		res = 2;
	}
	break;
	case tan : {
		port[0x3d8] = 0;
		port[0x3da] = 3;
		port[0x3de] = 0x14;
		for (i = 0; i <= 13; i ++) {
			port[0x3d4] = i;
			port[0x3d5] = tandy[i];
		}
		port[0x3da] = 2;
		port[0x3de] = 0;
		port[0x3d8] = port[0x3d8] | (11 & 0xef);
		port[0x3dd] = port[0x3dd] | 1;
		port[0x3df] = port[0x3df] | 0xc0;
		box(0, gd, 0, 0, 640, 200, 255);
		res = 1;
	}
	break;
	}
}

/* procedure affput(Chx,Gd,x,y,coul,char:int); external 'c:\mc\divaf.com'; */

void affcar(int gd, int x, int y, int coul, int chr) {
	if (res == 1)  affput(1, gd, ((cardinal)x >> 1), y, coul, chr);
	else affput(1, gd, x, y, coul, chr);
}

void putpix(int gd, int x, int y, int coul) {
	affput(0, gd, x, y, coul, 0);
}

} // End of namespace Mortevielle
