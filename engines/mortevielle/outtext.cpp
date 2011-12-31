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
#include "mortevielle/level15.h"
#include "mortevielle/sprint.h"
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

static void cinq_huit(char &c, int &ind, byte &pt, bool &the_end) {
	const uchar rap[32] = { 
		(uchar)',', (uchar)':', (uchar)'@', (uchar)'!', (uchar)'?', (uchar)'-', (uchar)'\207',
		(uchar)'\240', (uchar)'\205', (uchar)'\203', (uchar)'\202', (uchar)'\212', (uchar)'\210', (uchar)'/',
		(uchar)'\213', (uchar)'\214', (uchar)'\242', (uchar)'\047', (uchar)'\223', (uchar)'"', (uchar)'\227',
		(uchar)'\226', (uchar)'0', (uchar)'1', (uchar)'2', (uchar)'3', (uchar)'4', (uchar)'5',
		(uchar)'6', (uchar)'7', (uchar)'8', (uchar)'9'
	};
	int oct, ocd;

	/* 5-8 */
	oct = t_mot[ind];
	oct = (uint)(oct << (16 - pt)) >> (16 - pt);
	if (pt < 6) {
		ind = ind + 1;
		oct = oct << (5 - pt);
		pt = pt + 11;
		oct = oct | ((uint)t_mot[ind] >> pt);
	} else {
		pt = pt - 5;
		oct = (uint)oct >> pt;
	}

	switch (oct) {
	case 11 : {
		c = '$';
		the_end = true;
	}
	break;
	case 30:
	case 31 : {
		ocd = t_mot[ind];
		ocd = (uint)(ocd << (16 - pt)) >> (16 - pt);
		if (pt < 6) {
			ind = ind + 1;
			ocd = ocd << (5 - pt);
			pt = pt + 11;
			ocd = ocd | ((uint)t_mot[ind] >> pt);
		} else {
			pt = pt - 5;
			ocd = (uint)ocd >> pt;
		}
		if (oct == 30)  c = chr(tab30[ocd]);
		else c = chr(tab31[ocd]);
		if (c == '\0') {
			the_end = true;
			c = '#';
		}
	}
	break;
	default:
		c = chr(tabdr[oct]);
	}
}              /* 5-8 */

void deline(int num , char *l , int &tl) {
	int i, j, ts;
	char let;
	byte ps, k;
	bool the_end;

	/* DETEX */
	/*debug('  => DeLine');*/
	delig = "";
	ts = t_rec[num].indis;
	ps = t_rec[num].point;
	i = ts;
	tl = 1;
	j = 1;
	k = ps;
	the_end = false;
	do {
		cinq_huit(let, i, k, the_end);
		l[j] = let;
		if (j < 254)  delig = delig + let;
		j = j + 1;
	} while (!the_end);
	tl = j - 1;
	if (tl < 255)  delig = copy(delig, 1, tl - 1); /* enleve le $ */
}       /* DETEX */


void afftex(char *ch, int x, int y, int dx, int dy, int typ);


static int l_motsuiv(int p, char *ch, int &tab) {
	int c;

	int l_motsuiv_result;
	c = p;
	while ((ch[p] != ' ') && (ch[p] != '$') && (ch[p] != '@'))
		++p;
	l_motsuiv_result = tab * (p - c);
	return l_motsuiv_result;
}

void afftex(char *ch, int x, int y, int dx, int dy, int typ) {
	bool the_end;
	char touch;
	int xf, yf;
	int xc, yc;
	int tab, p;
	Common::String s;
	int i, j, nt;


	/*    debug('  .. Afftex');*/
	putxy(x, y);
	if (res == 1)  tab = 10;
	else tab = 6;
	dx = dx * 6;
	dy = dy * 6;
	xc = x;
	yc = y;
	xf = x + dx;
	yf = y + dy;
	p = 1;
	the_end = (ch[p] == '$');
	s = "";
	while (! the_end) {
		switch (ch[p]) {
		case '@' : {
			writeg(s, typ);
			s = "";
			p = p + 1;
			xc = x;
			yc = yc + 6;
			putxy(xc, yc);
		}
		break;
		case ' ' : {
			s = s + ' ';
			xc = xc + tab;
			p = p + 1;
			if (l_motsuiv(p, ch, tab) + xc > xf) {
				writeg(s, typ);
				s = "";
				xc = x;
				yc = yc + 6;
				if (yc > yf) {
					do {
						;
					} while (!keypressed());
					i = y;
					do {
						j = x;
						do {
							putxy(j, i);
							writeg(" ", 0);
							j = j + 6;
						} while (!(j > xf));
						i = i + 6;
					} while (!(i > yf));
					yc = y;
				}
				putxy(xc, yc);
			}
		}
		break;
		case '$' : {
			the_end = true;
			writeg(s, typ);
		}
		break;
		default: {
			s = s + ch[p];
			p = p + 1;
			xc = xc + tab;
		}
		}     /* case */
	}
}

} // End of namespace Mortevielle
