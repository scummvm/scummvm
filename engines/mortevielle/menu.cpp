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

#include "common/scummsys.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "mortevielle/menu.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

/* NIVEAU 14*/

/* overlay */ void menut(int no, Common::String nom) {
	byte h, l;
	Common::String s;


	/* debug('menut'); */
	h = hi(no);
	l = lo(no);
	s = nom;
	if (! tesok) {
		clrscr;
		mortevielle_exit(0);
	}
	while (length(s) < 30)  s = s + ' ';
	switch (h) {
	case invent  :
		if (l != 7)  inv[l] = string(' ') + s;
		break;
	case depla   :
		dep[l] = s;
		break;
	case action  :
		act[l] = s;
		break;
	case saction :
		self_[l] = s;
		break;
	case discut  :
		dis[l] = s;
		break;
	}
}

/* overlay */ void menu_disable(int no) {
	byte h, l;

	/* debug('menu_disable'); */
	h = hi(no);
	l = lo(no);
	switch (h) {
	case invent : {
		if (l > 6)  {
			inv[l][1] = '<';
			inv[l][22] = '>';
		} else inv[l][1] = '*';
	}
	break;
	case depla :
		dep[l][1] = '*';
		break;
	case action :
		act[l][1] = '*';
		break;
	case saction :
		self_[l][1] = '*';
		break;
	case discut :
		dis[l][1] = '*';
		break;
	}
}

/* overlay */ void menu_enable(int no) {
	byte h, l;

	/* debug('menu_disable'); */
	h = hi(no);
	l = lo(no);
	switch (h) {
	case invent : {
		inv[l][1] = ' ';
		inv[l][22] = ' ';
	}
	break;
	case depla :
		dep[l][1] = ' ';
		break;
	case action :
		act[l][1] = ' ';
		break;
	case saction : {
		self_[l][1] = ' ';
		self_[l][1] = ' ';
	}
	break;
	case discut :
		dis[l][1] = ' ';
		break;
	}
}

void menu_aff() {
	int ind_tabl, k, col;
	char c;
	int pt, x, y, color, msk,
	        num_letr;

	/* debug('menu_aff'); */
	hide_mouse();
	/*if not tesok then
	   begin
	     clrscr;
	     halt;
	   end;*/
	box(7, gd, 0, 0, 639, 10, 255);
	col = 28 * res;
	if (gd == cga)  color = 1;
	else color = 9;
	num_letr = 0;
	do {       /* lettre par lettre */
		num_letr = num_letr + 1;
		ind_tabl = 0;
		y = 1;
		do {      /* colonne par colonne */
			k = 0;
			x = col;
			do {     /* ligne par ligne */
				msk = 0x80;
				for (pt = 0; pt <= 7; pt ++) {
					if ((lettres[num_letr][ind_tabl] & msk) != 0) {
						putpix(gd, x + 1, y + 1, 0);
						putpix(gd, x, y + 1, 0);
						putpix(gd, x, y, color);
					}
					msk = (uint)msk >> 1;
					x = x + 1;
				}
				ind_tabl = succ(int, ind_tabl);
				k = succ(int, k);
			} while (!(k == 3));
			y = y + 1;
		} while (!(y == 9));
		col = col + 48 * res;
	} while (!(num_letr == 6));
	show_mouse();
}


void draw_menu() {
	/* debug('draw_menu'); */
	menu_aff();
	active_menu = true;
	msg4 = no_choice;
	msg3 = no_choice;
	choisi = false;
	clic = false;
	test0 = false;
}

void invers(int ix) {
	Common::String s;

	/* debug('invers'); */
	if (msg4 == no_choice)  return;
	putxy(don[msg3][1] << 3, succ(void, lo(msg4)) << 3);
	switch (msg3) {
	case 1 :
		s = inv[lo(msg4)];
		break;
	case 2 :
		s = dep[lo(msg4)];
		break;
	case 3 :
		s = act[lo(msg4)];
		break;
	case 4 :
		s = self_[lo(msg4)];
		break;
	case 5 :
		s = dis[lo(msg4)];
		break;
	case 6 :
		s = fic[lo(msg4)];
		break;
	case 7 :
		s = fic[1] + ' ' + chr(48 + lo(msg4));
		break;
	case 8 :
		if (lo(msg4) == 1)  s = recom;
		else s = fic[2] + ' ' + chr(47 + lo(msg4));
		break;
	}
	if ((s[1] != '*') && (s[1] != '<'))  writeg(s, ix);
	else msg4 = no_choice;
}

void util(int x, int y) {
	int ymx, dxcar, xmn, xmx, ix;

	/* debug('util'); */
	ymx = (don[msg3][4] << 3) + 16;
	dxcar = don[msg3][3];
	xmn = (don[msg3][1] << 2) * res;
	if (res == 1)  ix = 5;
	else ix = 3;
	xmx = dxcar * ix * res + xmn + 2;
	if ((x > xmn) && (x < xmx) && (y < ymx) && (y > 15)) {
		ix = pred(int, (uint)y >> 3) + (msg3 << 8);
		if (ix != msg4) {
			invers(1);
			msg4 = ix;
			invers(0);
		}
	} else if (msg4 != no_choice) {
		invers(1);
		msg4 = no_choice;
	}
}

void menu_down(int ii) {
	int cx, xcc;
	int xco, nb_lig;

	/* debug('menu_down'); */
	xco = don[ii][1];
	nb_lig = don[ii][4];
	hide_mouse();
	sauvecr(10, succ(byte, don[ii][2]) << 1);
	xco = xco << 3;
	if (res == 1)  cx = 10;
	else cx = 6;
	xcc = xco + (don[ii][3] * cx) + 6;
	box(15, gd, xco, 12, xcc, 10 + (don[ii][2] << 1), 255);
	box(0, gd, xcc, 12, xcc + 4, 10 + (don[ii][2] << 1), 255);
	box(0, gd, xco, 8 + (don[ii][2] << 1), xcc + 4, 12 + (don[ii][2] << 1), 255);
	putxy(xco, 16);
	cx = 0;
	do {
		cx = succ(int, cx);
		switch (ii) {
		case 1 :
			if (inv[cx][1] != '*')  writeg(inv[cx], 4);
			break;
		case 2 :
			if (dep[cx][1] != '*')  writeg(dep[cx], 4);
			break;
		case 3 :
			if (act[cx][1] != '*')  writeg(act[cx], 4);
			break;
		case 4 :
			if (self_[cx][1] != '*')  writeg(self_[cx], 4);
			break;
		case 5 :
			if (dis[cx][1] != '*')  writeg(dis[cx], 4);
			break;
		case 6 :
			writeg(fic[cx], 4);
			break;
		case 7 :
			writeg(fic[1] + ' ' + chr(48 + cx), 4);
			break;
		case 8 :
			if (cx == 1)  writeg(recom, 4);
			else writeg(fic[2] + ' ' + chr(47 + cx), 4);
			break;
		}
		putxy(xco, ywhere + 8);
	} while (!(cx == nb_lig));
	test0 = true;
	show_mouse();
}

void menu_up(int xx) {
	/* debug('menu_up'); */
	if (test0) {
		charecr(10, succ(byte, don[xx][2]) << 1);
		test0 = false;
	}
}

void erase_menu() {
	/* debug('erase_menu'); */
	active_menu = false;
	clic = false;
	menu_up(msg3);
}

void mdn() {
	int x, y, c, a, ix;
	bool tes;

	/* debug('mdn'); */
	if (! active_menu)  return;
	x = x_s;
	y = y_s;
	if (! clic) {
		if ((x == xprec) &&
		        (y == yprec))  return;
		else {
			xprec = x;
			yprec = y;
		}
		tes = (y < 11) && ((set::of(range(28 * res, 28 * res + 24), range(76 * res, 76 * res + 24), eos).has(x))
		                   || ((x > 124 * res) && (x < 124 * res + 24))
		                   || ((x > 172 * res) && (x < 172 * res + 24))
		                   || ((x > 220 * res) && (x < 220 * res + 24))
		                   || ((x > 268 * res) && (x < 268 * res + 24)));
		if (tes) {
			if (x < 76 * res)  ix = invent;
			else if (x < 124 * res)  ix = depla;
			else if (x < 172 * res)  ix = action;
			else if (x < 220 * res)  ix = saction;
			else if (x < 268 * res)  ix = discut;
			else ix = fichier;
			if ((ix != msg3) || (! test0))
				if (!((ix == fichier) && (set::of(sauve, charge, eos).has(msg3)))) {
					menu_up(msg3);
					menu_down(ix);
					msg3 = ix;
					msg4 = no_choice;
				}
		} else { /* Not in the MenuTitle line */
			if ((y > 11) && (test0))  util(x, y);
		}
	} else        /* il y a eu 'clic' */
		if ((msg3 == fichier) && (msg4 != no_choice)) {
			clic = false;
			menu_up(msg3);
			if (lo(msg4) == 1)  msg3 = 7;
			else msg3 = 8;
			menu_down(msg3);
		} else { /*  il y a eu clic sur un autre menu  */
			choisi = (test0) && (msg4 != no_choice);
			menu_up(msg3);
			msg[4] = msg4;
			msg[3] = msg3;
			msg3 = no_choice;
			msg4 = no_choice;
			clic = false;
		}
}

} // End of namespace Mortevielle
