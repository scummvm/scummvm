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

#include "common/endian.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "mortevielle/dialogs.h"
#include "mortevielle/graphics.h"
#include "mortevielle/level15.h"
#include "mortevielle/menu.h"
#include "mortevielle/mor.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/parole2.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

// For ScummVM, we need to do check for file errors where we do the file access
const int ioresult = 0;

void testfi() {
	if (ioresult != 0) {
		caff = Alert::show(err_mess, 1);
		g_vm->quitGame();
	}
}

/**
 * Read the current system time
 */
int readclock() {
	TimeDate dateTime;
	g_system->getTimeAndDate(dateTime);

	int m = dateTime.tm_min * 60;
	int h = dateTime.tm_hour * 3600;
	return h + m + dateTime.tm_sec;
}

void modif(int &nu) {
	if (nu == 26)
		nu = 25;
	else if ((nu > 29) && (nu < 36))
		nu -= 4;
	else if ((nu > 69) && (nu < 78))
		nu -= 37;
	else if ((nu > 99) && (nu < 194))
		nu -= 59;
	else if ((nu > 996) && (nu < 1000))
		nu -= 862;
	else if ((nu > 1500) && (nu < 1507))
		nu -= 1363;
	else if ((nu > 1507) && (nu < 1513))
		nu -= 1364;
	else if ((nu > 1999) && (nu < 2002))
		nu -= 1851;
	else if (nu == 2010)
		nu = 151;
	else if ((nu > 2011) && (nu < 2025))
		nu -= 1860;
	else if (nu == 2026)
		nu = 165;
	else if ((nu > 2029) && (nu < 2037))
		nu -= 1864;
	else if ((nu > 3000) && (nu < 3005))
		nu -= 2828;
	else if (nu == 4100)
		nu = 177;
	else if (nu == 4150)
		nu = 178;
	else if ((nu > 4151) && (nu < 4156))
		nu -= 3973;
	else if (nu == 4157)
		nu = 183;
	else if ((nu == 4160) || (nu == 4161))
		nu -= 3976;
}


void dessine(int ad, int x, int y) {
	hideMouse();
	writepal(numpal);
	pictout(ad, 0, x, y);
	showMouse();
}

void dessine_rouleau() {
	writepal(89);
	if (_currGraphicalDevice == MODE_HERCULES) {
		mem[0x7000 * 16 + 14] = 15;
	}
	hideMouse();
	pictout(0x73a2, 0, 0, 0);
	showMouse();
}


void text_color(int c) {
	color_txt = c;
}

/* NIVEAU 13 */


void text1(int x, int y, int nb, int m) {
	char st[1410];
	int tay;
	int co;


	/* debug('text'); */
	if (res == 1)
		co = 10;
	else
		co = 6;
	deline(m, st, tay);
	if ((y == 182) && (tay * co > nb * 6))
		y = 176;
	afftex(st, x, y, nb, 20, color_txt);
}

void initouv() {
	for (int cx = 1; cx <= 7; cx++)
		touv[cx] = chr(0);
}

void ecrf1() {
	// Large drawing
	g_vm->_screenSurface.drawBox(0, 11, 512, 163, 15);
}

void clsf1() {
	hideMouse();
	g_vm->_screenSurface.fillRect(0, Common::Rect(0, 11, 514, 175));

	showMouse();
}

void clsf2() {
	hideMouse();
	if (f2_all) {
		g_vm->_screenSurface.fillRect(0, Common::Rect(1, 176, 633, 199));
		g_vm->_screenSurface.drawBox(0, 175, 634, 24, 15);
		f2_all = false;
	} else {
		g_vm->_screenSurface.fillRect(0, Common::Rect(1, 176, 633, 190));
		g_vm->_screenSurface.drawBox(0, 175, 634, 15, 15);
	}
	showMouse();
}

void ecrf2() {
	text_color(5);
}

void ecr2(Common::String str_) {
	int tab;

	if (res == 1)
		tab = 10;
	else
		tab = 6;
	g_vm->_screenSurface.putxy(8, 177);
	int tlig = 59 + pred(int, res) * 36;
	if ((int)str_.size() < tlig)
		g_vm->_screenSurface.writeg(str_, 5);
	else if ((int)str_.size() < (tlig << 1)) {
		g_vm->_screenSurface.putxy(8, 176);
		g_vm->_screenSurface.writeg(copy(str_, 1, pred(int, tlig)), 5);
		g_vm->_screenSurface.putxy(8, 182);
		g_vm->_screenSurface.writeg(copy(str_, tlig, tlig << 1), 5);
	} else {
		f2_all = true;
		clsf2();
		g_vm->_screenSurface.putxy(8, 176);
		g_vm->_screenSurface.writeg(copy(str_, 1, pred(int, tlig)), 5);
		g_vm->_screenSurface.putxy(8, 182);
		g_vm->_screenSurface.writeg(copy(str_, tlig, pred(int, (tlig << 1))), 5);
		g_vm->_screenSurface.putxy(8, 190);
		g_vm->_screenSurface.writeg(copy(str_, tlig << 1, tlig * 3), 5);
	}
}

void clsf3() {
	hideMouse();
	g_vm->_screenSurface.fillRect(0, Common::Rect(1, 192, 633, 199));
	g_vm->_screenSurface.drawBox(0, 191, 634, 8, 15);
	showMouse();
}

void ecr3(Common::String text) {
	clsf3();
	g_vm->_screenSurface.putxy(8, 192);
	g_vm->_screenSurface.writeg(text, 5);
}

void ecrf6() {
	text_color(5);
	g_vm->_screenSurface.drawBox(62, 33, 363, 80, 15);
}

void ecrf7() {
	text_color(4);
}

void clsf10() {
	int co, cod;
	Common::String st;

	hideMouse();
	if (res == 1) {
		co = 634;
		cod = 534;
	} else {
		co = 600;
		cod = 544;
	}
	g_vm->_screenSurface.fillRect(15, Common::Rect(cod, 93, co, 98));
	if (s.conf < 33)
		st = g_vm->getString(S_COOL);
	else if (s.conf < 66)
		st = g_vm->getString(S_LOURDE);
	else if (s.conf > 65)
		st = g_vm->getString(S_MALSAINE);
	
	co = 580 - (g_vm->_screenSurface.getStringWidth(st) / 2);
	g_vm->_screenSurface.putxy(co, 92);
	g_vm->_screenSurface.writeg(st, 4);

	if (res == 1)
		co = 620;
	else
		co = 584;

	g_vm->_screenSurface.fillRect(15, Common::Rect(560, 24, co, 86));
	/* rempli(69,12,32,5,255);*/
	showMouse();
}

void stop() {
	hirs();
	_currGraphicalDevice = MODE_AMSTRAD1512;
	hirs();
	g_vm->quitGame();
}

void paint_rect(int x, int y, int dx, int dy) {
	int co;

	if (_currGraphicalDevice == MODE_CGA)
		co = 3;
	else
		co = 11;
	g_vm->_screenSurface.fillRect(co, Common::Rect(x, y, x + dx, y + dy));
}

int hazard(int min, int max) {
	return get_random_number(min, max);
}

void calch(int &j, int &h, int &m) {
	int nh = readclock();
	int th = jh + ((nh - mh) / t);
	m = ((th % 2) + vm) * 30;
	h = ((uint)th >> 1) + vh;
	if (m == 60) {
		m = 0;
		h = h + 1;
	}
	j = (h / 24) + vj;
	h = h - ((j - vj) * 24);
}

void conv(int x, int &y) {
	int cx = 1;
	y = 128;
	while (cx < x) {
		y = (uint)y >> 1;
		cx = succ(int, cx);
	}
}

/* NIVEAU 12 */
void okpas() {
	tesok = true;
}

void modobj(int m) {
	char str_[1410];
	int tay;

	Common::String strp = Common::String(' ');
	if (m != 500) {
		deline(m - 501 + c_st41, str_, tay);
		strp = delig;
	}
	g_vm->_menu.menut(g_vm->_menu._invt[8], strp);
	g_vm->_menu.disableMenuItem(g_vm->_menu._invt[8]);
}

void modobj2(int m, bool t1, bool t2) {
	char str_[1410];
	int tay;

	Common::String strp = Common::String(' ');
	if (t1 || t2)
		okpas();
	else
		tesok = false;;
	if (m != 500) {
		deline(m - 501 + c_st41, str_, tay);
		strp = delig;
	}
	g_vm->_menu.menut(g_vm->_menu._invt[8], strp);
	g_vm->_menu.disableMenuItem(g_vm->_menu._invt[8]);
}


void repon(int f, int m) {
	Common::String str_;
	Common::String str1;
	char st[1410];
	int i, xco, dx, caspe, tay;

	if ((m > 499) && (m < 563)) {
		deline(m - 501 + c_st41, st, tay);
		if (tay > ((58 + pred(int, res) * 37) << 1))
			f2_all = true;
		else
			f2_all = false;
		clsf2();
		afftex(st, 8, 176, 85, 3, 5);
	} else {
		modif(m);
		if (f == 8)
			f = 2;
		if (f == 1)
			f = 6;
		if (f == 2) {
			clsf2();
			ecrf2();
			text1(8, 182, 103, m);
			if ((m == 68) || (m == 69))
				s.teauto[40] = '*';
			if ((m == 104) && (caff == 14)) {
				s.teauto[36] = '*';
				if (s.teauto[39] == '*') {
					s.pourc[3] = '*';
					s.teauto[38] = '*';
				}
			}
		}
		if ((f >= 6) && (f <= 9)) {
			deline(m, st, tay);
			if (f == 6)
				i = 4;
			else
				i = 5;
			afftex(st, 80, 40, 60, 25, i);
			if (m == 180)
				s.pourc[6] = '*';
			if (m == 179)
				s.pourc[10] = '*';
		}
		if (f == 7) {         /* messint */
			ecrf7();
			deline(m, st, tay);
			if (res == 1) {
				xco = 252 - tay * 5;
				caspe = 100;
				dx = 80;
			} else {
				xco = 252 - tay * 3;
				caspe = 144;
				dx = 50;
			}
			if (tay < 40)
				afftex(st, xco, 86, dx, 3, 5);
			else
				afftex(st, caspe, 86, dx, 3, 5);
		}
	}
}

void t5(int cx) {
	if (cx == 10)
		blo = false;

	if (cx != 1) {
		bh1 = false;
		bf1 = false;
	}

	if (cx != 2)
		bh2 = false;

	if (cx != 4) {
		bh4 = false;
		bf4 = false;
	}

	if (cx != 5)
		bh5 = false;
	if (cx != 6)
		bh6 = false;
	if (cx != 8)
		bh8 = false;
	if (cx != 3)
		bt3 = false;
	if (cx != 7)
		bt7 = false;
	if (cx != 9)
		bh9 = false;
}

void affper(int per) {
	int cx;

	/* debug('affper'); */
	for (cx = 1; cx <= 8; cx ++)
		g_vm->_menu.disableMenuItem(g_vm->_menu._disc[cx]);
	clsf10();
	if ((per & 128) == 128) {
		g_vm->_screenSurface.putxy(560, 24);
		g_vm->_screenSurface.writeg("LEO", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._disc[1]);
	}
	if ((per & 64) == 64) {
		g_vm->_screenSurface.putxy(560, 32);
		g_vm->_screenSurface.writeg("PAT", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._disc[2]);
	}
	if ((per & 32) == 32) {
		g_vm->_screenSurface.putxy(560, 40);
		g_vm->_screenSurface.writeg("GUY", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._disc[3]);
	}
	if ((per & 16) == 16) {
		g_vm->_screenSurface.putxy(560, 48);
		g_vm->_screenSurface.writeg("EVA", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._disc[4]);
	}
	if ((per & 8) == 8) {
		g_vm->_screenSurface.putxy(560, 56);
		g_vm->_screenSurface.writeg("BOB", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._disc[5]);
	}
	if ((per & 4) == 4) {
		g_vm->_screenSurface.putxy(560, 64);
		g_vm->_screenSurface.writeg("LUC", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._disc[6]);
	}
	if ((per & 2) == 2) {
		g_vm->_screenSurface.putxy(560, 72);
		g_vm->_screenSurface.writeg("IDA", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._disc[7]);
	}
	if ((per & 1) == 1) {
		g_vm->_screenSurface.putxy(560, 80);
		g_vm->_screenSurface.writeg("MAX", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._disc[8]);
	}
	ipers = per;
}

void choix(int min, int max, int &per) {
	bool i;
	int cz;

	int haz = hazard(min, max);
	if (haz > 4) {
		haz = 8 - haz;
		i = true;
	} else
		i = false;

	int cx = 0;
	per = 0;
	while (cx < haz) {
		int cy = hazard(1, 8);
		conv(cy, cz);
		if ((per & cz) != cz) {
			++cx;
			per |= cz;
		}
	}
	if (i)
		per = 255 - per;
}

void cpl1(int &p) {
	int j, h, m;

	calch(j, h, m);
	if ((h > 7) || (h < 11))
		p = 25;
	else if ((h > 10) && (h < 14))
		p = 35;
	else if ((h > 13) && (h < 16))
		p = 50;
	else if ((h > 15) && (h < 18))
		p = 5;
	else if ((h > 17) && (h < 22))
		p = 35;
	else if ((h > 21) && (h < 24))
		p = 50;
	else if ((h >= 0) && (h < 8))
		p = 70;

	g_vm->_menu.mdn();
}

void cpl2(int &p) {
	int j, h, m;

	calch(j, h, m);
	if ((h > 7) && (h < 11))
		p = -2;
	if (h == 11)
		p = 100;
	if ((h > 11) && (h < 23))
		p = 10;
	if (h == 23)
		p = 20;
	if ((h >= 0) && (h < 8))
		p = 50;
}

void cpl3(int &p) {
	int j, h, m;

	calch(j, h, m);
	if (((h > 8) && (h < 10)) || ((h > 19) && (h < 24)))
		p = 34;
	if (((h > 9) && (h < 20)) || ((h >= 0) && (h < 9)))
		p = 0;
}

void cpl5(int &p) {
	int j, h, m;

	calch(j, h, m);
	if ((h > 6) && (h < 10))
		p = 0;
	if (h == 10)
		p = 100;
	if ((h > 10) && (h < 24))
		p = 15;
	if ((h >= 0) && (h < 7))
		p = 50;
}

void cpl6(int &p) {
	int j, h, m;

	calch(j, h, m);
	if (((h > 7) && (h < 13)) || ((h > 17) && (h < 20)))
		p = -2;
	if (((h > 12) && (h < 17)) || ((h > 19) && (h < 24)))
		p = 35;
	if (h == 17)
		p = 100;
	if ((h >= 0) && (h < 8))
		p = 60;
}

/**
 * Shows the you are alone message in the status area on the right hand side of the screen
 */
void person() {
	for (int cf = 1; cf <= 8; cf ++)
		g_vm->_menu.disableMenuItem(g_vm->_menu._disc[cf]);

	Common::String sYou = g_vm->getString(S_YOU);
	Common::String sAre = g_vm->getString(S_ARE);
	Common::String sAlone = g_vm->getString(S_ALONE);

	clsf10();
	g_vm->_screenSurface.putxy(580 - (g_vm->_screenSurface.getStringWidth(sYou) / 2), 30);
	g_vm->_screenSurface.writeg(sYou, 4);
	g_vm->_screenSurface.putxy(580 - (g_vm->_screenSurface.getStringWidth(sAre) / 2), 50);
	g_vm->_screenSurface.writeg(sAre, 4);
	g_vm->_screenSurface.putxy(580 - (g_vm->_screenSurface.getStringWidth(sAlone) / 2), 70);
	g_vm->_screenSurface.writeg(sAlone, 4);

	ipers = 0;
}

void chlm(int &per) {
	per = hazard(1, 2);
	if (per == 2)  per = 128;
}

void drawClock() {
	const int cv[3][13] = {
		{ 0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0 },
		{ 0,  5,  8, 10,  8,  5,  0, -5, -8, -10, -8, -5,  0 },
		{ 0, -5, -3,  0,  3,  5,  6,  5,  3,   0, -3, -5, -6 }
	};
	const int x = 580;
	const int y = 123;
	const int rg = 9;
	int h, co;


	hideMouse();
	
	paint_rect(570, 118, 20, 10);
	paint_rect(578, 114, 6, 18);
	if ((_currGraphicalDevice == MODE_CGA) || (_currGraphicalDevice == MODE_HERCULES))
		co = 0;
	else
		co = 1;

	if (_minute == 0)
		g_vm->_screenSurface.drawLine(((uint)x >> 1)*res, y, ((uint)x >> 1)*res, (y - rg), co);
	else 
		g_vm->_screenSurface.drawLine(((uint)x >> 1)*res, y, ((uint)x >> 1)*res, (y + rg), co);

	h = _hour;
	if (h > 12)
		h -= 12;
	if (h == 0)
		h = 12;

	g_vm->_screenSurface.drawLine(((uint)x >> 1)*res, y, ((uint)(x + cv[1][h]) >> 1)*res, y + cv[2][h], co);
	showMouse();
	g_vm->_screenSurface.putxy(568, 154);

	if (_hour > 11)
		g_vm->_screenSurface.writeg("PM ", 1);
	else
		g_vm->_screenSurface.writeg("AM ", 1);

	g_vm->_screenSurface.putxy(550, 160);
	if ((_day >= 0) && (_day <= 8)) {
		Common::String tmp = g_vm->getString(S_DAY);
		tmp.insertChar((char)(_day + 49), 0);
		g_vm->_screenSurface.writeg(tmp, 1);
	}
}

/*************
 * NIVEAU 11 *
 *************/

void debloc(int l) {
	num = 0;
	x = 0;
	y = 0;
	if ((l != 26) && (l != 15))
		t5(l);
	mpers = ipers;
}

void cpl10(int &p, int &h) {
	int j, m;

	calch(j, h, m);
	if (((h > 7) && (h < 11)) || ((h > 11) && (h < 14)) || ((h > 18) && (h < 21)))
		p = 100;
	if ((h == 11) || ((h > 20) && (h < 24)))
		p = 45;
	if (((h > 13) && (h < 17)) || (h == 18))
		p = 35;
	if (h == 17)
		p = 60;
	if ((h >= 0) && (h < 8))
		p = 5;
}

void cpl11(int &p, int &h) {
	int j, m;

	calch(j, h, m);
	if (((h > 8) && (h < 12)) || ((h > 20) && (h < 24)))
		p = 25;
	if (((h > 11) && (h < 14)) || ((h > 18) && (h < 21)))
		p = 5;
	if ((h > 13) && (h < 17))
		p = 55;
	if ((h > 16) && (h < 19))
		p = 45;
	if ((h >= 0) && (h < 9))
		p = 0;
}

void cpl12(int &p) {
	int j, h, m;

	calch(j, h, m);
	if (((h > 8) && (h < 15)) || ((h > 16) && (h < 22)))
		p = 55;
	if (((h > 14) && (h < 17)) || ((h > 21) && (h < 24)))
		p = 25;
	if ((h >= 0) && (h < 5))
		p = 0;
	if ((h > 4) && (h < 9))
		p = 15;
}

void cpl13(int &p) {
	p = 0;
}

void cpl15(int &p) {
	int j, h, m;

	calch(j, h, m);
	if ((h > 7) && (h < 12))
		p = 25;
	else if ((h > 11) && (h < 14))
		p = 0;
	else if ((h > 13) && (h < 18))
		p = 10;
	else if ((h > 17) && (h < 20))
		p = 55;
	else if ((h > 19) && (h < 22))
		p = 5;
	else if ((h > 21) && (h < 24))
		p = 15;
	else if ((h >= 0) && (h < 8))
		p = -15;
}

void cpl20(int &p, int &h) {
	int j, m;

	calch(j, h, m);
	if (h == 10)
		p = 65;
	else if ((h > 10) && (h < 21))
		p = 5;
	else if ((h > 20) && (h < 24))
		p = -15;
	else if ((h >= 0) && (h < 5))
		p = -300;
	else if ((h > 4) && (h < 10))
		p = -5;
}

void quelq1(int l) {
	int per;

	per = hazard(1, 2);
	if (l == 1) {
		if (per == 1)
			bh1 = true;
		else
			bf1 = true;
	} else if (l == 4) {
		if (per == 1)
			bh4 = true;
		else
			bf4 = true;
	}

	ipers = 10;
}

void quelq2() {
	if (li == 2)
		bh2 = true;
	else
		bh9 = true;

	ipers = 10;
}

void quelq5() {
	bh5 = true;
	ipers = 10;
}

void quelq6(int l) {
	if (l == 6)
		bh6 = true;
	else if (l == 8)
		bh8 = true;

	ipers = 10;
}

void quelq10(int h, int &per) {
	int min = 0, max = 0;

	if ((h >= 0) && (h < 8))
		chlm(per);
	else {
		if ((h > 7) && (h < 10)) {
			min = 5;
			max = 7;
		} else if ((h > 9) && (h < 12)) {
			min = 1;
			max = 4;
		} else if (((h > 11) && (h < 15)) || ((h > 18) && (h < 21))) {
			min = 6;
			max = 8;
		} else if (((h > 14) && (h < 19)) || ((h > 20) && (h < 24))) {
			min = 1;
			max = 5;
		}
		choix(min, max, per);
	}
	affper(per);
}

void quelq11(int h, int &per) {
	int min = 0, max = 0;

	if ((h >= 0) && (h < 8))
		chlm(per);
	else {
		if (((h > 7) && (h < 10)) || ((h > 20) && (h < 24))) {
			min = 1;
			max = 3;
		} else if (((h > 9) && (h < 12)) || ((h > 13) && (h < 19))) {
			min = 1;
			max = 4;
		} else if (((h > 11) && (h < 14)) || ((h > 18) && (h < 21))) {
			min = 1;
			max = 2;
		}
		choix(min, max, per);
	}
	affper(per);
}

void quelq12(int &per) {
	chlm(per);
	affper(per);
}

void quelq15(int &per) {
	int cx;
	bool test;

	per = 0;

	do {
		cx = hazard(1, 8);
		test = (((cx == 1) && (bh2 || bh9)) ||
		        ((cx == 2) && bh8) ||
		        ((cx == 3) && bh4) ||
		        ((cx == 4) && bf4) ||
		        ((cx == 5) && bh6) ||
		        ((cx == 6) && bh1) ||
		        ((cx == 7) && bf1) ||
		        ((cx == 8) && bh5));
	} while (!(! test));

	conv(cx, per);
	affper(per);
}

void quelq20(int h, int &per) {
	int min = 0, max = 0;

	if (((h >= 0) && (h < 10)) || ((h > 18) && (h < 24)))
		chlm(per);
	else {
		if ((h > 9) && (h < 12)) {
			min = 3;
			max = 7;
		} else if ((h > 11) && (h < 18)) {
			min = 1;
			max = 2;
		} else if (h == 18) {
			min = 2;
			max = 4;
		}
		choix(min, max, per);
	}
	affper(per);
}


void frap() {
	int j, h, m, haz;

	/* debug('o1 frap'); */
	calch(j, h, m);
	if ((h >= 0) && (h < 8))
		crep = 190;
	else {
		haz = hazard(1, 100);
		if (haz > 70)
			crep = 190;
		else
			crep = 147;
	}
}

void nouvp(int l, int &p) {
	p = 0;
	if (l == 1) {
		if (bh1)
			p = 4;
		if (bf1)
			p = 2;
	} else if (((l == 2) && (bh2)) || ((l == 9) && (bh9)))
		p = 128;
	else if (l == 4) {
		if (bh4)
			p = 32;
		if (bf4)
			p = 16;
	} else if ((l == 5) && (bh5))
		p = 1;
	else if ((l == 6) && (bh6))
		p = 8;
	else if ((l == 8) && (bh8))
		p = 64;
	else if (((l == 3) && (bt3)) || ((l == 7) && (bt7)))
		p = 9;

	if (p != 9)
		affper(p);
}



void tip(int ip, int &cx) {
	if (ip == 128)
		cx = 1;
	else if (ip == 64)
		cx = 2;
	else if (ip == 32)
		cx = 3;
	else if (ip == 16)
		cx = 4;
	else if (ip == 8)
		cx = 5;
	else if (ip == 4)
		cx = 6;
	else if (ip == 2)
		cx = 7;
	else if (ip == 1)
		cx = 8;
}


void ecfren(int &p, int &haz, int cf, int l) {
	/* debug('o1 ecfren'); */
	if (l == 0)
		person();
	p = -500;
	haz = 0;
	if (((l == 1) && (! bh1) && (! bf1)) || ((l == 4) && (! bh4) && (! bf4)))
		cpl1(p);
	if ((l == 2) && (! bh2) && (! bh9))
		cpl2(p);
	if (((l == 3) && (! bt3)) || ((l == 7) && (! bt7)))
		cpl3(p);
	if ((l == 5) && (! bh5))
		cpl5(p);
	if (((l == 6) && (! bh6)) || ((l == 8) && (! bh8)))
		cpl6(p);
	if ((l == 9) && (! bh9) && (! bh2))
		p = 10;
	if (((l == 2) && (bh9)) || ((l == 9) && (bh2)))
		p = -400;
	if (p != -500) {
		p = p + cf;
		haz = hazard(1, 100);
	}
}

void becfren(int l) {
	int haz;

	/* debug('o1 becfren'); */
	if ((l == 1) || (l == 4)) {
		haz = hazard(1, 2);
		if (l == 1)
			if (haz == 1)
				bh1 = true;
			else
				bf1 = true;
		if (l == 4)
			if (haz == 1)
				bh4 = true;
			else
				bf4 = true;
	} else if (l == 2)
		bh2 = true;
	else if (l == 3)
		bt3 = true;
	else if (l == 5)
		bh5 = true;
	else if (l == 6)
		bh6 = true;
	else if (l == 7)
		bt7 = true;
	else if (l == 8)
		bh8 = true;
	else if (l == 9)
		bh9 = true;
}

/* NIVEAU 10 */
void init_nbrepm() {
	const byte ipm[9] = { 0, 4, 5, 6, 7, 5, 6, 5, 8 };

	/* debug('init_nbrepm'); */
	for (int idx = 0; idx < 9; ++idx)
		nbrepm[idx] = ipm[idx];
}

void phaz(int &haz, int &p, int cf) {
	/* debug('phaz'); */
	p += cf;
	haz = hazard(1, 100);
}

void inzon() {
	int cx;

	/* debug('o2 inzon'); */
	copcha();

	s.ipre  = false;
	s.derobj = 0;
	s.icave = 0;
	s.iboul = 0;
	s.ibag  = 0;
	s.ipuit = 0;
	s.ivier = 0;
	s.iloic = 136;
	s.icryp = 141;
	s.conf  = hazard(4, 10);
	s.mlieu = 21;

	for (cx = 2; cx <= 6; cx ++)
		s.sjer[cx] = chr(0);

	s.sjer[1] = chr(113);
	s.heure = chr(20);

	for (cx = 1; cx <= 10; cx ++)
		s.pourc[cx] = ' ';

	for (cx = 1; cx <= 6; cx ++)
		s.teauto[cx] = '*';

	for (cx = 7; cx <= 9; cx ++)
		s.teauto[cx] = ' ';

	for (cx = 10; cx <= 28; cx ++)
		s.teauto[cx] = '*';

	for (cx = 29; cx <= 42; cx ++)
		s.teauto[cx] = ' ';

	s.teauto[33] = '*';

	for (cx = 1; cx <= 8; cx ++)
		nbrep[cx] = 0;

	init_nbrepm();
}

void dprog() {
	/* debug('o2 dprog'); */
	li = 21;
	/* jh:= t_settime(0);*/
	jh = 0;
	if (! s.ipre)
		blo = true;
	t = ti1;
	mh = readclock();
}

void pl1(int cf) {
	int p, haz;

	/* debug('o2 pl1'); */
	if (((li == 1) && (! bh1) && (! bf1)) || ((li == 4) && (! bh4) && (! bf4))) {
		cpl1(p);
		phaz(haz, p, cf);
		if (haz > p)
			person();
		else
			quelq1(li);
	}
}

void pl2(int cf) {
	int p, haz;

	/* debug('o2 pl2'); */
	if (! bh2) {
		cpl2(p);
		phaz(haz, p, cf);
		if (haz > p)
			person();
		else
			quelq2();
	}
}

void pl5(int cf) {
	int p, haz;

	/* debug('o2 pl5'); */
	if (! bh5) {
		cpl5(p);
		phaz(haz, p, cf);
		if (haz > p)
			person();
		else
			quelq5();
	}
}

void pl6(int cf) {
	int p, haz;

	/* debug('o2 pl6'); */
	if (((li == 6) && (! bh6)) || ((li == 8) && (! bh8))) {
		cpl6(p);
		phaz(haz, p, cf);
		if (haz > p)
			person();
		else
			quelq6(li);
	}
}

void pl9(int cf) {
	int p, haz;

	/* debug('o2 pl9'); */
	if (! bh9) {
		cf = -10;
		phaz(haz, p, cf);
		if (haz > p)
			person();
		else
			quelq2();
	}
}

void pl10(int cf) {
	int p, h, haz;

	/* debug('o2 pl10'); */
	cpl10(p, h);
	phaz(haz, p, cf);
	if (haz > p)
		person();
	else
		quelq10(h, p);
}

void pl11(int cf) {
	int p, h, haz;

	/* debug('o2 pl11'); */
	cpl11(p, h);
	phaz(haz, p, cf);
	if (haz > p)
		person();
	else
		quelq11(h, p);
}

void pl12(int cf) {
	int p, haz;

	/* debug('o2 pl12'); */
	cpl12(p);
	phaz(haz, p, cf);
	if (haz > p)
		person();
	else
		quelq12(p);
}

void pl13(int cf) {
	int p, haz;

	/* debug('o2 pl13'); */
	cpl13(p);
	phaz(haz, p, cf);
	if (haz > p)
		person();
	else
		quelq12(p);
}

void pl15(int cf) {
	int p, haz;

	/* debug('o2 pl15'); */
	cpl15(p);
	phaz(haz, p, cf);
	if (haz > p)
		person();
	else
		quelq15(p);
}

void pl20(int cf) {
	int p, h, haz;

	/* debug('o2 pl20'); */
	cpl20(p, h);
	phaz(haz, p, cf);
	if (haz > p)
		person();
	else
		quelq20(h, p);
}

void t11(int l11, int &a) {
	int p, haz, h = 0;

	/* debug('o2 t11'); */
	ecfren(p, haz, s.conf, l11);
	li = l11;
	if ((l11 > 0) && (l11 < 10)) {
		if (p != -500) {
			if (haz > p) {
				person();
				a = 0;
			} else {
				becfren(li);
				nouvp(li, a);
			}
		} else
			nouvp(li, a);
	}

	if (l11 > 9) {
		if ((l11 > 15) && (l11 != 20) && (l11 != 26))
			person();
		else {
			if (l11 == 10)
				cpl10(p, h);
			if (l11 == 11)
				cpl11(p, h);
			if (l11 == 12)
				cpl12(p);
			if ((l11 == 13) || (l11 == 14))
				cpl13(p);
			if ((l11 == 15) || (l11 == 26))
				cpl15(p);
			if (l11 == 20)
				cpl20(p, h);
			p = p + s.conf;
			haz = hazard(1, 100);
			if (haz > p) {
				person();
				a = 0;
			} else {
				if (l11 == 10)
					quelq10(h, p);
				if (l11 == 11)
					quelq11(h, p);
				if ((l11 == 12) || (l11 == 13) || (l11 == 14))
					quelq12(p);
				if ((l11 == 15) || (l11 == 26))
					quelq15(p);
				if (l11 == 20)
					quelq20(h, p);
				a = p;
			}
		}
	}
}

void cavegre() {
	int haz;

	/* debug('cavegre'); */
	s.conf = s.conf + 2;
	if (s.conf > 69)
		s.conf += (s.conf / 10);
	clsf3();
	ecrf2();
	ecr3(g_vm->getString(S_SOMEONE_ENTERS));
	haz = (hazard(0, 4)) - 2;
	parole(2, haz, 1);

	// Useless?
	for (haz = 0; haz <= 3000; haz++);
	clsf3();
	person();
}

void writetp(Common::String s, int t) {
	if (res == 2)
		g_vm->_screenSurface.writeg(s, t);
	else
		g_vm->_screenSurface.writeg(copy(s, 1, 25), t);
}

/**
 * Shows the waiting message when changing scenes.
 * @remarks	Because modern computesr are so much quicker. There's no point in showing
 * a waiting message between scenes.
 */
void messint(int nu) {
	// Method is deprecated
}

void aniof(int ouf, int num) {
	if ((caff == 7) && ((num == 4) || (num == 5)))
		return;
	
	if ((caff == 10) && (num == 7))
		num = 6;

	if (caff == 12) {
		if (num == 3)
			num = 4;
		else if (num == 4)
			num = 3;
	}

	int ad = adani;
	int offset = animof(ouf, num);

	GfxSurface surface;
	surface.decode(&mem[ad * 16 + offset]);
	g_vm->_screenSurface.drawPicture(surface, 0, 12);

	ecrf1();
}

void musique(int so) {
	bool i;
	int haz;

	/* debug('musique'); */
	if (so == 0) { /*musik(0)*/
		;
	} else if ((prebru == 0) && (! s.ipre)) {
		parole(10, 1, 1);
		++prebru;
	} else {
		i = false;
		if ((s.mlieu == 19) || (s.mlieu == 21) || (s.mlieu == 22)) {
			haz = hazard(1, 3);
			if (haz == 2) {
				haz = hazard(2, 4);
				parole(9, haz, 1);
				i = true;
			}
		}
		if (s.mlieu == 20) {
			haz = hazard(1, 2);
			if (haz == 1) {
				parole(8, 1, 1);
				i = true;
			}
		}
		if (s.mlieu == 24) {
			haz = hazard(1, 2);
			if (haz == 2) {
				parole(12, 1, 1);
				i = true;
			}
		}
		if (s.mlieu == 23) {
			parole(13, 1, 1);
			i = true;
		}
		if (! i) {
			haz = hazard(1, 17);
			parole(haz, 1, 2);
		}
	}
}

/* NIVEAU 9 */
void dessin(int ad) {
	if (ad != 0)
		dessine(ades, ((ad % 160) * 2), (ad / 160));
	else {
		clsf1();
		if (caff > 99) {
			dessine(ades, 60, 33);
			g_vm->_screenSurface.drawBox(118, 32, 291, 121, 15);         // Medium box
		} else if (caff > 69) {
			dessine(ades, 112, 48);           /* tˆtes */                    //Translation: Heads
			g_vm->_screenSurface.drawBox(222, 47, 155, 91, 15);
		} else {
			dessine(ades, 0, 12);
			ecrf1();
			if ((caff < 30) || (caff > 32)) {
				for (int cx = 1; cx <= 6; cx ++) {
					if (ord(touv[cx]) != 0)
						aniof(1, ord(touv[cx]));
				}

				if (caff == 13) {
					if (s.iboul == 141)
						aniof(1, 7);

					if (s.ibag == 159)
						aniof(1, 6);
				}
				if ((caff == 14) && (s.icave == 151))
					aniof(1, 2);

				if ((caff == 17) && (s.ivier == 143))
					aniof(1, 1);

				if ((caff == 24) && (s.ipuit != 0))
					aniof(1, 1);
			}
			
			if (caff < 26)
				musique(1);
		}
	}
}

} // End of namespace Mortevielle
