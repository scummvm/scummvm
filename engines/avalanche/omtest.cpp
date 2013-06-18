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

#include "graph.h"
/*#include "Oopmenu.h"*/
/*#include "Rodent.h"*/
/*#include "Crt.h"*/
/*#include "Enhanced.h"*/

namespace Avalanche {

varying_string<5> st;
byte fv;

void graphics() {
	integer gd, gm; ;
	gd = 3;
	gm = 0;
	initgraph(gd, gm, "");
}

/*$F+ ... All ddm__procs and do__procs must be compiled in Far-Call state. */

void ddm__file() {
	{
		start_afresh;
		opt("Load...", 'L', "f3", true);
		opt("Save...", 'S', "f2", false);
		opt("Save As...", 'A', "ctrl-f2", false);
		opt("OS Shell...", 'O', "f2", true);
		opt("Untrash screen", 'U', "f11", true);
		display;
	}
}

void ddm__heart() {
	{
		start_afresh;
		opt("About...", 'A', "shift-f10", true);
		opt("Boss Key", 'B', "alt-B", true);
		opt("Help...", 'H', "f1", true);
		opt("Status screen", 'S', "f12", true);
		opt("Quit", 'Q', "f10", true);
		display;
	}
}

void ddm__action() {
	{
		start_afresh;
		opt("Get up", 'G', "", true);
		opt("Open door", 'O', "", true);
		opt("Pause game", 'P', "", true);
		opt("Look around", 'L', "", true);
		opt("Inventory", 'I', "Tab", true);
		opt("Do the boogie", 'b', "", true);
		display;
	}
}

void ddm__objects() {
	{
		start_afresh;
		opt("Bell", 'B', "", true);
		opt("Wine", 'W', "", true);
		opt("Chastity Belt", 'C', "", true);
		opt("Crossbow Bolt", 't', "", true);
		opt("Crossbow", 'r', "", true);
		opt("Potion", 'P', "", true);
		display;
	}
}

void ddm__people() {
	{
		start_afresh;
		opt("Avalot", 'A', "", true);
		opt("Spludwick", 'S', "", true);
		opt("Arkata", 'k', "", true);
		opt("Dogfood", 'D', "", true);
		opt("Geida", 'G', "", true);
		display;
	}
}

void ddm__use() {
	{
		start_afresh;
		opt("Drink", 'D', "", true);
		opt("Wear", 'W', "", true);
		opt("Give to [du Lustie]", 'G', "", true);
		display;
	}
}

void do__stuff() {
	varying_string<2> st;
	str(o.choicenum + 1, st);
	setfillstyle(1, 6);
	setcolor(14);
	bar(0, 177, 640, 200);
	outtextxy(320, 177, string("You just chose: ") + st);
}

void do__heart() {
	switch (o.choicenum) {
	case 0:
		outtextxy(100, 100, "A really funny game!");
		break;
	case 1:
		outtextxy(100, 120, "You ought to be working!");
		break;
	case 2:
		outtextxy(100, 140, "No help available, so THERE!");
		break;
	case 3:
		outtextxy(100, 160, "Everything's COOL and FROODY!");
		break;
	case 4:
		exit(0);
		break;
	}
}

/*$F- ... End of ddm__procs */

int main(int argc, const char *argv[]) {
	pio_initialize(argc, argv);
	graphics();
	setfillstyle(6, 6);
	bar(0, 0, 639, 199);
	resetmouse;
	m.init;
	o.init;
	{
		create('H', '\3', '#', ddm__heart(), do__heart());
		create('F', "File", '!', ddm__file(), do__stuff());
		create('A', "Action", '\36', ddm__action(), do__stuff());
		create('O', "Objects", '\30', ddm__objects(), do__stuff());
		create('P', "People", '\31', ddm__people(), do__stuff());
		create('W', "With", '\21', ddm__use(), do__stuff());
		update;
	}
	do {
		showmousecursor;
		do {
			getbuttonpressinfo(1);
			getbuttonreleaseinfo(1);
			getbuttonstatus;
			if (menunow)  o.lightup;
		} while (!((buttonpresscount > 0) || (buttonreleasecount > 0) | keypressede));
		hidemousecursor;
		if (buttonpresscount > 0) {
			if (mousey > 10) {
				if (!((o.firstlix) &
				        ((mousex >= flx1) && (mousex <= flx2) &&
				         (mousey >= 12) && (mousey <= fly)))) {
					;     /* Clicked OUTSIDE the menu. */
					if (o.menunow)  wipe;
					setcolor(2);
					for (fv = 1; fv <= 17; fv ++) circle(mousex, mousey, fv * 3);
					setcolor(0);
					for (fv = 1; fv <= 17; fv ++) circle(mousex, mousey, fv * 3);
				}
			} else {
				;     /* Clicked on menu bar */
				m.getmenu(mousex);
			}
		} else {
			;     /* NOT clicked button... */
			if (buttonreleasecount > 0) {
				if ((firstlix) &
				        ((mousex >= flx1) && (mousex <= flx2) &&
				         (mousey >= 12) && (mousey <= fly)))
					select((mousey - 14) / 10);
			} else {
				;     /* NOT clicked or released button, so must be keypress */
				readkeye;
				parsekey(inchar, extd);
			}
		}
	} while (!false);
	return EXIT_SUCCESS;
}

} // End of namespace Avalanche.