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

/* DROPDOWN	A customised version of Oopmenu (qv). */

#ifndef DROPDOWN2_H
#define DROPDOWN2_H

#include "avalanche/color.h"

#include "common/scummsys.h"
#include "common/str.h"



namespace Avalanche {
class AvalancheEngine;

class Dropdown;



typedef void (Dropdown::*DropdownFunc)();



class HeadType {
public:
	Common::String _title;
	char _trigger, _altTrigger;
	byte _position;
	int16 _xpos, _xright;
	DropdownFunc _setupFunc, _chooseFunc;

	void init(char trig, char alTtrig, Common::String title, byte pos, DropdownFunc setupFunc, DropdownFunc chooseFunc, Dropdown *dr);
	void draw();
	void highlight();
	bool parseAltTrigger(char key);

private:
	Dropdown *_dr;
};



struct OptionType {
	Common::String _title;
	byte _trigger;
	Common::String _shortcut;
	bool _valid;
};



class MenuItem {
public:
	OptionType _options[12];
	byte _optionNum;
	uint16 _width, _left;
	bool _firstlix;
	int16 _flx1, _flx2, fly;
	byte _oldY; // used by lightUp */
	bool _activeNow; // Is there an active option now?
	byte _activeNum; // And if so, which is it?
	byte _choiceNum; // Your choice?
	byte _highlightNum;

	void init(Dropdown *dr);
	void reset();
	void setupOption(Common::String title, char trigger, Common::String shortcut, bool valid);
	void display();
	void wipe();
	void lightUp(Common::Point cursorPos); // This makes the menu highlight follow the mouse.
	void displayOption(byte y, bool highlit);
	void moveHighlight(int8 inc);
	void select(byte which); // Choose which one you want.
	void parseKey(char c);

private:
	Dropdown *_dr;
};



class MenuBar {
public:
	HeadType _menuItems[8];
	byte _menuNum;

	void init(Dropdown *dr);
	void createMenuItem(char trig, Common::String title, char altTrig, DropdownFunc setupFunc, DropdownFunc chooseFunc);
	void draw();
	void parseAltTrigger(char c);
	void setupMenuItem(byte which);
	void chooseMenuItem(int16 x);

private:
	Dropdown *_dr;
};



class Dropdown {
public:
	friend HeadType;
	friend MenuItem;
	friend MenuBar;

	MenuItem _activeMenuItem;
	MenuBar _menuBar;

	Common::String people;



	Dropdown(AvalancheEngine *vm);

	void parseKey(char r, char re);

	void updateMenu();

	void setupMenu(); // Standard menu bar.

private:
	static const byte kIndent = 5;
	static const byte kSpacing = 10;

	static const byte kMenuBackgroundColor = lightgray;
	static const byte kMenuFontColor = black;
	static const byte kMenuBorderColor = black;
	static const byte kHighlightBackgroundColor = black;
	static const byte kHighlightFontColor = white;
	static const byte kDisabledColor = darkgray;



	AvalancheEngine *_vm;



	Common::String selectGender(byte x); // Returns "im" for boys, and "er" for girls.

	void findWhatYouCanDoWithIt();

	void drawMenuItem(int16 x, int16 y, char t, Common::String z, bool valid);

	void drawHighlightedMenuItem(int16 x, int16 y, char t, Common::String z, bool valid); // Highlighted. TODO: It's too similar to chalk! Unify these two!!!

	void bleep();

	void setupMenuGame();
	void setupMenuFile();
	void setupMenuAction();
	void setupMenuPeople();
	void setupMenuObjects();
	void setupMenuWith();

	void runMenuGame();
	void runMenuFile();
	void runMenuAction();
	void runMenuObjects();
	void runMenuPeople();
	void runMenuWith();
};

} // End of namespace Avalanche.

#endif // DROPDOWN2_H
