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

#ifndef MORTEVIELLE_MENU_H
#define MORTEVIELLE_MENU_H

#include "common/str.h"

namespace Mortevielle {

class Menu {
private:
	void util(int x, int y);
	void invers(int ix);
	void menuDown(int ii);
public:
	bool _menuActive;
	Common::String _inventoryStringArray[9];
	Common::String _moveStringArray[8];
	Common::String _actionStringArray[22];
	Common::String _selfStringArray[7];
	Common::String _discussStringArray[9];
	int _discussMenu[9];
	int _inventoryMenu[9];
	int _moveMenu[8];
public:
	void menut(int no, Common::String name);
	void disableMenuItem(int no);
	void enableMenuItem(int no);
	void displayMenu();
	void drawMenu();
	void menuUp(int xx);
	void eraseMenu();
	void mdn();
	void initMenu();
};

enum {
	MENU_INVENTORY = 1, MENU_MOVE = 2, MENU_ACTION = 3, MENU_SELF = 4,
	MENU_DISCUSS = 5,   MENU_FILE = 6, MENU_SAVE = 7,   MENU_LOAD = 8
};

} // End of namespace Mortevielle
#endif
