/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "sky/sky.h"

#define MICE_FILE	60300
#define NO_MAIN_OBJECTS	24
#define NO_LINC_OBJECTS	21

uint32 SkyMouse::_mouseMainObjects[24] = {
	65,
	9,
	66,
	64,
	8,
	63,
	10,
	11,
	71,
	76,
	37,
	36,
	42,
	75,
	79,
	6,
	74,
	39,
	49,
	43,
	34,
	35,
	77,
	38
};

uint32 SkyMouse::_mouseLincObjects[21] = {
	24625,
	24649,
	24827,
	24651,
	24583,
	24581,
	24582,
	24628,
	24650,
	24629,
	24732,
	24631,
	24584,
	24630,
	24626,
	24627,
	24632,
	24643,
	24828,
	24830,
	24829
};

SkyMouse::SkyMouse(OSystem *system, SkyDisk *skyDisk) {

	_skyDisk = skyDisk;
	_system = system;
	_mouseB = 0;
	
	_miceData = _skyDisk->loadFile(MICE_FILE, NULL);
	fixMouseTransparency(_miceData, _skyDisk->_lastLoadedFileSize);

	//load in the object mouse file
	_objectMouseData = _skyDisk->loadFile(MICE_FILE + 1, NULL);
	fixMouseTransparency(_objectMouseData, _skyDisk->_lastLoadedFileSize);
}

SkyMouse::~SkyMouse( ){
	free (_miceData);
	free (_objectMouseData);
}

void SkyMouse::replaceMouseCursors(uint16 fileNo) {
	_skyDisk->loadFile(fileNo, _objectMouseData);
	fixMouseTransparency(_objectMouseData, _skyDisk->_lastLoadedFileSize);
}

bool SkyMouse::fnAddHuman(void) {
	//reintroduce the mouse so that the human can control the player
	//could still be switched out at high-level

	if (!SkyLogic::_scriptVariables[MOUSE_STOP]) {
		SkyLogic::_scriptVariables[MOUSE_STATUS] |= 6;	//cursor & mouse

		if (_mouseY < 2) //stop mouse activating top line
			_mouseY = 2;
		
		_system->warp_mouse(_mouseX, _mouseY);
	
		//force the pointer engine into running a get-off
		//even if it's over nothing

		//KWIK-FIX
		//get off may contain script to remove mouse pointer text
		//surely this script should be run just in case
		//I am going to try it anyway
		if (SkyLogic::_scriptVariables[GET_OFF])
			_skyLogic->script((uint16)SkyLogic::_scriptVariables[GET_OFF],(uint16)(SkyLogic::_scriptVariables[GET_OFF] >> 16));
	
		SkyLogic::_scriptVariables[SPECIAL_ITEM] = 0xFFFFFFFF;
		SkyLogic::_scriptVariables[GET_OFF] = RESET_MOUSE;
	}

	return true;
}

void SkyMouse::fnSaveCoods(void) { 
	SkyLogic::_scriptVariables[SAFEX] = _mouseX + TOP_LEFT_X;
	SkyLogic::_scriptVariables[SAFEY] = _mouseY + TOP_LEFT_Y;
}

void SkyMouse::lockMouse(void) {
	SkyState::_systemVars.systemFlags |= SF_MOUSE_LOCKED;
}

void SkyMouse::unlockMouse(void) {
	SkyState::_systemVars.systemFlags &= ~SF_MOUSE_LOCKED;
}

void SkyMouse::restoreMouseData(uint16 frameNum) {
	warning("Stub: SkyMouse::restoreMouseData");
}

void SkyMouse::drawNewMouse() {
	warning("Stub: SkyMouse::drawNewMouse");
	//calculateMouseValues();
	//saveMouseData();
	//drawMouse();
}

void SkyMouse::waitMouseNotPressed(void) {

	bool mousePressed = true;
	OSystem::Event event;
	while (mousePressed) {
		_system->delay_msecs(20);
		while (_system->poll_event(&event)) {
			if ((event.event_code == OSystem::EVENT_LBUTTONUP) ||
				(event.event_code == OSystem::EVENT_QUIT))
				mousePressed = false;
		}
	}
}

//original sky uses different colors for transparency than our backends do,
//so we simply swap our "transparent"-white with another one.
void SkyMouse::fixMouseTransparency(byte *mouseData, uint32 size) {
	uint32 curPos = sizeof(struct dataFileHeader);
	uint32 cursorSize = ((struct dataFileHeader *)mouseData)->s_sp_size;

	while (curPos < size) {
		byte *cursor = mouseData + curPos;
		for (uint32 i = 0; i < cursorSize; i++) {
			if (cursor[i] == 255)
				cursor[i] = 242;
			else
				if (cursor[i] == 0)
					cursor[i] = 255;
		}
		curPos += cursorSize;
	}
}

void SkyMouse::spriteMouse(uint16 frameNum, uint8 mouseX, uint8 mouseY) {
	
	_currentCursor = frameNum;

	byte *newCursor = _miceData;
	newCursor += ((struct dataFileHeader *)_miceData)->s_sp_size * frameNum;
	newCursor += sizeof(struct dataFileHeader);

	uint16 mouseWidth = ((struct dataFileHeader *)_miceData)->s_width;
	uint16 mouseHeight = ((struct dataFileHeader *)_miceData)->s_height;

	_system->set_mouse_cursor(newCursor, mouseWidth, mouseHeight, mouseX, mouseY);
	if (frameNum == MOUSE_BLANK) _system->show_mouse(false);
	else _system->show_mouse(true);
}

void SkyMouse::mouseEngine(uint16 mouseX, uint16 mouseY) {
	_mouseX = mouseX;
	_mouseY = mouseY;

	_logicClick = (_mouseB > 0); // click signal is available for SkyLogic for one gamecycle

	if (!SkyLogic::_scriptVariables[MOUSE_STOP]) {
		if (SkyLogic::_scriptVariables[MOUSE_STATUS] & (1 << 1)) {
			pointerEngine(mouseX + TOP_LEFT_X, mouseY + TOP_LEFT_Y);
			if (SkyLogic::_scriptVariables[MOUSE_STATUS] & (1 << 2)) //buttons enabled?
				buttonEngine1();
		}
	}	
	_mouseB = 0;	//don't save up buttons
}

void SkyMouse::pointerEngine(uint16 xPos, uint16 yPos) {
	uint32 currentListNum = SkyLogic::_scriptVariables[MOUSE_LIST_NO];
	uint16 *currentList;
	do {
		currentList = (uint16 *)SkyState::fetchCompact(currentListNum);
		while ((*currentList != 0) && (*currentList != 0xFFFF)) {
			uint16 itemNum = *currentList;
			Compact *itemData = SkyState::fetchCompact(itemNum);
			currentList++;
			if ((itemData->screen == SkyLogic::_scriptVariables[SCREEN]) &&	(itemData->status & 16)) {
				if (itemData->xcood + ((int16)itemData->mouseRelX) > xPos) continue;
				if (itemData->xcood + ((int16)itemData->mouseRelX) + itemData->mouseSizeX < xPos) continue;
				if (itemData->ycood + ((int16)itemData->mouseRelY) > yPos) continue;
				if (itemData->ycood + ((int16)itemData->mouseRelY) + itemData->mouseSizeY < yPos) continue;
				// we've hit the item
				if (SkyLogic::_scriptVariables[SPECIAL_ITEM] == itemNum)
					return;
				SkyLogic::_scriptVariables[SPECIAL_ITEM] = itemNum;
				if (SkyLogic::_scriptVariables[GET_OFF])
					_skyLogic->mouseScript(SkyLogic::_scriptVariables[GET_OFF], itemData);
				SkyLogic::_scriptVariables[GET_OFF] = itemData->mouseOff;
				if (itemData->mouseOn)
					_skyLogic->mouseScript(itemData->mouseOn, itemData);
				return;
			}
		}
		if (*currentList == 0xFFFF) currentListNum = currentList[1];

	} while (*currentList != 0);
	if (SkyLogic::_scriptVariables[SPECIAL_ITEM] != 0) {
		SkyLogic::_scriptVariables[SPECIAL_ITEM] = 0;
		
		if (SkyLogic::_scriptVariables[GET_OFF])
			_skyLogic->script((uint16)SkyLogic::_scriptVariables[GET_OFF],(uint16)(SkyLogic::_scriptVariables[GET_OFF] >> 16));
		SkyLogic::_scriptVariables[GET_OFF] = 0;
	}
}

void SkyMouse::buttonPressed(uint8 button) {
	
	_mouseB = button;
}

void SkyMouse::buttonEngine1(void) {
	//checks for clicking on special item
	//"compare the size of this routine to S1 mouse_button"

	if (_mouseB) {	//anything pressed?
		SkyLogic::_scriptVariables[BUTTON] = _mouseB;
		if (SkyLogic::_scriptVariables[SPECIAL_ITEM]) { //over anything?
			Compact *item = SkyState::fetchCompact(SkyLogic::_scriptVariables[SPECIAL_ITEM]);
			if (item->mouseClick)
				_skyLogic->mouseScript(item->mouseClick, item);
		}
	}
}

uint16 SkyMouse::findMouseCursor(uint32 itemNum) {

	uint8 cnt;
	for (cnt = 0; cnt < NO_MAIN_OBJECTS; cnt++) {
		if (itemNum == _mouseMainObjects[cnt]) {
			return cnt;
		}
	}
	for (cnt = 0; cnt < NO_LINC_OBJECTS; cnt++) {
		if (itemNum == _mouseLincObjects[cnt]) {
			return cnt;
		}
	}
    return 0;
}

void SkyMouse::fnOpenCloseHand(bool open) {
    
	if ((!open) && (!SkyLogic::_scriptVariables[OBJECT_HELD])) {
		spriteMouse(1, 0, 0);
		return;
	}
	uint16 cursor = findMouseCursor(SkyLogic::_scriptVariables[OBJECT_HELD]) << 1;
	if (open)
		cursor++;

	uint32 size = ((dataFileHeader*)_objectMouseData)->s_sp_size;
	uint8 *srcData;
	uint8 *destData;
	
	srcData = (uint8 *)_objectMouseData + size * cursor + sizeof(dataFileHeader);
	destData = (uint8 *)_miceData + sizeof(dataFileHeader);
	memcpy(destData, srcData, size);
    spriteMouse(0, 5, 5);
}

