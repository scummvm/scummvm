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

/* DROPDOWN	A customized version of Oopmenu (qv). */

#ifndef DROPDOWN2_H
#define DROPDOWN2_H

#include "avalanche/color.h"

#include "common/scummsys.h"
#include "common/str.h"

namespace Avalanche {
class AvalancheEngine;

class Dropdown;

typedef void (Dropdown::*func)();

class headtype {
public:
	Common::String title;
	char trigger, alttrigger;
	byte position;
	int16 xpos, xright;
	func do_setup, do_choose;

	headtype *init(char trig, char alttrig, Common::String name, byte p, func dw, func dc, Dropdown *dr);
	void display();
	void highlight();
	bool extdparse(char c);

private:
	Dropdown *_dr;
};



struct optiontype {
	Common::String title;
	byte trigger;
	Common::String shortcut;
	bool valid;
};

class onemenu {
public:
	optiontype oo[12];
	byte number;
	uint16 width, left;
	bool firstlix;
	int16 flx1, flx2, fly;
	byte oldy; /* used by Lightup */
	bool menunow; /* Is there a menu now? */
	byte menunum; /* And if so, which is it? */
	byte choicenum; /* Your choice? */
	byte highlightnum;

	void init(Dropdown *dr);
	void start_afresh();
	void opt(Common::String n, char tr, Common::String key, bool val);
	void display();
	void wipe();
	void lightup(Common::Point cursorPos); // This makes the menu highlight follow the mouse.
	void displayopt(byte y, bool highlit);
	void movehighlight(int8 add);
	void select(byte n);
	void keystroke(char c);

private:
	Dropdown *_dr;
};



class menuset {
public:
	headtype ddms[8];
	byte howmany;

	void init(Dropdown *dr);
	void create(char t, Common::String n, char alttrig, func dw, func dc);
	void update();
	void extd(char c);
	void getcertain(byte fv);
	void getmenu(int16 x);

private:
	Dropdown *_dr;
};



class Dropdown {
public:
	friend class headtype;
	friend class onemenu;
	friend class menuset;

	onemenu ddm_o;
	menuset ddm_m;

	Common::String people;



	Dropdown(AvalancheEngine *vm);

	void find_what_you_can_do_with_it();

	void parsekey(char r, char re);

	void menu_link(); /* DDM menu-bar funcs */

	void standard_bar();

private:
	AvalancheEngine *_vm;

	static const byte indent = 5;
	static const byte spacing = 10;

	static const byte menu_b = lightgray; /* Windowsy */
	static const byte menu_f = black;
	static const byte menu_border = black;
	static const byte highlight_b = black;
	static const byte highlight_f = white;
	static const byte disabled = darkgray;



	char r;
	byte fv;



	void chalk(int16 x, int16 y, char t, Common::String z, bool valid);

	void hlchalk(int16 x, int16 y, char t, Common::String z, bool valid); // Highlighted. TODO: It's too similar to chalk! Unify these two!!!

	void bleep();


	
	void ddm__game();
	void ddm__file();
	void ddm__action();
	void ddm__people();
	void ddm__objects();
	void ddm__with();

	Common::String himher(byte x); // Returns "im" for boys, and "er" for girls.

	void do__game();
	void do__file();
	void do__action();
	void do__objects();
	void do__people();
	void do__with();

	void checkclick(Common::Point cursorPos); // Only for when the menu's displayed!
};

} // End of namespace Avalanche.

#endif // DROPDOWN2_H
