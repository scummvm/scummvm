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

#include "mortevielle/mort.h"
#include "mortevielle/var_mor.h"          /* les variables */                       //Translation: Variables
#include "mortevielle/keyboard.h"          /* la gestion du clavier */              //Translation: Keyboard manager
#include "mortevielle/mouse.h"          /* la souris  */                            //Translation: Mouse
#include "mortevielle/level15.h"          /* les fonctions de base */               //Translation: Lowlevel functions
#include "mortevielle/droite.h"          /* pour faire une droite */                //Translation: Display a lign
#include "mortevielle/boite.h"                                                      //Translation: Box
#include "mortevielle/sprint.h"          /* l'affichage des textes */               //Translation: Display texts
#include "mortevielle/outtext.h"          /* fonctions sup‚rieures d'affichage */   //Translation: Highlevel display functions
#include "mortevielle/parole.h"          /* les fonctions de la parole */           //Translation: Speech functions
#include "mortevielle/alert.h"
#include "mortevielle/parole2.h"          /* les fonctions de la parole */          //Translation: Speech functions
/*  debug              le debugging */                                              //Translation: Debugger
#include "mortevielle/ques.h"          /* les questions */                          //Translation: Questions
#include "mortevielle/menu.h"          /* les fonctions du menu */                  //Translation: Menu functions
#include "mortevielle/mor.h"          /* divers */                                  //Translation: Misc
#include "mortevielle/taffich.h"          /* chargement des dessins */              //Translation: pictures loader
#include "mortevielle/ovd1.h"          /* les fonctions sur disque 1 */             //Translation: Disk functions
#include "mortevielle/mor2.h"          /* divers */                                 //Translation: Misc
#include "mortevielle/actions.h"          /* les actions */                         //Translation: Actions
#include "mortevielle/prog.h"          /* fonctions toplevel */                     //Translation: toplevel functions
#include "mortevielle/disk.h"          /* chargements et sauvegardes */             //Translation: Load/Save
/*  ecrihexa            utilitaire */                                               //Translation: Toolbox

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

int mortevielle_main(int argc, const char *argv[]) {
	/*init_debug;*/
	/*  ecri_seg;*/
	//pio_initialize(argc, argv);
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
