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
#include "mouse.h"
#include "menu.h"
#include "screen.h"
#include "logic.h"
#include "resman.h"
#include "objectman.h"
#include "sworddefs.h"
#include "system.h"
#include "swordres.h"

SwordMouse::SwordMouse(OSystem *system, ResMan *pResMan, ObjectMan *pObjMan) {
	_resMan = pResMan;
	_objMan = pObjMan;
	_system = system;
	_numObjs = 0;
	_mouseStatus = 0; // mouse off and unlocked
	_getOff = 0;

	for (uint8 cnt = 0; cnt < 17; cnt++) {
        _pointers[cnt] = (MousePtr*)_resMan->openFetchRes(MSE_POINTER + cnt);
#ifdef SCUMM_BIG_ENDIAN
		uint16 *data = (uint16*)pointers[cnt];
		for (uint8 endCnt = 0; endCnt < 5; endCnt++)
			data[endCnt] = READ_LE_UINT16(data + endCnt);
#endif
		fixTransparency(_pointers[cnt]->data + 0x30, _pointers[cnt]->sizeX * _pointers[cnt]->sizeY * _pointers[cnt]->numFrames);
	}
	/*_resMan->resOpen(MSE_POINTER);		// normal mouse (1 frame anim)
	_resMan->resOpen(MSE_OPERATE);
	_resMan->resOpen(MSE_PICKUP);
	_resMan->resOpen(MSE_EXAMINE);
	_resMan->resOpen(MSE_MOUTH);
	_resMan->resOpen(MSE_BECKON_L);
	_resMan->resOpen(MSE_BECKON_R);
	_resMan->resOpen(MSE_ARROW0);
	_resMan->resOpen(MSE_ARROW1);
	_resMan->resOpen(MSE_ARROW2);
	_resMan->resOpen(MSE_ARROW3);
	_resMan->resOpen(MSE_ARROW4);
	_resMan->resOpen(MSE_ARROW5);
	_resMan->resOpen(MSE_ARROW6);
	_resMan->resOpen(MSE_ARROW7);
	_resMan->resOpen(MSE_ARROW8);		// UPWARDS
	_resMan->resOpen(MSE_ARROW9);*/		// DOWNWARDS
	// luggage & chess stuff is opened dynamically
}

void SwordMouse::useLogic(SwordLogic *pLogic) {
	_logic = pLogic;
}

void SwordMouse::addToList(int id, BsObject *compact) {
	_objList[_numObjs].id = id;
	_objList[_numObjs].compact = compact;
	_numObjs++;
}

void SwordMouse::flushEvents(void) {
	_lastState = _state = 0;
}

void SwordMouse::engine(uint16 x, uint16 y, uint16 eventFlags) {
	//warning("Stub: SwordMouse::engine");
	_state = 0; // all mouse events are flushed after one cycle.
	if (_lastState) { // delay all events by one cycle to notice L_button + R_button clicks correctly.
		_state = _lastState | eventFlags;
		_lastState = 0;
	} else if (eventFlags)
		_lastState = eventFlags;
	if (!(_mouseStatus & 1)) {  // no human?
		// if the mouse is turned off, I want the menu automatically removed,
		// except while in conversation, while examining a menu object or while combining two menu objects!
		/*if ((!subject_status)&&(!menu_looking)&&(!second_icon))
		{
			HideMenu(TOP_MENU);
			menu_status=0;
		}*/
		_numObjs = 0;
		return;	// no human, so we don't want the mouse engine
	}
	// todo: check menus here.
	SwordLogic::_scriptVars[MOUSE_X] = SwordLogic::_scriptVars[SCROLL_OFFSET_X] + x + 128;
	SwordLogic::_scriptVars[MOUSE_Y] = SwordLogic::_scriptVars[SCROLL_OFFSET_Y] + y + 128 - 40;

	//-
	int32 touchedId = 0;
	uint16 clicked = 0;
	if (y > 40) {
		for (uint16 priority = 0; (priority < 10) && (!touchedId); priority++) {
			for (uint16 cnt = 0; cnt < _numObjs; cnt++) {
				if ((_objList[cnt].compact->o_priority == priority) && 
					(SwordLogic::_scriptVars[MOUSE_X] >= (uint32)_objList[cnt].compact->o_mouse_x1) &&
					(SwordLogic::_scriptVars[MOUSE_X] <= (uint32)_objList[cnt].compact->o_mouse_x2) &&
					(SwordLogic::_scriptVars[MOUSE_Y] >= (uint32)_objList[cnt].compact->o_mouse_y1) &&
					(SwordLogic::_scriptVars[MOUSE_Y] <= (uint32)_objList[cnt].compact->o_mouse_y2)) {
						touchedId = _objList[cnt].id;
						clicked = cnt;
				}
			}
		}
		if (touchedId != (int)SwordLogic::_scriptVars[SPECIAL_ITEM]) { //the mouse collision situation has changed in one way or another
			SwordLogic::_scriptVars[SPECIAL_ITEM] = touchedId;
			debug(9, "New special item: %X\n", touchedId);
			if (_getOff) { // there was something else selected before, run its get-off script
				_logic->runMouseScript(NULL, _getOff);
				_getOff = 0;
			}
			if (touchedId) { // there's something new selected, now.
				// BsObject *compact = _objMan->fetchObject(SwordLogic::_scriptVars[SPECIAL_ITEM]);

				if	(_objList[clicked].compact->o_mouse_on)	//run its get on
					_logic->runMouseScript(_objList[clicked].compact, _objList[clicked].compact->o_mouse_on);

				_getOff = _objList[clicked].compact->o_mouse_off; //setup get-off for later
			}
		}
	} else
		SwordLogic::_scriptVars[SPECIAL_ITEM] = 0;
	if (_state & (BS1L_BUTTON_DOWN | BS1R_BUTTON_DOWN)) {
		// todo: handle menus
		SwordLogic::_scriptVars[MOUSE_BUTTON] = _state;
		if (SwordLogic::_scriptVars[SPECIAL_ITEM]) {
			BsObject *compact = _objMan->fetchObject(SwordLogic::_scriptVars[SPECIAL_ITEM]);
			_logic->runMouseScript(compact, compact->o_mouse_click);
		}
	}
	_numObjs = 0;
}

uint16 SwordMouse::testEvent(void) {
	return _state;
}

void SwordMouse::setLuggage(uint32 resId, uint32 rate) {
	warning("stub: SwordMouse::setLuggage(%d, %d)", resId, rate);
}

void SwordMouse::setPointer(uint32 resId, uint32 rate) {
	_currentPtrId = resId - MSE_POINTER;
	_rate = rate;
	_rateCnt = 1;
	_frame = 0;
	if (resId == 0) {
		_system->set_mouse_cursor(NULL, 0, 0, 0, 0);
		_system->show_mouse(false);
	} else {
        animate();
		_system->show_mouse(true);
	}
}

void SwordMouse::animate(void) {
	if (_rateCnt && (_mouseStatus == 1)) {
		_rateCnt--;
		if (!_rateCnt) {
			_rateCnt = _rate;
			_frame = (_frame + 1) % _pointers[_currentPtrId]->numFrames;
			uint16 size = _pointers[_currentPtrId]->sizeX * _pointers[_currentPtrId]->sizeY;
			_system->set_mouse_cursor(_pointers[_currentPtrId]->data + 0x30 + _frame * size, _pointers[_currentPtrId]->sizeX, _pointers[_currentPtrId]->sizeY, _pointers[_currentPtrId]->hotSpotX, _pointers[_currentPtrId]->hotSpotY);
		}
	}
}

void SwordMouse::fnNoHuman(void) {
	if (_mouseStatus & 2) // locked, can't do anything
		return ;
	_mouseStatus = 0; // off & unlocked
	setLuggage(0, 0);
	setPointer(0, 0);
}

void SwordMouse::fnAddHuman(void) {
	if (_mouseStatus & 2) // locked, can't do anything
		return ;
	_mouseStatus = 1;
	// SwordLogic::_scriptVars[SPECIAL_ITEM] = -1;
	SwordLogic::_scriptVars[SPECIAL_ITEM] = 0; // _scriptVars is unsigned...
	_getOff = SCR_std_off;
	setPointer(MSE_POINTER, 0);
	_mouseCount = 3;

}

void SwordMouse::fnBlankMouse(void) {
	setPointer(0, 0);
}

void SwordMouse::fnNormalMouse(void) {
	setPointer(MSE_POINTER, 0);
}

void SwordMouse::fnLockMouse(void) {
	_mouseStatus |= 2;
}

void SwordMouse::fnUnlockMouse(void) {
    _mouseStatus &= 1;
}

void SwordMouse::giveCoords(uint16 *x, uint16 *y) {
	*x = _mouseX;
	*y = _mouseY;
}

void SwordMouse::fixTransparency(uint8 *data, uint32 size) {
	for (uint32 cnt = 0; cnt < size; cnt++)
		if (data[cnt] == 0)
			data[cnt] = 255;
}
