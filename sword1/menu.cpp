/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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

#include "stdafx.h"
#include "menu.h"
#include "resman.h"
#include "scummsys.h"
#include "common/util.h"
#include "system.h"
#include "mouse.h"
#include "screen.h"
#include "logic.h"

SwordMenuIcon::SwordMenuIcon(uint8 menuType, uint8 menuPos, uint32 resId, uint32 frame, SwordScreen *screen) {
	_menuType = menuType;
	_menuPos = menuPos;
	_resId = resId;
	_frame = frame;
	_screen = screen;
	_selected = false;
}

bool SwordMenuIcon::wasClicked(uint16 mouseX, uint16 mouseY) {
	if (((_menuType == MENU_TOP) && (mouseY >= 40)) || ((_menuType == MENU_BOT) && (mouseY < 440)))
		return false;
	if ((mouseX >= _menuPos * 40) && (mouseX < (_menuPos + 1) * 40))
		return true;
	else
		return false;
}

void SwordMenuIcon::setSelect(bool pSel) {
	_selected = pSel;
}

void SwordMenuIcon::draw(void) {
	uint16 x = _menuPos * 40;
	uint16 y = (_menuType == MENU_TOP)?(0):(440);
	_screen->showFrame(x, y, _resId, _frame + (_selected ? 1 : 0));
}

SwordMenu::SwordMenu(SwordScreen *pScreen, SwordMouse *pMouse) {
	_screen = pScreen;
	_mouse = pMouse;
	_subjectBarShown = false;
	_objectBarShown = false;
	for (uint8 cnt = 0; cnt < TOTAL_subjects; cnt++)
		_subjects[cnt] = NULL;
	for (uint8 cnt = 0; cnt < TOTAL_pockets; cnt++)
		_objects[cnt] = NULL;
	_inMenu = 0;
}

uint8 SwordMenu::checkMenuClick(uint8 menuType) {
	uint16 mouseEvent = _mouse->testEvent();
	if (!mouseEvent)
		return 0;
	uint16 x, y;
	_mouse->giveCoords(&x, &y);
	if (menuType == MENU_BOT) {
		for (uint8 cnt = 0; cnt < SwordLogic::_scriptVars[IN_SUBJECT]; cnt++)
			if (_subjects[cnt]->wasClicked(x, y))
				if (mouseEvent & BS1L_BUTTON_DOWN) {
					SwordLogic::_scriptVars[OBJECT_HELD] = _subjectBar[cnt];
					buildSubjects();
				} else if (mouseEvent & BS1L_BUTTON_UP) {
					if (SwordLogic::_scriptVars[OBJECT_HELD] == _subjectBar[cnt])
						return cnt + 1;
					else {
						SwordLogic::_scriptVars[OBJECT_HELD] = 0;
						buildSubjects();
					}
				}
	} else {
		for (uint8 cnt = 0; cnt < _inMenu; cnt++) {
			if (_objects[cnt]->wasClicked(x, y))
				if (mouseEvent & BS1L_BUTTON_DOWN) {
					SwordLogic::_scriptVars[OBJECT_HELD] = _menuList[cnt];
					buildMenu();
				} else if (mouseEvent & BS1L_BUTTON_UP) {
					if (SwordLogic::_scriptVars[OBJECT_HELD] == _menuList[cnt]) {
						return cnt + 1;
					} else {
						SwordLogic::_scriptVars[OBJECT_HELD] = 0;
						buildMenu();
					}
				}
		}
	}
	return 0;
}

void SwordMenu::buildSubjects(void) {
	_screen->clearMenu(MENU_BOT);
	for (uint8 cnt = 0; cnt < 16; cnt++)
		if (_subjects[cnt]) {
			delete _subjects[cnt];
			_subjects[cnt] = NULL;
		}
	for (uint8 cnt = 0; cnt < SwordLogic::_scriptVars[IN_SUBJECT]; cnt++) {
		uint32 res = _subjectList[(_subjectBar[cnt] & 65535) - BASE_SUBJECT].subjectRes;
		uint32 frame = _subjectList[(_subjectBar[cnt] & 65535) - BASE_SUBJECT].frameNo;
		_subjects[cnt] = new SwordMenuIcon(MENU_BOT, cnt, res, frame, _screen);
		_subjects[cnt]->setSelect(SwordLogic::_scriptVars[OBJECT_HELD] == _subjectBar[cnt]);
		_subjects[cnt]->draw();
	}
}

void SwordMenu::refresh(uint8 menuType) {
	//warning("stub: SwordMenu::refresh())");
}

void SwordMenu::buildMenu(void) {
	uint32 *pockets = SwordLogic::_scriptVars + POCKET_1;
	_inMenu = 0;
	for (uint32 pocketNo = 0; pocketNo < TOTAL_pockets; pocketNo++)
		if (pockets[pocketNo]) {
			_menuList[_inMenu] = pocketNo + 1;
			_inMenu++;
		}
	for (uint32 menuSlot = 0; menuSlot < _inMenu; menuSlot++) {
		_objects[menuSlot] = new SwordMenuIcon(MENU_TOP, menuSlot, _objectDefs[_menuList[menuSlot]].bigIconRes, _objectDefs[_menuList[menuSlot]].bigIconFrame, _screen);
		uint32 objHeld = SwordLogic::_scriptVars[OBJECT_HELD];

		// check highlighting
		if (SwordLogic::_scriptVars[MENU_LOOKING] || _subjectBarShown) { // either we're in the chooser or we're doing a 'LOOK AT'
			if ((!objHeld) || (objHeld == _menuList[menuSlot]))
				_objects[menuSlot]->setSelect(true);
		} else if (_secondItem) { // clicked luggage onto 2nd icon - we need to colour-highlight the 2 relevant icons & grey out the rest
			if ((_menuList[menuSlot] == objHeld) || (_menuList[menuSlot] == _secondItem))
				_objects[menuSlot]->setSelect(true);
		} else { // this object is selected - ie. GREYED OUT
			if (objHeld != _menuList[menuSlot])
				_objects[menuSlot]->setSelect(true);
		}
	}
}

void SwordMenu::showMenu(uint8 menuType) {
	if (menuType == MENU_TOP) {
		for (uint8 cnt = 0; cnt < _inMenu; cnt++)
			_objects[cnt]->draw();
	}
}

void SwordMenu::fnStartMenu(void) {
	SwordLogic::_scriptVars[OBJECT_HELD] = 0;  // icon no longer selected
	SwordLogic::_scriptVars[MENU_LOOKING] = 0; // second icon no longer selected (after using one on another)
	_secondItem = 0;                           // no longer 'looking at' an icon
	buildMenu();
	if (_inMenu > 0) {	// if there's something in the object menu
		_objectBarShown = true;
		showMenu(MENU_TOP);
	} else {
		_objectBarShown = false;
		_inMenu = 0;
	}
}

void SwordMenu::fnEndMenu(void) {
	if (_objectBarShown) {
		for (uint32 cnt = 0; cnt < _inMenu; cnt++)
			delete _objects[cnt];
		_screen->clearMenu(MENU_TOP);
		_screen->clearMenu(MENU_BOT);
		_objectBarShown = false;
	}
}

void SwordMenu::fnChooser(BsObject *compact) {
	SwordLogic::_scriptVars[OBJECT_HELD] = 0;
	buildSubjects();
	compact->o_logic = LOGIC_choose;
	_mouse->controlPanel(true); // so the mouse cursor will be shown.
	_subjectBarShown = true;
}

void SwordMenu::fnEndChooser(void) {
	SwordLogic::_scriptVars[OBJECT_HELD] = 0;
	_screen->clearMenu(MENU_BOT);
	_screen->clearMenu(MENU_TOP);
	for (uint8 cnt = 0; cnt < 16; cnt++)
		if (_subjects[cnt]) {
			delete _subjects[cnt];
			_subjects[cnt] = NULL;
		}
	_mouse->controlPanel(false);
	_subjectBarShown = false;
}

void SwordMenu::checkTopMenu(void) {
	if (_objectBarShown)
		checkMenuClick(MENU_TOP);
}

int SwordMenu::logicChooser(BsObject *compact) {
	uint8 objSelected = 0;
	if (_objectBarShown)
		objSelected = checkMenuClick(MENU_TOP);
	if (!objSelected)
		objSelected = checkMenuClick(MENU_BOT);
	if (objSelected) {
		compact->o_logic = LOGIC_script;
		return 1;
	}
	return 0;
}

void SwordMenu::fnAddSubject(int32 sub) {
	_subjectBar[SwordLogic::_scriptVars[IN_SUBJECT]] = sub;
	SwordLogic::_scriptVars[IN_SUBJECT]++;
}

void SwordMenu::cfnReleaseMenu(void) {
	_screen->clearMenu(MENU_TOP);
}
