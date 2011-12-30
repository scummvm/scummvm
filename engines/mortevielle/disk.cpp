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

#include "mortevielle/disk.h"

namespace Mortevielle {

void dem1() {
	untyped_file f;
	int k;

	/* -- demande de disk 1 -- */
	assign(f, "mort.005");
	/*$i-*/
	k = ioresult;
	reset(f);
	while (ioresult != 0) {
		show_mouse();
		k = do_alert(al_mess, 1);
		hide_mouse();
		reset(f);
	}
	close(f);
}

void takesav(int n) {
	untyped_file f;
	int i;
	Common::String st;

	dem1();
	/* -- chargement du fichier 'sauve#n.mor' -- */
	st = string("sav") + chr(n + 48) + ".mor";
	assign(f, st);
	reset(f, 497);
	blockread(f, s1, 1);
	if (ioresult != 0) {
		i = do_alert(err_mess, 1);
		exit(0);
	}
	s = s1;
	for (i = 0; i <= 389; i ++) tabdon[i + acha] = bufcha[i];
	close(f);
}

void ld_game(int n) {
	hide_mouse();
	maivid();
	takesav(n);
	/* -- disquette 2 -- */
	dem2();
	/* -- mises en place -- */
	theure();
	dprog();
	antegame();
	show_mouse();
}

void sv_game(int n) {
	untyped_file f;
	int i;

	hide_mouse();
	tmaj3();
	dem1();
	/* -- sauvegarde du fichier 'sauve#n.mor' -- */
	for (i = 0; i <= 389; i ++) bufcha[i] = tabdon[i + acha];
	s1 = s;
	if (s1.mlieu == 26)  s1.mlieu = 15;
	assign(f, string("sav") + chr(n + 48) + ".mor");
	rewrite(f, 497);
	blockwrite(f, s1, 1);
	close(f);
	dem2();
	show_mouse();
}

} // End of namespace Mortevielle
