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

#include "mortevielle/outtext.h"
#include "mortevielle/graphics.h"
#include "mortevielle/level15.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/var_mor.h"
#include "common/str.h"

namespace Mortevielle {

Common::String delig;
const byte tabdr[32] = {
	32, 101, 115,  97, 114, 105, 110,
	117, 116, 111, 108,  13, 100,  99,
	112, 109,  46, 118, 130,  39, 102,
	98,  44, 113, 104, 103,  33,  76,
	85, 106,  30,  31
};


const byte tab30[32] = {
	69,  67,  74, 138, 133, 120,  77, 122,
	121,  68,  65,  63,  73,  80,  83,  82,
	156,  45,  58,  79,  49,  86,  78,  84,
	71,  81,  64,  66, 135,  34, 136,  91
};



const byte tab31[32]= {
	93,  47,  48,  53,  50,  70, 124,  75,
	72, 147, 140, 150, 151,  57,  56,  51,
	107, 139,  55,  89, 131,  37,  54,  88,
	119,   0,   0,   0,   0,   0,   0,   0
};

void deline(int num , char *l , int &tl);
typedef unsigned char uchar;

static void cinq_huit(char &c, int &idx, byte &pt, bool &the_end) {
/*
	const uchar rap[32] = { 
		(uchar)',', (uchar)':', (uchar)'@', (uchar)'!', (uchar)'?', (uchar)'-', (uchar)'\207',
		(uchar)'\240', (uchar)'\205', (uchar)'\203', (uchar)'\202', (uchar)'\212', (uchar)'\210', (uchar)'/',
		(uchar)'\213', (uchar)'\214', (uchar)'\242', (uchar)'\047', (uchar)'\223', (uchar)'"', (uchar)'\227',
		(uchar)'\226', (uchar)'0', (uchar)'1', (uchar)'2', (uchar)'3', (uchar)'4', (uchar)'5',
		(uchar)'6', (uchar)'7', (uchar)'8', (uchar)'9'
	};
*/
	uint16 oct, ocd;

	/* 5-8 */
	oct = t_mot[idx];
	oct = ((uint16)(oct << (16 - pt))) >> (16 - pt);
	if (pt < 6) {
		++idx;
		oct = oct << (5 - pt);
		pt += 11;
		oct = oct | ((uint)t_mot[idx] >> pt);
	} else {
		pt -= 5;
		oct = (uint)oct >> pt;
	}

	switch (oct) {
	case 11:
		c = '$';
		the_end = true;
		break;
	case 30:
	case 31:
		ocd = t_mot[idx];
		ocd = (uint16)(ocd << (16 - pt)) >> (16 - pt);
		if (pt < 6) {
			++idx;
			ocd = ocd << (5 - pt);
			pt += 11;
			ocd = ocd | ((uint)t_mot[idx] >> pt);
		} else {
			pt -= 5;
			ocd = (uint)ocd >> pt;
		}

		if (oct == 30)
			c = chr(tab30[ocd]);
		else
			c = chr(tab31[ocd]);

		if (c == '\0') {
			the_end = true;
			c = '#';
		}
		break;
	default:
		c = chr(tabdr[oct]);
		break;
	}
}              /* 5-8 */

/**
 * Decode and extract the line with the given Id
 */
void deline(int num, char *l , int &tl) {
	if (num < 0) {
		warning("deline: num < 0! Skipping");
		return;
	}

	// DETEX
	delig = "";
	int ts = t_rec[num].indis;
	byte ps = t_rec[num].point;
	int i = ts;
	tl = 1;
	int j = 1;
	byte k = ps;
	bool endFl = false;
	char let;
	do {
		cinq_huit(let, i, k, endFl);
		l[j] = let;
		if (j < 254)
			delig += let;
		++j;
	} while (!endFl);
	tl = j - 1;
	if (tl < 255)
		// Remove trailing '$'
		delig.deleteLastChar();
}


void afftex(char *ch, int x, int y, int dx, int dy, int typ);


static int l_motsuiv(int p, char *ch, int &tab) {
	int c = p;

	while ((ch[p] != ' ') && (ch[p] != '$') && (ch[p] != '@'))
		++p;

	return tab * (p - c);
}

void afftex(char *ch, int x, int y, int dx, int dy, int typ) {
	bool the_end;
	int tab;
	Common::String s;
	int i, j;


	/*    debug('  .. Afftex');*/
	g_vm->_screenSurface.putxy(x, y);
	if (res == 1)
		tab = 10;
	else
		tab = 6;
	dx *= 6;
	dy *= 6;
	int xc = x;
	int yc = y;
	int xf = x + dx;
	int yf = y + dy;
	int p = 1;
	the_end = (ch[p] == '$');
	s = "";
	while (!the_end) {
		switch (ch[p]) {
		case '@':
			g_vm->_screenSurface.writeg(s, typ);
			s = "";
			++p;
			xc = x;
			yc += 6;
			g_vm->_screenSurface.putxy(xc, yc);
			break;
		case ' ':
			s += ' ';
			xc += tab;
			++p;
			if (l_motsuiv(p, ch, tab) + xc > xf) {
				g_vm->_screenSurface.writeg(s, typ);
				s = "";
				xc = x;
				yc += 6;
				if (yc > yf) {
					do {
						;
					} while (!keypressed());
					i = y;
					do {
						j = x;
						do {
							g_vm->_screenSurface.putxy(j, i);
							g_vm->_screenSurface.writeg(" ", 0);
							j += 6;
						} while (!(j > xf));
						i += 6;
					} while (!(i > yf));
					yc = y;
				}
				g_vm->_screenSurface.putxy(xc, yc);
			}
			break;
		case '$':
			the_end = true;
			g_vm->_screenSurface.writeg(s, typ);
			break;
		default:
			s += ch[p];
			++p;
			xc += tab;
			break;
		}     /* case */
	}
}

} // End of namespace Mortevielle
