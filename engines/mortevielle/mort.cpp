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

#include "ptoc.h"

#include "mortevielle/mort.h"
#include "mortevielle/var_mor.h"          /* les variables */
#include "mortevielle/keyboard.h"          /* la gestion du clavier */
#include "mortevielle/mouse.h"          /* la souris  */
#include "mortevielle/level15.h"          /* les fonctions de base */
#include "mortevielle/droite.h"          /* pour faire une droite */
#include "mortevielle/boite.h"
#include "mortevielle/sprint.h"          /* l'affichage des textes */
#include "mortevielle/outtext.h"          /* fonctions sup‚rieures d'affichage */
#include "mortevielle/parole.h"          /* les fonctions de la parole */
#include "mortevielle/alert.h"
#include "mortevielle/parole2.h"          /* les fonctions de la parole */
/*  debug              le debugging */
#include "mortevielle/ques.h"          /* les questions */
#include "mortevielle/menu.h"          /* les fonctions du menu */
#include "mortevielle/mor.h"          /* divers */
#include "mortevielle/taffich.h"          /* chargement des dessins */
#include "mortevielle/ovd1.h"          /* les fonctions sur disque 1 */
#include "mortevielle/mor2.h"          /* divers */
#include "mortevielle/actions.h"          /* les actions */
#include "mortevielle/prog.h"          /* fonctions toplevel */
#include "mortevielle/disk.h"          /* chargements et sauvegardes */
/*  ecrihexa            utilitaire */

namespace Mortevielle {

void divers(int np, bool b) {
	teskbd();
	do {
		parole(np, 0, 0);
		atf3f8(key);
		if (newgd != gd) {
			gd = newgd;
			hirs();
			aff50(b);
		}
	} while (!(key == 66));
}

/* NIVEAU 0 */

int main(int argc, const char *argv[]) {
	/*init_debug;*/
	/*  ecri_seg;*/
	pio_initialize(argc, argv);
	gd = cga;
	newgd = gd;
	zuul = false;
	tesok = false;
	chartex();
	charpal();
	charge_cfiph();
	charge_cfiec();
	zzuul(adcfiec + 161, 0, 1644);
	c_zzz = 1;
	init_nbrepm();
	init_mouse();
	/*  crep:=memw[$7f00:0];
	  memw[$7f00:0]:= crep+1;
	  if (memw[$7f00:0]<> crep+1) then
	     begin
	       hirs;
	       show_mouse;
	       crep:=do_alert('[1][ 512 k  minimum requis  !!][OK]',1);
	       clrscr;
	       halt;
	     end;
	  if (Dseg+$1000>adbruit5) then
	     begin
	       hirs;
	       show_mouse;
	       crep:=do_alert('[1][ Il ne reste pas assez de |place m‚moire  !!][OK]',1);
	       clrscr;
	       halt;
	     end;*/
	init_lieu();
	arret = false;
	sonoff = false;
	f2_all = false;
	textcolor(9);
	teskbd();
	dialpre();
	newgd = gd;
	teskbd();
	if (newgd != gd)  gd = newgd;
	hirs();
	ades = 0x7000;
	aff50(false);
	mlec = 0;
	divers(142, false);

	ani50();
	divers(143, true);
	suite();
	music();
	adzon();
	takesav(0);
	if (rech_cfiec)  charge_cfiec();
	for (crep = 1; crep <= c_zzz; crep ++) zzuul(adcfiec + 161, 0, 1644);
	charge_bruit5();
	init_menu();

	theure();
	dprog();
	hirs();
	dessine_rouleau();
	show_mouse();
	do {
		tjouer();
	} while (!arret);
	hide_mouse();
	clrscr;
	/*out_debug;*/
	return EXIT_SUCCESS;
}

} // End of namespace Mortevielle
