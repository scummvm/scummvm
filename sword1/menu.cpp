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

enum {
	MENU_CLOSED,
	MENU_CLOSING,
	MENU_OPENING,
	MENU_OPEN
};

const byte SwordMenu::_fadeEffectTop[64] = {
	1, 7, 5, 3, 2, 4, 6, 0,
	3, 1, 7, 5, 4, 6, 0, 2,
	5, 3, 1, 7, 6, 0, 2, 4,
	7, 5, 3, 1, 0, 2, 4, 6,
	7, 5, 3, 1, 0, 2, 4, 6,
	5, 3, 1, 7, 6, 0, 2, 4,
	3, 1, 7, 5, 4, 6, 0, 2,
	1, 7, 5, 3, 2, 4, 6, 0
};

const byte SwordMenu::_fadeEffectBottom[64] = {
	7, 6, 5, 4, 3, 2, 1, 0,
	0, 7, 6, 5, 4, 3, 2, 1,
	1, 0, 7, 6, 5, 4, 3, 2,
	2, 1, 0, 7, 6, 5, 4, 3,
	3, 2, 1, 0, 7, 6, 5, 4,
	4, 3, 2, 1, 0, 7, 6, 5,
	5, 4, 3, 2, 1, 0, 7, 6,
	6, 5, 4, 3, 2, 1, 0, 7 
};

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

void SwordMenuIcon::draw(const byte *fadeMask, int8 fadeStatus) {
	uint16 x = _menuPos * 40;
	uint16 y = (_menuType == MENU_TOP)?(0):(440);
	_screen->showFrame(x, y, _resId, _frame + (_selected ? 1 : 0), fadeMask, fadeStatus);
}

SwordMenu::SwordMenu(SwordScreen *pScreen, SwordMouse *pMouse) {
	uint8 cnt;
	_screen = pScreen;
	_mouse = pMouse;
	_subjectBarStatus = MENU_CLOSED;
	_objectBarStatus = MENU_CLOSED;
	_fadeSubject = 0;
	_fadeObject = 0;
	for (cnt = 0; cnt < TOTAL_subjects; cnt++)
		_subjects[cnt] = NULL;
	for (cnt = 0; cnt < TOTAL_pockets; cnt++)
		_objects[cnt] = NULL;
	_inMenu = 0;
}

uint8 SwordMenu::checkMenuClick(uint8 menuType) {
	bool refreshMenus = false;
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
					refreshMenus = true;
				} else if (mouseEvent & BS1L_BUTTON_UP) {
					if (SwordLogic::_scriptVars[OBJECT_HELD] == _subjectBar[cnt])
						return cnt + 1;
					else {
						SwordLogic::_scriptVars[OBJECT_HELD] = 0;
						refreshMenus = true;
					}
				}
	} else {
		for (uint8 cnt = 0; cnt < _inMenu; cnt++) {
			if (_objects[cnt]->wasClicked(x, y))
				if (mouseEvent & BS1L_BUTTON_DOWN) {
					if (SwordLogic::_scriptVars[OBJECT_HELD]) {
						if (SwordLogic::_scriptVars[OBJECT_HELD] == _menuList[cnt]) {
							_mouse->setLuggage(0, 0);
							SwordLogic::_scriptVars[OBJECT_HELD] = 0; // reselected => deselect it
						} else { // the player is clicking another item on this one.
							   // run its use-script, if there is one
							SwordLogic::_scriptVars[SECOND_ITEM] = _menuList[cnt];
						}
					} else
						SwordLogic::_scriptVars[OBJECT_HELD] = _menuList[cnt];
					refreshMenus = true;
				} else if (mouseEvent & BS1L_BUTTON_UP) {
					if (SwordLogic::_scriptVars[OBJECT_HELD] == _menuList[cnt]) {
						_mouse->setLuggage(_objectDefs[_menuList[cnt]].luggageIconRes, 0);
						return cnt + 1;
					} else {
						_mouse->setLuggage(0, 0);
						SwordLogic::_scriptVars[OBJECT_HELD] = 0;
						refreshMenus = true;
					}
				}
		}
	}
	if (refreshMenus) {
		if (_objectBarStatus == MENU_OPEN) {
			buildMenu();
			for (uint8 cnt = 0; cnt < 16; cnt++) {
				if (_objects[cnt])
					_objects[cnt]->draw();
				else
					_screen->showFrame(cnt * 40, 0, 0xffffffff, 0);
			}
		}

		if (_subjectBarStatus == MENU_OPEN) {
			buildSubjects();
			for (uint8 cnt = 0; cnt < 16; cnt++) {
				if (_subjects[cnt])
					_subjects[cnt]->draw();
				else
					_screen->showFrame(cnt * 40, 440, 0xffffffff, 0);
			}
		}
	}
	return 0;
}

void SwordMenu::buildSubjects(void) {
	uint8 cnt;
	for (cnt = 0; cnt < 16; cnt++)
		if (_subjects[cnt]) {
			delete _subjects[cnt];
			_subjects[cnt] = NULL;
		}
	for (cnt = 0; cnt < SwordLogic::_scriptVars[IN_SUBJECT]; cnt++) {
		uint32 res = _subjectList[(_subjectBar[cnt] & 65535) - BASE_SUBJECT].subjectRes;
		uint32 frame = _subjectList[(_subjectBar[cnt] & 65535) - BASE_SUBJECT].frameNo;
		_subjects[cnt] = new SwordMenuIcon(MENU_BOT, cnt, res, frame, _screen);
		if (SwordLogic::_scriptVars[OBJECT_HELD])
			_subjects[cnt]->setSelect(_subjectBar[cnt] == SwordLogic::_scriptVars[OBJECT_HELD]);
		else
			_subjects[cnt]->setSelect(true);
	}
}

void SwordMenu::refresh(uint8 menuType) {
	uint i;

	if (menuType == MENU_TOP) {
		if (_objectBarStatus == MENU_OPENING || _objectBarStatus == MENU_CLOSING) {
			for (i = 0; i < 16; i++) {
				if (_objects[i])
					_objects[i]->draw(_fadeEffectTop, _fadeObject);
				else
					_screen->showFrame(i * 40, 0, 0xffffffff, 0, _fadeEffectTop, _fadeObject);
			}
		}
		if (_objectBarStatus == MENU_OPENING) {
			if (_fadeObject < 8)
				_fadeObject++;
			else
				_objectBarStatus = MENU_OPEN;
		} else if (_objectBarStatus == MENU_CLOSING) {
			if (_fadeObject > 0)
				_fadeObject--;
			else {
				for (i = 0; i < _inMenu; i++) {
					delete _objects[i];
					_objects[i] = NULL;
				}
				_objectBarStatus = MENU_CLOSED;
			}
		}
	} else {
		if (_subjectBarStatus == MENU_OPENING || _subjectBarStatus == MENU_CLOSING) {
			for (i = 0; i < 16; i++) {
				if (_subjects[i])
					_subjects[i]->draw(_fadeEffectBottom, _fadeSubject);
				else
					_screen->showFrame(i * 40, 440, 0xffffffff, 0, _fadeEffectBottom, _fadeSubject);
			}
		}
		if (_subjectBarStatus == MENU_OPENING) {
			if (_fadeSubject < 8)
				_fadeSubject++;
			else
				_subjectBarStatus = MENU_OPEN;
		} else if (_subjectBarStatus == MENU_CLOSING) {
			if (_fadeSubject > 0)
				_fadeSubject--;
			else {
				for (i = 0; i < SwordLogic::_scriptVars[IN_SUBJECT]; i++) {
					delete _subjects[i];
					_subjects[i] = NULL;
				}
				_subjectBarStatus = MENU_CLOSED;
			}
		}
	}
}

void SwordMenu::buildMenu(void) {
	uint32 *pockets = SwordLogic::_scriptVars + POCKET_1;
	for (uint8 cnt = 0; cnt < _inMenu; cnt++)
		if (_objects[cnt]) {
			delete _objects[cnt];
			_objects[cnt] = NULL;
		}
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
		if (SwordLogic::_scriptVars[MENU_LOOKING] || _subjectBarStatus == MENU_OPEN) { // either we're in the chooser or we're doing a 'LOOK AT'
			if ((!objHeld) || (objHeld == _menuList[menuSlot]))
				_objects[menuSlot]->setSelect(true);
		} else if (SwordLogic::_scriptVars[SECOND_ITEM]) { // clicked luggage onto 2nd icon - we need to colour-highlight the 2 relevant icons & grey out the rest
			if ((_menuList[menuSlot] == objHeld) || (_menuList[menuSlot] == SwordLogic::_scriptVars[SECOND_ITEM]))
				_objects[menuSlot]->setSelect(true);
		} else { // this object is selected - ie. GREYED OUT
			if (objHeld != _menuList[menuSlot])
				_objects[menuSlot]->setSelect(true);
		}
	}
}

void SwordMenu::showMenu(uint8 menuType) {
	if (menuType == MENU_TOP) {
		if (_objectBarStatus == MENU_OPEN) {
			for (uint8 cnt = 0; cnt < 16; cnt++) {
				if (_objects[cnt])
					_objects[cnt]->draw();
				else
					_screen->showFrame(cnt * 40, 0, 0xffffffff, 0);
			}
		} else if (_objectBarStatus == MENU_CLOSED) {
			_objectBarStatus = MENU_OPENING;
			_fadeObject = 0;
		} else if (_objectBarStatus == MENU_CLOSING)
			_objectBarStatus = MENU_OPENING;
	}
}

void SwordMenu::fnStartMenu(void) {
	SwordLogic::_scriptVars[OBJECT_HELD]  = 0; // icon no longer selected
	SwordLogic::_scriptVars[SECOND_ITEM]  = 0; // second icon no longer selected (after using one on another)
	SwordLogic::_scriptVars[MENU_LOOKING] = 0; // no longer 'looking at' an icon
	_mouse->setLuggage(0, 0);
	buildMenu();
	_mouse->controlPanel(true); // so that the arrow cursor will be shown.
	showMenu(MENU_TOP);
}

void SwordMenu::fnEndMenu(void) {
	if (_objectBarStatus != MENU_CLOSED) {
		_objectBarStatus = MENU_CLOSING;
		_mouse->controlPanel(false);
	}
}

void SwordMenu::fnChooser(BsObject *compact) {
	SwordLogic::_scriptVars[OBJECT_HELD] = 0;
	_mouse->setLuggage(0, 0);
	buildSubjects();
	compact->o_logic = LOGIC_choose;
	_mouse->controlPanel(true); // so the mouse cursor will be shown.
	_subjectBarStatus = MENU_OPENING;
}

void SwordMenu::fnEndChooser(void) {
	SwordLogic::_scriptVars[OBJECT_HELD] = 0;
	_subjectBarStatus = MENU_CLOSING;
	_objectBarStatus = MENU_CLOSING;
	_mouse->controlPanel(false);
	_mouse->setLuggage(0, 0);
}

void SwordMenu::checkTopMenu(void) {
	if (_objectBarStatus == MENU_OPEN)
		checkMenuClick(MENU_TOP);
}

int SwordMenu::logicChooser(BsObject *compact) {
	uint8 objSelected = 0;
	if (_objectBarStatus == MENU_OPEN)
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
