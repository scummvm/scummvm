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

#include "mortevielle/actions.h"
#include "mortevielle/alert.h"
#include "mortevielle/menu.h"
#include "mortevielle/mor.h"
#include "mortevielle/mor2.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/prog.h"
#include "mortevielle/taffich.h"
#include "mortevielle/var_mor.h"
#include "mortevielle/mouse.h"

namespace Mortevielle {

/* overlay */ void change_gd(int ngd) {
	hide_mouse();
	gd = ngd;
	hirs();
	init_mouse();
	show_mouse();
	dessine_rouleau();
	tinke();
	pendule();
	if (ipers != 0)  affper(ipers);
	else person();
	clsf2();
	clsf3();
	maff = 68;
	afdes(0);
	repon(2, crep);
	menu_aff();
}

/* overlay */ void antegame() {
	int k;
	bool test[3];
	char g[8];


	hide_mouse();
	imen = false;
	g[1] = 'M';
	perdu = true;
	anyone = false;
	okdes = true;
	test[0] = false;
	test[1] = false;
	g[0] = '\040';
	col = false;
	tesok = true;
	test[2] = false;
	g[7] = g[0];
	g[2] = 'A';
	cache = false;
	brt = false;
	maff = 68;
	g[5] = 'E';
	mnumo = 0;
	prebru = 0;
	g[4] = 'T';
	x = 0;
	y = 0;
	num = 0;
	hdb = 0;
	hfb = 0;
	cs = 0;
	is = 0;
	k = 0;
	ment = 0;
	syn = true;
	fouil = true;
	mchai = 0;
	inei = 0;
	initouv();
	g[3] = 'S';
	g[6] = 'R';
	iouv = 0;
	dobj = 0;
	affrep();
	stpou = ind_mess;
	while ((test[k] == false) && (k < 2)) {
		++k;

		// Original read in the boot sector here of each drive and did some calculations -
		// presumably for copy protection. This has been replaced with hardcoded success
		test[k] = true;

		okdes = false;
		solu = true;
		perdu = false;
		fouil = false;
	}
	person();
	tinke();
	pendule();
	afdes(0);
	repon(2, crep);
	clsf3();
	solu = false;
	tmlieu(s.mlieu);
	modinv();
	if (s.derobj != 0)  modobj2(s.derobj + 400, test[1], test[2]);
	else tesok = test[1] || test[2];
	show_mouse();
}


/* NIVEAU 3 */
/* procedure PROGRAMME */
void tmaj3() {
	int j, h, m;

	calch(j, h, m);
	if (m == 30)  m = 1;
	h = h + (j * 24);
	m = m + (h * 2);
	s.heure = chr(m);
}

void tsitu()

{
	int h, j, m;

	if (! col)  clsf2();
	syn = false;
	iesc = false;
	if (anyone)  goto L1;
	if (brt)
		if ((msg[3] == depla) || (msg[4] == sortir) || (msg[4] == dormir) ||
		        (msg[4] == manger)) {
			ctrm = 4;
			goto L2;
		}
	if (msg[3] == depla)       taller();    //Translation: go to
	if (msg[3] == discut)      tparler();   //Translation: talk
	if (msg[3] == invent)      tsprendre(); //Translation: inventory/take
	if (msg[4] == attacher)    tattacher(); //Translation: tie
	if (msg[4] == attendre)    tattendre(); //Translation: wait
	if (msg[4] == defoncer)    tdefoncer(); //Translation: smash
	if (msg[4] == dormir)      tdormir();   //Translation: sleep
	if (msg[4] == ecouter)     tecouter();  //Translation: listen
	if (msg[4] == entrer)      tentrer();   //Translation: enter
	if (msg[4] == fermer)      tfermer();   //Translation: close
	if (msg[4] == fouiller)    tfouiller(); //Translation: search
	if (msg[4] == frapper)     tfrapper();  //Translation: hit
	if (msg[4] == gratter)     tgratter();  //Translation: scratch
	if (msg[4] == lire)        tlire();     //Translation: read
	if (msg[4] == manger)      tmanger();   //Translation: eat
	if (msg[4] == mettre)      tmettre();   //Translation: put
	if (msg[4] == ouvrir)      touvrir();   //Translation: open
	if (msg[4] == prendre)     tprendre();  //Translation: take
	if (msg[4] == regarder)    tregarder(); //Translation: look
	if (msg[4] == sentir)      tsentir();   //Translation: smell
	if (msg[4] == sonder)      tsonder();   //Translation: probe
	if (msg[4] == sortir)      tsortir();   //Translation: exit
	if (msg[4] == soulever)    tsoulever(); //Translation: lift
	if (msg[4] == tourner)     ttourner();  //Translation: turn
	if (msg[4] == scacher) {                //Translation: hide
		tcacher();
		goto L1;
	}
	if (msg[4] == sfouiller)   tsfouiller();//Translation: search 
	if (msg[4] == slire)       tslire();    //Translation: read
	if (msg[4] == sposer)      tposer();    //Translation: put
	if (msg[4] == sregarder)   tsregarder();//Translation: look
	cache = false;
L1:
	if (anyone) {
		quelquun();
		anyone = false;
		goto L2;
	}
	calch(j, h, m);
	if ((((h == 12) || (h == 13) || (h == 19)) && (s.mlieu != 10)) ||
	        ((h > 0) && (h < 6) && (s.mlieu != 0)))  s.conf = s.conf + 1;
	if (((s.mlieu < 16) || (s.mlieu > 19)) && (s.mlieu != 23)
	        && (s.mlieu != 0) && (s.derobj != 152) && (! perdu)) {
		if ((s.conf > 99) && (h > 8) && (h < 16)) {
			crep = 1501;
			tperd();
		}
		if ((s.conf > 99) && (h > 0) && (h < 9)) {
			crep = 1508;
			tperd();
		}
		if ((j > 1) && (h > 8) && (! perdu)) {
			crep = 1502;
			tperd();
		}
	}
L2:
	mennor();
}

void sv_game(int n);

void ld_game(int n);

void tecran() {
	const char idem[] = "Idem";
	const int lim = 20000;
	int temps = 0;
	char inkey = '\0';
	bool oo, funct = 0;

	clsf3();
	oo = false;
	ctrm = 0;
	if (! iesc) {
		draw_menu();
		imen = true;
		temps = 0;
		key = 0;
		funct = false;
		inkey = '.';

		do {
			mdn();
			tinke();
			mov_mouse(funct, inkey);
			CHECK_QUIT;
			temps = temps + 1;
		} while (!((choisi) || (temps > lim) || (funct) || (anyone)));

		erase_menu();
		imen = false;
		if ((inkey == '\1') || (inkey == '\3') || (inkey == '\5') || (inkey == '\7') || (inkey == '\11')) {
			change_gd((uint)pred(int, ord(inkey)) >> 1);
			return;
		}
		if (choisi && (msg[3] == sauve))  sv_game(msg[4] & 7);
		if (choisi && (msg[3] == charge))  ld_game(pred(int, msg[4] & 7));
		if (inkey == '\103') {       /* F9 */
			temps = do_alert(stpou, 1);
			return;
		} else if (inkey == '\77') {
			if ((mnumo != no_choice) && ((msg[3] == action) || (msg[3] == saction))) {
				msg[4] = mnumo;
				ecr3(idem);
			} else return;
		} else if (inkey == '\104') {
			if ((x != 0) && (y != 0))  num = 9999;
			return;
		}
	}
	if (inkey == '\73') {
		arret = true;
		tmaj3();
	} else {
		if ((funct) && (inkey != '\77'))  return;
		if (temps > lim) {
			repon(2, 141);
			if (num == 9999)  num = 0;
		} else {
			mnumo = msg[3];
			if ((msg[3] == action) || (msg[3] == saction))  mnumo = msg[4];
			if (! anyone) {
				if ((fouil) || (obpart)) {
					if (y_s < 12)  return;
					if ((msg[4] == sonder) || (msg[4] == soulever)) {
						oo = true;
						if ((msg[4] == soulever) || (obpart)) {
							finfouil();
							caff = s.mlieu;
							crep = 998;
						} else tsuiv();
						mennor();
					}
				}
			}
			do {
				if (! oo)  tsitu();
				if ((ctrm == 0) && (! perdu) && (! solu)) {
					taffich();
					if (okdes) {
						okdes = false;
						dessin(0);
					}
					if ((! syn) || (col))  repon(2, crep);
				}
			} while (!(! syn));
			if (ctrm != 0)  tctrm();
		}
	}
}

/* NIVEAU 1 */

void theure() {
	vj = ord(s.heure);
	vh = vj % 48;
	vj = vj / 48;
	vm = vh % 2;
	vh = vh / 2;
	heu = vh;
	if (vm == 1)  min = 30;
	else min = 0;
}


void tjouer() {
	antegame();
	do {
		tecran();
		CHECK_QUIT;
	} while (!((arret) || (solu) || (perdu)));
	if (solu)  tmaj1();
	else if (perdu)  tencore();
}

} // End of namespace Mortevielle
