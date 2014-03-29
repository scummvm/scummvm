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

struct MenuItem {
	int _menuId;
	int _actionId;
};

class Menu {
private:
	MortevielleEngine *_vm;

	byte _charArr[6][24];
	int _msg3;
	int _msg4;

	void util(Common::Point pos);
	void invert(int indx);
	void menuDown(int ii);

public:
	Menu(MortevielleEngine *vm);

	bool _menuActive;
	bool _menuSelected;
	bool _multiTitle;
	bool _menuDisplayed;
	Common::String _inventoryStringArray[9];
	Common::String _moveStringArray[8];
	Common::String _actionStringArray[22];
	Common::String _selfStringArray[7];
	Common::String _discussStringArray[9];
	MenuItem _discussMenu[9];
	MenuItem _inventoryMenu[9];
	MenuItem _moveMenu[8];

	int _opcodeAttach;
	int _opcodeWait;
	int _opcodeForce;
	int _opcodeSleep;
	int _opcodeListen;
	int _opcodeEnter;
	int _opcodeClose;
	int _opcodeSearch;
	int _opcodeKnock;
	int _opcodeScratch;
	int _opcodeRead;
	int _opcodeEat;
	int _opcodePlace;
	int _opcodeOpen;
	int _opcodeTake;
	int _opcodeLook;
	int _opcodeSmell;
	int _opcodeSound;
	int _opcodeLeave;
	int _opcodeLift;
	int _opcodeTurn;
	int _opcodeSHide;
	int _opcodeSSearch;
	int _opcodeSRead;
	int _opcodeSPut;
	int _opcodeSLook;
	MenuItem _actionMenu[12];

	void readVerbNums(Common::File &f, int dataSize);
	void setText(MenuItem item, Common::String name);
	void setDestinationText(int roomId);
	void setInventoryText();
	void disableMenuItem(MenuItem item);
	void enableMenuItem(MenuItem item);
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
