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

#include "mortevielle/ovd1.h"

namespace Mortevielle {

void dem2() {
	untyped_file f;
	int k;

	/* -- demande de disk 2 -- */
	while (ioresult != 0);
	assign(f, "mort.001");
	/*$i-*/
	reset(f);
	while (ioresult != 0) {
		show_mouse();
		k = do_alert(al_mess2, 1);
		hide_mouse();
		reset(f);
	}
	close(f);
}

void ani50() {
	crep = animof(1, 1);
	pictout(adani, crep, 63, 12);
	crep = animof(2, 1);
	pictout(adani, crep, 63, 12);
	f2_all = (res == 1);
	repon(2, c_paroles + 143);
}

/*overlay procedure apzuul;
  begin
    if (c_zzz=1) or (c_zzz=2) then
     begin
       zzuul(adcfiec+161,0,1644);
       c_zzz:=succ(c_zzz);
     end;
  end;*/

/* overlay */ void aff50(bool c) {
	int k;

	caff = 50;
	maff = 0;
	taffich();
	dessine(ades, 63, 12);
	if (c)  ani50();
	else repon(2, c_paroles + 142);
	f3f8();
}

/* overlay */ void init_menu() {
	int i, j, tai;
	phrase st;
	untyped_file f;

	assign(f, "menufr.mor");
	/*$i-*/
	reset(f, 144);
	blockread(f, lettres, 1);
	close(f);
	/* ----  Demande de changement de disquette  ---- */
	dem2();


	for (i = 1; i <= 8; i ++)
		inv[i] = "*                     ";
	inv[7] = "< -*-*-*-*-*-*-*-*-*- ";
	for (i = 1; i <= 7; i ++)
		dep[i] = "*                       ";
	i = 1;
	do {
		deline(i + c_action, st, tai);
		act[i] = delig + "          ";
		if (i < 9) {
			if (i < 6) {
				deline(i + c_saction, st, tai);
				self_[i] = delig + "       ";
			}
			deline(i + c_dis, st, tai);
			dis[i] = delig + ' ';
		}
		i = succ(int, i);
	} while (!(i == 22));
	for (i = 1; i <= 8; i ++) {
		disc[i] = 0x500 + i;
		if (i < 8)  depl[i] = 0x200 + i;
		invt[i] = 0x100 + i;
		if (i > 6)  menu_disable(invt[i]);
	}
	msg3 = no_choice;
	msg4 = no_choice;
	msg[3] = no_choice;
	msg[4] = no_choice;
	clic = false;
}


/* overlay */ void charpal() {
	file<tabdb> f;
	file<tfxx> ft;
	int i, j, k;
	file<byte> fb;
	byte b;

	assign(ft, "fxx.mor");
	/*$i-*/
	reset(ft);
	if (ioresult != 0) {
		caff = do_alert(err_mess, 1);
		exit(0);
	}
	ft >> l;
	close(ft);
	assign(f, "plxx.mor");
	reset(f);
	for (i = 0; i <= 90; i ++) f >> tabpal[i];
	close(f);
	assign(fb,  "cxx.mor");
	reset(fb);
	for (j = 0; j <= 90; j ++) {
		fb >> palcga[j].p;
		for (i = 0; i <= 15; i ++) {
			nhom &with = palcga[j].a[i];

			fb >> b;
			with.n = (cardinal)b >> 4;
			with.hom[0] = ((cardinal)b >> 2) & 3;
			with.hom[1] = b & 3;
		}
	}
	palcga[10].a[9] = palcga[10].a[5];
	for (j = 0; j <= 14; j ++) {
		fb >> tpt[j].tax;
		fb >> tpt[j].tay;
		for (i = 1; i <= 20; i ++)
			for (k = 1; k <= 20; k ++)
				fb >> tpt[j].des[i][k];
	}
	close(fb);
}

/* overlay */ void chartex() {
	int i;
	untyped_file fibyte;
	phrase s;

	/* debug('o3 chartex'); */
	i = 0;
	assign(fibyte, "TXX.INP");
	/*$i-*/
	reset(fibyte);
	blockread(fibyte, t_mot, 125);
	close(fibyte);
	assign(sauv_t, "TXX.NTP");
	reset(sauv_t);
	do {
		sauv_t >> t_rec[i];
		i = i + 1;
	} while (!(eof(sauv_t)));
	close(sauv_t);
	deline(578, s, i);
	al_mess = delig;
	deline(579, s, i);
	err_mess = delig;
	deline(580, s, i);
	ind_mess = delig;
	deline(581, s, i);
	al_mess2 = delig;
}

/* overlay */ void dialpre()

{
	int cy, tay;
	phrase st;
	float ix;
	char ch;


	/* debug('o3 dialpre'); */
	cy = 0;
	clrscr;
	textcolor(9);
	do {
		cy = cy + 1;
		deline(cy + c_dialpre, st, tay);
		gotoxy(40 - tay / 2, wherey + 1);
		output << delig;
	} while (!(cy == 20));
	ix = 0;
	do {
		ix = ix + 1;
	} while (!(keypressed() | (ix == 5e5)));
	crep = 998;
	textcolor(1);
	gotoxy(1, 21);
	clreol;
	gotoxy(1, 23);
	output << "CARTE GRAPHIQUE      CGA    EGA    HERCULE/AT&T400    TANDY    AMSTRAD1512";
	gotoxy(12, 24);
	output << "Ctrl       C      E            H             T           A";
	do {
		input >> kbd >> ch;
	} while (!(set::of('\1', '\3', '\5', '\24', '\10', eos).has(ch)));
	switch (ch) {
	case '\1':
	case '\3':
	case '\5' :
		gd = (cardinal)ord(ch) >> 1;
		break;
	case '\10' :
		gd = her;
		break;
	case '\24' :
		gd = tan;
		break;
	}
	gotoxy(1, 24);
	clreol;
	gotoxy(1, 23);
	clreol;
	gotoxy(26, 23);
	output << "Jeu au Clavier / … la Souris";
	textcolor(4);
	gotoxy(33, 23);
	output << 'C';
	gotoxy(48, 23);
	output << 'S';
	do {
		input >> kbd >> ch;
	} while (!(set::of('C', 'S', eos).has(upcase(ch))));
	int_m = (upcase(ch) == 'S');
}

/* overlay */ void init_lieu() {
	file<tab_mlieu> f_lieu;

	/* debug('o3 init_lieu'); */
	assign(f_lieu, "MXX.mor");
	/*$i-*/
	reset(f_lieu);
	f_lieu >> v_lieu;
	close(f_lieu);
}


/* overlay */ void music() {
	untyped_file fic;
	int k;
	bool fin;
	char ch;
	float x, y;


	/* debug('o3 music'); */
	if (sonoff)  return;
	rech_cfiec = true;
	assign(fic, "mort.img");
	/*$i-*/
	reset(fic);
	blockread(fic, mem[0x3800 + 0], 500);
	blockread(fic, mem[0x47a0 + 0], 123);
	close(fic);
	demus(0x3800, 0x5000, 623);
	addfix = (float)((tempo_mus - addv[1])) / 256;
	cctable(tbi);

	fin = false;
	k = 0;
	do {
		fin = keypressed();
		musyc(tbi, 9958 , tempo_mus);
		k = k + 1;
		fin = fin | keypressed() | (k >= 5);
	} while (!fin);
	while (keypressed())  input >> kbd >> ch;
}


/* overlay */ void charge_bruit5() {
	untyped_file f;

	assign(f, "bruit5");
	/*$i-*/
	reset(f);
	if (ioresult != 0) {
		caff = do_alert(err_mess, 1);
		exit(0);
	}
	blockread(f, mem[adbruit5 + 0], 149);
	/*blockread(f,mem[$5CB0:0],100);
	blockread(f,mem[$3D1F:0],49);*/
	close(f);
}

/* overlay */ void charge_cfiec() {
	untyped_file f;

	assign(f, "cfiec.mor");
	/*$i-*/
	reset(f);
	blockread(f, mem[adcfiec + 0], 511);
	blockread(f, mem[adcfiec + 4088 + 0], 311);
	close(f);
	rech_cfiec = false;
}


/* overlay */ void charge_cfiph() {
	untyped_file f;

	assign(f, "cfiph.mor");
	/*$i-*/
	reset(f, 256);
	blockread(f, t_cph, 50);
	close(f);
}


/* overlay */ void suite() {
	varying_string<25> cpr;

	hirs();
	repon(7, 2035);
	caff = 51;
	taffich();
	teskbd();
	if (newgd != gd)  gd = newgd;
	hirs();
	dessine(ades, 0, 0);
	gotoxy(20 * pred(int, res) + 8, 24);
	textcolor(7);
	cpr = "COPYRIGHT 1989 : LANKHOR";
	if (set::of(ega, ams, cga, eos).has(gd))  output << cpr;
	else {
		putxy(104 + 72 * res, 190);
		writeg(cpr, 0);
	}
}

} // End of namespace Mortevielle
