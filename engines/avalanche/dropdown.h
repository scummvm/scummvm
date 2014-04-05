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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* Original name: DROPDOWN		A customized version of Oopmenu (qv). */

#ifndef AVALANCHE_DROPDOWN_H
#define AVALANCHE_DROPDOWN_H

#include "common/str.h"

namespace Avalanche {
class AvalancheEngine;

class DropDownMenu;

typedef void (DropDownMenu::*MenuFunc)();
static const Color kMenuBackgroundColor = kColorLightgray;
static const Color kMenuBorderColor = kColorBlack;

class HeadType {
public:
	Common::String _title;
	char _trigger, _altTrigger;
	byte _position;
	int16 _xpos, _xright;
	MenuFunc _setupFunc, _chooseFunc;

	void init(char trig, char alTtrig, Common::String title, byte pos, MenuFunc setupFunc, MenuFunc chooseFunc, DropDownMenu *menu);
	void draw();
	void highlight();
	bool parseAltTrigger(char key);

private:
	DropDownMenu *_dropdown;
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
	uint16 _width, _left;
	bool _firstlix;
	int16 _flx1, _flx2, _fly;
	bool _activeNow; // Is there an active option now?
	byte _activeNum; // And if so, which is it?
	byte _choiceNum; // Your choice?

	void init(DropDownMenu *menu);
	void reset();
	void setupOption(Common::String title, char trigger, Common::String shortcut, bool valid);
	void display();
	void wipe();
	void lightUp(Common::Point cursorPos);
	void select(byte which);

private:
	byte _oldY; // used by lightUp
	byte _optionNum;
	byte _highlightNum;

	DropDownMenu *_dropdown;

	void displayOption(byte y, bool highlit);
	void moveHighlight(int8 inc);

	// CHECKME: Useless function?
	void parseKey(char c);
};

class MenuBar {
public:
	HeadType _menuItems[8];
	byte _menuNum;

	MenuBar();
	void init(DropDownMenu *menu);
	void createMenuItem(char trig, Common::String title, char altTrig, MenuFunc setupFunc, MenuFunc chooseFunc);
	void draw();
	void chooseMenuItem(int16 x);

private:
	DropDownMenu *_dropdown;

	void setupMenuItem(byte which);
	// CHECKME: Useless function
	void parseAltTrigger(char c);
};

class DropDownMenu {
public:
	friend class HeadType;
	friend class MenuItem;
	friend class MenuBar;

	MenuItem _activeMenuItem;
	MenuBar _menuBar;

	DropDownMenu(AvalancheEngine *vm);

	void update();
	void setup(); // Standard menu bar.
	bool isActive();
	void init();
	void resetVariables();

private:
	static const byte kIndent = 5;
	static const byte kSpacing = 10;

//	Checkme: Useless constants?
//	static const Color kMenuFontColor = kColorBlack;
//	static const Color kHighlightBackgroundColor = kColorBlack;
//	static const Color kHighlightFontColor = kColorWhite;
//	static const Color kDisabledColor = kColorDarkgray;

	Common::String people;
	Common::String _verbStr; // what you can do with your object. :-)
	bool _menuActive; // Kludge so we don't have to keep referring to the menu.
	People _lastPerson; // Last person to have been selected using the People menu.

	AvalancheEngine *_vm;

	Common::String selectGender(byte x); // Returns "im" for boys, and "er" for girls.
	void findWhatYouCanDoWithIt();
	void drawMenuText(int16 x, int16 y, char trigger, Common::String text, bool valid, bool highlighted);
	void bleep();

	char getThingChar(byte which);
	byte getNameChar(People whose);
	Common::String getThing(byte which);

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

	// CHECKME: Useless function?
	void parseKey(char r, char re);
};

} // End of namespace Avalanche.

#endif // AVALANCHE_DROPDOWN_H
