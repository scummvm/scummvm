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
#include "mortevielle/alert.h"
#include "mortevielle/droite.h"
#include "mortevielle/level15.h"
#include "mortevielle/menu.h"
#include "mortevielle/mor.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/parole2.h"
#include "mortevielle/sprint.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

// For ScummVM, we need to do check for file errors where we do the file access
const int ioresult = 0;

void testfi() {
	if (ioresult != 0) {
		caff = do_alert(err_mess, 1);
		mortevielle_exit(0);
	}
}

/**
 * Read the current system time
 */
int readclock() {
	int m, h;

	/* debug('readclock');*/
	TimeDate dateTime;
	g_system->getTimeAndDate(dateTime);

	m = dateTime.tm_min * 60;
	h = dateTime.tm_hour * 3600;
	return h + m + dateTime.tm_sec;
}

void modif(int &nu) {
	/* debug('modif'); */
	if (nu == 26)  nu = 25;
	if ((nu > 29) && (nu < 36))  nu = nu - 4;
	if ((nu > 69) && (nu < 78))  nu = nu - 37;
	if ((nu > 99) && (nu < 194))  nu = nu - 59;
	if ((nu > 996) && (nu < 1000))  nu = nu - 862;
	if ((nu > 1500) && (nu < 1507))  nu = nu - 1363;
	if ((nu > 1507) && (nu < 1513))  nu = nu - 1364;
	if ((nu > 1999) && (nu < 2002))  nu = nu - 1851;
	if (nu == 2010)  nu = 151;
	if ((nu > 2011) && (nu < 2025))  nu = nu - 1860;
	if (nu == 2026)  nu = 165;
	if ((nu > 2029) && (nu < 2037))  nu = nu - 1864;
	if ((nu > 3000) && (nu < 3005))  nu = nu - 2828;
	if (nu == 4100)  nu = 177;
	if (nu == 4150)  nu = 178;
	if ((nu > 4151) && (nu < 4156))  nu = nu - 3973;
	if (nu == 4157)  nu = 183;
	if ((nu == 4160) || (nu == 4161))  nu = nu - 3976;
}


void dessine(int ad, int x, int y) {
	/* debug('dessine'); */
	hide_mouse();
	writepal(numpal);
	pictout(ad, 0, x, y);
	show_mouse();
}

void dessine_rouleau() {
	/* debug('dessine_rouleau'); */
	writepal(89);
	if (gd == her) {
		mem[0x7000 * 16 + 14] = 15;
	}
	hide_mouse();
	pictout(0x73a2, 0, 0, 0);
	show_mouse();
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
	if (res == 1)  co = 10;
	else co = 6;
	deline(m, st, tay);
	if ((y == 182) && (tay * co > nb * 6))  y = 176;
	afftex(st, x, y, nb, 20, color_txt);
}

void initouv() {
	int cx;

	/* debug('initouv'); */
	for (cx = 1; cx <= 7; cx ++) touv[cx] = chr(0);
}

void ecrf1() {
	// Large drawing
	g_vm->_screenSurface.drawBox(0, 11, 512, 163, 15);
}

void clsf1() {
//	int i, j;

	/* debug('clsf1'); */
	hide_mouse();
	box(0, gd, 0, 11, 514, 175, 255);
	/* if Gd=ams then port[$3DD]:=15;
	 case Gd of
	         ams,
	         cga : begin
	                 for j:=0 to 63 do
	                    begin
	                      for i:=6 to 86 do
	                         begin
	                           mem[$B800:i*80+j]:=0;
	                           mem[$Ba00:i*80+j]:=0;
	                         end;
	                      mem[$b800:6960+j]:=0;
	                      mem[$ba00:400+j]:=0;
	                    end;
	               end;
	         ega : begin
	                 port[$3C4]:= 2;
	                 port[$3C5]:= 15;
	                 port[$3CE]:= 8;
	                 port[$3CF]:= 255;
	                 for j:=0 to 63 do
	                    for i:=11 to 174 do
	                       mem[$A000:i*80+j]:=0;
	               end;
	         tan : begin
	                 for j:=0 to 128 do
	                    begin
	                      for i:=3 to 43 do
	                         begin
	                           mem[$B800:i*160+j]:=0;
	                           mem[$Ba00:i*160+j]:=0;
	                           mem[$bc00:i*160+j]:=0;
	                           if i<>43 then mem[$be00:i*160+j]:=0;
	                         end;
	                      mem[$be00:320+j]:=0;
	                    end;
	               end;
	 end;
	 droite(256*res,11,256*res,175,0);*/
	show_mouse();
}

void clsf2() {
//	int i, j;

	/* debug('clsf2'); */
	hide_mouse();
	if (f2_all) {
		box(0, gd, 1, 176, 633, 199, 255);
		g_vm->_screenSurface.drawBox(0, 175, 634, 24, 15);
		f2_all = false;
	} else {
		box(0, gd, 1, 176, 633, 190, 255);
		g_vm->_screenSurface.drawBox(0, 175, 634, 15, 15);
	}
	show_mouse();
}

void ecrf2() {
	/* debug('ecrf2'); */
	text_color(5);
	/*g_vm->_screenSurface.drawBox(0,175,630,15,15);*/
}

void ecr2(Common::String str_) {
	int tab;
	int tlig;

	/* debug('ecr2 : '+str_);*/
	if (res == 1)  tab = 10;
	else tab = 6;
	g_vm->_screenSurface.putxy(8, 177);
	tlig = 59 + pred(int, res) * 36;
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
//	int i, j;

	/* debug('clsf3'); */
	hide_mouse();
	box(0, gd, 1, 192, 633, 199, 255);
	g_vm->_screenSurface.drawBox(0, 191, 634, 8, 15);
	show_mouse();
}

void ecr3(Common::String text) {
	/* debug('ecr3 : '+text);*/
	clsf3();
	g_vm->_screenSurface.putxy(8, 192);
	g_vm->_screenSurface.writeg(text, 5);
}

void ecrf6() {
	/* debug('ecrf6'); */
	text_color(5);
	g_vm->_screenSurface.drawBox(62, 33, 363, 80, 15);
}

void ecrf7() {
	text_color(4);
	/* draw_mode(2);
	 text_height(4*res);
	 text_style(normal);*/
}

void clsf10() {
	int co, cod;
	Common::String st;

	/* debug('clsf10'); */
	hide_mouse();
	if (res == 1) {
		co = 634;
		cod = 534;
	} else {
		co = 600;
		cod = 544;
	}
	box(15, gd, cod, 93, co, 98, 255);
	if (s.conf < 33)
		st = g_vm->getString(S_COOL);
	else if (s.conf < 66)
		st = g_vm->getString(S_LOURDE);
	else if (s.conf > 65)
		st = g_vm->getString(S_MALSAINE);
	if (res == 1)
		co = 10;
	else co = 6;
	co = 574 - ((uint)co * st.size() >> 1);
	g_vm->_screenSurface.putxy(co, 92);
	g_vm->_screenSurface.writeg(st, 4);
	if (res == 1)  co = 620;
	else co = 584;
	box(15, gd, 560, 24, co, 86, 255);
	/* rempli(69,12,32,5,255);*/
	show_mouse();
}

void stop() {
	clrscr;
	hirs();
	gd = ams;
	hirs();
	mortevielle_exit(0);
}

void paint_rect(int x, int y, int dx, int dy) {
	int co;

	/* debug('paint_rect'); */
	if (gd == cga)  co = 3;
	else co = 11;
	box(co, gd, x, y, x + dx, y + dy, 255);
}

int hazard(int min, int max) {
	/* debug('hazard'); */
	return get_random_number(min, max);
}

void calch(int &j, int &h, int &m) {
	int th, nh;

	/* debug('calch');*/
	nh = readclock();
	th = jh + ((nh - mh) / t);
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
	int cx;

	/* debug('conv'); */
	cx = 1;
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
	Common::String strp;
	int tay;

	/* debug('modobj'); */
	strp = ' ';
	if (m != 500) {
		deline(m - 501 + c_st41, str_, tay);
		strp = delig;
	}
	menut(invt[8], strp);
	menu_disable(invt[8]);
}

void modobj2(int m, bool t1, bool t2) {
	char str_[1410];
	Common::String strp;
	int tay;

	/* debug('modobj'); */
	strp = ' ';
	if (t1 || t2)  okpas();
	else tesok = false;;
	if (m != 500) {
		deline(m - 501 + c_st41, str_, tay);
		strp = delig;
	}
	menut(invt[8], strp);
	menu_disable(invt[8]);
}


void repon(int f, int m) {
	Common::String str_;
	Common::String str1;
	char st[1410];
//	text1 fic;
	int i, xco, dx, caspe, tay;

	/* debug('repon fenetre nø'+chr(f+48));*/
	if ((m > 499) && (m < 563)) {
		deline(m - 501 + c_st41, st, tay);
		if (tay > ((58 + pred(int, res) * 37) << 1))  f2_all = true;
		else f2_all = false;
		clsf2();
		afftex(st, 8, 176, 85, 3, 5);
	} else {
		modif(m);
		if (f == 8)  f = 2;
		if (f == 1)  f = 6;
		if (f == 2) {
			clsf2();
			ecrf2();
			text1(8, 182, 103, m);
			if ((m == 68) || (m == 69))  s.teauto[40] = '*';
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
			if (f == 6)  i = 4;
			else i = 5;
			afftex(st, 80, 40, 60, 25, i);
			if (m == 180)  s.pourc[6] = '*';
			if (m == 179)  s.pourc[10] = '*';
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
			if (tay < 40)  afftex(st, xco, 86, dx, 3, 5);
			else afftex(st, caspe, 86, dx, 3, 5);
		}
	}
}

void f3f8() {
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

void t5(int cx) {
	/* debug('t5'); */
	if (cx == 10)  blo = false;
	if (cx != 1) {
		bh1 = false;
		bf1 = false;
	}
	if (cx != 2)  bh2 = false;
	if (cx != 4) {
		bh4 = false;
		bf4 = false;
	}
	if (cx != 5)  bh5 = false;
	if (cx != 6)  bh6 = false;
	if (cx != 8)  bh8 = false;
	if (cx != 3)  bt3 = false;
	if (cx != 7)  bt7 = false;
	if (cx != 9)  bh9 = false;
}

void affper(int per) {
	int cx;

	/* debug('affper'); */
	for (cx = 1; cx <= 8; cx ++) menu_disable(disc[cx]);
	clsf10();
	if ((per & 128) == 128) {
		g_vm->_screenSurface.putxy(560, 24);
		g_vm->_screenSurface.writeg("LEO", 4);
		menu_enable(disc[1]);
	}
	if ((per & 64) == 64) {
		g_vm->_screenSurface.putxy(560, 32);
		g_vm->_screenSurface.writeg("PAT", 4);
		menu_enable(disc[2]);
	}
	if ((per & 32) == 32) {
		g_vm->_screenSurface.putxy(560, 40);
		g_vm->_screenSurface.writeg("GUY", 4);
		menu_enable(disc[3]);
	}
	if ((per & 16) == 16) {
		g_vm->_screenSurface.putxy(560, 48);
		g_vm->_screenSurface.writeg("EVA", 4);
		menu_enable(disc[4]);
	}
	if ((per & 8) == 8) {
		g_vm->_screenSurface.putxy(560, 56);
		g_vm->_screenSurface.writeg("BOB", 4);
		menu_enable(disc[5]);
	}
	if ((per & 4) == 4) {
		g_vm->_screenSurface.putxy(560, 64);
		g_vm->_screenSurface.writeg("LUC", 4);
		menu_enable(disc[6]);
	}
	if ((per & 2) == 2) {
		g_vm->_screenSurface.putxy(560, 72);
		g_vm->_screenSurface.writeg("IDA", 4);
		menu_enable(disc[7]);
	}
	if ((per & 1) == 1) {
		g_vm->_screenSurface.putxy(560, 80);
		g_vm->_screenSurface.writeg("MAX", 4);
		menu_enable(disc[8]);
	}
	ipers = per;
}

/* overlay */ void choix(int min, int max, int &per) {
	bool i;
	int haz, cx, cy, cz;

	/* debug('o0 choix'); */
	haz = hazard(min, max);
	if (haz > 4) {
		haz = 8 - haz;
		i = true;
	} else i = false;
	cx = 0;
	per = 0;
	while (cx < haz) {
		cy = hazard(1, 8);
		conv(cy, cz);
		if ((per & cz) != cz) {
			cx = cx + 1;
			per = (per | cz);
		}
	}
	if (i)  per = 255 - per;
	i = false;
}

/* overlay */ void cpl1(int &p) {
	int j, h, m;

	/* debug('o0 cpl1'); */
	calch(j, h, m);
	if ((h > 7) || (h < 11))  p = 25;
	if ((h > 10) && (h < 14))  p = 35;
	if ((h > 13) && (h < 16))  p = 50;
	if ((h > 15) && (h < 18))  p = 5;
	if ((h > 17) && (h < 22))  p = 35;
	if ((h > 21) && (h < 24))  p = 50;
	if ((h >= 0) && (h < 8))  p = 70;
	mdn();
}

/* overlay */ void cpl2(int &p) {
	int j, h, m;

	/* debug('o0 cpl2'); */
	calch(j, h, m);
	if ((h > 7) && (h < 11))  p = -2;
	if (h == 11)  p = 100;
	if ((h > 11) && (h < 23))  p = 10;
	if (h == 23)  p = 20;
	if ((h >= 0) && (h < 8))  p = 50;
}

/* overlay */ void cpl3(int &p) {
	int j, h, m;

	/* debug('o0 cpl3'); */
	calch(j, h, m);
	if (((h > 8) && (h < 10)) || ((h > 19) && (h < 24)))  p = 34;
	if (((h > 9) && (h < 20)) || ((h >= 0) && (h < 9)))  p = 0;
}

/* overlay */ void cpl5(int &p) {
	int j, h, m;

	/* debug('o0 cpl5'); */
	calch(j, h, m);
	if ((h > 6) && (h < 10))  p = 0;
	if (h == 10)  p = 100;
	if ((h > 10) && (h < 24))  p = 15;
	if ((h >= 0) && (h < 7))  p = 50;
}

/* overlay */ void cpl6(int &p) {
	int j, h, m;

	/* debug('o0 cpl6'); */
	calch(j, h, m);
	if (((h > 7) && (h < 13)) || ((h > 17) && (h < 20)))  p = -2;
	if (((h > 12) && (h < 17)) || ((h > 19) && (h < 24)))  p = 35;
	if (h == 17)  p = 100;
	if ((h >= 0) && (h < 8))  p = 60;
}

/**
 * Shows the you are alone message in the status area on the right hand side of the screen
 */
void person() {
	/* debug('person'); */
	for (int cf = 1; cf <= 8; cf ++)
		menu_disable(disc[cf]);

	clsf10();
	g_vm->_screenSurface.putxy(560, 30);
	g_vm->_screenSurface.writeg(g_vm->getString(S_YOU), 4);
	g_vm->_screenSurface.putxy(560, 50);
	g_vm->_screenSurface.writeg(g_vm->getString(S_ARE), 4);
	g_vm->_screenSurface.putxy(560, 70);
	g_vm->_screenSurface.writeg(g_vm->getString(S_ALONE), 4);
	ipers = 0;
}

void chlm(int &per) {
	/* debug('chlm'); */
	per = hazard(1, 2);
	if (per == 2)  per = 128;
}

void pendule() {
	const int cv[3][13] = {
		{ 0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0 },
		{ 0,  5,  8, 10,  8,  5,  0, -5, -8, -10, -8, -5,  0 },
		{ 0, -5, -3,  0,  3,  5,  6,  5,  3,   0, -3, -5, -6 }
	};
	const int x = 580;
	const int y = 123;
	const int rg = 9;
	int h, co;


	/* debug('pendule'); */
	hide_mouse();
	/*paint_rect(572,114,18,20);
	paint_rect(577,114,6,20);*/
	paint_rect(570, 118, 20, 10);
	paint_rect(578, 114, 6, 18);
	if ((gd == cga) || (gd == her))
		co = 0;
	else co = 1;
	if (min == 0)  droite(((uint)x >> 1)*res, y, ((uint)x >> 1)*res, (y - rg), co);
	else droite(((uint)x >> 1)*res, y, ((uint)x >> 1)*res, (y + rg), co);
	h = heu;
	if (h > 12)  h = h - 12;
	if (h == 0)  h = 12;
	droite(((uint)x >> 1)*res, y, ((uint)(x + cv[1][h]) >> 1)*res, y + cv[2][h], co);
	show_mouse();
	g_vm->_screenSurface.putxy(568, 154);
	if (heu > 11)
		g_vm->_screenSurface.writeg("PM ", 1);
	else
		g_vm->_screenSurface.writeg("AM ", 1);
	g_vm->_screenSurface.putxy(550, 160);
	if ((jou >= 0) && (jou <= 8)) {
		Common::String tmp = g_vm->getString(S_DAY);
		tmp.insertChar((char)(jou + 49), 0);
		g_vm->_screenSurface.writeg(tmp, 1);
	}
}

/*************
 * NIVEAU 11 *
 *************/

void debloc(int l) {
	/* debug('debloc'); */
	num = 0;
	x = 0;
	y = 0;
	if ((l != 26) && (l != 15))  t5(l);
	mpers = ipers;
}

/* overlay */ void cpl10(int &p, int &h) {
	int j, m;

	/* debug('o1 cpl10'); */
	calch(j, h, m);
	if (((h > 7) && (h < 11)) || ((h > 11) && (h < 14))
	        || ((h > 18) && (h < 21)))  p = 100;
	if ((h == 11) || ((h > 20) && (h < 24)))  p = 45;
	if (((h > 13) && (h < 17)) || (h == 18))  p = 35;
	if (h == 17)  p = 60;
	if ((h >= 0) && (h < 8))  p = 5;
}

/* overlay */ void cpl11(int &p, int &h) {
	int j, m;

	/* debug('o1 cpl11'); */
	calch(j, h, m);
	if (((h > 8) && (h < 12)) || ((h > 20) && (h < 24)))  p = 25;
	if (((h > 11) && (h < 14)) || ((h > 18) && (h < 21)))  p = 5;
	if ((h > 13) && (h < 17))  p = 55;
	if ((h > 16) && (h < 19))  p = 45;
	if ((h >= 0) && (h < 9))  p = 0;
}

/* overlay */ void cpl12(int &p) {
	int j, h, m;

	/* debug('o1 cpl12'); */
	calch(j, h, m);
	if (((h > 8) && (h < 15)) || ((h > 16) && (h < 22)))  p = 55;
	if (((h > 14) && (h < 17)) || ((h > 21) && (h < 24)))  p = 25;
	if ((h >= 0) && (h < 5))  p = 0;
	if ((h > 4) && (h < 9))  p = 15;
}

/* overlay */ void cpl13(int &p) {
	/* debug('o1 cpl13'); */
	p = 0;
}

/* overlay */ void cpl15(int &p) {
	int j, h, m;

	/* debug('o1 cpl15'); */
	calch(j, h, m);
	if ((h > 7) && (h < 12))  p = 25;
	if ((h > 11) && (h < 14))  p = 0;
	if ((h > 13) && (h < 18))  p = 10;
	if ((h > 17) && (h < 20))  p = 55;
	if ((h > 19) && (h < 22))  p = 5;
	if ((h > 21) && (h < 24))  p = 15;
	if ((h >= 0) && (h < 8))  p = -15;
}

/* overlay */ void cpl20(int &p, int &h) {
	int j, m;

	/* debug('o1 cpl20'); */
	calch(j, h, m);
	if (h == 10)  p = 65;
	if ((h > 10) && (h < 21))  p = 5;
	if ((h > 20) && (h < 24))  p = -15;
	if ((h >= 0) && (h < 5))  p = -300;
	if ((h > 4) && (h < 10))  p = -5;
}

/* overlay */ void quelq1(int l) {
	int per;

	/* debug('o1 quelq1'); */
	per = hazard(1, 2);
	if (l == 1)
		if (per == 1)  bh1 = true;
		else bf1 = true;
	if (l == 4)
		if (per == 1)  bh4 = true;
		else bf4 = true;
	ipers = 10;
}

/* overlay */ void quelq2() {
	/* debug('o1 quelq2'); */
	if (li == 2)  bh2 = true;
	else bh9 = true;
	ipers = 10;
}

/* overlay */ void quelq5() {
	/* debug('o1 quelq5'); */
	bh5 = true;
	ipers = 10;
}

/* overlay */ void quelq6(int l) {
	/* debug('o1 quelq6'); */
	if (l == 6)  bh6 = true;
	if (l == 8)  bh8 = true;
	ipers = 10;
}

/* overlay */ void quelq10(int h, int &per) {
	int min = 0, max = 0;

	/* debug('o1 quelq10'); */
	if ((h >= 0) && (h < 8))  chlm(per);
	else {
		if ((h > 7) && (h < 10)) {
			min = 5;
			max = 7;
		}
		if ((h > 9) && (h < 12)) {
			min = 1;
			max = 4;
		}
		if (((h > 11) && (h < 15)) || ((h > 18) && (h < 21))) {
			min = 6;
			max = 8;
		}
		if (((h > 14) && (h < 19)) || ((h > 20) && (h < 24))) {
			min = 1;
			max = 5;
		}
		choix(min, max, per);
	}
	affper(per);
}

/* overlay */ void quelq11(int h, int &per) {
	int min = 0, max = 0;

	/* debug('o1 quelq11'); */
	if ((h >= 0) && (h < 8))  chlm(per);
	else {
		if (((h > 7) && (h < 10)) || ((h > 20) && (h < 24))) {
			min = 1;
			max = 3;
		}
		if (((h > 9) && (h < 12)) || ((h > 13) && (h < 19))) {
			min = 1;
			max = 4;
		}
		if (((h > 11) && (h < 14)) || ((h > 18) && (h < 21))) {
			min = 1;
			max = 2;
		}
		choix(min, max, per);
	}
	affper(per);
}

/* overlay */ void quelq12(int &per) {
	/* debug('o1 quelq12'); */
	chlm(per);
	affper(per);
}

/* overlay */ void quelq15(int &per) {
	int cx;
	bool test;


	/* debug('o1 quelq15'); */
	per = 0;
	if (per == 0) {
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
}

/* overlay */ void quelq20(int h, int &per) {
	int min = 0, max = 0;

	/* debug('o1 quelq20'); */
	if (((h >= 0) && (h < 10)) || ((h > 18) && (h < 24)))  chlm(per);
	else {
		if ((h > 9) && (h < 12)) {
			min = 3;
			max = 7;
		}
		if ((h > 11) && (h < 18)) {
			min = 1;
			max = 2;
		}
		if (h == 18) {
			min = 2;
			max = 4;
		}
		choix(min, max, per);
	}
	affper(per);
}


/* overlay */void frap() {
	int j, h, m, haz;

	/* debug('o1 frap'); */
	calch(j, h, m);
	if ((h >= 0) && (h < 8))  crep = 190;
	else {
		haz = hazard(1, 100);
		if (haz > 70)  crep = 190;
		else crep = 147;
	}
}

/* overlay */ void nouvp(int l, int &p) {
	/* debug('o1 nouvp'); */
	p = 0;
	if (l == 1) {
		if (bh1)  p = 4;
		if (bf1)  p = 2;
	}
	if (((l == 2) && (bh2)) || ((l == 9) && (bh9)))  p = 128;
	if (l == 4) {
		if (bh4)  p = 32;
		if (bf4)  p = 16;
	}
	if ((l == 5) && (bh5))  p = 1;
	if ((l == 6) && (bh6))  p = 8;
	if ((l == 8) && (bh8))  p = 64;
	if (((l == 3) && (bt3)) || ((l == 7) && (bt7)))  p = 9;
	if (p != 9)  affper(p);
}



/* overlay */ void tip(int ip, int &cx) {
	/* debug('o1 tip'); */
	if (ip == 128)  cx = 1;
	else if (ip == 64)   cx = 2;
	else if (ip == 32)   cx = 3;
	else if (ip == 16)   cx = 4;
	else if (ip == 8)    cx = 5;
	else if (ip == 4)    cx = 6;
	else if (ip == 2)    cx = 7;
	else if (ip == 1)    cx = 8;
}


/* overlay */ void ecfren(int &p, int &haz, int cf, int l) {
	/* debug('o1 ecfren'); */
	if (l == 0)  person();
	p = -500;
	haz = 0;
	if (((l == 1) && (! bh1) && (! bf1))
	        || ((l == 4) && (! bh4) && (! bf4)))  cpl1(p);
	if ((l == 2) && (! bh2) && (! bh9))  cpl2(p);
	if (((l == 3) && (! bt3)) || ((l == 7) && (! bt7)))  cpl3(p);
	if ((l == 5) && (! bh5))  cpl5(p);
	if (((l == 6) && (! bh6)) || ((l == 8) && (! bh8)))  cpl6(p);
	if ((l == 9) && (! bh9) && (! bh2))  p = 10;
	if (((l == 2) && (bh9)) || ((l == 9) && (bh2)))  p = -400;
	if (p != -500) {
		p = p + cf;
		haz = hazard(1, 100);
	}
}

/* overlay */ void becfren(int l) {
	int haz;

	/* debug('o1 becfren'); */
	if ((l == 1) || (l == 4)) {
		haz = hazard(1, 2);
		if (l == 1)
			if (haz == 1)  bh1 = true;
			else bf1 = true;
		if (l == 4)
			if (haz == 1)  bh4 = true;
			else bf4 = true;
	}
	if (l == 2)  bh2 = true;
	else if (l == 3)  bt3 = true;
	else if (l == 5)  bh5 = true;
	else if (l == 6)  bh6 = true;
	else if (l == 7)  bt7 = true;
	else if (l == 8)  bh8 = true;
	else if (l == 9)  bh9 = true;
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
	p = p + cf;
	haz = hazard(1, 100);
}

/* overlay */ void inzon() {
	int cx;

	/* debug('o2 inzon'); */
	copcha();
	{
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
		for (cx = 2; cx <= 6; cx ++) s.sjer[cx] = chr(0);
		s.sjer[1] = chr(113);
		s.heure = chr(20);
		for (cx = 1; cx <= 10; cx ++) s.pourc[cx] = ' ';
		for (cx = 1; cx <= 6; cx ++) s.teauto[cx] = '*';
		for (cx = 7; cx <= 9; cx ++) s.teauto[cx] = ' ';
		for (cx = 10; cx <= 28; cx ++) s.teauto[cx] = '*';
		for (cx = 29; cx <= 42; cx ++) s.teauto[cx] = ' ';
		s.teauto[33] = '*';
	}
	for (cx = 1; cx <= 8; cx ++) nbrep[cx] = 0;
	init_nbrepm();
}

/* overlay */ void dprog() {
	/* debug('o2 dprog'); */
	li = 21;
	/* jh:= t_settime(0);*/
	jh = 0;
	if (! s.ipre)  blo = true;
	t = ti1;
	mh = readclock();
}

/* overlay */ void pl1(int cf) {
	int p, haz;

	/* debug('o2 pl1'); */
	if (((li == 1) && (! bh1) && (! bf1))
	        || ((li == 4) && (! bh4) && (! bf4))) {
		cpl1(p);
		phaz(haz, p, cf);
		if (haz > p)  person();
		else quelq1(li);
	}
}

/* overlay */ void pl2(int cf) {
	int p, haz;

	/* debug('o2 pl2'); */
	if (! bh2) {
		cpl2(p);
		phaz(haz, p, cf);
		if (haz > p)  person();
		else quelq2();
	}
}

/* overlay */ void pl5(int cf) {
	int p, haz;

	/* debug('o2 pl5'); */
	if (! bh5) {
		cpl5(p);
		phaz(haz, p, cf);
		if (haz > p)  person();
		else quelq5();
	}
}

/* overlay */ void pl6(int cf) {
	int p, haz;

	/* debug('o2 pl6'); */
	if (((li == 6) && (! bh6)) || ((li == 8) && (! bh8))) {
		cpl6(p);
		phaz(haz, p, cf);
		if (haz > p)  person();
		else quelq6(li);
	}
}

/* overlayi */ void pl9(int cf) {
	int p, haz;

	/* debug('o2 pl9'); */
	if (! bh9) {
		cf = -10;
		phaz(haz, p, cf);
		if (haz > p)  person();
		else quelq2();
	}
}

/* overlayi */ void pl10(int cf) {
	int p, h, haz;

	/* debug('o2 pl10'); */
	cpl10(p, h);
	phaz(haz, p, cf);
	if (haz > p)  person();
	else quelq10(h, p);
}

/* overlay */ void pl11(int cf) {
	int p, h, haz;

	/* debug('o2 pl11'); */
	cpl11(p, h);
	phaz(haz, p, cf);
	if (haz > p)  person();
	else quelq11(h, p);
}

/* overlay */ void pl12(int cf) {
	int p, haz;

	/* debug('o2 pl12'); */
	cpl12(p);
	phaz(haz, p, cf);
	if (haz > p)  person();
	else quelq12(p);
}

/* overlay */ void pl13(int cf) {
	int p, haz;

	/* debug('o2 pl13'); */
	cpl13(p);
	phaz(haz, p, cf);
	if (haz > p)  person();
	else quelq12(p);
}

/* overlay */ void pl15(int cf) {
	int p, haz;

	/* debug('o2 pl15'); */
	cpl15(p);
	phaz(haz, p, cf);
	if (haz > p)  person();
	else quelq15(p);
}

/* overlay */ void pl20(int cf) {
	int p, h, haz;

	/* debug('o2 pl20'); */
	cpl20(p, h);
	phaz(haz, p, cf);
	if (haz > p)  person();
	else quelq20(h, p);
}

/* overlay */ void t11(int l11, int &a) {
	int p, haz, h = 0;

	/* debug('o2 t11'); */
	ecfren(p, haz, s.conf, l11);
	li = l11;
	if ((l11 > 0) && (l11 < 10))
		if (p != -500) {
			if (haz > p) {
				person();
				a = 0;
			} else {
				becfren(li);
				nouvp(li, a);
			}
		} else nouvp(li, a);
	if (l11 > 9)
		if ((l11 > 15) && (l11 != 20) && (l11 != 26))  person();
		else {
			if (l11 == 10)  cpl10(p, h);
			if (l11 == 11)  cpl11(p, h);
			if (l11 == 12)  cpl12(p);
			if ((l11 == 13) || (l11 == 14))  cpl13(p);
			if ((l11 == 15) || (l11 == 26))  cpl15(p);
			if (l11 == 20)  cpl20(p, h);
			p = p + s.conf;
			haz = hazard(1, 100);
			if (haz > p) {
				person();
				a = 0;
			} else {
				if (l11 == 10)  quelq10(h, p);
				if (l11 == 11)  quelq11(h, p);
				if ((l11 == 12) || (l11 == 13) || (l11 == 14))  quelq12(p);
				if ((l11 == 15) || (l11 == 26))  quelq15(p);
				if (l11 == 20)  quelq20(h, p);
				a = p;
			}
		}
}

/* overlay */ void cavegre()

{
	int haz;

	/* debug('cavegre'); */
	s.conf = s.conf + 2;
	if (s.conf > 69)  s.conf = s.conf + (s.conf / 10);
	clsf3();
	ecrf2();
	ecr3(g_vm->getString(S_SOMEONE_ENTERS));
	haz = (hazard(0, 4)) - 2;
	parole(2, haz, 1);
	for (haz = 0; haz <= 3000; haz ++);
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
	/*	Method is deprecated
	clsf1();
	clsf2();
	clsf3();

	GfxSurface surface;
	surface.decode(&mem[0x73a2 * 16 + 1008]);

	WRITE_LE_UINT16(&mem[0x7413 * 16 + 12], 80);
	WRITE_LE_UINT16(&mem[0x7413 * 16 + 14], 40);
	writepal(90);

	g_vm->_screenSurface.drawPicture(surface, 0, 0);
	g_vm->_screenSurface.drawPicture(surface, 0, 70);

	repon(7, nu);
	*/
}

void aniof(int ouf, int num) {
	int ad, offset;

	/* debug('aniof'); */
	if ((caff == 7) && ((num == 4) || (num == 5)))  return;
	if ((caff == 10) && (num == 7))  num = 6;
	if (caff == 12)  if (num == 3)  num = 4;
		else if (num == 4)  num = 3;
	ad = adani;
	offset = animof(ouf, num);

	GfxSurface surface;
	surface.decode(&mem[ad * 16 + offset]);
	g_vm->_screenSurface.drawPicture(surface, 0, 12);

	ecrf1();
}

void musique(int so) {
	bool i;
	int haz;
	/* dep: array[1..5] of float;*/

	/* debug('musique'); */
	if (so == 0) { /*musik(0)*/
		;
	} else if ((prebru == 0) && (! s.ipre)) {
		parole(10, 1, 1);
		prebru = prebru + 1;
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
			/* dep[1]:= 1416;
			 dep[2]:= 1512;
			 dep[3]:= 1692;
			 dep[4]:= 1884;
			 dep[5]:= 2046;
			 haz:= hazard(1,5);
			 musik(dep[haz]);*/
			haz = hazard(1, 17);
			parole(haz, 1, 2);
		}
	}
}

/* NIVEAU 9 */
void dessin(int ad) {
	int cx;

	/* debug('dessin'); */
	if (ad != 0)  dessine(ades, ((ad % 160) * 2), (ad / 160));
	else {
		clsf1();
		if (caff > 99) {
			dessine(ades, 60, 33);
			g_vm->_screenSurface.drawBox(118, 32, 291, 121, 15);         /* cadre moyen */          //Translation: Medium box
		} else if (caff > 69) {
			dessine(ades, 112, 48);           /* tˆtes */                    //Translation: Heads
			g_vm->_screenSurface.drawBox(222, 47, 155, 91, 15);
		} else {
			dessine(ades, 0, 12);
			ecrf1();
			if ((caff < 30) || (caff > 32)) {
				for (cx = 1; cx <= 6; cx ++)
					if (ord(touv[cx]) != 0)  aniof(1, ord(touv[cx]));
				if (caff == 13) {
					if (s.iboul == 141)  aniof(1, 7);
					if (s.ibag == 159)  aniof(1, 6);
				}
				if ((caff == 14) && (s.icave == 151))  aniof(1, 2);
				if ((caff == 17) && (s.ivier == 143))  aniof(1, 1);
				if ((caff == 24) && (s.ipuit != 0))  aniof(1, 1);
			}
			if (caff < 26)  musique(1);
		}
	}
}

} // End of namespace Mortevielle
