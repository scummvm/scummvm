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

#include "mortevielle/mor2.h"
#include "mortevielle/alert.h"
#include "mortevielle/boite.h"
#include "mortevielle/keyboard.h"
#include "mortevielle/menu.h"
#include "mortevielle/mor.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/parole2.h"
#include "mortevielle/taffich.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

const int men[12] = { 0,
		scacher, attacher, defoncer, dormir, 
		entrer,  fermer,   frapper,  manger,
		mettre,  ouvrir,   sortir
};

void tinke() {
	const char m1[] = "Mince! Vous entendez du bruit...";
	const char d1[] = " | Vous devriez avoir remarqu‚|       ";
	const char d2[] = "% des indices...";
	const char d3 = '[';
	const char d4 = ']';
	const char d5 = '1';
	const char d6[] = "OK";
	int cx, haz, nh, cf, j, h, m;
	Common::String stpo;
	bool am;

	anyone = false;
	calch(j, h, m);
	if (j != jou) {
		jou = j;
		cx = 0;
		do {
			cx = cx + 1;
			if (nbrepm[cx] != 0)  nbrepm[cx] = nbrepm[cx] - 1;
			nbrep[cx] = 0;
		} while (!(cx == 8));
	}
	if ((h > heu) || ((h == 0) && (heu == 23))) {
		heu = h;
		min = 0;
		pendule();
		cf = 0;
		for (cx = 1; cx <= 10; cx ++) if (s.pourc[cx] == '*')  cf = cf + 1;
		if (cf == 10)  stpo = "10";
		else stpo = chr(cf + 48);

		stpou = Common::String(d3) + d5 + d4 + d3 + d1 + stpo + '0' + d2 + d4 + d3 + d6 + d4;
	}
	if (m > min) {
		min = 30;
		pendule();
	}
	if (y_s < 12)  return;
	if (! blo) {
		if ((h == 12) || ((h > 18) && (h < 21)) || ((h >= 0) && (h < 7)))
			t = ti2;
		else t = ti1;
		cf = s.conf;
		if ((cf > 33) && (cf < 66))  t = t - (t / 3);
		if (cf > 65)  t = t - ((t / 3) * 2);
		nh = readclock();
		if ((nh - mh) > t) {
			am = active_menu;
			erase_menu();
			jh = jh + ((nh - mh) / t);
			mh = nh;
			switch (li) {
			case 1:
			case 4 :
				pl1(cf);
				break;
			case 2 :
				pl2(cf);
				break;
			case 5 :
				pl5(cf);
				break;
			case 6:
			case 8 :
				pl6(cf);
				break;
			case 9 :
				pl9(cf);
				break;
			case 10 :
				pl10(cf);
				break;
			case 11 :
				pl11(cf);
				break;
			case 12 :
				pl12(cf);
				break;
			case 13:
			case 14 :
				pl13(cf);
				break;
			case 15:
			case 26 :
				pl15(cf);
				break;
			case 20 :
				pl20(cf);
				break;
			}
			if ((mpers != 0) && (ipers != 10))  mpers = ipers;
			if ((mpers == 0) && (ipers > 0))
				if ((s.mlieu == 13) || (s.mlieu == 14))  cavegre();
				else if (ipers == 10) {
					ipers = 0;
					if (! brt) {
						brt = true;
						hdb = readclock();
						haz = hazard(1, 5);
						if (haz < 5) {
							clsf3();
							ecrf2();
							ecr3(m1);
							haz = (hazard(0, 4)) - 2;
							parole(1, haz, 1);
							clsf3();
						}
					}
				}
			if (am)  draw_menu();
		}
	}
	hfb = readclock();
	if ((brt) && ((hfb - hdb) > 17)) {
		nouvp(li, cx);
		brt = false;
		hdb = 0;
		if ((s.mlieu > 0) && (s.mlieu < 10))  anyone = true;
	}
}

void fenat(char ans) {
	int coul;

	hide_mouse();
	if (gd == cga)  coul = 2;
	else if (gd == her)  coul = 1;
	else coul = 12;
	affcar(gd, 306, 193, coul, ord(ans));
	boite(300, 191, 16, 8, 15);
	show_mouse();
}


/* NIVEAU 8 */
void afdes(int ad) {
	taffich();
	dessin(ad);
	okdes = false;
}

void tkey1(bool d) {
	bool quest;
	int x, y, c;
	int key;

	hide_mouse();
	fenat('K');
	while (keypressed())  key = testou();
	do {
		if (d)  tinke();
		quest = keypressed();
		read_pos_mouse(x, y, c);
	} while (!(quest || (c != 0) || (d && anyone)));
	if (quest)  key = testou();
	clic = false;
	show_mouse();
}

void tmlieu(int mli) {
	int cx, j, i, tail;
	Common::String nomp;
	char st[1410];


	if (mli == 26)  mli = 15;
	i = 1;
	while ((i < 8) && (v_lieu[i][mli] != 0)) {
		cx = v_lieu[i][mli];
		deline(cx + c_tmlieu, st, tail);
		nomp = delig;
		while (nomp.size() < 30)  nomp = nomp + ' ';
		menut(depl[i], nomp);
		i = i + 1;
	}
	nomp = "*                   ";
	for (cx = 7; cx >= i; cx --) menut(depl[cx], nomp);
}


/* NIVEAU 7 */
void tlu(int af, int ob) {
	caff = 32;
	afdes(0);
	repon(6, ob + 4000);
	repon(2, 999);
	tkey1(true);
	caff = af;
	msg[3] = no_choice;
	crep = 998;
}

void delin(int n) {
	char s[1410];
	int t;

	deline(n, s, t);
}

void affrep() {
	caff = s.mlieu;
	crep = s.mlieu;
}

void mfouen()

{
	int cx;

	tmlieu(s.mlieu);
	for (cx = 1; cx <= 11; cx ++) menu_enable(men[cx]);
	/* menu_enable( scacher);
	 menu_enable(menup, attacher);
	 menu_enable(menup, defoncer);
	 menu_enable(menup, dormir);
	 menu_enable(menup, entrer);
	 menu_enable(menup, fermer);
	 menu_enable(menup, frapper);
	 menu_enable(menup, manger);
	 menu_enable(menup, mettre);
	 menu_enable(menup, ouvrir);
	 menu_enable(menup, sortir);  */
	menut(sonder, " sonder   ");
	menut(soulever, " soulever ");
}

void atf3f8(int &key) {
	do {
		key = testou();
	} while (!((key == 61) || (key == 66)));
}

/* NIVEAU 6 */

void tperd() {
	int cx;

	initouv();
	ment = 0;
	iouv = 0;
	mchai = 0;
	mfouen();
	if (! blo)  t11(21, cx);
	perdu = true;
	clsf1();
	boite(60, 35, 400, 50, 15);
	repon(9, crep);
	clsf2();
	clsf3();
	col = false;
	syn = false;
	okdes = false;
}

void tsort() {
	int cx;

	if ((iouv > 0) && (s.mlieu != 0))
		if (s.conf < 50)  s.conf = s.conf + 2;
		else s.conf = s.conf + (s.conf / 10);
	for (cx = 1; cx <= 7; cx ++) touv[cx] = chr(0);
	ment = 0;
	iouv = 0;
	mchai = 0;
	debloc(s.mlieu);
}

void st4(int ob) {
	crep = 997;
	switch (ob) {
	case 114 :
		crep = 109;
		break;
	case 110 :
		crep = 107;
		break;
	case 158 :
		crep = 113;
		break;
	case 152:
	case 153:
	case 154:
	case 155:
	case 156:
	case 150:
	case 100:
	case 157:
	case 160:
	case 161 :
		tlu(caff, ob);
		break;
	}
}

void cherjer(int ob, bool &d) {
	int cx;

	d = false;
	for (cx = 1; cx <= 6; cx ++) d = (d || (ord(s.sjer[cx]) == ob));
	if (s.derobj == ob)  d = true;
}

void st1sama() {
	s.mlieu = 10;
	affrep();
}

void modinv() {
	int cx, cy, tay, i, r;
	char nom[1410];
	Common::String nomp;

	cy = 0;
	for (cx = 1; cx <= 6; cx ++)
		if (s.sjer[cx] != chr(0)) {
			cy = succ(int, cy);
			r = (ord(s.sjer[cx]) + 400);
			deline(r - 501 + c_st41, nom, tay);
			nomp = delig;
			menut(invt[cy], nomp);
			menu_enable(invt[cx]);
		}
	if (cy < 6)
		for (cx = cy + 1; cx <= 6; cx ++) {
			menut(invt[cx], "                       ");
			menu_disable(invt[cx]);
		}
}

void sparl(float adr, float rep) {
	const int haut[9] = { 0, 0, 1, -3, 6, -2, 2, 7, -1 };
	int key, ad, tay, repint;
	char st[1410];

	repint = abs((int)rep);
	hide_mouse();
	deline(repint + c_paroles, st, tay);
	afftex(st, 230, 4, 65, 24, 5);
	f3f8();
	/*boite(43,30,90,19,15);
	gotoxy(7,5);
	write('F3: Encore');
	gotoxy(7,6);
	write('F8: Stop');*/
	key = 0;
	do {
		parole(repint, haut[caff - 69], 0);
		atf3f8(key);
	} while (!(key == 66));
	hirs();
	show_mouse();
}

void finfouil() {
	fouil = false;
	obpart = false;
	cs = 0;
	is = 0;
	mfouen();
}

void mfoudi() {
	int cx;

	for (cx = 1; cx <= 7; cx ++) menu_disable(depl[cx]);
	for (cx = 1; cx <= 11; cx ++) menu_disable(men[cx]);
	/* menu_disable(menup, scacher);
	 menu_disable(menup, attacher);
	 menu_disable(menup, defoncer);
	 menu_disable(menup, dormir);
	 menu_disable(menup, entrer);
	 menu_disable(menup, fermer);
	 menu_disable(menup, frapper);
	 menu_disable(menup, manger);
	 menu_disable(menup, mettre);
	 menu_disable(menup, ouvrir);
	 menu_disable(menup, sortir);  */
	menut(sonder, " -SUITE- ");
	menut(soulever, " -STOP-  ");
}

void mennor() {
	menu_up(msg[3]);
}

void premtet() {
	dessine(ades, 10, 80);
	boite(18, 79, 155, 91, 15);
}

/* NIVEAU 5 */
void ajchai() {
	int cx, cy, lderobj;


	cy = (acha + pred(int, pred(int, mchai) * 10));
	cx = 0;
	do {
		cx = cx + 1;
	} while (!((cx > 9) || (tabdon[cy + cx] == 0)));
	if (tabdon[cy + cx] == 0) {
		lderobj = s.derobj;
		tabdon[cy + cx] = lderobj;
	} else crep = 192;
}

void ajjer(int ob) {
	int cx;

	cx = 0;
	do {
		cx = cx + 1;
	} while (!((cx > 5) || (ord(s.sjer[cx]) == 0)));
	if (ord(s.sjer[cx]) == 0) {
		s.sjer[(cx)] = chr(ob);
		modinv();
	} else crep = 139;
}

void t1sama() {    /* On entre dans le manoir */
	int j, h, m;
	bool d;

	calch(j, h, m);
	if ((h < 5) && (s.mlieu > 18)) {
		cherjer(137, d);
		if (! d) {        /* On n'a pas les cl‚s et il est tard */
			crep = 1511;
			tperd();
		} else st1sama();
	} else if (! s.ipre) {     /* c'est votre premiŠre fois ? */
		ipers = 255;
		affper(ipers);
		caff = 77;
		afdes(0);
		boite(223, 47, 155, 91, 15);
		repon(2, 33);
		tkey1(false);
		mennor();
		hide_mouse();
		hirs();
		premtet();
		sparl(0, 140);
		dessine_rouleau();
		pendule();
		show_mouse();
		s.mlieu = 0;
		affrep();
		t5(10);
		if (! blo)  t11(0, m);
		ipers = 0;
		mpers = 0;
		s.ipre = true;
		/*chech;*/
	} else  st1sama();
}

void t1vier() {
	s.mlieu = 17;
	affrep();
}

void t1neig() {
	inei = inei + 1;
	if (inei > 2) {
		crep = 1506;
		tperd();
	} else {
		okdes = true;
		s.mlieu = 19;
		affrep();
	}
}

void t1deva() {
	inei = 0;
	s.mlieu = 21;
	affrep();
}

void t1derr() {
	s.mlieu = 22;
	affrep();
}

void t1deau() {
	crep = 1503;
	tperd();
}

void tctrm() {
	repon(2, (3000 + ctrm));
	ctrm = 0;
}


void quelquun()

{
	int haz, cx;

	if (imen)  erase_menu();
	finfouil();
	crep = 997;
L1:
	if (! cache) {
		if (crep == 997)  crep = 138;
		repon(2, crep);
		if (crep == 138)  parole(5, 2, 1);
		else parole(4, 4, 1);
		if (iouv == 0)  s.conf = s.conf + 2;
		else if (s.conf < 50)  s.conf = s.conf + 4;
		else
			s.conf = s.conf + (3 * (s.conf / 10));
		tsort();
		tmlieu(15);
		tip(ipers, cx);
		caff = 69 + cx;
		crep = caff;
		msg[3] = discut;
		msg[4] = disc[cx];
		syn = true;
		col = true;
	} else {
		haz = hazard(1, 3);
		if (haz == 2) {
			cache = false;
			crep = 137;
			goto L1;
		} else {
			repon(2, 136);
			haz = (hazard(0, 4)) - 2;
			parole(3, haz, 1);
			clsf2();
			person();
			debloc(21);
			affrep();
		}
	}
	if (imen)  draw_menu();
}

void tsuiv() {
	int cx, tbcl;
	int cl, cy;

	cy = acha + ((mchai - 1) * 10) - 1;
	cx = 0;
	do {
		cx = cx + 1;
		cs = cs + 1;
		cl = cy + cs;
		tbcl = tabdon[cl];
	} while (!((tbcl != 0) || (cs > 9)));
	if ((tbcl != 0) && (cs < 11)) {
		is = is + 1;
		caff = tbcl;
		crep = caff + 400;
		if (ipers != 0)  s.conf = s.conf + 2;
	} else {
		affrep();
		finfouil();
		if (cx > 9)  crep = 131;
	}
}

void tfleche() {
	bool qust;
	char touch;
	int dummy, cd, xmo, ymo;

	if (num == 9999)  return;
	fenat(chr(152));
	rect = false;
	do {
		touch = '\0';

		do {
			mov_mouse(qust, touch);
			if (clic)  rect = (x_s < 256 * res) && (y_s < 176) && (y_s > 12);
			tinke();
		} while (!(qust || rect || anyone));

		if (qust && (touch == '\103'))  dummy = do_alert(stpou, 1);
	} while (!((touch == '\73') || ((touch == '\104') && (x != 0) && (y != 0)) ||
	           (anyone) || (rect)));
	if (touch == '\73')  iesc = true;
	if (rect) {
		x = x_s;
		y = y_s;
	}
}

void tcoord(int sx)

{
	int sy, ix, iy, cb, cy, ib;
	int a, b, atdon;


	num = 0;
	crep = 999;
	a = 0;
	atdon = amzon + 3;
	cy = 0;
	while (cy < caff) {
		a = a + tabdon[atdon];
		atdon = atdon + 4;
		cy = succ(int, cy);
	}
	/*     for cy:=0 to caff-1 do
	          a:=a+ tabdon[amzon+3+4*cy];
	     b:= peek(amzon+3+4*caff); */
	b = tabdon[atdon];
	if (b == 0)  goto L1;
	a = a + fleche;
	cb = 0;
	for (cy = 0; cy <= (sx - 2); cy ++) {
		ib = (tabdon[a + cb] << 8) + tabdon[succ(int, a + cb)];
		cb = cb + (ib * 4) + 2;
	}
	ib = (tabdon[a + cb] << 8) + tabdon[succ(int, a + cb)];
	if (ib == 0)  goto L1;
	cy = 1;
	do {
		cb = cb + 2;
		sx = tabdon[a + cb] * res;
		sy = tabdon[succ(int, a + cb)];
		cb = cb + 2;
		ix = tabdon[a + cb] * res;
		iy = tabdon[succ(int, a + cb)];
		cy = cy + 1;
	} while (!(((x >= sx) && (x <= ix) && (y >= sy) && (y <= iy))
	           || (cy > ib)));
	if ((x >= sx) && (x <= ix) && (y >= sy) && (y <= iy))

	{
		num = cy - 1;
		return;
	}
L1:
	crep = 997;
}


void st7(int ob) {
	crep = 183;
	if ((ob == 116) || (ob == 144))  crep = 104;
	if ((ob == 126) || (ob == 111))  crep = 108;
	if (ob == 132)  crep = 111;
	if (ob == 142)  crep = 112;
	if (crep == 183)  st4(ob);
}

void treg(int ob) {
	int mdes;

	mdes = caff;
	caff = ob;
	if (((caff > 29) && (caff < 33)) || (caff == 144) ||
	        (caff == 147) || (caff == 149) || (msg[4] == sregarder)) {
		afdes(0);
		if ((caff > 29) && (caff < 33))  repon(2, caff);
		else repon(2, (caff + 400));
		tkey1(true);
		caff = mdes;
		msg[3] = 0;
		crep = 998;
	} else {
		obpart = true;
		crep = caff + 400;
		mfoudi();
	}
}

void avpoing(int &ob) {
	crep = 999;
	if (s.derobj != 0)  ajjer(s.derobj);
	if (crep != 139) {
		modobj(ob + 400);
		s.derobj = ob;
		ob = 0;
	}
}

void rechai(int &ch) {
	int cx;

	cx = s.mlieu;
	if (s.mlieu == 16)  cx = 14;
	ch = tabdon[achai + (cx * 7) + pred(int, num)];
}

void t23coul(int &l) {
	bool d;

	cherjer(143, d);
	l = 14;
	if (! d) {
		crep = 1512;
		tperd();
	}
}

void maivid() {
	s.derobj = 0;
	modobj(500);
}

void st13(int ob) {
	if ((ob == 114) || (ob == 116) || (ob == 126) || (ob == 132) ||
	        (ob == 111) || (ob == 106) || (ob == 102) || (ob == 100) ||
	        (ob == 110) || (ob == 153) || (ob == 154) || (ob == 155) ||
	        (ob == 156) || (ob == 157) || (ob == 144) || (ob == 158) ||
	        (ob == 150) || (ob == 152))  crep = 999;
	else crep = 105;
}

void aldepl() {
	int dummy;

	dummy = do_alert("[1][Alors, utilisez le menu DEP...][ok]", 1);
}

} // End of namespace Mortevielle
