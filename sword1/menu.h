/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef BSMENU_H
#define BSMENU_H

#include "sworddefs.h"
#include "object.h"

class SwordScreen;
class SwordMouse;
class ResMan;

#define MENU_TOP 0
#define MENU_BOT 1

struct Subject {
	uint32 subjectRes;
	uint32 frameNo;
};

struct MenuObject {
	int32 textDesc;
	uint32 bigIconRes;
	uint32 bigIconFrame;
	uint32 luggageIconRes;
	uint32 useScript;
};

class SwordMenuIcon {
public:
	SwordMenuIcon(uint8 menuType, uint8 menuPos, uint32 resId, uint32 frame, SwordScreen *screen);
	bool wasClicked(uint16 mouseX, uint16 mouseY);
	void setSelect(bool pSel);
	void draw(const byte *fadeMask = NULL, int8 fadeStatus = 0);

private:
	uint8 _menuType, _menuPos;
	uint32 _resId, _frame;
	bool _selected;
	SwordScreen *_screen;
};

class SwordMenu {
public:
	SwordMenu(SwordScreen *pScreen, SwordMouse *pMouse);
	void fnChooser(BsObject *compact);
	void fnEndChooser(void);
	void fnAddSubject(int32 sub);
	void cfnReleaseMenu(void);
	int logicChooser(BsObject *compact);
	void engine(void);
	void refresh(uint8 menuType);
	void fnStartMenu(void);
	void fnEndMenu(void);
	void checkTopMenu(void);
	static const MenuObject _objectDefs[TOTAL_pockets + 1];

private:
	void buildSubjects(void);
	void buildMenu(void);
	void showMenu(uint8 menuType);
	byte _subjectBarStatus;
	byte _objectBarStatus;
	int8 _fadeSubject;
	int8 _fadeObject;
	void refreshMenus(void);
	uint8 checkMenuClick(uint8 menuType);
	//- lower menu, speech subjects:
	SwordMenuIcon *_subjects[16];
	uint32 _subjectBar[16];
	//- top menu, items
	SwordMenuIcon *_objects[TOTAL_pockets];
	uint32 _menuList[TOTAL_pockets];
	uint32 _inMenu;

	SwordScreen *_screen;
	SwordMouse *_mouse;
	static const Subject _subjectList[TOTAL_subjects];

	static const byte _fadeEffectTop[64];
	static const byte _fadeEffectBottom[64];
};

#endif //BSMENU_H
