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
 * Copyright (c) 1987-1989 Lankhor
 */

#ifndef MORTEVIELLE_MENU_H
#define MORTEVIELLE_MENU_H

#include "common/rect.h"
#include "common/str.h"

namespace Mortevielle {
class MortevielleEngine;

enum {
	MENU_NONE = 0, MENU_INVENTORY = 1, MENU_MOVE = 2, MENU_ACTION = 3,
	MENU_SELF = 4, MENU_DISCUSS = 5,   MENU_FILE = 6, MENU_SAVE = 7,
	MENU_LOAD = 8
};

const int OPCODE_NONE = 0;

struct menuItem {
	int _menuId;
	int _actionId;
};

class Menu {
private:
	MortevielleEngine *_vm;

	byte _charArr[7][24];
	int _msg3;
	int _msg4;

	void util(Common::Point pos);
	void invert(int indx);
	void menuDown(int ii);

public:
	Menu();

	bool _menuActive;
	bool _menuSelected;
	bool _multiTitle;
	bool _menuDisplayed;
	Common::String _inventoryStringArray[9];
	Common::String _moveStringArray[8];
	Common::String _actionStringArray[22];
	Common::String _selfStringArray[7];
	Common::String _discussStringArray[9];
	menuItem _discussMenu[9];
	menuItem _inventoryMenu[9];
	menuItem _moveMenu[8];

	int OPCODE_ATTACH;
	int OPCODE_WAIT;
	int OPCODE_FORCE;
	int OPCODE_SLEEP;
	int OPCODE_LISTEN;
	int OPCODE_ENTER;
	int OPCODE_CLOSE;
	int OPCODE_SEARCH;
	int OPCODE_KNOCK;
	int OPCODE_SCRATCH;
	int OPCODE_READ;
	int OPCODE_EAT;
	int OPCODE_PLACE;
	int OPCODE_OPEN;
	int OPCODE_TAKE;
	int OPCODE_LOOK;
	int OPCODE_SMELL;
	int OPCODE_SOUND;
	int OPCODE_LEAVE;
	int OPCODE_LIFT;
	int OPCODE_TURN;
	int OPCODE_SHIDE;
	int OPCODE_SSEARCH;
	int OPCODE_SREAD;
	int OPCODE_SPUT;
	int OPCODE_SLOOK;
	menuItem _actionMenu[12];

	void setParent(MortevielleEngine *vm);
	void readVerbNums(Common::File &f, int dataSize);
	void setText(int menuId, int actionId, Common::String name);
	void setDestinationText(int roomId);
	void setInventoryText();
	void disableMenuItem(int menuId, int actionId);
	void enableMenuItem(int menuId, int actionId);
	void displayMenu();
	void drawMenu();
	void menuUp(int msgId);
	void eraseMenu();
	void updateMenu();
	void initMenu();

	void setSearchMenu();
	void unsetSearchMenu();
};

} // End of namespace Mortevielle
#endif
