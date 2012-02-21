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

#include "common/str.h"
#include "mortevielle/dialogs.h"
#include "mortevielle/keyboard.h"
#include "mortevielle/level15.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mor.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/parole2.h"
#include "mortevielle/taffich.h"

namespace Mortevielle {

static const int nligne = 7;

int Alert::show(const Common::String &msg, int n) {
	int coldep, esp, i, caseNumb, quoi, ix;
	Common::String st, chaine;
	int limit[3][3];
	char dumi;
	Common::String s[3];
	int cx, cy, nbcol, lignNumb;
	bool newaff, test, test1, test2, test3, dum;
	Common::String cas;

	// Make a copy of the current screen surface for later restore
	g_vm->_backgroundSurface.copyFrom(g_vm->_screenSurface);

	/*debug('** do_alert **');*/
	memset(&limit[0][0], 0, sizeof(int) * 3 * 3);
	int do_alert_result;
	hideMouse();
	while (keypressed())
		dumi = get_ch();	// input >> kbd >> dumi;

	g_vm->setMouseClick(false);
	decodeAlertDetails(msg, caseNumb, lignNumb, nbcol, chaine, cas);
	sauvecr(50, (nligne + 1) << 4);

	i = 0;
	if (chaine == "") {
		drawAlertBox(10, 5, nbcol);
	} else {
		drawAlertBox(8, 7, nbcol);
		i = 0;
		g_vm->_screenSurface._textPos.y = 70;
		do {
			cx = 320;
			st = "";
			while ((chaine[i + 1] != '\174') && (chaine[i + 1] != '\135')) {
				++i;
				st = st + chaine[i];
				if (res == 2)
					cx -= 3;
				else
					cx -= 5;
			}
			g_vm->_screenSurface.putxy(cx, g_vm->_screenSurface._textPos.y);
			g_vm->_screenSurface._textPos.y += 6;
			g_vm->_screenSurface.writeg(st, 4);
			++i;
		} while (!(chaine[i] == ']'));
	}
	if (caseNumb == 1)
		esp = nbcol - 40;
	else
		esp = (uint)(nbcol - caseNumb * 40) >> 1;
	coldep = 320 - ((uint)nbcol >> 1) + ((uint)esp >> 1);
	setButtonText(cas, coldep, caseNumb, &s[0], esp);
	limit[1][1] = ((uint)(coldep) >> 1) * res;
	limit[1][2] = limit[1][1] + 40;
	if (caseNumb == 1) {
		limit[2][1] = limit[2][2];
	} else {
		limit[2][1] = ((uint)(320 + ((uint)esp >> 1)) >> 1) * res;
		limit[2][2] = (limit[2][1]) + 40;
	}
	showMouse();
	quoi = 0;
	dum = false;
	do {
		dumi = '\377';
		moveMouse(dum, dumi);
		CHECK_QUIT0;

		cx = x_s;
		cy = y_s;
		test = (cy > 95) && (cy < 105);
		newaff = false;
		if (test) {
			test1 = (cx > limit[1][1]) && (cx < limit[1][2]);
			test2 = test1;
			if (caseNumb > 1)
				test2 = test1 || ((cx > limit[2][1]) && (cx < limit[2][2]));
			if (test2) {
				newaff = true;
				if (test1)
					ix = 1;
				else
					ix = 2;
				if (ix != quoi) {
					hideMouse();
					if (quoi != 0) {
						setPosition(quoi, coldep, esp);

						Common::String tmp(" ");
						tmp += s[quoi];
						tmp += " ";
						g_vm->_screenSurface.writeg(tmp, 0);
					}
					setPosition(ix, coldep, esp);

					Common::String tmp2 = " ";
					tmp2 += s[ix];
					tmp2 += " ";
					g_vm->_screenSurface.writeg(tmp2, 1);

					quoi = ix;
					showMouse();
				}
			}
		}
		if ((quoi != 0) && ! newaff) {
			hideMouse();
			setPosition(quoi, coldep, esp);

			Common::String tmp3(" ");
			tmp3 += s[quoi];
			tmp3 += " ";
			g_vm->_screenSurface.writeg(tmp3, 0);

			quoi = 0;
			showMouse();
		}
		test3 = (cy > 95) && (cy < 105) && (((cx > limit[1][1]) && (cx < limit[1][2]))
		                                    || ((cx > limit[2][1]) && (cx < limit[2][2])));
	} while (!g_vm->getMouseClick());
	g_vm->setMouseClick(false);
	hideMouse();
	if (!test3)  {
		quoi = n;
		setPosition(n, coldep, esp);
		Common::String tmp4(" ");
		tmp4 += s[n];
		tmp4 += " ";
		g_vm->_screenSurface.writeg(tmp4, 1);
	}
	charecr(50, succ(int, nligne) << 4);
	showMouse();

	/* Restore the background area */
	g_vm->_screenSurface.copyFrom(g_vm->_backgroundSurface, 0, 0);

	do_alert_result = quoi;
	return do_alert_result;
}

void Alert::decodeAlertDetails(Common::String s, int &nbc, int &lineNumb, int &col, Common::String &c, Common::String &cs) {
	int i, k;
	bool v;

	//val(s[2], nbc, i);
	nbc = atoi(s.c_str() + 1);
	i = 0;

	c = "";
	lineNumb = 0;
	i = 5;
	k = 0;
	v = true;
	col = 0;

	while (s[i] != ']') {
		c += s[i];
		if ((s[i] == '|') || (s[i + 1] == ']')) {
			if (k > col)
				col = k;
			k = 0;
			++lineNumb;
		} else if (s[i] != ' ')
			v = false;
		++i;
		++k;
	}
	if (v)  {
		c = "";
		col = 20;
	} else {
		c += ']';
		col += 6;
	}
	++i;
	cs = copy(s, i, 30);
	if (res == 2)
		col *= 6;
	else
		col *= 10;
}

void Alert::setPosition(int ji, int coldep, int esp) {
	g_vm->_screenSurface.putxy(coldep + (40 + esp) *pred(int, ji), 98);
}

/**
 * Alert function - Draw Alert Box
 * @remarks	Originally called 'fait_boite'
 */
void Alert::drawAlertBox(int lidep, int nli, int tx) {
	if (tx > 640)
		tx = 640;
	int x = 320 - ((uint)tx >> 1);
	int y = pred(int, lidep) << 3;
	int xx = x + tx;
	int yy = y + (nli << 3);
	g_vm->_screenSurface.fillRect(15, Common::Rect(x, y, xx, yy));
	g_vm->_screenSurface.fillRect(0, Common::Rect(x, y + 2, xx, y + 4));
	g_vm->_screenSurface.fillRect(0, Common::Rect(x, yy - 4, xx, yy - 2));
}


/**
 * Alert function - Set Button Text
 * @remarks	Originally called 'fait_choix'
 */
void Alert::setButtonText(Common::String c, int coldep, int nbcase, Common::String *str, int esp) {
	int i, l, x;
	char ch;

	i = 1;
	x = coldep;
	for (l = 1; l <= nbcase; ++l) {
		str[l] = "";
		do {
			++i;
			ch = c[i];
			str[l] += ch;
		} while (!(c[i + 1] == ']'));
		i += 2;

		while (str[l].size() < 3)
			str[l] += ' ';

		g_vm->_screenSurface.putxy(x, 98);

		Common::String tmp(" ");
		tmp += str[l];
		tmp += " ";

		g_vm->_screenSurface.writeg(tmp, 0);
		x += esp + 40;
	}
}

/*------------------------------------------------------------------------*/

bool Ques::show() {
	const int ta[11] = {0, 511, 516, 524, 531, 545, 552, 559, 563, 570, 576};
	const int ok[11] = {0, 4, 7, 1, 6, 4, 4, 2, 5, 3, 1 };

	bool q, func, test;
	int i, j, k, y, memk;
	int tay , tmax;
	int rep, prem, der;
	char st[1410];
	char key;
	rectangle coor[max_rect];
	Common::String chaines[15];
	int compte;


	bool ques_result;
	test = false;
	i = 0;
	compte = 0;

	do {
		hideMouse();
		hirs();
		showMouse();
		++i;
		deline(ta[i], st, tay);
		if (res == 1)
			y = 29;
		else
			y = 23;
		g_vm->_screenSurface.fillRect(15, Common::Rect(0, 14, 630, y));
		afftex(st, 20, 15, 100, 2, 0);
		if (i != 10) {
			prem = ta[i] + 1;
			der = ta[i + 1] - 1;
		} else {
			prem = 503;
			der = 510;
		}
		y = 35;
		tmax = 0;
		memk = 1;
		for (j = prem; j <= der; ++j) {
			deline(j, st, tay);
			if (tay > tmax)
				tmax = tay;
			afftex(st, 100, y, 100, 1, 0);
			chaines[memk] = delig;
			++memk;
			y += 8;
		}
		for (j = 1; j <= succ(int, der - prem); ++j) {
			rectangle &with = coor[j];

			with.x1 = 45 * res;
			with.x2 = (tmax * 3 + 55) * res;
			with.y1 = 27 + j * 8;
			with.y2 = 34 + j * 8;
			with.etat = true;

			while ((int)chaines[j].size() < tmax) {
				chaines[j] += ' ';
			}
		}
		coor[j + 1].etat = false;
		if (res == 1)
			rep = 10;
		else
			rep = 6;
		g_vm->_screenSurface.drawBox(80, 33, 40 + tmax * rep, (der - prem) * 8 + 16, 15);
		rep = 0;
		j = 0;
		memk = 0;
		do {
			g_vm->setMouseClick(false);
			tesok = false;
			moveMouse(func, key);
			CHECK_QUIT0;

			k = 1;
			while (coor[k].etat && ! dans_rect(coor[k]))  k = k + 1;
			if (coor[k].etat) {
				if ((memk != 0) && (memk != k)) {
//					for (j = 1; j <= tmax; ++j)
//						st[j] = chaines[memk][j];
					strncpy(st, chaines[memk].c_str(), tmax);
//
					st[1 + tmax] = '$';
					afftex(st, 100, 27 + memk * 8, 100, 1, 0);
				}
				if (memk != k) {
//					for (j = 1; j <= tmax; ++j)
//						st[j] = chaines[k][j];
					strncpy(st, chaines[k].c_str(), tmax);
					st[1 + tmax] = '$';
					afftex(st, 100, 27 + k * 8, 100, 1, 1);
					memk = k;
				}
			} else if (memk != 0) {
//				for (j = 1; j <= tmax; ++j)
//					st[j] = chaines[memk][j];
				strncpy(st, chaines[memk].c_str(), tmax);
				st[1 + tmax] = '$';
				afftex(st, 100, 27 + memk * 8, 100, 1, 0);
				memk = 0;
			}
		} while (!((memk != 0) && g_vm->getMouseClick()));
		if (memk == ok[i])
			++compte;
		else {
			if (i == 5)
				++i;
			if ((i == 7) || (i == 8))
				i = 10;
		}
		if (i == 10)
			q = /*testprot*/ true;
	} while (!(i == 10));
	ques_result = (compte == 10) && q;
	return ques_result;
}

/*------------------------------------------------------------------------*/

/**
 * Draw the F3/F8 dialog
 */
void f3f8::draw() {
	Common::String f3 = g_vm->getString(S_F3);
	Common::String f8 = g_vm->getString(S_F8);

	// Write the F3 and F8 text strings
	g_vm->_screenSurface.putxy(3, 44);
	g_vm->_screenSurface.writeg(f3, 5);
	g_vm->_screenSurface._textPos.y = 51;
	g_vm->_screenSurface.writeg(f8, 5);

	// Get the width of the written text strings
	int f3Width = g_vm->_screenSurface.getStringWidth(f3);
	int f8Width = g_vm->_screenSurface.getStringWidth(f8);

	// Write out the bounding box
	g_vm->_screenSurface.drawBox(0, 42, MAX(f3Width, f8Width) + 6, 16, 7);
}

void f3f8::divers(int np, bool b) {
	teskbd();
	do {
		parole(np, 0, 0);
		atf3f8(key);
		CHECK_QUIT;

		if (newgd != gd) {
			gd = newgd;
			hirs();
			aff50(b);
		}
	} while (!(key == 66));
}

void f3f8::atf3f8(int &key) {
	do {
		key = testou();
		CHECK_QUIT;
	} while (!((key == 61) || (key == 66)));
}

void f3f8::aff50(bool c) {
	caff = 50;
	_maff = 0;
	taffich();
	dessine(ades, 63, 12);
	if (c)
		ani50();
	else
		repon(2, c_paroles + 142);
	
	// Draw the f3/f8 dialog
	draw();
}

void f3f8::ani50() {
	crep = animof(1, 1);
	pictout(adani, crep, 63, 12);
	crep = animof(2, 1);
	pictout(adani, crep, 63, 12);
	f2_all = (res == 1);
	repon(2, c_paroles + 143);
}


} // End of namespace Mortevielle
