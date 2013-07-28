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

enum verbs {OPCODE_NONE = 0, OPCODE_ATTACH = 0x301, OPCODE_WAIT = 0x302,  OPCODE_FORCE = 0x303,   OPCODE_SLEEP = 0x304, OPCODE_LISTEN = 0x305,
OPCODE_ENTER = 0x306,  OPCODE_CLOSE = 0x307, OPCODE_SEARCH = 0x308,  OPCODE_KNOCK = 0x309, OPCODE_SCRATCH = 0x30a,
OPCODE_READ = 0x30b,   OPCODE_EAT = 0x30c,   OPCODE_PLACE = 0x30d,   OPCODE_OPEN = 0x30e,  OPCODE_TAKE = 0x30f,
OPCODE_LOOK = 0x310,   OPCODE_SMELL = 0x311, OPCODE_SOUND = 0x312,   OPCODE_LEAVE = 0x313, OPCODE_LIFT = 0x314,
OPCODE_TURN = 0x315,   OPCODE_SHIDE = 0x401, OPCODE_SSEARCH = 0x402, OPCODE_SREAD = 0x403, OPCODE_SPUT = 0x404,
OPCODE_SLOOK = 0x405};

struct menuItem {
	int _menuId;
	int _actionId;
};

static const menuItem _actionMenu[12] = {
	{OPCODE_NONE   >> 8, OPCODE_NONE & 0xFF},
	{OPCODE_SHIDE  >> 8, OPCODE_SHIDE & 0xFF},
	{OPCODE_ATTACH >> 8, OPCODE_ATTACH & 0xFF}, 
	{OPCODE_FORCE  >> 8, OPCODE_FORCE & 0xFF}, 
	{OPCODE_SLEEP  >> 8, OPCODE_SLEEP & 0xFF},
	{OPCODE_ENTER  >> 8, OPCODE_ENTER & 0xFF}, 
	{OPCODE_CLOSE  >> 8, OPCODE_CLOSE & 0xFF},  
	{OPCODE_KNOCK  >> 8, OPCODE_KNOCK & 0xFF},
	{OPCODE_EAT    >> 8, OPCODE_EAT & 0xFF},
	{OPCODE_PLACE  >> 8, OPCODE_PLACE & 0xFF},
	{OPCODE_OPEN   >> 8, OPCODE_OPEN & 0xFF},
	{OPCODE_LEAVE  >> 8, OPCODE_LEAVE & 0xFF}
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
	void initMenu(MortevielleEngine *vm);

	void setSearchMenu();
	void unsetSearchMenu();
};

} // End of namespace Mortevielle
#endif
