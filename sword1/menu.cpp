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
	for (uint8 cnt = 0; cnt < 16; cnt++)
		_subjects[cnt] = NULL;
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
				if (mouseEvent == BS1L_BUTTON_DOWN) {
					SwordLogic::_scriptVars[OBJECT_HELD] = _subjectBar[cnt];
					buildSubjects();
					return 0;
				} else if (mouseEvent == BS1L_BUTTON_UP) {
					if (SwordLogic::_scriptVars[OBJECT_HELD] == _subjectBar[cnt])
						return cnt + 1;
					else {
						SwordLogic::_scriptVars[OBJECT_HELD] = 0;
						buildSubjects();
						return 0;
					}
				}
	} else {
		return 0;
	}
	return 0;
}

void SwordMenu::buildSubjects(void) {
	// uint8 subDest = 0;
	clearMenu(MENU_BOT);
	for (uint8 cnt = 0; cnt < 16; cnt++)
		if (_subjects[cnt]) {
			delete _subjects[cnt];
			_subjects[cnt] = NULL;
		}
	for (uint8 cnt = 0; cnt < SwordLogic::_scriptVars[IN_SUBJECT]; cnt++) {
		uint32 res = _subjectList[(_subjectBar[cnt] & 65535) - BASE_SUBJECT].subjectRes;
		uint32 frame = _subjectList[(_subjectBar[cnt] & 65535) - BASE_SUBJECT].frameNo;
		_subjects[cnt] = new SwordMenuIcon(MENU_BOT, cnt, res, frame, _screen);
		_subjects[cnt]->setSelect(SwordLogic::_scriptVars[OBJECT_HELD] == (_subjectBar[cnt]&0xFFFF));
		_subjects[cnt]->draw();
	}
	//_system->update_screen();
}

void SwordMenu::refresh(uint8 menuType) {
	//warning("stub: SwordMenu::refresh())");
}

void SwordMenu::clearMenu(uint8 menuType) {
	warning("stub: SwordMenu::clearMenu()");
}

void SwordMenu::fnStartMenu(void) {
	warning("stub: SwordMenu::fnStartMenu()");
}

void SwordMenu::fnEndMenu(void) {
	warning("stub: SwordMenu::clearMenu()");
}

void SwordMenu::fnChooser(BsObject *compact) {
	SwordLogic::_scriptVars[OBJECT_HELD] = 0;
	buildSubjects();
	compact->o_logic = LOGIC_choose;
}

void SwordMenu::fnEndChooser(void) {
	SwordLogic::_scriptVars[OBJECT_HELD] = 0;
	clearMenu(MENU_BOT);
	clearMenu(MENU_TOP);
	//_system->update_screen();
	for (uint8 cnt = 0; cnt < 16; cnt++)
		if (_subjects[cnt]) {
			delete _subjects[cnt];
			_subjects[cnt] = NULL;
		}
}

int SwordMenu::logicChooser(BsObject *compact) {
	if (checkMenuClick(MENU_BOT)) {
		compact->o_logic = LOGIC_script;
		return 1;
	} else 
		return 0;
}

void SwordMenu::fnAddSubject(int32 sub) {
	_subjectBar[SwordLogic::_scriptVars[IN_SUBJECT]] = sub;
	SwordLogic::_scriptVars[IN_SUBJECT]++;
}

void SwordMenu::cfnReleaseMenu(void) {
	clearMenu(MENU_TOP);
	//_system->update_screen();
}
