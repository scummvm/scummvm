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
#include "menu.h"

SwordMouse::SwordMouse(OSystem *system, ResMan *pResMan, ObjectMan *pObjMan) {
	_resMan = pResMan;
	_objMan = pObjMan;
	_system = system;
	_currentPtr = NULL;
}

void SwordMouse::initialize(void) {
	_numObjs = 0;
	SwordLogic::_scriptVars[MOUSE_STATUS] = 0; // mouse off and unlocked
	_getOff = 0;
	_inTopMenu = false;
	_mouseOverride = false;
	_currentPtrId = _currentLuggageId = 0;

	for (uint8 cnt = 0; cnt < 17; cnt++)	 // force res manager to keep mouse 
		_resMan->resOpen(MSE_POINTER + cnt); // cursors in memory all the time
	
	createPointer(0, 0);
}

void SwordMouse::controlPanel(bool on) { // true on entering cpanel, false when leaving
	static uint32 savedPtrId = 0;
	if (on) {
		savedPtrId = _currentPtrId;
		_mouseOverride = true;
		setLuggage(0, 0);
		setPointer(MSE_POINTER, 0);
	} else {
		_currentPtrId = savedPtrId;
		_mouseOverride = false;
		setLuggage(_currentLuggageId, 0);
		setPointer(_currentPtrId, 0);
	}
}

void SwordMouse::useLogicAndMenu(SwordLogic *pLogic, SwordMenu *pMenu) {
	_logic = pLogic;
	_menu = pMenu;
}

void SwordMouse::addToList(int id, BsObject *compact) {
	_objList[_numObjs].id = id;
	_objList[_numObjs].compact = compact;
	_numObjs++;
}

void SwordMouse::engine(uint16 x, uint16 y, uint16 eventFlags) {
	_state = 0; // all mouse events are flushed after one cycle.
	if (_lastState) { // delay all events by one cycle to notice L_button + R_button clicks correctly.
		_state = _lastState | eventFlags;
		_lastState = 0;
	} else if (eventFlags)
		_lastState = eventFlags;

	// if we received both, mouse down and mouse up event in this cycle, resort them so that
	// we'll receive the up event in the next one.
	if ((_state & MOUSE_DOWN_MASK) && (_state & MOUSE_UP_MASK)) {
		_lastState = _state & MOUSE_UP_MASK;
		_state &= MOUSE_DOWN_MASK;
	}

	_mouseX = x;
	_mouseY = y;
	if (!(SwordLogic::_scriptVars[MOUSE_STATUS] & 1)) {  // no human?
		_numObjs = 0;
		return;	// no human, so we don't want the mouse engine
	}

	if (!SwordLogic::_scriptVars[TOP_MENU_DISABLED]) {
		if (y < 40) { // okay, we are in the top menu.
			if (!_inTopMenu) // are we just entering it?
				_menu->fnStartMenu();
			_menu->checkTopMenu();
			_inTopMenu = true;
		} else if (_inTopMenu) { // we're not in the menu. did we just leave it?
			_menu->fnEndMenu();
			_inTopMenu = false;
		}
	} else if (_inTopMenu) {
		_menu->fnEndMenu();
		_inTopMenu = false;
	}

	SwordLogic::_scriptVars[MOUSE_X] = SwordLogic::_scriptVars[SCROLL_OFFSET_X] + x + 128;
	SwordLogic::_scriptVars[MOUSE_Y] = SwordLogic::_scriptVars[SCROLL_OFFSET_Y] + y + 128 - 40;

	//-
	int32 touchedId = 0;
	uint16 clicked = 0;
	if (y > 40) {
		for (uint16 priority = 0; (priority < 10) && (!touchedId); priority++) {
			for (uint16 cnt = 0; (cnt < _numObjs) && (!touchedId); cnt++) {
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
			if (_getOff) { // there was something else selected before, run its get-off script
				_logic->runMouseScript(NULL, _getOff);
				_getOff = 0;
			}
			if (touchedId) { // there's something new selected, now.
				if	(_objList[clicked].compact->o_mouse_on)	//run its get on
					_logic->runMouseScript(_objList[clicked].compact, _objList[clicked].compact->o_mouse_on);

				_getOff = _objList[clicked].compact->o_mouse_off; //setup get-off for later
			}
		}
	} else
		SwordLogic::_scriptVars[SPECIAL_ITEM] = 0;
	if (_state & MOUSE_DOWN_MASK) {
		if (_inTopMenu && SwordLogic::_scriptVars[SECOND_ITEM])
			_logic->runMouseScript(NULL, _menu->_objectDefs[SwordLogic::_scriptVars[SECOND_ITEM]].useScript);
		
		SwordLogic::_scriptVars[MOUSE_BUTTON] = _state & MOUSE_DOWN_MASK;
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

void SwordMouse::createPointer(uint32 ptrId, uint32 luggageId) {
	if (_currentPtr) {
		free(_currentPtr);
		_currentPtr = NULL;
	}
	if (ptrId) {
		MousePtr *lugg = NULL;
		MousePtr *ptr = (MousePtr*)_resMan->openFetchRes(ptrId);
		uint16 resSizeX = FROM_LE_16(ptr->sizeX);
		uint16 resSizeY = FROM_LE_16(ptr->sizeY);
		uint16 noFrames = FROM_LE_16(ptr->numFrames);
		if (luggageId) {
			lugg = (MousePtr*)_resMan->openFetchRes(luggageId);
			resSizeX = MAX(resSizeX, (uint16)((resSizeX / 2) + FROM_LE_16(lugg->sizeX)));
			resSizeY = MAX(resSizeY, (uint16)((resSizeY / 2) + FROM_LE_16(lugg->sizeY)));
		}
		_currentPtr = (MousePtr*)malloc(sizeof(MousePtr) + resSizeX * resSizeY * noFrames);
		_currentPtr->hotSpotX = FROM_LE_16(ptr->hotSpotX);
		_currentPtr->hotSpotY = FROM_LE_16(ptr->hotSpotY);
		_currentPtr->numFrames = noFrames;
		_currentPtr->sizeX = resSizeX;
		_currentPtr->sizeY = resSizeY;
		uint8 *ptrData = (uint8*)_currentPtr + sizeof(MousePtr);
		memset(ptrData, 255, resSizeX * resSizeY * noFrames);
		if (luggageId) {
			uint8 *dstData = ptrData + resSizeX - FROM_LE_16(lugg->sizeX);
			for (uint32 frameCnt = 0; frameCnt < noFrames; frameCnt++) {
				uint8 *luggSrc = (uint8*)lugg + sizeof(MousePtr);
				dstData += (resSizeY - FROM_LE_16(lugg->sizeY)) * resSizeX;
				for (uint32 cnty = 0; cnty < FROM_LE_16(lugg->sizeY); cnty++) {
					for (uint32 cntx = 0; cntx < FROM_LE_16(lugg->sizeX); cntx++)
						if (luggSrc[cntx])
							dstData[cntx] = luggSrc[cntx];
					dstData += resSizeX;
					luggSrc += FROM_LE_16(lugg->sizeX);
				}
			}
			_resMan->resClose(luggageId);
		}
		uint8 *dstData = ptrData;
		uint8 *srcData = (uint8*)ptr + sizeof(MousePtr);
		for (uint32 frameCnt = 0; frameCnt < noFrames; frameCnt++) {
			for (uint32 cnty = 0; cnty < FROM_LE_16(ptr->sizeY); cnty++) {
				for (uint32 cntx = 0; cntx < FROM_LE_16(ptr->sizeX); cntx++)
					if (srcData[cntx])
						dstData[cntx] = srcData[cntx];
				srcData += FROM_LE_16(ptr->sizeX);
				dstData += resSizeX;
			}
			dstData += (resSizeY - FROM_LE_16(ptr->sizeY)) * resSizeX;
		}
		_resMan->resClose(ptrId);
	}
}

void SwordMouse::setPointer(uint32 resId, uint32 rate) {
	_currentPtrId = resId;
	_frame = 0;

	createPointer(resId, _currentLuggageId);

	if ((resId == 0) || (!(SwordLogic::_scriptVars[MOUSE_STATUS] & 1) && (!_mouseOverride))) {
		_system->set_mouse_cursor(NULL, 0, 0, 0, 0);
		_system->show_mouse(false);
	} else {
		animate();
		_system->show_mouse(true);
	}
}

void SwordMouse::setLuggage(uint32 resId, uint32 rate) {
	_currentLuggageId = resId;
	_frame = 0;
	createPointer(_currentPtrId, resId);
}

void SwordMouse::animate(void) {
	if ((SwordLogic::_scriptVars[MOUSE_STATUS] == 1) || (_mouseOverride && _currentPtr)) {
		_frame = (_frame + 1) % _currentPtr->numFrames;
		uint8 *ptrData = (uint8*)_currentPtr + sizeof(MousePtr);
		ptrData += _frame * _currentPtr->sizeX * _currentPtr->sizeY;
		_system->set_mouse_cursor(ptrData, _currentPtr->sizeX, _currentPtr->sizeY, _currentPtr->hotSpotX, _currentPtr->hotSpotY);
	}
}

void SwordMouse::fnNoHuman(void) {
	if (SwordLogic::_scriptVars[MOUSE_STATUS] & 2) // locked, can't do anything
		return ;
	SwordLogic::_scriptVars[MOUSE_STATUS] = 0; // off & unlocked
	setLuggage(0, 0);
	setPointer(0, 0);
}

void SwordMouse::fnAddHuman(void) {
	if (SwordLogic::_scriptVars[MOUSE_STATUS] & 2) // locked, can't do anything
		return ;
	SwordLogic::_scriptVars[MOUSE_STATUS] = 1;
	SwordLogic::_scriptVars[SPECIAL_ITEM] = 0;
	_getOff = SCR_std_off;
	setPointer(MSE_POINTER, 0);
}

void SwordMouse::fnBlankMouse(void) {
	setPointer(0, 0);
}

void SwordMouse::fnNormalMouse(void) {
	setPointer(MSE_POINTER, 0);
}

void SwordMouse::fnLockMouse(void) {
	SwordLogic::_scriptVars[MOUSE_STATUS] |= 2;
}

void SwordMouse::fnUnlockMouse(void) {
	SwordLogic::_scriptVars[MOUSE_STATUS] &= 1;
}

void SwordMouse::giveCoords(uint16 *x, uint16 *y) {
	*x = _mouseX;
	*y = _mouseY;
}
