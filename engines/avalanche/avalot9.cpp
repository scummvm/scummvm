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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/*
  €ﬂ‹ €ﬂ‹ ‹ﬂﬂ‹  ﬂ€ﬂ €ﬂﬂ  ‹ﬂ ﬂ€ﬂ      ‹ﬂ€ﬂ‹  ﬂ€ﬂ €‹  € ‹€ﬂﬂ  ﬂ€ﬂ €ﬂ‹ €ﬂﬂ €
  €ﬂ  €€  €  € ‹ €  €ﬂﬂ ﬂ‹   €      €  €  €  €  € €‹€  ﬂﬂﬂ‹  €  €€  €ﬂﬂ €
  ﬂ   ﬂ ﬂ  ﬂﬂ   ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ      ﬂ  ﬂ  ﬂ ﬂﬂﬂ ﬂ  ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ ﬂ ﬂﬂﬂ ﬂﬂﬂ

                 AVALOT           The kernel of the program. */


#include "graph.h"
/*#include "Crt.h"*/
/*#include "Trip5.h"*/
/*#include "Gyro.h"*/
/*#include "Lucerna.h"*/
/*#include "Scrolls.h"*/
/*#include "Basher.h"*/
/*#include "Dropdown.h"*/
/*#include "Pingo.h"*/
/*#include "Logger.h"*/
/*#include "Timeout.h"*/
/*#include "Celer.h"*/
/*#include "Enid.h"*/
/*#include "Incline.h"*/
/*#include "Closing.h"*/
/*#include "Visa.h"*/


namespace Avalanche {

void setup() {
	integer gd, gm;

	checkbreak = false;
	visible = m_no;
	to_do = 0;
	lmo = false;
	resetscroll;
	Randomize();
	setup_vmc;
	on_virtual;
	gd = 3;
	gm = 0;
	initgraph(gd, gm, "");
	holdthedawn = true;
	dusk;
	cmp = 177;
	mouse_init;  /*on;*/ dropsok = true;
	ctrl = ckey;
	oldjw = 177;
	mousetext = "";
	c = 999;
	settextjustify(0, 0);
	ddmnow = false;
	load_digits;
	cheat = false;
	cp = 0;
	curpos = 1;
	quote = true;
	ledstatus = 177;
	defaultled = 2;
	/* TSkellern:=0; { Replace with a more local variable sometime }*/
	dna.rw = stopped;
	enid_filename = ""; /* undefined. */
	toolbar;
	state(2);
	copy03;
	lastscore = "TJA";

	/* for gd:=0 to 1 do
	 begin
	  setactivepage(gd); outtextxy(7,177,chr(48+gd));
	 end;*/

	loadtrip;

	if ((filetoload == "") & (~ reloaded))
		newgame; /* no game was requested- load the default */
	else {
		if (~ reloaded)  avvy_background;
		standard_bar;
		sprite_run;
		if (reloaded)  edna_reload;
		else {
			/* Filename given on the command line (or loadfirst) */
			edna_load(filetoload);
			if (there_was_a_problem) {
				display("So let's start from the beginning instead...");
				holdthedawn = true;
				dusk;
				newgame;
			}
		}
	}

	if (~ reloaded) {
		soundfx = ~ soundfx;
		fxtoggle;
		thinkabout(money, a_thing);
	}

	get_back_loretta;
	gm = getpixel(0, 0);
	setcolor(7);
	holdthedawn = false;
	dawn;
	cursoron = false;
	cursor_on;
	newspeed;

	if (~ reloaded)
		dixi('q', 83); /* Info on the game, etc. */
}

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	setup();

	do {

		clock;
		keyboard_link;
		menu_link;
		readstick;
		force_numlock;
		get_back_loretta;
		trippancy_link;
		pics_link;
		checkclick;

		if (visible == m_virtual)  plot_vmc(mx, my, cp);
		flip_page; /* <<<! */
		slowdown;
		if (visible == m_virtual)  wipe_vmc(cp);

		one_tick;

	} while (!lmo);

	restorecrtmode();
	if (logging)  close(logfile);

	end_of_program;
	return EXIT_SUCCESS;
}

/*  typein; commanded; last:=current; */

} // End of namespace Avalanche.